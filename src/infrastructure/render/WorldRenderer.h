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
	 * @brief 3D の箱庭(地面・台座・タイル・小物・主人公)を描く
	 * @details 固定カメラ(縦視野角 32 度、俯角 50 度)で見下ろす。
	 *          タイルの回転アニメーションや通電度のフェードなど、見た目だけの状態はここで持つ。
	 *          座標: X = 右、Y = 上、Z = 手前。盤面の中心が原点で、1 マス = 1.0。
	 */
	class WorldRenderer
	{
	public:
		/**
		 * @brief 初期化する(タイル用の板メッシュの作成、カメラの設定)
		 * @param assets 読み込み済みのアセット
		 * @param shader 色の復元シェーダー(タイルのモデルに設定する)
		 */
		void init(const resource::Assets& assets, const RestoreShader& shader);

		/// 作ったメッシュなどを解放する
		void unload();

		/**
		 * @brief 見た目の状態を 1 フレーム分進める(タイルのバネ、通電度、カメラ)
		 * @param dt 経過秒数
		 * @param flow ゲームの状態
		 * @param events このフレームに起きたこと
		 * @param input このフレームの操作(ホバー中のマスを浮かせるのに使う)
		 * @param cameraShake カメラの揺れ(infrastructure::fx::Effects から)
		 */
		void update(float dt, const game::flow::GameFlow& flow, const game::event::GameEventList& events, const game::flow::GameInput& input, Vector3 cameraShake);

		/**
		 * @brief 箱庭を描く(BeginMode3D 〜 EndMode3D を含む)
		 * @param flow ゲームの状態
		 * @param shader 色の復元シェーダー
		 */
		void draw(const game::flow::GameFlow& flow, const RestoreShader& shader) const;

		/// 現在のカメラ(マウスからマスを求めるときに使う)
		const Camera3D& getCamera() const { return m_camera; }

		/**
		 * @brief マスの中心のワールド座標
		 * @param row 行
		 * @param col 列
		 * @return 盤面の上(y = 0)の座標
		 */
		static Vector3 cellToWorld(int row, int col);

	private:
		/**
		 * @brief タイル 1 枚分の見た目の状態
		 */
		struct TileVisual
		{
			core::Spring m_angle;  // 時計回りの回転角(度)。回転数 × 90 を目標にバネで動く
			float m_tapScale{};    // タップ時に一瞬大きくなる量
			float m_power{};       // 通電度(0〜1)。色づきと光に使う
			float m_poweredTime{}; // 電気が届いてからの秒数(電源から遠いほど遅れて色づく)
			float m_hover{};       // マウスが乗っている度合い(0〜1)。回せるタイルを浮かせる
		};

		/// 盤面の状態に合わせて、すべてのタイルの見た目を即座にそろえる(ステージ開始・やりなおし時)
		void snapTiles(const game::board::Board& board);

		/// 画面の縦横比とポインター位置から、カメラの位置を決める
		void updateCamera(Vector3 cameraShake);

		/// 空・山・村の帯を描く(3D の前。2D)
		void drawBackground() const;
		/// 手前の花と柵の帯を描く(3D の後。2D)
		void drawForeground() const;
		void drawGround() const;
		void drawTiles(const game::board::Board& board, int stageIndex) const;
		/// 通電中のタイルに光を重ねる(加算合成)
		void drawGlows(const game::board::Board& board) const;
		void drawProps(const game::flow::GameFlow& flow) const;

		const resource::Assets* m_assets{};
		Camera3D m_camera{};
		Model m_tileModel{}; // タイル用の 1×1 の板(GenMeshPlane)。色の復元シェーダーで描く
		Model m_glowModel{}; // 光用の 1×1 の板。標準のシェーダーで描く(光はグレーにしない)
		Texture2D m_glowTexture{}; // 中心が明るく外へ消えていく円
		std::array<TileVisual, game::board::Board::SIZE * game::board::Board::SIZE> m_tileVisuals{};
		Vector2 m_parallax{}; // ポインター位置によるカメラのずれ(なめらかに追従させる)
		// 小物のせり上がり(0 = 地面の下、1 = 立っている)。[ステージ][小物]
		std::array<std::array<core::Spring, game::data::MAX_PROPS_PER_STAGE>, game::data::STAGES.size()> m_propRise{};
		bool m_needsSnap{ true };
		bool m_isShowingAllProps{}; // F2: 全ステージの小物を表示する(配置の確認用)
		float m_time{};
	};
} // namespace infrastructure::render
