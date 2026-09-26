#pragma once
#include "raylib.h"
#include "game/data/Config.h"
#include <algorithm>

// UI の座標系
//
// UI は横長なら 1280×720、縦長なら 720×1280 を基準に作り、実際の画面に合わせて拡大縮小する。
// 画面が大きい・高解像度(Web で devicePixelRatio が 2 など)なら拡大し、小さい画面では縮小する。
// 縦長のスマートフォンでは UI の幅が 720 になるので、UI のコードは幅が狭くても崩れないようにする。
// UI のコードは GetScreenWidth() ではなく logicalWidth() / logicalHeight() を、
// GetMousePosition() ではなく uiMousePosition() を使うこと。
namespace infrastructure::ui
{
	/// 画面が縦長か
	inline bool isPortrait()
	{
		return GetScreenHeight() > GetScreenWidth();
	}

	/// 基準の大きさ(横長 1280×720 / 縦長 720×1280)に対する、画面の拡大率(縦横どちらにも収まる大きさ)
	inline float uiScale()
	{
		const float baseWidth{ static_cast<float>(isPortrait() ? game::data::SCREEN_HEIGHT : game::data::SCREEN_WIDTH) };
		const float baseHeight{ static_cast<float>(isPortrait() ? game::data::SCREEN_WIDTH : game::data::SCREEN_HEIGHT) };
		const float scaleX{ GetScreenWidth() / baseWidth };
		const float scaleY{ GetScreenHeight() / baseHeight };
		return std::max(0.01f, std::min(scaleX, scaleY));
	}

	/// UI の座標系での画面の幅
	inline float logicalWidth()
	{
		return GetScreenWidth() / uiScale();
	}

	/// UI の座標系での画面の高さ
	inline float logicalHeight()
	{
		return GetScreenHeight() / uiScale();
	}

	/// UI の座標系でのマウス(タッチ)の位置
	inline Vector2 uiMousePosition()
	{
		const Vector2 mouse{ GetMousePosition() };
		const float scale{ uiScale() };
		return Vector2{ mouse.x / scale, mouse.y / scale };
	}

	/// UI を描くときのカメラ(BeginMode2D に渡す)
	inline Camera2D uiCamera()
	{
		Camera2D camera{};
		camera.zoom = uiScale();
		return camera;
	}
} // namespace infrastructure::ui
