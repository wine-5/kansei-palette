#include "InputReader.h"

namespace infrastructure::input
{
	game::flow::GameInput InputReader::read(const Camera3D& camera, const ui::UiAction& uiAction) const
	{
		game::flow::GameInput input{};
		input.m_isConfirmPressed = uiAction.m_isConfirmClicked || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
		input.m_isResetPressed = uiAction.m_isResetClicked || IsKeyPressed(KEY_R);
		input.m_isSoundTogglePressed = uiAction.m_isSoundToggleClicked;

		// TODO: 実装する
		// - ボタンの上でなければ pickCell でホバー中のマスを求める
		// - IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ならタップしたマスにする
		// - m_hasAnyInput(考え込むタイマーのリセット用)を設定する
		(void)camera;
		return input;
	}

	void InputReader::pickCell(const Camera3D& camera, Vector2 screenPos, int& outRow, int& outCol)
	{
		// TODO: 実装する(GetScreenToWorldRay → y = 0 の平面との交点 → x, z から行と列)
		(void)camera;
		(void)screenPos;
		outRow = -1;
		outCol = -1;
	}
} // namespace infrastructure::input
