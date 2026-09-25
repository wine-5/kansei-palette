#pragma once
#include "raylib.h"
#include "core/Spring.h"
#include "game/board/Board.h"
#include "game/event/GameEvent.h"
#include "game/flow/GameFlow.h"
#include <array>

namespace infrastructure::resource
{
	class Assets;
}

namespace infrastructure::render
{
	class RestoreShader;

	/**
	 * @brief 3D の箱庭(台座・地面・タイル・小物・主人公)を描く
	 * @details 固定カメラ(縦視野角 32 度、俯角 50 度)で見下ろす。
	 *          タイルの回転アニメーションや光の強さなど、見た目だけの状態はここで持つ。
	 */
	class WorldRenderer
	{
	public:
		/**
		 * @brief 初期化する(カメラの設定、タイル用の板メッシュの作成)
		 * @param assets 読み込み済みのアセット
		 */
		void init(const resource::Assets& assets);

		/// 作ったメッシュなどを解放する
		void unload();

		/**
		 * @brief 見た目の状態を 1 フレーム分進める(タイルのバネ、通電度、小物のせり上がり、カメラ)
		 * @param dt 経過秒数
		 * @param flow ゲームの状態
		 * @param events このフレームに起きたこと
		 * @param cameraShake カメラの揺れ(infrastructure::fx::Effects から)
		 */
		void update(float dt, const game::flow::GameFlow& flow, const game::event::GameEventList& events, Vector3 cameraShake);

		/**
		 * @brief 箱庭を描く(BeginMode3D 〜 EndMode3D を含む)
		 * @param flow ゲームの状態
		 * @param shader 色の復元シェーダー
		 */
		void draw(const game::flow::GameFlow& flow, const RestoreShader& shader) const;

		/// 現在のカメラ(マウスからマスを求めるときに使う)
		const Camera3D& getCamera() const { return m_camera; }

	private:
		/**
		 * @brief タイル 1 枚分の見た目の状態
		 */
		struct TileVisual
		{
			core::Spring m_angle;   // 回転角(度)
			float m_tapScale{};     // タップ時に一瞬大きくなる量
			float m_power{};        // 通電度(0〜1)。色づきと光に使う
		};

		const resource::Assets* m_assets{};
		Camera3D m_camera{};
		Model m_tileModel{}; // タイル用の板(GenMeshPlane)
		std::array<TileVisual, game::board::Board::SIZE * game::board::Board::SIZE> m_tileVisuals{};
	};
} // namespace infrastructure::render
