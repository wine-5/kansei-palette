"""日本語フォントを Web 向けにサブセット化(使う文字だけに絞る)するスクリプト。

Noto Sans JP の元ファイル(可変フォント, 約 9.5MB)をそのまま resources/ に入れると、
Web 版は起動時にそれを丸ごとダウンロードするため読み込みが遅くなる。
ここで収録文字を絞り、太さを 1 つに固定した静的 TTF(1MB 前後)に変換する。

収録する文字:
  - ASCII / 全角記号 / ひらがな / カタカナ
  - JIS 第 1 水準漢字(約 3000 字。よく使う漢字はほぼ入る)
  - src/ 以下のソースに書かれている全ての非 ASCII 文字(第 2 水準の漢字なども拾う)
  - --text で渡したファイルに含まれる文字

必要なもの: Python 3 と fonttools (pip install fonttools)

使い方:
  python tools/subset_font.py NotoSansJP[wght].ttf resources/fonts/NotoSansJP-Regular-subset.ttf
  python tools/subset_font.py NotoSansJP[wght].ttf out.ttf --weight 700      # 太字で作る
  python tools/subset_font.py NotoSansJP[wght].ttf out.ttf --no-jis1         # ソースで使っている文字だけ(最小サイズ)
"""

import argparse
import pathlib

from fontTools import subset
from fontTools.ttLib import TTFont
from fontTools.varLib import instancer

ROOT = pathlib.Path(__file__).resolve().parent.parent


def base_chars() -> set[str]:
    chars = {chr(c) for c in range(0x20, 0x7F)}           # ASCII
    chars |= {chr(c) for c in range(0x3000, 0x3040)}      # 全角スペース・句読点・括弧など
    chars |= {chr(c) for c in range(0x3041, 0x3097)}      # ひらがな
    chars |= {chr(c) for c in range(0x30A0, 0x3100)}      # カタカナ(長音符「ー」含む)
    chars |= {chr(c) for c in range(0xFF01, 0xFF5F)}      # 全角英数・記号
    chars |= set("…‥・〜－‐―×÷±→←↑↓♪★☆○●◎◇◆□■△▲▽▼※")
    return chars


def jis1_kanji() -> set[str]:
    # JIS X 0208 の 16〜47 区が第 1 水準漢字。EUC-JP の 0xB0A1〜0xCFD3 に対応する。
    chars = set()
    for hi in range(0xB0, 0xD0):
        for lo in range(0xA1, 0xFF):
            if hi == 0xCF and lo > 0xD3:
                break
            chars.add(bytes([hi, lo]).decode("euc_jp"))
    return chars


def chars_in_sources() -> set[str]:
    chars = set()
    for path in (ROOT / "src").rglob("*"):
        if path.suffix in {".cpp", ".h", ".hpp", ".c"}:
            chars |= {c for c in path.read_text(encoding="utf-8") if ord(c) > 0x7F}
    return chars


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", help="元フォント(可変フォント .ttf / 静的 .ttf どちらでも可)")
    parser.add_argument("output", help="出力する .ttf")
    parser.add_argument("--weight", type=float, default=400, help="可変フォントの太さ(100〜900, 既定 400)")
    parser.add_argument("--no-jis1", action="store_true", help="JIS 第 1 水準漢字を含めない")
    parser.add_argument("--text", action="append", default=[], help="追加で収録する文字を含むテキストファイル")
    args = parser.parse_args()

    chars = base_chars() | chars_in_sources()
    if not args.no_jis1:
        chars |= jis1_kanji()
    for text_file in args.text:
        chars |= set(pathlib.Path(text_file).read_text(encoding="utf-8"))
    chars -= {"\n", "\r", "\t"}

    font = TTFont(args.input)
    options = subset.Options()
    options.layout_features = []   # raylib はカーニング等の OpenType 機能を使わないので削る
    options.name_IDs = ["*"]       # 著作権・ライセンス表記は残す
    options.notdef_outline = True
    subsetter = subset.Subsetter(options)
    subsetter.populate(text="".join(sorted(chars)))
    subsetter.subset(font)

    # 可変フォントは raylib(stb_truetype)が太さを選べないため、指定の太さで静的フォントに固定する
    if "fvar" in font:
        font = instancer.instantiateVariableFont(font, {"wght": args.weight})

    out = pathlib.Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    font.save(out)
    print(f"{len(chars)} chars -> {out} ({out.stat().st_size / 1024:.0f} KB)")


if __name__ == "__main__":
    main()
