#include "Application.h"
#include "raylib.h"

void Application::init()
{
	m_assets.load();
	m_restoreShader.load();
	m_worldRenderer.init(m_assets, m_restoreShader);
	m_shaderPreview.init();
	m_effects.init();
}

void Application::runFrame()
{
	const float dt{ GetFrameTime() };

	// 入力 → game 層
	const infrastructure::ui::UiAction uiAction{ updateUi() };
	const game::flow::GameInput input{ m_inputReader.read(m_worldRenderer.getCamera(), uiAction) };
	m_flow.update(dt, input);

	// game 層で起きたこと → 音・演出
	const game::event::GameEventList& events{ m_flow.getEvents() };
	m_audio.onEvents(events, m_flow.isSoundOn());
	m_effects.onEvents(events, m_flow);
	m_effects.update(dt);
	m_worldRenderer.update(dt, m_flow, events, input, m_effects.getCameraShake());
	m_shaderPreview.update();
	m_restoreShader.setRestoreLevel(m_shaderPreview.isForcingFullRestore() ? game::flow::RestoreLevel{ 1.0f, 1.0f, 1.0f } : m_flow.getRestore());

	draw();
}

void Application::shutdown()
{
	m_audio.unload();
	m_effects.unload();
	m_shaderPreview.unload();
	m_worldRenderer.unload();
	m_restoreShader.unload();
	m_assets.unload();
}

infrastructure::ui::UiAction Application::updateUi()
{
	switch (m_flow.getPhase())
	{
	case game::flow::GamePhase::Title:
	case game::flow::GamePhase::Ending: return m_titleScreen.update(m_flow);
	case game::flow::GamePhase::ClearCard: return m_clearCard.update(m_flow);
	case game::flow::GamePhase::StageIntro:
	case game::flow::GamePhase::Playing:
	case game::flow::GamePhase::Clearing: return m_hud.update(m_flow);
	}
	return infrastructure::ui::UiAction{};
}

void Application::draw()
{
	BeginDrawing();

	m_worldRenderer.draw(m_flow, m_restoreShader);
	BeginMode3D(m_worldRenderer.getCamera());
	m_effects.drawWorld(m_worldRenderer.getCamera());
	EndMode3D();
	m_effects.drawScreen();

	switch (m_flow.getPhase())
	{
	case game::flow::GamePhase::Title:
	case game::flow::GamePhase::Ending: m_titleScreen.draw(m_flow, m_assets); break;
	case game::flow::GamePhase::ClearCard:
		m_hud.draw(m_flow, m_assets);
		m_clearCard.draw(m_flow, m_assets);
		break;
	case game::flow::GamePhase::StageIntro:
	case game::flow::GamePhase::Playing:
	case game::flow::GamePhase::Clearing: m_hud.draw(m_flow, m_assets); break;
	}

	m_shaderPreview.draw(m_restoreShader);
	if (m_shaderPreview.isVisible())
		DrawFPS(GetScreenWidth() - 100, GetScreenHeight() - 30); // 確認画面(F1)を開いているときだけ出す
	EndDrawing();
}
