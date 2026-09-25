#include "WorldRenderer.h"
#include "RestoreShader.h"
#include "infrastructure/resource/Assets.h"
#include "game/data/Config.h"

namespace infrastructure::render
{
	void WorldRenderer::init(const resource::Assets& assets)
	{
		m_assets = &assets;

		// TODO: 実装する(注視点から俯角と距離でカメラ位置を求める。距離は画面の縦横比で調整する)
		m_camera.target = Vector3{ game::data::CAMERA_TARGET_X, game::data::CAMERA_TARGET_Y, game::data::CAMERA_TARGET_Z };
		m_camera.position = Vector3{ 0.0f, 10.0f, 9.0f };
		m_camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
		m_camera.fovy = game::data::CAMERA_FOVY_DEG;
		m_camera.projection = CAMERA_PERSPECTIVE;

		// TODO: 実装する(m_tileModel = LoadModelFromMesh(GenMeshPlane(...)))
	}

	void WorldRenderer::unload()
	{
		// TODO: 実装する(UnloadModel)
	}

	void WorldRenderer::update(float dt, const game::flow::GameFlow& flow, const game::event::GameEventList& events, Vector3 cameraShake)
	{
		// TODO: 実装する
		// - TileRotated でそのタイルのバネの目標角度を 90 度進める。タップで m_tapScale を与える
		// - 通電度を POWER_RISE_RATE / POWER_FALL_RATE で近づける(電源からの距離ぶん遅らせる)
		// - ポインター位置による視差とカメラの揺れをカメラに反映する
		(void)dt;
		(void)flow;
		(void)events;
		(void)cameraShake;
	}

	void WorldRenderer::draw(const game::flow::GameFlow& flow, const RestoreShader& shader) const
	{
		BeginMode3D(m_camera);
		shader.begin();

		// TODO: 実装する(地面・台座 → タイル → 小物 → 主人公の順に描く)
		(void)flow;
		DrawGrid(10, 1.0f); // 仮: 枠組みの確認用に格子を描く(実装したら消す)

		shader.end();
		EndMode3D();
	}
} // namespace infrastructure::render
