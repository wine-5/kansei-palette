// かんせいのパレット
// ネイティブ(Windows/Linux/macOS)と Web(Emscripten) で同じソースをビルドする。
#include "Application.h"
#include "raylib.h"
#include "game/data/Config.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

namespace
{
	// Web では 1 フレームごとに関数を呼び出される構造になるため、
	// main() のローカル変数ではなく、ここに置いておく
	Application g_application;
} // namespace

int main()
{
	InitWindow(game::data::SCREEN_WIDTH, game::data::SCREEN_HEIGHT, "かんせいのパレット");
	g_application.init();

#if defined(PLATFORM_WEB)
	// 【Web の落とし穴: メインループ】
	// ブラウザでは while ループで処理を抱え込むとタブが固まる(描画もイベントも止まる)。
	// そのため 1 フレーム分の関数をブラウザに登録し、requestAnimationFrame ごとに呼んでもらう。
	// 第 2 引数 0 = ブラウザのリフレッシュレートに合わせる(SetTargetFPS は使わない)。
	// 第 3 引数 1 = ここから戻らない(以降の行はページが閉じるまで実行されない)。
	emscripten_set_main_loop([] { g_application.runFrame(); }, 0, 1);
#else
	SetTargetFPS(60);
	while (!WindowShouldClose())
		g_application.runFrame();
#endif

	// Web ではここに到達しない(タブを閉じればブラウザが全て解放する)
	g_application.shutdown();
	CloseWindow();
	return 0;
}
