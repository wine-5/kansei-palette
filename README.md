# かんせいのパレット(KanseiPalette)

ゲームジャム用の C++ / raylib のゲーム。同じソースからネイティブ版(Windows / Linux / macOS)と Web 版(Emscripten → itch.io)をビルドできます。

- raylib 6.0(CMake の FetchContent で自動取得)
- Emscripten 6.0.10(emsdk)
- C++17 / CMake 3.24 以上

## すぐに試す

| やりたいこと | 方法 |
|---|---|
| Web 版をブラウザで確認 | VS Code で **F5**(構成「Web (Edge)」) |
| ネイティブ版を起動 | VS Code で構成「Native (Windows)」を選んで F5 / `cmake -S . -B build` → `cmake --build build` |
| itch.io 提出用 zip を作る | `powershell -ExecutionPolicy Bypass -File tools\web.ps1 package` |
| 他の人に遊んでもらう | main に push → <https://wine-5.github.io/kansei-palette/> に自動公開(GitHub Pages) |

初めての環境では先に [docs/SETUP.md](docs/SETUP.md) の手順で emsdk などを入れてください。

## フォルダ構成

```
CMakeLists.txt          ネイティブ / Web 共通のビルド設定
src/
  Main.cpp              ウィンドウ作成とメインループ
  Application.h / .cpp  毎フレームの処理をつなぐ
  core/                 どこからでも使う小さな部品(raylib 禁止)
  game/                 ゲームのルール(raylib 禁止)
  infrastructure/       描画・入力・音・リソース・演出・UI(raylib を使う)
tests/                  盤面ロジックのテスト
resources/              画像・音・フォント(Web 版では index.data に埋め込まれる)
  images/ sounds/ fonts/
art/                    画像の元になるアセットシート(ゲームからは直接読まない)
web/shell.html          Web 版の index.html のひな形
tools/
  web.ps1               Web のビルド / ローカルサーバー / zip 作成(Windows)
  serve_web.py          キャッシュ無効のローカルサーバー
  subset_font.py        日本語フォントのサブセット化
  slice_sheet.py        アセットシートを透過 PNG に切り出して resources/images/ に保存
  gen_sounds.py         効果音を合成して resources/sounds/ に WAV で保存(標準ライブラリだけで動く)
.github/workflows/
  pages.yml             main への push で GitHub Pages に自動公開
docs/
  SETUP.md              セットアップ手順書(emsdk・ビルド・itch.io 提出)
  DXLIB_TO_RAYLIB.md    DxLib → raylib 対応表
  architecture.md       アーキテクチャ設計(層の分け方・1 フレームの流れ)
  企画書.md / 要件定義書.md
```

## 開発するとき

- 設計は [docs/architecture.md](docs/architecture.md)。ルールは `game/`、見た目・音・入力は `infrastructure/` に書く
- 画面に出す日本語は `infrastructure/ui/UiText.h`(ステージ名・ヒントは `game/data/Stages.h`)に書く。ここに無い漢字は「?」になる
- 調整用の数値は `game/data/Config.h` に集める
- 画像・音は `resources/` に置き、`"resources/..."` のパスで読み込む

## Web 版の落とし穴(詳細は各ソースのコメント)

| 項目 | 内容 | 該当箇所 |
|---|---|---|
| メインループ | `while` で回すとブラウザが固まる。`emscripten_set_main_loop` に 1 フレーム分の関数を渡す | `src/Main.cpp` |
| ファイルアクセス | `--preload-file` で `resources/` を `index.data` に詰めて仮想 FS から読む。`file://` では起動しない | `CMakeLists.txt` |
| 自動再生制限 | クリック・キー入力の前に音声を初期化すると無音。最初の操作後に `InitAudioDevice()` | `src/infrastructure/audio/Audio.h` |
| スレッド | `std::thread` は使わない(itch.io で必要なヘッダ設定が面倒) | `CMakeLists.txt` |
| 待ち処理 | `WaitTime()` や待ちループは使わない。タイマー変数で状態を進める | `docs/DXLIB_TO_RAYLIB.md` |

## ライセンス

- raylib: zlib/libpng License
- Noto Sans JP(`resources/fonts/`): SIL Open Font License 1.1(`resources/fonts/OFL.txt`)。サブセット化済み
- 効果音(`resources/sounds/`): `tools/gen_sounds.py` で合成したもの
