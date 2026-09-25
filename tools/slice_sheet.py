"""アセットシート(マゼンタ背景の 1 枚絵)を、透過 PNG の個別ファイルに切り出すスクリプト。

  - 背景のマゼンタを透明にし、縁に残るマゼンタのにじみも取り除く
  - つながった絵ごとに切り出し、シート上の位置から名前を付けて resources/images/ に保存する

扱うシート(art/ に置く):
  AssetsSheet1.png           主人公・小物・NPC・UI アイコン(REGIONS で位置と名前を決める)
  AssetsSheet_Blocks1〜7.png タイル。1 枚 = 1 テーマ。行 = 種類(直線・曲がり・T字・十字・電源・ゴール・ロック・空き)、
                             列 = 見た目違い。resources/images/tiles/<テーマ>/<種類>_<番号>.png になる。
                             書き出すテーマと見た目違いの数は EXPORT_THEMES / VARIANTS_PER_KIND で決める
  BG.png                     背景。横長の帯(空・山・村・手前の花)を上から順に切り出す

必要なもの: Python 3、Pillow、numpy (pip install pillow numpy)

使い方:
  python tools/slice_sheet.py                         # art/ のシートをすべて切り出す
  python tools/slice_sheet.py --preview preview/      # 切り出し範囲を描き込んだ確認用画像も出す

出力先のフォルダにあった PNG は、切り出す前に消す(シートから消えた絵が残らないように)。
"""

import argparse
import pathlib
from collections import deque

import numpy as np
from PIL import Image, ImageDraw

ROOT = pathlib.Path(__file__).resolve().parent.parent
ART_DIR = ROOT / "art"
DEFAULT_OUT = ROOT / "resources" / "images"

# 背景とみなすマゼンタからの色の距離。KEY_INNER 以下は完全に透明、KEY_OUTER 以上は完全に不透明
KEY_INNER = 60.0
KEY_OUTER = 150.0
# 小さすぎる塊(きらめきの粒など)は無視する(2x2 に縮小した画素数)
MIN_COMPONENT = 150

# タイルは、石の板(プレート)の中心に合わせた正方形で切り出し、同じ大きさにそろえる。
# 接点の丸が板の外にはみ出すので、板の 1.16 倍の範囲を切り出す(ゲーム側の TILE_TEXTURE_SCALE と同じ値)
TILE_PLATE_SCALE = 1.16
TILE_IMAGE_SIZE = 104
TILE_KINDS = ["straight", "corner", "tee", "cross", "source", "goal", "locked", "blank"]
TILE_SHEETS = {
    "AssetsSheet_Blocks1.png": "meadow",
    "AssetsSheet_Blocks2.png": "snow",
    "AssetsSheet_Blocks3.png": "desert",
    "AssetsSheet_Blocks4.png": "night",
    "AssetsSheet_Blocks5.png": "sea",
    "AssetsSheet_Blocks6.png": "forest",
    "AssetsSheet_Blocks7.png": "lava",
}

# Web 版の読み込みを軽くするため、ゲームで使うテーマ・見た目違いの数だけを書き出す
# (7 テーマ × 98 枚をすべて書き出すと約 17MB になる)
EXPORT_THEMES = {"meadow", "sea", "forest"}
VARIANTS_PER_KIND = 4

# 背景の帯の名前(上から順)
BG_SHEET = "BG.png"
BG_LAYERS = ["sky", "mountains", "village", "foreground"]

# AssetsSheet1.png の領域ごとに、切り出した絵へ付ける名前。
# 領域内の絵は「行ごと(上から) → 行の中で左から」の順に並べ、names の順に名前を付ける。
# rows は行を分ける y 座標(その値より上が 1 行目)。
# (上半分のタイルは AssetsSheet_Blocks に置き換えたので切り出さない)
SPRITE_SHEET = "AssetsSheet1.png"
REGIONS = [
    {
        "box": (0, 436, 1536, 548),
        "rows": [],
        "dir": "hero",
        "names": [f"hero_{i:02d}" for i in range(21)],
    },
    {
        "box": (0, 548, 792, 654),
        "rows": [],
        "dir": "hero",
        "names": [f"hero_{i:02d}" for i in range(21, 32)],
    },
    {
        "box": (792, 548, 1536, 804),
        "rows": [686],
        "dir": "props",
        "names": ["cottage", "tree", "pine", "flowers", "lamp", "fountain", "fence",
                  "balloon", "well", "crate_barrel", "signpost", "statue", "bridge", "hay", "mushroom"],
    },
    {
        "box": (0, 654, 792, 804),
        "rows": [],
        "dir": "npc",
        "names": ["old_man", "fox", "robot", "umbrella_girl", "cat", "hooded", "chick", "scarecrow"],
    },
    {
        "box": (990, 804, 1536, 1024),
        "rows": [886, 940],
        "dir": "ui",
        "names": ["retry", "back", "home", "sound_on", "sound_off", "hint",
                  "star_0", "star_1", "star_2", "hearts", "ribbon", "board",  # ハート 3 つはくっついているので 1 枚
                  "drop_red", "drop_blue", "drop_yellow", "drop_green", "drop_orange", "drop_purple"],
    },
]


def estimate_background(rgb: np.ndarray) -> np.ndarray:
    """マゼンタに近い画素の中央値を背景色とする(シートの四隅が背景とは限らないため)"""
    r, g, b = rgb[..., 0].astype(int), rgb[..., 1].astype(int), rgb[..., 2].astype(int)
    magenta = (r > 200) & (g < 60) & (b > 200)
    return np.median(rgb[magenta], axis=0)


def key_out(rgb: np.ndarray, bg: np.ndarray) -> np.ndarray:
    """背景色を透明にした RGBA を返す。半透明の縁は背景色が混ざる前の色に戻す"""
    rgbf = rgb.astype(np.float32)
    dist = np.sqrt(((rgbf - bg) ** 2).sum(axis=2))
    alpha = np.clip((dist - KEY_INNER) / (KEY_OUTER - KEY_INNER), 0.0, 1.0)
    safe = np.maximum(alpha, 1e-3)[..., None]
    color = np.clip((rgbf - (1.0 - alpha[..., None]) * bg) / safe, 0, 255)
    rgba = np.dstack([color, alpha * 255.0]).round().astype(np.uint8)
    rgba[alpha == 0] = 0
    return rgba


def find_components(mask: np.ndarray):
    """2x2 に縮小したマスクで、つながった塊の外接矩形(元の解像度)を求める"""
    h2, w2 = mask.shape[0] // 2, mask.shape[1] // 2
    small = mask[:h2 * 2, :w2 * 2].reshape(h2, 2, w2, 2).any(axis=(1, 3))
    label = np.zeros(small.shape, dtype=np.int32)
    boxes = []
    for y in range(h2):
        for x in range(w2):
            if not small[y, x] or label[y, x]:
                continue
            label[y, x] = len(boxes) + 1
            queue = deque([(y, x)])
            x0 = x1 = x
            y0 = y1 = y
            count = 0
            while queue:
                cy, cx = queue.popleft()
                count += 1
                x0, x1, y0, y1 = min(x0, cx), max(x1, cx), min(y0, cy), max(y1, cy)
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        ny, nx = cy + dy, cx + dx
                        if 0 <= ny < h2 and 0 <= nx < w2 and small[ny, nx] and not label[ny, nx]:
                            label[ny, nx] = label[y, x]
                            queue.append((ny, nx))
            boxes.append((x0 * 2, y0 * 2, x1 * 2 + 2, y1 * 2 + 2, count))
    return boxes


def detect_sprites(rgba: np.ndarray):
    """絵の外接矩形の一覧。にじみ(半透明の縁)を含めずに検出する(含めると隣り合う絵がくっつく)"""
    opaque = rgba[..., 3] > 128
    boxes = [c for c in find_components(opaque) if c[4] >= MIN_COMPONENT]
    # 区切り線のような細長い塊や、シート全体に広がる塊は除く
    return [c for c in boxes if (c[2] - c[0]) >= 16 and (c[3] - c[1]) >= 16
            and (c[2] - c[0]) < 400 and (c[3] - c[1]) < 300], opaque


def split_by_rows(component, rows, opaque: np.ndarray):
    """行の境目をまたいでつながってしまった塊(雲のにじみで上下の絵がくっつく等)を行ごとに分ける"""
    x0, y0, x1, y1, count = component
    cuts = [r for r in rows if y0 < r < y1]
    if not cuts:
        return [component]
    pieces = []
    for top, bottom in zip([y0] + cuts, cuts + [y1]):
        ys, xs = np.nonzero(opaque[top:bottom, x0:x1])
        if len(ys) == 0:
            continue
        if ys.max() - ys.min() < 16:
            continue  # 境目で切れた細い端切れは捨てる
        pieces.append((x0 + xs.min(), top + ys.min(), x0 + xs.max() + 1, top + ys.max() + 1, count))
    return pieces


def split_wide(component):
    """横に並んだ正方形の絵がくっついた塊を、等分して分ける(空きタイルの行など)"""
    x0, y0, x1, y1, count = component
    width, height = x1 - x0, y1 - y0
    pieces = max(1, round(width / height))
    if pieces == 1:
        return [component]
    step = width / pieces
    return [(round(x0 + i * step), y0, round(x0 + (i + 1) * step), y1, count) for i in range(pieces)]


def crop_tile(rgba: np.ndarray, box) -> Image.Image:
    """板の中心を求め、板の TILE_PLATE_SCALE 倍の正方形を TILE_IMAGE_SIZE に縮小して返す"""
    x0, y0, x1, y1 = box[:4]
    alpha = rgba[y0:y1, x0:x1, 3] > 128
    rows, cols = alpha.sum(axis=1), alpha.sum(axis=0)
    # はみ出した接点は細いので、6 割以上埋まっている行・列だけを板とみなす
    plate_rows = np.nonzero(rows > 0.6 * rows.max())[0]
    plate_cols = np.nonzero(cols > 0.6 * cols.max())[0]
    cy = y0 + (plate_rows.min() + plate_rows.max() + 1) / 2
    cx = x0 + (plate_cols.min() + plate_cols.max() + 1) / 2
    plate = max(plate_rows.max() - plate_rows.min(), plate_cols.max() - plate_cols.min()) + 1
    half = plate * TILE_PLATE_SCALE / 2
    left, top = int(round(cx - half)), int(round(cy - half))
    side = int(round(half * 2))
    # このタイルの外接矩形の外(隣のタイルやシートの外)は透明で埋める
    canvas = np.zeros((side, side, 4), dtype=np.uint8)
    sx0, sy0 = max(left, x0), max(top, y0)
    sx1, sy1 = min(left + side, x1), min(top + side, y1)
    canvas[sy0 - top:sy1 - top, sx0 - left:sx1 - left] = rgba[sy0:sy1, sx0:sx1]
    return Image.fromarray(canvas, "RGBA").resize((TILE_IMAGE_SIZE, TILE_IMAGE_SIZE), Image.LANCZOS)


def prepare_folder(folder: pathlib.Path, cleaned: set) -> None:
    """出力先を作り、最初の 1 回だけ古い PNG を消す"""
    folder.mkdir(parents=True, exist_ok=True)
    if folder not in cleaned:
        for old in folder.glob("*.png"):
            old.unlink()
        cleaned.add(folder)


def save_sprite(rgba: np.ndarray, box, path: pathlib.Path) -> None:
    """1px の余白を付けて切り出す(拡大・回転したときに縁が欠けないように)"""
    x0, y0, x1, y1 = box[:4]
    x0, y0 = max(x0 - 1, 0), max(y0 - 1, 0)
    x1, y1 = min(x1 + 1, rgba.shape[1]), min(y1 + 1, rgba.shape[0])
    Image.fromarray(rgba[y0:y1, x0:x1], "RGBA").save(path, optimize=True)


def slice_sprite_sheet(rgba, out_dir, cleaned, draw) -> int:
    components, opaque = detect_sprites(rgba)
    total = 0
    for region in REGIONS:
        rx0, ry0, rx1, ry1 = region["box"]
        inside = [c for c in components
                  if rx0 <= (c[0] + c[2]) / 2 < rx1 and ry0 <= (c[1] + c[3]) / 2 < ry1]
        inside = [piece for c in inside for piece in split_by_rows(c, region["rows"], opaque)]

        def row_of(c):
            cy = (c[1] + c[3]) / 2
            return sum(1 for r in region["rows"] if cy >= r)

        inside.sort(key=lambda c: (row_of(c), c[0]))
        names = region["names"]
        if len(inside) != len(names):
            raise SystemExit(f"{region['dir']}: 絵が {len(inside)} 個見つかりました({len(names)} 個のはず)。REGIONS を確認してください")

        folder = out_dir / region["dir"]
        prepare_folder(folder, cleaned)
        for box, name in zip(inside, names):
            save_sprite(rgba, box, folder / f"{name}.png")
            total += 1
            if draw:
                draw.rectangle(box[:4], outline=(0, 255, 0))
                draw.text((box[0] + 2, box[1] + 2), name, fill=(255, 255, 0))
    return total


def slice_tile_sheet(rgba, theme, out_dir, cleaned, draw) -> int:
    components, _ = detect_sprites(rgba)
    tiles = [piece for c in components for piece in split_wide(c)]

    # 縦位置が近いものを同じ行にまとめる(行 = タイルの種類)
    tiles.sort(key=lambda c: (c[1] + c[3]) / 2)
    rows = []
    for tile in tiles:
        cy = (tile[1] + tile[3]) / 2
        if rows and cy - rows[-1][-1][0] < 40:
            rows[-1].append((cy, tile))
        else:
            rows.append([(cy, tile)])
    if len(rows) != len(TILE_KINDS):
        raise SystemExit(f"tiles/{theme}: {len(rows)} 行見つかりました({len(TILE_KINDS)} 行のはず)")

    folder = out_dir / "tiles" / theme
    prepare_folder(folder, cleaned)
    total = 0
    for kind, row in zip(TILE_KINDS, rows):
        for index, (_, box) in enumerate(sorted(row, key=lambda item: item[1][0])[:VARIANTS_PER_KIND]):
            crop_tile(rgba, box).save(folder / f"{kind}_{index:02d}.png", optimize=True)
            total += 1
            if draw:
                draw.rectangle(box[:4], outline=(0, 255, 255))
                draw.text((box[0] + 2, box[1] + 2), f"{kind[:2]}{index}", fill=(255, 255, 0))
    return total


def slice_background(rgba, out_dir, cleaned, draw) -> int:
    # 不透明な画素が多い行が続く範囲を 1 本の帯とみなす
    filled = (rgba[..., 3] > 128).mean(axis=1) > 0.05
    bands = []
    start = None
    for y, isFilled in enumerate(list(filled) + [False]):
        if isFilled and start is None:
            start = y
        elif not isFilled and start is not None:
            if y - start >= 16:
                bands.append((start, y))
            start = None
    if len(bands) != len(BG_LAYERS):
        raise SystemExit(f"bg: 帯が {len(bands)} 本見つかりました({len(BG_LAYERS)} 本のはず)")

    folder = out_dir / "bg"
    prepare_folder(folder, cleaned)
    for name, (top, bottom) in zip(BG_LAYERS, bands):
        Image.fromarray(rgba[top:bottom], "RGBA").save(folder / f"{name}.png", optimize=True)
        if draw:
            draw.rectangle((0, top, rgba.shape[1] - 1, bottom - 1), outline=(0, 255, 0))
            draw.text((4, top + 4), name, fill=(255, 255, 0))
    return len(bands)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--art", default=str(ART_DIR), help="シートを置いたフォルダ")
    parser.add_argument("--out", default=str(DEFAULT_OUT), help="出力先フォルダ")
    parser.add_argument("--preview", help="切り出し範囲を描き込んだ確認用画像を出すフォルダ")
    args = parser.parse_args()

    art_dir, out_dir = pathlib.Path(args.art), pathlib.Path(args.out)
    preview_dir = pathlib.Path(args.preview) if args.preview else None
    if preview_dir:
        preview_dir.mkdir(parents=True, exist_ok=True)
    cleaned = set()

    jobs = [(SPRITE_SHEET, lambda rgba, draw: slice_sprite_sheet(rgba, out_dir, cleaned, draw))]
    jobs += [(sheet, lambda rgba, draw, theme=theme: slice_tile_sheet(rgba, theme, out_dir, cleaned, draw))
             for sheet, theme in TILE_SHEETS.items() if theme in EXPORT_THEMES]
    jobs += [(BG_SHEET, lambda rgba, draw: slice_background(rgba, out_dir, cleaned, draw))]

    for sheet, job in jobs:
        path = art_dir / sheet
        if not path.exists():
            print(f"skip: {sheet} がありません")
            continue
        rgb = np.asarray(Image.open(path).convert("RGB"))
        rgba = key_out(rgb, estimate_background(rgb))
        preview = Image.fromarray(rgb.copy()) if preview_dir else None
        count = job(rgba, ImageDraw.Draw(preview) if preview else None)
        if preview is not None and preview_dir is not None:
            preview.save(preview_dir / sheet)
        print(f"{sheet}: {count} files")


if __name__ == "__main__":
    main()
