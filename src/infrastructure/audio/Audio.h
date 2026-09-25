#pragma once
#include "raylib.h"
#include "game/event/GameEvent.h"

namespace infrastructure::audio
{
	/**
	 * @brief 効果音を鳴らす(要件定義 6章「サウンド」)
	 * @details 【Web の落とし穴: 自動再生制限】
	 *          ブラウザは、ユーザーが操作する前のページで音を鳴らすことを禁止している。
	 *          操作前に InitAudioDevice() すると無音になるため、GameStarted(「はじめる」のクリック)を
	 *          受け取ってから初期化と読み込みを行う。
	 */
	class Audio
	{
	public:
		/**
		 * @brief このフレームに起きたことに応じて効果音を鳴らす
		 * @param events 起きたこと
		 * @param isSoundOn 音が ON か(OFF なら鳴らさない)
		 */
		void onEvents(const game::event::GameEventList& events, bool isSoundOn);

		/// 音声デバイスと効果音を解放する
		void unload();

	private:
		/// 音声デバイスを初期化し、効果音を読み込む(最初のクリック後に 1 回だけ)
		void initialize();

		bool m_isInitialized{};
		// TODO: 効果音(回転、点灯、クリア、色が戻る、エンディング、回せない、やりなおし)の Sound を持つ
	};
} // namespace infrastructure::audio
