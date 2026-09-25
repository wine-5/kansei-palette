# アーキテクチャ設計

## 基本方針

DxLib-3D と同じく、依存方向を一方向に制御するレイヤードアーキテクチャにする。
ただし 2 日間のジャムなので、層の間をつなぐ仕組みは軽くする。

| DxLib-3D | このゲーム | 理由 |
|---|---|---|
| レイヤードアーキテクチャ | 残す | ルール(盤面・通電・進行)を raylib から切り離し、描画なしでテストできるようにする |
| 命名規則・フォルダと名前空間の一致 | 残す | 普段と同じ感覚で書ける |
| ECS | 使わない | 動くものはタイル 25 枚・小物 10 個以下・主人公 1 人・星くずだけ。配列と構造体で足りる |
| ServiceLocator / core/interface | 使わない | 描画・音の実装は raylib の 1 種類だけで、差し替えない |
| EventBus | GameEvent の配列にする | game 層が「起きたこと」を積み、infrastructure 層が毎フレーム読む |

---

## 全体構造

```
Main.cpp / Application     起動とメインループ、各層をつなぐ
        ↓
infrastructure/            raylib を使う(描画・入力・音・リソース・演出・UI)
        ↓
game/                      ゲームのルールだけ(raylib を include 禁止)
        ↓
core/                      どこからでも使う小さな部品(raylib を include 禁止)
```

- 上の層は下の層を include してよい。逆は禁止。
- infrastructure は game の状態を**直接読んで**描く(インターフェースは挟まない)。
- **game と core は `kansei_game` という別ライブラリとしてビルドし、raylib をリンクしない。**
  game/ や core/ で `#include "raylib.h"` を書くとコンパイルエラーになるので、依存方向がビルドで守られる。

層の判断基準:

```
core            → 何が変わっても影響を受けない
game            → ゲームのルールが変わったら影響を受ける
infrastructure  → raylib(見た目・音・入力の方法)が変わったら影響を受ける
```

---

## 1 フレームの流れ(Application::runFrame)

```
UI(ボタン判定) ─┐
InputReader(raylib の入力 → マス・ボタン) → GameInput
        ↓
GameFlow::update(dt, input)
  盤面・主人公・色の復元度を更新し、起きたことを GameEvent に積む
        ↓
Audio::onEvents / Effects::onEvents   ← GameEvent を読んで音・演出を出す
WorldRenderer::update                 ← タイルの回転アニメーションなど見た目の状態を進める
        ↓
draw: WorldRenderer → Effects → UI(タイトル / HUD / クリアカード)
```

game 層は「音を鳴らせ」と命令せず、「ゴールが光った(GoalLit)」という事実だけを記録する。
演出を削っても game 層のコードは変わらない。

---

## フォルダ構成

```
src/
├── Main.cpp                      ウィンドウ作成、メインループ(Web / ネイティブの切り替え)
├── Application.h/.cpp            毎フレームの処理をつなぐ
├── core/                         namespace core
│   ├── Direction.h               N/E/S/W の 4 ビット、回転
│   ├── Spring.h                  バネ
│   └── Easing.h                  指数的な接近、イージング
├── game/                         namespace game::*(raylib 禁止)
│   ├── board/                    Tile, TileType, Board(回転・通電・クリア判定), StageParser
│   ├── data/                     Stages.h(ステージデータ), Config.h(調整値)
│   ├── flow/                     GameFlow(進行管理), GamePhase, GameInput, RestoreLevel
│   ├── hero/                     HeroState(ポーズとタイマー), HeroPose
│   └── event/                    GameEvent
└── infrastructure/               namespace infrastructure::*
    ├── resource/                 Assets(画像・フォント), JpFont(日本語フォント読み込み)
    ├── input/                    InputReader(マウス・タッチ → マス)
    ├── render/                   WorldRenderer(3D 箱庭), RestoreShader(色の復元)
    ├── fx/                       Effects(フラッシュ・揺れ), Particles(星くず)
    ├── ui/                       TitleScreen, Hud, ClearCard, Button, UiAction, UiText(文章)
    └── audio/                    Audio(効果音、最初のクリック後に初期化)
tests/
└── BoardTest.cpp                 3 ステージの初期状態と解答を確かめる(raylib なし)
```

`.cpp` / `.h` を追加すると、CMake の再構成時に自動で拾われる(`file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`)。
`src/core` と `src/game` 以下は `kansei_game` に、それ以外は本体の実行ファイルに入る。

---

## 見た目の状態とルールの状態を分ける

| 状態 | 置き場所 | 例 |
|---|---|---|
| ルールの状態 | game | タイルの回転数(タップした瞬間に確定)、通電しているか、電源からの距離 |
| 見た目の状態 | infrastructure | バネで回る途中の角度、タップ時の拡大、通電度(0〜1 のフェード) |

判定はアニメーションを待たずに行う(要件定義 3章「回転のルール」)ので、この 2 つは別に持つ。

---

## 規約

- 命名規則・コメント規則は DxLib-3D の `docs/conventions/naming_convention.md` に従う。
- 整形は `.clang-format`(DxLib-3D と同じ。タブ、Allman)。
- raylib のマクロ(`RED`, `WHITE`, `PI`, `DEG2RAD` など)と同じ名前の定数を作らない。
- raylib を使うファイルでは `windows.h` を include しない(`DrawText`, `CloseWindow` などが衝突する)。

---

## テスト

```powershell
cmake --build build --config Debug --target KanseiTests
.\build\Debug\KanseiTests.exe
```

Board と StageParser を実装すると、すべて成功になる(枠組みの段階では失敗する)。

---

## 未決定事項

- アセットのフォルダ名(要件定義 7章は `assets/`、テンプレートは `resources/`)
- 本番フォント(Noto Sans JP のままか、Zen Maru Gothic に替えるか)
- 画面サイズの追従(1280×720 を基準に、ブラウザ幅に合わせて伸縮させる方法)
