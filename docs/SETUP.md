# セットアップ手順書

ゲームジャム開始前にこの手順で環境を作り、「ネイティブで起動」「ブラウザで起動」「クリック後に音が鳴る」まで確認しておいてください。

## 使用バージョン(2026-09-25 時点で確認した最新の安定版)

| もの | バージョン | 確認方法 |
|---|---|---|
| raylib | **6.0** | GitHub のタグ一覧の最新。`CMakeLists.txt` の `RAYLIB_VERSION` で指定 |
| Emscripten (emsdk) | **6.0.10** | emsdk の `latest` が指すバージョン |
| CMake | 3.24 以上 | 動作確認は 4.2 で実施 |

> 次のジャムで使うときは、raylib は <https://github.com/raysan5/raylib/releases>、emsdk は `emsdk install latest` で最新版を確認してください。

---

## 1. 必要なツール

### Windows(動作確認済みの環境)

1. **Visual Studio 2022 以降**(「C++ によるデスクトップ開発」ワークロード)
   - 動作確認は Visual Studio Community 2026 で実施
2. **CMake**: <https://cmake.org/download/> か `winget install Kitware.CMake`
3. **Git**: `winget install Git.Git`
4. **Ninja**(Web ビルドで使う): `winget install Ninja-build.Ninja`
   - 入れなくても、`tools/web.ps1` が Visual Studio 同梱の Ninja を自動で探して使います
5. **VS Code** + 拡張機能「C/C++」(F5 起動用。任意)

### Linux / macOS(⚠ 未検証)

この手順書は Windows で検証しました。Linux / macOS は一般的な手順を載せていますが、実機では確認していません。

- Linux(Ubuntu): `sudo apt install build-essential cmake git ninja-build libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev`
- macOS: `xcode-select --install` と `brew install cmake ninja`

---

## 2. emsdk(Emscripten)のインストール

### Windows(PowerShell)

```powershell
cd C:\
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
.\emsdk.ps1 install latest
.\emsdk.ps1 activate latest
```

- `C:\emsdk` 以外に置いた場合は、環境変数 `EMSDK` にそのフォルダを設定してください(`tools/web.ps1` が参照します)。
- すでに emsdk がある場合は `git pull` してから `install latest` / `activate latest` で更新できます。

確認:

```powershell
C:\emsdk\emsdk_env.ps1     # このターミナルで emcc が使えるようになる(毎回必要)
emcc --version             # 6.0.10 と表示されれば OK
```

### Linux / macOS(⚠ 未検証)

```bash
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk
./emsdk install latest
./emsdk activate latest
source ~/emsdk/emsdk_env.sh   # ターミナルを開くたびに必要
emcc --version
```

---

## 3. ネイティブビルド

初回は raylib をダウンロードするため 1 分ほどかかります。

### Windows

```powershell
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\KanseiPalette.exe
```

- `build\KanseiPalette.slnx`(VS 2022 なら `.sln`)を Visual Studio で開いて F5 でも起動できます(作業ディレクトリは設定済み)。
- VS Code なら「実行とデバッグ」で **Native (Windows)** を選んで F5。

### Linux / macOS(⚠ 未検証)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/KanseiPalette
```

---

## 4. Web ビルドと動作確認

### いちばん簡単な方法(Windows + VS Code)

VS Code でこのフォルダを開き、**F5**(構成「Web (Edge)」)。

1. Web ビルド(`tools/web.ps1 build`)
2. ローカルサーバー起動(`http://localhost:8080`、2 回目以降は起動済みのものを再利用)
3. Edge でゲームが開く

の順で自動実行されます。ソースを直したらもう一度 F5 するだけです。

### コマンドで行う方法(Windows / PowerShell)

```powershell
powershell -ExecutionPolicy Bypass -File tools\web.ps1 build    # build-web\web\index.html などを生成
powershell -ExecutionPolicy Bypass -File tools\web.ps1 serve    # Ctrl+C で停止
```

ブラウザで <http://localhost:8080/> を開きます。

`tools/web.ps1` が中でやっていること(手で打つ場合):

```powershell
C:\emsdk\emsdk_env.ps1
emcmake cmake -S . -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-web
```

### Linux / macOS(⚠ 未検証)

```bash
source ~/emsdk/emsdk_env.sh
emcmake cmake -S . -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-web
python3 tools/serve_web.py build-web/web 8080
```

### 確認ポイント

- 「クリックしてスタート」が表示される → クリックすると BGM が鳴る → 以降クリックで効果音
- 日本語(感性パレット など)が表示される
- 矢印キーで円が動く(キーが効かないときは一度ゲーム画面をクリック)

### うまく動かないとき

| 症状 | 原因と対処 |
|---|---|
| `index.html` をダブルクリックすると真っ黒 | `file://` では wasm を読めない。必ずローカルサーバー経由で開く |
| 直したのに反映されない | ブラウザのキャッシュ。`tools/serve_web.py` はキャッシュ無効にしているが、他のサーバーを使うときは Ctrl+F5 |
| 画像を追加したのに読み込めない | `resources/` の外に置いていないか確認。`resources/` 内なら再ビルドで自動的に `index.data` に入る |
| 音が鳴らない | クリック前に `InitAudioDevice()` していないか確認(`src/main.cpp` の `StartAudio()` 参照) |
| 画面が固まる | `while` ループや `WaitTime()` でブラウザを止めていないか確認 |
| エラーの詳細を見たい | ブラウザで F12 → コンソール。raylib のログ(`INFO:` / `WARNING:`)もここに出る |

---

## 5. itch.io への提出

### zip を作る

```powershell
powershell -ExecutionPolicy Bypass -File tools\web.ps1 package
```

VS Code ならタスク「web: package (itch.io zip)」でも可。`build-web\KanseiPalette-web.zip` ができます。

中身は次の 4 ファイルで、`index.html` が **zip の直下** にあることが重要です(フォルダごと zip すると itch.io で起動しません)。

```
index.html   ← itch.io はこれを開く
index.js
index.wasm
index.data   ← resources/ の中身
```

手で作る場合は `build-web\web` の中の 4 ファイルを選択して zip にしてください。

### itch.io の設定

1. <https://itch.io/game/new> を開く
2. **Kind of project** を **HTML** にする
3. **Uploads** で zip をアップロードし、**This file will be played in the browser** にチェック
4. **Embed options**
   - **Viewport dimensions**: `960 × 540`(`src/main.cpp` の画面サイズと同じにする)
   - **Fullscreen button**: オン推奨
   - **SharedArrayBuffer support**: オフのまま(このテンプレートはスレッドを使わないので不要)
   - **Mobile friendly**: スマホ対応をするならオン
5. 一度 **Draft** のまま保存し、ページを開いて起動・音・日本語表示を確認してから公開(ジャムのページから提出)

> ⚠ itch.io 上での動作は、この環境からは確認していません(ローカルサーバー + Edge / Chrome で確認済み)。ジャム前にテスト用の下書きページを作って一度アップロードしておくと安心です。

---

## 6. 日本語フォントの更新

`resources/fonts/NotoSansJP-Regular-subset.ttf` には、ASCII・かな・全角記号・**JIS 第 1 水準漢字(約 3000 字)**が入っています(約 930KB。元のフォントは約 9.5MB)。よく使う漢字はほぼ入っているため、通常は作り直す必要はありません。

第 2 水準の漢字などを使って表示が「?」になったときだけ、次の手順でフォントを作り直します。

```powershell
# Python 3 が必要(emsdk 同梱の C:\emsdk\python\...\python.exe でも可)
pip install fonttools
# Google Fonts から NotoSansJP[wght].ttf をダウンロード
# https://github.com/google/fonts/tree/main/ofl/notosansjp
python tools\subset_font.py "NotoSansJP[wght].ttf" resources\fonts\NotoSansJP-Regular-subset.ttf
```

スクリプトは `src/` 内のソースに書かれた日本語文字もすべて拾って収録します。

**フォント側に字形があっても、プログラム側でその文字を読み込んでいないと「?」になります。** 画面に出す文章は `src/main.cpp` の `text` 名前空間に書き、`AllTexts()` に足してください(`LoadJapaneseFont` がそこから文字を集めます)。

### ライセンス

Noto Sans JP は **SIL Open Font License 1.1**(`resources/fonts/OFL.txt`)。ゲームへの同梱・再配布・商用利用が可能です。サブセット化は改変にあたりますが、予約フォント名は「Source」でフォント名「Noto Sans JP」には含まれないため、名前を変える必要はありません。同梱する場合は `OFL.txt` も一緒に配布してください(Web 版は `index.data` に入っています)。クレジット欄に「Noto Sans JP (SIL OFL 1.1)」と書いておくと丁寧です。
