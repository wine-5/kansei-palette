# DxLib → raylib 対応表

raylib 6.0 の関数名で書いています。関数の一覧は公式のチートシート <https://www.raylib.com/cheatsheet/cheatsheet.html> が便利です。

## まず押さえる違い(引っかかりやすい点)

1. **座標指定が「右下の座標」ではなく「幅・高さ」**
   `DrawBox(x1, y1, x2, y2, ...)` → `DrawRectangle(x, y, width, height, color)`。当たり判定の `Rectangle` も `{x, y, width, height}`。
2. **色は `Color` 構造体(RGBA 各 0〜255)**
   DxLib の `GetColor(r, g, b)` は `unsigned int` を返しますが、raylib は `Color{r, g, b, a}`。定義済みの色(`RED`, `RAYWHITE` など)がある。
   ⚠ raylib にも `GetColor()` がありますが、**`0xRRGGBBAA` の 16 進数から Color を作る別物**です。
3. **透明度は描画時の色(tint)で指定する**
   `SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128)` の代わりに `DrawTexture(tex, x, y, Fade(WHITE, 0.5f))`。加算合成などは `BeginBlendMode(BLEND_ADDITIVE)` 〜 `EndBlendMode()`。`SetDrawBright` も tint の色で表現する。
4. **Image と Texture は別物**
   - `Image`: CPU(メモリ)上の画像。ピクセルの加工ができる(`ImageResize`, `ImageCrop` など)。描画はできない
   - `Texture2D`: GPU 上の画像。描画できる。DxLib のグラフィックハンドルに相当
   - ふつうは `LoadTexture()` で直接読み込めばよい。加工したいときだけ `LoadImage` → 加工 → `LoadTextureFromImage` → `UnloadImage`
   - どちらも `InitWindow()` の後に読み込む(Texture は GPU が必要)
5. **ハンドル(int)ではなく構造体を持つ**
   `int handle = LoadGraph(...)` の代わりに `Texture2D tex = LoadTexture(...)`。`tex.width` / `tex.height` でサイズが取れる(`GetGraphSize` 不要)。解放は `UnloadTexture(tex)` と自分で呼ぶ。
6. **日本語フォントは「使う文字」を指定して読み込む**
   既定フォントは ASCII のみ。`DrawString` の感覚で `DrawText("日本語")` と書くと「?」になる。`LoadFontEx` に使う文字(コードポイント)を渡してテクスチャに焼き込む必要がある → `infrastructure::resource::loadJapaneseFont()` を使い、表示する文章は `src/infrastructure/ui/UiText.h` に登録する。フォントサイズは読み込み時に決まる(描画時に大きくするとぼやける)。
7. **描画は `BeginDrawing()` 〜 `EndDrawing()` で囲む**
   `ClearDrawScreen` → `ClearBackground`、`ScreenFlip` → `EndDrawing`。`SetDrawScreen(DX_SCREEN_BACK)` は不要(常にダブルバッファ)。
8. **キーの「押した瞬間」が標準で取れる**
   DxLib では自前でフレームカウントを持っていた処理が `IsKeyPressed`(押した瞬間)/ `IsKeyReleased`(離した瞬間)で済む。
9. **時間の単位は秒(float)**
   `GetNowCount()`(ミリ秒)ではなく `GetTime()`(秒, double)。移動量は `GetFrameTime()`(前フレームからの秒数)を掛けて書くと FPS に依存しない。
10. **音量は 0.0〜1.0**
    `ChangeVolumeSoundMem(0〜255)` ではなく `SetSoundVolume(sound, 0.0〜1.0)`。
11. **三角形の頂点は反時計回り**
    `DrawTriangle` は頂点を反時計回りに渡さないと何も描かれない。
12. **Web ではブロッキング処理をしない**
    `WaitTimer` / `WaitKey` / 自前の `while` 待ちループはブラウザを固まらせる。待ちは状態(シーン)とタイマー変数で表現する。

---

## 対応表

### 初期化・メインループ

| DxLib | raylib | 備考 |
|---|---|---|
| `SetGraphMode(w, h, 32)` + `DxLib_Init()` | `InitWindow(w, h, "タイトル")` | |
| `ChangeWindowMode(TRUE)` | (既定でウィンドウ) | 全画面は `ToggleFullscreen()` |
| `SetMainWindowText` | `SetWindowTitle` | |
| `DxLib_End()` | `CloseWindow()` | |
| `ProcessMessage() == 0` | `!WindowShouldClose()` | Web では `emscripten_set_main_loop` を使う(`src/Main.cpp` 参照) |
| `SetDrawScreen(DX_SCREEN_BACK)` | 不要 | |
| `ClearDrawScreen()` | `BeginDrawing()` + `ClearBackground(color)` | |
| `ScreenFlip()` | `EndDrawing()` | |
| FPS 制御(自前 or `SetWaitVSyncFlag`) | `SetTargetFPS(60)` | Web では使わない(ブラウザ側で制御) |
| `WaitTimer(ms)` | `WaitTime(秒)` | Web では使用禁止(固まる) |

### 画像

| DxLib | raylib | 備考 |
|---|---|---|
| `LoadGraph("a.png")` | `LoadTexture("a.png")` | 戻り値は `Texture2D` |
| `DeleteGraph(h)` | `UnloadTexture(tex)` | |
| `GetGraphSize(h, &w, &h)` | `tex.width`, `tex.height` | |
| `DrawGraph(x, y, h, TRUE)` | `DrawTexture(tex, x, y, WHITE)` | 透過は PNG のアルファで自動 |
| `DrawRotaGraph(x, y, scale, angle, h, TRUE)` | `DrawTexturePro(tex, src, dst, origin, 角度(度), WHITE)` | 角度は**度**(DxLib はラジアン)。`DrawTextureEx(tex, pos, 角度, 倍率, WHITE)` でも可(左上基準で回転) |
| `DrawExtendGraph(x1, y1, x2, y2, h, TRUE)` | `DrawTexturePro(tex, src, {x, y, w, h}, {0, 0}, 0, WHITE)` | |
| `DrawRectGraph` / `DerivationGraph` | `DrawTextureRec(tex, {sx, sy, w, h}, pos, WHITE)` | 元画像の一部を描く |
| `LoadDivGraph`(分割読み込み) | 1 枚の `Texture2D` + `DrawTextureRec` で切り出す | 分割用の関数はない |
| `DrawTurnGraph`(左右反転) | `DrawTextureRec` で `src.width` を負にする | |
| `SetTransColor` | PNG のアルファを使う | |
| `MakeScreen` + `SetDrawScreen(h)` | `LoadRenderTexture(w, h)` + `BeginTextureMode(rt)` 〜 `EndTextureMode()` | 描画結果は `rt.texture`(上下反転しているので src.height を負に) |

### 図形

| DxLib | raylib | 備考 |
|---|---|---|
| `DrawBox(x1, y1, x2, y2, c, TRUE)` | `DrawRectangle(x, y, w, h, c)` | **幅・高さ**で指定 |
| `DrawBox(..., FALSE)` | `DrawRectangleLines(x, y, w, h, c)` | |
| `DrawCircle(x, y, r, c, TRUE)` | `DrawCircle(x, y, r, c)` / `DrawCircleV(pos, r, c)` | 枠だけは `DrawCircleLines` |
| `DrawLine(x1, y1, x2, y2, c)` | `DrawLine(x1, y1, x2, y2, c)` | 太さ指定は `DrawLineEx` |
| `DrawTriangle(...)` | `DrawTriangle(v1, v2, v3, c)` | 頂点は反時計回り |
| `DrawPixel(x, y, c)` | `DrawPixel(x, y, c)` | |
| `GetColor(r, g, b)` | `Color{r, g, b, 255}` | `ColorFromHSV` も便利 |

### 文字

| DxLib | raylib | 備考 |
|---|---|---|
| `DrawString(x, y, "abc", c)` | `DrawText("abc", x, y, size, c)` | 既定フォントは ASCII のみ |
| `DrawFormatString(x, y, c, "%d", n)` | `DrawText(TextFormat("%d", n), x, y, size, c)` | |
| `CreateFontToHandle` / `LoadFontDataToHandle` | `LoadFontEx(path, size, codepoints, count)` | 日本語は `loadJapaneseFont()`(`src/infrastructure/resource/JpFont.h`) |
| `DrawStringToHandle` | `DrawTextEx(font, "…", pos, size, spacing, c)` | フォントは `Assets::getUiFont()` から取る |
| `GetDrawStringWidth` | `MeasureText` / `MeasureTextEx` | 中央寄せに使う |
| `DeleteFontToHandle` | `UnloadFont(font)` | |

### 入力

| DxLib | raylib | 備考 |
|---|---|---|
| `CheckHitKey(KEY_INPUT_SPACE)` | `IsKeyDown(KEY_SPACE)` | 押している間ずっと true |
| (自前のカウンタで押した瞬間を判定) | `IsKeyPressed(KEY_SPACE)` | 押した瞬間だけ true |
| | `IsKeyReleased(KEY_SPACE)` | 離した瞬間だけ true |
| `KEY_INPUT_Z` / `KEY_INPUT_RETURN` / `KEY_INPUT_ESCAPE` | `KEY_Z` / `KEY_ENTER` / `KEY_ESCAPE` | ESC は既定でウィンドウを閉じる(`SetExitKey(KEY_NULL)` で無効化) |
| `GetMousePoint(&x, &y)` | `GetMousePosition()` | `Vector2` が返る |
| `GetMouseInput() & MOUSE_INPUT_LEFT` | `IsMouseButtonDown(MOUSE_BUTTON_LEFT)` | 押した瞬間は `IsMouseButtonPressed` |
| `GetMouseWheelRotVol()` | `GetMouseWheelMove()` | |
| `GetJoypadInputState(DX_INPUT_PAD1)` | `IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)` | スティックは `GetGamepadAxisMovement` |

### 音

| DxLib | raylib | 備考 |
|---|---|---|
| (DxLib_Init で自動) | `InitAudioDevice()` | **Web ではクリック後に呼ぶ**(`src/infrastructure/audio/Audio.h`) |
| `LoadSoundMem("se.wav")` | `LoadSound("se.wav")` | 効果音向け(全体をメモリに展開) |
| `PlaySoundMem(h, DX_PLAYTYPE_BACK)` | `PlaySound(se)` | |
| `StopSoundMem(h)` | `StopSound(se)` | |
| `CheckSoundMem(h)` | `IsSoundPlaying(se)` | |
| `ChangeVolumeSoundMem(0〜255, h)` | `SetSoundVolume(se, 0.0〜1.0)` | |
| `DeleteSoundMem(h)` | `UnloadSound(se)` | |
| `PlayMusic` / `LoadSoundMem` + `DX_PLAYTYPE_LOOP`(BGM) | `LoadMusicStream` + `PlayMusicStream` | **毎フレーム `UpdateMusicStream` が必要**。ループは `music.looping = true` |
| `StopMusic` | `StopMusicStream` / `PauseMusicStream` | |

対応形式は WAV / OGG / MP3 / QOA / XM / MOD(FLAC は既定で無効)。BGM は WAV だとファイルが大きくなるので OGG 推奨(Web の読み込み時間に効く)。

### 数学・時間・当たり判定

| DxLib | raylib | 備考 |
|---|---|---|
| `GetRand(n)`(0〜n) | `GetRandomValue(0, n)` | シードは `SetRandomSeed` |
| `GetNowCount()`(ミリ秒) | `GetTime()`(秒) | |
| (前フレームとの差分を自前計算) | `GetFrameTime()`(秒) | |
| 矩形の当たり判定(自前) | `CheckCollisionRecs(a, b)` | |
| 円の当たり判定(自前) | `CheckCollisionCircles(c1, r1, c2, r2)` | |
| 点と矩形(自前) | `CheckCollisionPointRec(point, rect)` | ボタンのクリック判定に便利 |
| `VECTOR` + `VAdd` など | `Vector2` / `Vector3` + `raymath.h` の `Vector2Add` など | `#include "raymath.h"` |

### カメラ・3D(3D に切り替える場合)

| DxLib | raylib | 備考 |
|---|---|---|
| (描画位置を自前でずらす) | `Camera2D` + `BeginMode2D(cam)` 〜 `EndMode2D()` | スクロール・ズーム・画面揺れ |
| `SetCameraPositionAndTarget_UpVecY` | `Camera3D` + `BeginMode3D(cam)` 〜 `EndMode3D()` | |
| `MV1LoadModel("a.mv1")` | `LoadModel("a.glb")` | glTF / OBJ / IQM / VOX / M3D。MV1 は不可 |
| `MV1DrawModel(h)` | `DrawModel(model, pos, scale, WHITE)` | `BeginMode3D` の中で呼ぶ |
| `DrawCube3D` など | `DrawCube`, `DrawSphere`, `DrawPlane`, `DrawGrid` | |

3D に切り替えるときも、`Update()` / `Draw()` の構造と Web ビルド設定はそのまま使えます。`Draw()` の中で `BeginMode3D` 〜 `EndMode3D` で 3D を描き、その後に UI(文字など)を 2D で描く形になります。
