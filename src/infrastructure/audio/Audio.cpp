#include "Audio.h"
#include <algorithm>
#include <initializer_list>
#include <cstdio>
#include <string>

namespace
{
	// 音は tools/gen_sounds.py で作ったもの
	constexpr const char* SOUND_DIR{ "resources/sounds/" };

	/**
	 * @brief 効果音を読み込む
	 * @param name ファイル名(拡張子なし)
	 * @return 読み込んだ音
	 */
	Sound loadEffect(const char* name)
	{
		return LoadSound((std::string(SOUND_DIR) + name + ".wav").c_str());
	}
} // namespace

namespace infrastructure::audio
{
	void Audio::onEvents(const game::event::GameEventList& events, bool isSoundOn)
	{
		for (const auto& event : events)
		{
			if (event.m_type == game::event::GameEventType::GameStarted && !m_isInitialized)
				initialize();
		}
		if (!m_isInitialized || !isSoundOn)
			return;

		for (const auto& event : events)
		{
			switch (event.m_type)
			{
			case game::event::GameEventType::TileRotated: PlaySound(m_rotate); break;
			case game::event::GameEventType::TileBlocked: PlaySound(m_blocked); break;
			case game::event::GameEventType::GoalLit:
			{
				// 点灯したゴールが多いほど高い音にする(m_value = 点灯数)
				const int index{ std::clamp(event.m_value - 1, 0, static_cast<int>(m_goals.size()) - 1) };
				PlaySound(m_goals[index]);
				break;
			}
			case game::event::GameEventType::StageCleared: PlaySound(m_clear); break;
			case game::event::GameEventType::ColorRestoring: PlaySound(m_restore); break;
			case game::event::GameEventType::EndingStarted: PlaySound(m_ending); break;
			case game::event::GameEventType::StageReset: PlaySound(m_reset); break;
			default: break;
			}
		}
	}

	void Audio::unload()
	{
		if (!m_isInitialized)
			return;
		for (Sound* sound : { &m_rotate, &m_blocked, &m_clear, &m_restore, &m_ending, &m_reset })
			UnloadSound(*sound);
		for (Sound& sound : m_goals)
			UnloadSound(sound);
		CloseAudioDevice();
		m_isInitialized = false;
	}

	void Audio::initialize()
	{
		InitAudioDevice();
		m_rotate = loadEffect("rotate");
		m_blocked = loadEffect("blocked");
		m_clear = loadEffect("clear");
		m_restore = loadEffect("restore");
		m_ending = loadEffect("ending");
		m_reset = loadEffect("reset");
		for (size_t i{}; i < m_goals.size(); ++i)
		{
			char name[16]{};
			std::snprintf(name, sizeof(name), "goal_%zu", i);
			m_goals[i] = loadEffect(name);
		}
		m_isInitialized = true;
	}
} // namespace infrastructure::audio
