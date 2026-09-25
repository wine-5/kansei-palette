// ゲームジャム用 raylib ひな形
// ネイティブ(Windows/Linux/macOS)と Web(Emscripten) で同じソースをビルドする。

#include "raylib.h"
#include "infrastructure/resource/JpFont.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

namespace {

constexpr int kScreenWidth = 960;
constexpr int kScreenHeight = 540;

Font g_font{};

// ゲームの状態はグローバル(無名名前空間)にまとめておく。
// Web では 1 フレームごとに関数を呼び出される構造になるため、
// main() のローカル変数に状態を持たせることができない。

void Update(float dt) {
    (void)dt;  // 経過秒数。移動量などに掛けて使う
}

void Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // テキストを描画してみる
    // void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint)
    DrawTextEx(g_font, "日本語テスト", Vector2{kScreenWidth / 2 , kScreenHeight / 2}, 40, 1,BLACK);

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
    g_font = infrastructure::resource::loadJapaneseFont("resources/fonts/NotoSansJP-Regular-subset.ttf", 48,
                          "日本語テスト");
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
    CloseWindow();
    return 0;
}
