// ゲームジャム用 raylib ひな形
// ネイティブ(Windows/Linux/macOS)と Web(Emscripten) で同じソースをビルドする。

#include <string>

#include "raylib.h"
#include "jp_font.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

namespace {

constexpr int kScreenWidth = 960;
constexpr int kScreenHeight = 540;

// 画面に出す日本語の文章はここにまとめる。
// フォント読み込み時にこれらの文字(特に漢字)を拾ってアトラスに焼き込むため、
// ここに無い漢字を DrawTextEx すると「?」になる。
namespace text {
constexpr const char* kTitle = "感性パレット";
constexpr const char* kHint = "矢印キーで円を動かせます";
constexpr const char* kSample = "日本語テキスト表示のサンプル。ひらがな・カタカナ・漢字ＯＫ！";
constexpr const char* kClickToStart = "クリックしてスタート";
constexpr const char* kClickForSe = "クリックで効果音";
}  // namespace text

std::string AllTexts() {
    return std::string(text::kTitle) + text::kHint + text::kSample + text::kClickToStart + text::kClickForSe;
}

constexpr int kFontSize = 48;  // アトラスに焼くサイズ(表示する最大サイズに合わせる)

// ゲームの状態はグローバル(無名名前空間)にまとめておく。
// Web では 1 フレームごとに関数を呼び出される構造になるため、
// main() のローカル変数に状態を持たせることができない。
struct Game {
    Vector2 circlePos{kScreenWidth / 2.0f, kScreenHeight / 2.0f};
    float circleSpeed = 240.0f;  // px/秒
    Texture2D palette{};
    Font font{};
    float time = 0.0f;

    bool audioStarted = false;  // 最初のクリック後に true
    Sound se{};
    Music bgm{};
};

Game g;

// 【Web の落とし穴: 自動再生制限】
// ブラウザはユーザーが操作(クリック・キー入力・タップ)する前のページで音を鳴らすことを禁止している。
// 操作前に InitAudioDevice() すると音声出力(AudioContext)が停止状態で作られ、PlaySound しても無音になる。
// そこで「クリックしてスタート」画面を挟み、最初の操作を受けてから音声デバイスを初期化・読み込み・再生する。
// ネイティブでも同じ流れにしておくと、Web だけ挙動が違うという事故を防げる。
void StartAudio() {
    InitAudioDevice();
    // Sound: 全体をメモリに展開する。短い効果音向け(DxLib の LoadSoundMem 相当)
    g.se = LoadSound("resources/sounds/se_click.wav");
    // Music: 少しずつ読み込みながら再生する(ストリーミング)。長い BGM 向け
    g.bgm = LoadMusicStream("resources/sounds/bgm_loop.wav");
    g.bgm.looping = true;
    SetMusicVolume(g.bgm, 0.6f);
    PlayMusicStream(g.bgm);
    g.audioStarted = true;
}

void StopAudio() {
    if (!g.audioStarted) return;
    UnloadMusicStream(g.bgm);
    UnloadSound(g.se);
    CloseAudioDevice();
}

void LoadResources() {
    // パスは実行時のカレントディレクトリからの相対パス。
    // Web では --preload-file で仮想ファイルシステムの /resources に置かれるので同じパスで読める。
    // Texture は GPU 側の画像なので InitWindow の後でないと読み込めない(DxLib の LoadGraph と同じ感覚)。
    g.palette = LoadTexture("resources/images/palette.png");
    SetTextureFilter(g.palette, TEXTURE_FILTER_BILINEAR);  // 回転・拡大時のギザギザを抑える(ドット絵なら外す)
    // Web では読み込みが重くなるので、フォントファイルは tools/subset_font.py で縮小したものを使う
    g.font = LoadJapaneseFont("resources/fonts/NotoSansJP-Regular-subset.ttf", kFontSize, AllTexts().c_str());
}

void UnloadResources() {
    UnloadFont(g.font);
    UnloadTexture(g.palette);
}

// DrawText(既定フォント・ASCII のみ)の日本語版。size は表示ピクセルサイズ
void DrawTextJp(const char* str, float x, float y, float size, Color color) {
    DrawTextEx(g.font, str, Vector2{x, y}, size, 1.0f, color);
}

void Update(float dt) {
    g.time += dt;

    const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);  // タッチ操作でも true になる
    if (!g.audioStarted) {
        if (clicked || GetKeyPressed() != 0) StartAudio();
        return;  // スタート前はゲームを進めない
    }

    // Music はバッファを補充するため毎フレーム呼ぶ必要がある(呼ばないと音が途切れる・止まる)。
    // Web ではタブが非表示の間メインループが止まるので、BGM も止まる。
    UpdateMusicStream(g.bgm);

    if (clicked) PlaySound(g.se);

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

    DrawTextJp(text::kTitle, 40, 40, 48, DARKGRAY);
    DrawTextJp(text::kSample, 40, 110, 24, GRAY);
    DrawTextJp(text::kHint, 40, kScreenHeight - 50.0f, 24, GRAY);
    DrawTextJp(text::kClickForSe, 40, kScreenHeight - 84.0f, 24, GRAY);

    if (!g.audioStarted) {
        // スタート画面: 半透明の黒で覆い、中央に案内を出す
        DrawRectangle(0, 0, kScreenWidth, kScreenHeight, Fade(BLACK, 0.6f));
        const float size = 40.0f;
        const Vector2 textSize = MeasureTextEx(g.font, text::kClickToStart, size, 1.0f);
        const float alpha = 0.6f + 0.4f * (g.time - static_cast<int>(g.time));  // ゆっくり点滅
        DrawTextJp(text::kClickToStart, (kScreenWidth - textSize.x) / 2, (kScreenHeight - textSize.y) / 2,
                   size, Fade(RAYWHITE, alpha));
    }

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
    StopAudio();
    UnloadResources();
    CloseWindow();
    return 0;
}
