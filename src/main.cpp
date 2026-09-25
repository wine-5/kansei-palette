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
    Texture2D palette{};
    float time = 0.0f;
};

Game g;

void LoadResources() {
    // パスは実行時のカレントディレクトリからの相対パス。
    // Web では --preload-file で仮想ファイルシステムの /resources に置かれるので同じパスで読める。
    // Texture は GPU 側の画像なので InitWindow の後でないと読み込めない(DxLib の LoadGraph と同じ感覚)。
    g.palette = LoadTexture("resources/images/palette.png");
}

void UnloadResources() {
    UnloadTexture(g.palette);
}

void Update(float dt) {
    g.time += dt;

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

    // 画像を回転させて描く。DrawTexturePro は DxLib の DrawRotaGraph に近い(origin が回転中心)
    Rectangle src{0, 0, static_cast<float>(g.palette.width), static_cast<float>(g.palette.height)};
    Rectangle dst{kScreenWidth - 120.0f, 120.0f, src.width * 1.5f, src.height * 1.5f};
    Vector2 origin{dst.width / 2, dst.height / 2};
    DrawTexturePro(g.palette, src, dst, origin, g.time * 45.0f, WHITE);

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
    LoadResources();

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

    // Web ではここに到達しない(タブを閉じればブラウザが全て解放する)
    UnloadResources();
    CloseWindow();
    return 0;
}
