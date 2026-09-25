#pragma once
#include "game/flow/GameFlow.h"
#include "infrastructure/audio/Audio.h"
#include "infrastructure/fx/Effects.h"
#include "infrastructure/input/InputReader.h"
#include "infrastructure/render/RestoreShader.h"
#include "infrastructure/render/WorldRenderer.h"
#include "infrastructure/resource/Assets.h"
#include "infrastructure/ui/ClearCard.h"
#include "infrastructure/ui/Hud.h"
#include "infrastructure/ui/TitleScreen.h"

/**
 * @brief ゲーム全体をつなぐ
 * @details 毎フレーム「入力を読む → game 層を進める → 起きたことを音・演出へ → 描く」の順に処理する。
 *          Web では 1 フレームごとに呼び出される構造になるため、状態はすべてこのクラスのメンバに持つ。
 */
class Application
{
public:
	/// 初期化する(InitWindow の後に呼ぶ)
	void init();

	/// 1 フレーム分の処理を行う
	void runFrame();

	/// 終了処理(CloseWindow の前に呼ぶ。Web では呼ばれない)
	void shutdown();

private:
	/// 現在の進行状態に応じた UI のボタン操作を判定する
	infrastructure::ui::UiAction updateUi();

	/// 描く
	void draw() const;

	game::flow::GameFlow m_flow;
	infrastructure::resource::Assets m_assets;
	infrastructure::render::RestoreShader m_restoreShader;
	infrastructure::render::WorldRenderer m_worldRenderer;
	infrastructure::fx::Effects m_effects;
	infrastructure::audio::Audio m_audio;
	infrastructure::input::InputReader m_inputReader;
	infrastructure::ui::TitleScreen m_titleScreen;
	infrastructure::ui::Hud m_hud;
	infrastructure::ui::ClearCard m_clearCard;
};
