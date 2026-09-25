// ゲームジャム用 raylib ひな形
// ネイティブ(Windows/Linux/macOS)と Web(Emscripten) で同じソースをビルドする。

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

namespace {

constexpr int kScreenWidth = 960;
constexpr int kScreenHeight = 540;

// ゲームの状態はグローバル(無名名前空間)にまとめておく。
// Web では 1 フレームごとに関数を呼び出される構造になるため、
// main() のローカル変数に状態を持たせることができない。
struct Game {
    Vector2 circlePos{kScreenWidth / 2.0f, kScreenHeight / 2.0f};
    float circleSpeed = 240.0f;  // px/秒
};

Game g;

void Update(float dt) {
    // 矢印キーで円を動かす
    if (IsKeyDown(KEY_RIGHT)) g.circlePos.x += g.circleSpeed * dt;
    if (IsKeyDown(KEY_LEFT))  g.circlePos.x -= g.circleSpeed * dt;
    if (IsKeyDown(KEY_DOWN))  g.circlePos.y += g.circleSpeed * dt;
    if (IsKeyDown(KEY_UP))    g.circlePos.y -= g.circleSpeed * dt;
}

void Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawCircleV(g.circlePos, 40.0f, MAROON);
    DrawFPS(10, 10);

    EndDrawing();
}

// 1 フレーム分の処理。ネイティブでは while ループから、Web ではブラウザから呼ばれる。
void UpdateDrawFrame() {
    Update(GetFrameTime());
    Draw();
}

}  // namespace

int main() {
    InitWindow(kScreenWidth, kScreenHeight, "KanseiPalette");

#if defined(PLATFORM_WEB)
    // 【Web の落とし穴: メインループ】
    // ブラウザでは while ループで処理を抱え込むとタブが固まる(描画もイベントも止まる)。
    // そのため 1 フレーム分の関数をブラウザに登録し、requestAnimationFrame ごとに呼んでもらう。
    // 第 2 引数 0 = ブラウザのリフレッシュレートに合わせる(SetTargetFPS は使わない)。
    // 第 3 引数 1 = ここから戻らない(以降の行はページが閉じるまで実行されない)。
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    CloseWindow();
    return 0;
}
