# KanseiPalette

ゲームジャム用の C++ / raylib テンプレート。同じソースからネイティブ版(Windows / Linux / macOS)と Web 版(Emscripten → itch.io)をビルドできます。

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
  main.cpp              メインループ(Update / Draw)とサンプル(画像・日本語・音)
  jp_font.h / .cpp      日本語フォント読み込み(使う文字を集めて LoadFontEx)
resources/              画像・音・フォント(Web 版では index.data に埋め込まれる)
  images/ sounds/ fonts/
web/shell.html          Web 版の index.html のひな形
tools/
  web.ps1               Web のビルド / ローカルサーバー / zip 作成(Windows)
  serve_web.py          キャッシュ無効のローカルサーバー
  subset_font.py        日本語フォントのサブセット化
.github/workflows/
  pages.yml             main への push で GitHub Pages に自動公開
docs/
  SETUP.md              セットアップ手順書(emsdk・ビルド・itch.io 提出)
  DXLIB_TO_RAYLIB.md    DxLib → raylib 対応表
```

## ゲームを書き始めるとき

- 状態は `src/main.cpp` の `Game` 構造体に足し、処理は `Update(dt)`、描画は `Draw()` に書く
- 画面に出す日本語は `text` 名前空間に定義して `AllTexts()` に追加する(しないと「?」になる)
- 画像・音は `resources/` に置き、`"resources/..."` のパスで読み込む
- 音の読み込み・再生は `StartAudio()`(最初のクリック後)以降に行う
- 必要になったら Dear ImGui(デバッグ UI)や Box2D(2D 物理)を `FetchContent` で追加する

## Web 版の落とし穴(詳細は各ソースのコメント)

| 項目 | 内容 | 該当箇所 |
|---|---|---|
| メインループ | `while` で回すとブラウザが固まる。`emscripten_set_main_loop` に 1 フレーム分の関数を渡す | `src/main.cpp` の `main()` |
| ファイルアクセス | `--preload-file` で `resources/` を `index.data` に詰めて仮想 FS から読む。`file://` では起動しない | `CMakeLists.txt` |
| 自動再生制限 | クリック・キー入力の前に音声を初期化すると無音。最初の操作後に `InitAudioDevice()` | `src/main.cpp` の `StartAudio()` |
| スレッド | `std::thread` は使わない(itch.io で必要なヘッダ設定が面倒) | `CMakeLists.txt` |
| 待ち処理 | `WaitTime()` や待ちループは使わない。タイマー変数で状態を進める | `docs/DXLIB_TO_RAYLIB.md` |

## ライセンス

- raylib: zlib/libpng License
- Noto Sans JP(`resources/fonts/`): SIL Open Font License 1.1(`resources/fonts/OFL.txt`)。サブセット化済み
- サンプルの画像・効果音・BGM(`resources/images/`, `resources/sounds/`): このリポジトリ用にプログラムで生成したもの。自由に差し替え・削除してよい
