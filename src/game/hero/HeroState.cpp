#include "HeroState.h"

namespace game::hero
{
	void HeroState::update(float dt, bool hasInput)
	{
		// TODO: 実装する
		// - 汗・目が回る・考え込むは決まった秒数で待機に戻す
		// - 操作がない時間が HERO_IDLE_THINK_TIME を超えたら考え込む(その後は HERO_THINK_REPEAT_TIME ごと)
		// - 走り中は HERO_RUN_SPEED で立ち位置まで進み、着いたら待機に戻す
		(void)hasInput;
		m_poseTime += dt;
	}

	void HeroState::startRunIn()
	{
		// TODO: 実装する
	}

	void HeroState::placeAtTitle()
	{
		// TODO: 実装する
	}

	void HeroState::onTileBlocked()
	{
		// TODO: 実装する
	}

	void HeroState::onReset()
	{
		// TODO: 実装する
	}

	void HeroState::onStageCleared()
	{
		// TODO: 実装する
	}

	void HeroState::onEnding()
	{
		// TODO: 実装する
	}
} // namespace game::hero
