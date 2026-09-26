#include "HeroState.h"
#include "game/data/Config.h"

namespace game::hero
{
	void HeroState::update(float dt, bool hasInput)
	{
		m_poseTime += dt;

		switch (m_pose)
		{
		case HeroPose::Run:
			// 立ち位置まで走り、着いたら待機に戻す
			m_x += data::HERO_RUN_SPEED * dt;
			if (m_x >= data::HERO_PLAY_X)
			{
				m_x = data::HERO_PLAY_X;
				changePose(HeroPose::Idle);
			}
			break;
		case HeroPose::Sweat:
			if (m_poseTime >= data::HERO_SWEAT_DURATION)
				changePose(HeroPose::Idle);
			break;
		case HeroPose::Dizzy:
			if (m_poseTime >= data::HERO_DIZZY_DURATION)
				changePose(HeroPose::Idle);
			break;
		case HeroPose::Think:
			// 考え込んだ後は、もう少し短い間隔で再び考え込む
			if (m_poseTime >= data::HERO_THINK_DURATION)
			{
				m_idleTimer = 0.0f;
				m_thinkInterval = data::HERO_THINK_REPEAT_TIME;
				changePose(HeroPose::Idle);
			}
			break;
		default: break;
		}

		// 操作がしばらくないと考え込む(操作があれば間隔を最初の長さに戻す)
		if (hasInput)
		{
			m_idleTimer = 0.0f;
			m_thinkInterval = data::HERO_IDLE_THINK_TIME;
			if (m_pose == HeroPose::Think)
				changePose(HeroPose::Idle);
		}
		else if (m_pose == HeroPose::Idle && m_canThink)
		{
			m_idleTimer += dt;
			if (m_idleTimer >= m_thinkInterval)
				changePose(HeroPose::Think);
		}
	}

	void HeroState::startRunIn()
	{
		m_x = data::HERO_RUN_START_X;
		m_z = data::HERO_PLAY_Z;
		m_canThink = true;
		m_idleTimer = 0.0f;
		m_thinkInterval = data::HERO_IDLE_THINK_TIME;
		changePose(HeroPose::Run);
	}

	void HeroState::placeAtTitle()
	{
		m_x = data::HERO_TITLE_X;
		m_z = data::HERO_TITLE_Z;
		m_canThink = false; // タイトルでは考え込まない
		changePose(HeroPose::Idle);
	}

	void HeroState::onTileBlocked()
	{
		if (m_pose != HeroPose::Run)
			changePose(HeroPose::Sweat);
	}

	void HeroState::onReset()
	{
		if (m_pose != HeroPose::Run)
			changePose(HeroPose::Dizzy);
	}

	void HeroState::onStageCleared()
	{
		changePose(HeroPose::Cheer);
	}

	void HeroState::onEnding()
	{
		placeAtTitle();
		changePose(HeroPose::JoyStar);
	}

	void HeroState::changePose(HeroPose pose)
	{
		m_pose = pose;
		m_poseTime = 0.0f;
	}
} // namespace game::hero
