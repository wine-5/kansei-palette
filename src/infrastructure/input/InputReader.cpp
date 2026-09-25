#include "InputReader.h"
#include "game/board/Board.h"
#include "game/data/Config.h"
#include <cmath>

namespace infrastructure::input
{
	game::flow::GameInput InputReader::read(const Camera3D& camera, const ui::UiAction& uiAction) const
	{
		game::flow::GameInput input{};
		input.m_isConfirmPressed = uiAction.m_isConfirmClicked || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
		input.m_isResetPressed = uiAction.m_isResetClicked || IsKeyPressed(KEY_R);
		input.m_isSoundTogglePressed = uiAction.m_isSoundToggleClicked;

		// ボタンの上にあるときは、その下のマスを触ったことにしない
		if (!uiAction.m_isPointerOverUi)
		{
			pickCell(camera, GetMousePosition(), input.m_hoveredRow, input.m_hoveredCol);
			// タッチのタップも raylib がマウスの左クリックとして扱う
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				input.m_tappedRow = input.m_hoveredRow;
				input.m_tappedCol = input.m_hoveredCol;
			}
		}

		input.m_hasAnyInput = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || GetKeyPressed() != 0;
		return input;
	}

	void InputReader::pickCell(const Camera3D& camera, Vector2 screenPos, int& outRow, int& outCol)
	{
		outRow = -1;
		outCol = -1;

		// マウス位置から飛ばしたレイと、盤面の平面(y = 0)との交点を求める
		const Ray ray{ GetScreenToWorldRay(screenPos, camera) };
		if (std::fabs(ray.direction.y) < 1e-6f)
			return;
		const float t{ -ray.position.y / ray.direction.y };
		if (t < 0.0f)
			return;
		const float x{ ray.position.x + ray.direction.x * t };
		const float z{ ray.position.z + ray.direction.z * t };

		// 盤面の中心が原点、1 マス = TILE_SIZE(infrastructure::render::WorldRenderer::cellToWorld の逆)
		constexpr float half{ game::board::Board::SIZE / 2.0f };
		const int col{ static_cast<int>(std::floor(x / game::data::TILE_SIZE + half)) };
		const int row{ static_cast<int>(std::floor(z / game::data::TILE_SIZE + half)) };
		if (!game::board::Board::isInside(row, col))
			return;
		outRow = row;
		outCol = col;
	}
} // namespace infrastructure::input
