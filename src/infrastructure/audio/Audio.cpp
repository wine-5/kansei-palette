#include "Audio.h"

namespace infrastructure::audio
{
	void Audio::onEvents(const game::event::GameEventList& events, bool isSoundOn)
	{
		for (const auto& event : events)
		{
			if (event.m_type == game::event::GameEventType::GameStarted && !m_isInitialized)
				initialize();
		}

		// TODO: 実装する(isSoundOn なら、出来事の種類ごとに PlaySound。GoalLit は点灯数で音の高さを変える)
		(void)isSoundOn;
	}

	void Audio::unload()
	{
		if (!m_isInitialized)
			return;
		// TODO: 実装する(読み込んだ Sound を UnloadSound)
		CloseAudioDevice();
		m_isInitialized = false;
	}

	void Audio::initialize()
	{
		InitAudioDevice();
		// TODO: 実装する(効果音を LoadSound で読み込む)
		m_isInitialized = true;
	}
} // namespace infrastructure::audio
