// かんせいのパレット
// ネイティブ(Windows/Linux/macOS)と Web(Emscripten) で同じソースをビルドする。
#include "Application.h"
#include "raylib.h"
#include "game/data/Config.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <algorithm>
#endif

namespace
{
	// Web では 1 フレームごとに関数を呼び出される構造になるため、
	// main() のローカル変数ではなく、ここに置いておく
	Application g_application;

#if defined(PLATFORM_WEB)
	/// 高解像度の画面でも重くなりすぎないよう、devicePixelRatio はここまでに抑える
	constexpr double MAX_DEVICE_PIXEL_RATIO{ 2.0 };

	/**
	 * @brief キャンバスをブラウザの表示領域いっぱいに合わせ、実際の画素数で描けるようにする
	 * @details 【Web の落とし穴: キャンバスの大きさ】
	 *          キャンバスの画素数(canvas.width)と、画面上の表示サイズ(CSS の width)は別物。
	 *          1280×720 で描いてブラウザに引き伸ばしてもらうと、高解像度の画面(表示倍率 200% など)ではぼやける。
	 *          そこで画素数 = 表示サイズ × devicePixelRatio にして描き、UI は ui::uiScale() で拡大する。
	 *          itch.io の埋め込み(iframe)でも、iframe の大きさに合わせて伸縮する。
	 */
	void syncCanvasSize()
	{
		const double ratio{ std::min(MAX_DEVICE_PIXEL_RATIO, EM_ASM_DOUBLE({ return window.devicePixelRatio || 1; })) };
		const int cssWidth{ EM_ASM_INT({ return window.innerWidth; }) };
		const int cssHeight{ EM_ASM_INT({ return window.innerHeight; }) };
		const int width{ std::max(1, static_cast<int>(cssWidth * ratio)) };
		const int height{ std::max(1, static_cast<int>(cssHeight * ratio)) };
		if (width != GetScreenWidth() || height != GetScreenHeight())
			SetWindowSize(width, height);
		// SetWindowSize は CSS の大きさも画素数に書き換えるので、表示サイズを戻す
		// (EM_ASM の中の JavaScript でカンマ区切りの宣言を書くと、C++ のマクロの引数として分かれてしまうので避ける)
		EM_ASM({
			var canvas = Module.canvas;
			var w = $0 + 'px';
			var h = $1 + 'px';
			if (canvas.style.width !== w) canvas.style.width = w;
			if (canvas.style.height !== h) canvas.style.height = h;
		}, cssWidth, cssHeight);
	}
#endif
} // namespace

int main()
{
	// MSAA 4x: タイルや台座の縁のギザギザ(ジャギー)をなめらかにする
	unsigned int flags{ FLAG_MSAA_4X_HINT };
#if !defined(PLATFORM_WEB)
	// Windows の表示倍率(125% / 150% など)で拡大されるとき、引き伸ばしでぼやけないよう実際の画素数で描く。
	// ウィンドウの大きさも変えられるようにする(縦長・横長での見え方の確認用)。
	// (Web 版は syncCanvasSize() で同じことをする)
	flags |= FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE;
#endif
	SetConfigFlags(flags);
	InitWindow(game::data::SCREEN_WIDTH, game::data::SCREEN_HEIGHT, "かんせいのパレット");
	g_application.init();

#if defined(PLATFORM_WEB)
	// 【Web の落とし穴: メインループ】
	// ブラウザでは while ループで処理を抱え込むとタブが固まる(描画もイベントも止まる)。
	// そのため 1 フレーム分の関数をブラウザに登録し、requestAnimationFrame ごとに呼んでもらう。
	// 第 2 引数 0 = ブラウザのリフレッシュレートに合わせる(SetTargetFPS は使わない)。
	// 第 3 引数 1 = ここから戻らない(以降の行はページが閉じるまで実行されない)。
	emscripten_set_main_loop([] {
		syncCanvasSize();
		g_application.runFrame();
	}, 0, 1);
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
