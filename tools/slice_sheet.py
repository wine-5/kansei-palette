"""アセットシート(マゼンタ背景の 1 枚絵)を、透過 PNG の個別ファイルに切り出すスクリプト。

  - 背景のマゼンタを透明にし、縁に残るマゼンタのにじみも取り除く
  - つながった絵ごとに切り出し、シート上の位置から名前を付けて resources/images/ に保存する

必要なもの: Python 3、Pillow、numpy (pip install pillow numpy)

使い方:
  python tools/slice_sheet.py                         # art/AssetsSheet1.png → resources/images/
  python tools/slice_sheet.py --preview preview.png   # 切り出し範囲を描き込んだ確認用画像も出す

シートの配置が変わったら、下の REGIONS(領域と名前の対応)を直す。
"""

import argparse
import pathlib
from collections import deque

import numpy as np
from PIL import Image, ImageDraw

ROOT = pathlib.Path(__file__).resolve().parent.parent
DEFAULT_SHEET = ROOT / "art" / "AssetsSheet1.png"
DEFAULT_OUT = ROOT / "resources" / "images"

# 背景とみなすマゼンタからの色の距離。KEY_INNER 以下は完全に透明、KEY_OUTER 以上は完全に不透明
KEY_INNER = 60.0
KEY_OUTER = 150.0
# 小さすぎる塊(きらめきの粒など)は無視する(2x2 に縮小した画素数)
MIN_COMPONENT = 150

TILE_THEMES = ["grass", "snow", "desert", "night", "sea", "forest", "lava", "sky"]
TILE_NAMES = ["straight", "corner", "tee", "cross", "source", "goal", "locked", "blank"]

# 領域ごとに、切り出した絵へ付ける名前。
# 領域内の絵は「行ごと(上から) → 行の中で左から」の順に並べ、names の順に名前を付ける。
# rows は行を分ける y 座標(その値より上が 1 行目)。
REGIONS = []
for index, theme in enumerate(TILE_THEMES):
    left = (index % 4) * 384
    top = (index // 4) * 217
    REGIONS.append({
        "box": (left, top, left + 384, top + 217),
        "rows": [top + 111],  # 上下の行の間のすき間
        "dir": f"tiles/{theme}",
        "names": TILE_NAMES,
    })
REGIONS += [
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
    """四隅付近の色の中央値を背景色とする"""
    corners = np.concatenate([rgb[:8, :8].reshape(-1, 3), rgb[:8, -8:].reshape(-1, 3),
                              rgb[-8:, :8].reshape(-1, 3), rgb[-8:, -8:].reshape(-1, 3)])
    return np.median(corners, axis=0)


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


def split_by_rows(component, rows, opaque: np.ndarray):
    """行の境目をまたいでつながってしまった塊(雲のにじみで上下のタイルがくっつく等)を行ごとに分ける"""
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


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--sheet", default=str(DEFAULT_SHEET), help="アセットシートの PNG")
    parser.add_argument("--out", default=str(DEFAULT_OUT), help="出力先フォルダ")
    parser.add_argument("--preview", help="切り出し範囲を描き込んだ確認用画像の出力先")
    args = parser.parse_args()

    rgb = np.asarray(Image.open(args.sheet).convert("RGB"))
    bg = estimate_background(rgb)
    rgba = key_out(rgb, bg)
    # 絵の検出は、にじみ(半透明の縁)を含めずに行う。含めると隣り合う絵がくっつく
    opaque = rgba[..., 3] > 128

    components = [c for c in find_components(opaque) if c[4] >= MIN_COMPONENT]
    # 区切り線のような細長い塊は除く
    components = [c for c in components if (c[2] - c[0]) >= 16 and (c[3] - c[1]) >= 16
                  and (c[2] - c[0]) < 400 and (c[3] - c[1]) < 300]

    out_dir = pathlib.Path(args.out)
    preview = Image.fromarray(rgb.copy()) if args.preview else None
    draw = ImageDraw.Draw(preview) if preview else None
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
        folder.mkdir(parents=True, exist_ok=True)
        for (x0, y0, x1, y1, _), name in zip(inside, names):
            # 1px の余白を付けて切り出す(拡大・回転したときに縁が欠けないように)
            x0, y0 = max(x0 - 1, 0), max(y0 - 1, 0)
            x1, y1 = min(x1 + 1, rgb.shape[1]), min(y1 + 1, rgb.shape[0])
            Image.fromarray(rgba[y0:y1, x0:x1], "RGBA").save(folder / f"{name}.png", optimize=True)
            total += 1
            if draw:
                draw.rectangle((x0, y0, x1, y1), outline=(0, 255, 0))
                draw.text((x0 + 2, y0 + 2), name, fill=(255, 255, 0))

    if preview:
        preview.save(args.preview)
    print(f"{total} files -> {out_dir}")


if __name__ == "__main__":
    main()
