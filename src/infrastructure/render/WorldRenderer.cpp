#include "WorldRenderer.h"
#include "RestoreShader.h"
#include "core/Easing.h"
#include "game/data/Config.h"
#include "infrastructure/resource/Assets.h"
#include "rlgl.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
	// 箱庭の大きさ(ワールド単位)
	// 地面は空に浮かぶ小さな島にして、その奥に背景の帯(山・村)を見せる
	constexpr float GROUND_RADIUS{ 6.4f };
	constexpr float GROUND_BOTTOM_RADIUS{ 5.4f }; // 島の底(少しすぼめて、浮いた土の塊に見せる)
	constexpr float GROUND_THICKNESS{ 0.9f };
	constexpr float LAWN_RADIUS{ 5.9f };
	constexpr float PEDESTAL_SIZE{ 6.0f };
	constexpr float PEDESTAL_HEIGHT{ 0.32f };
	constexpr float PLATE_SIZE{ 5.5f };
	constexpr float PLATE_THICKNESS{ 0.02f }; // 板は台座の上に載せる(同じ高さに面があるとちらつく)
	constexpr float TILE_LIFT{ PLATE_THICKNESS + 0.01f }; // タイルは板より少し浮かせて、ちらつき(Z ファイティング)を防ぐ

	// 箱庭の色(色の復元シェーダーで、戻っていない色相はグレーになる)
	constexpr Color GROUND_COLOR{ 196, 172, 124, 255 };
	constexpr Color LAWN_COLOR{ 118, 176, 84, 255 };
	constexpr Color PEDESTAL_COLOR{ 150, 138, 124, 255 };
	constexpr Color PLATE_COLOR{ 92, 82, 74, 255 };

	// 立ち絵は垂直に立てず、上端を奥へ倒してカメラの方へ向ける(俯角 50 度の 0.9 倍)
	constexpr float BILLBOARD_TILT_DEG{ 45.0f };
	// 気球は地面から浮かせ、上下にゆらゆらさせる
	constexpr float BALLOON_FLOAT{ 0.9f };
	constexpr float BALLOON_BOB{ 0.08f };

	// 背景の帯: 画面の上半分に空・山・村を重ね、その手前に島(箱庭)を置く。
	// カメラの俯角が深く地平線がほぼ画面の上端に来るので、地平線ではなく画面の高さを基準に置く。
	// *_BOTTOM = 帯の下端の位置(画面の高さに対する割合)、*_SCALE = 帯の幅(画面の幅に対する倍率)
	// *_PARALLAX = 視差でずらす量(ピクセル)。手前の帯ほど大きく動かす
	constexpr float SKY_BOTTOM{ 0.40f };
	constexpr float SKY_SCALE{ 1.4f };
	constexpr float MOUNTAINS_BOTTOM{ 0.55f };
	constexpr float MOUNTAINS_SCALE{ 1.15f };
	constexpr float VILLAGE_BOTTOM{ 0.82f };
	constexpr float VILLAGE_SCALE{ 1.45f };
	constexpr float BG_WIDTH_SCALE{ 1.15f }; // 視差でずらしても端が見えないよう、画面より少し広く描く
	// 縦長の画面での帯の最低の高さ(画面の高さに対する割合)
	constexpr float SKY_MIN_HEIGHT{ 0.40f };
	constexpr float MOUNTAINS_MIN_HEIGHT{ 0.10f };
	constexpr float VILLAGE_MIN_HEIGHT{ 0.26f };
	constexpr float MOUNTAINS_PARALLAX{ 30.0f };
	constexpr float VILLAGE_PARALLAX{ 60.0f };
	constexpr float FOREGROUND_PARALLAX{ 90.0f };
	constexpr float FOREGROUND_HEIGHT_RATIO{ 0.17f }; // 手前の花の帯の高さ(画面の高さに対する割合)
	// 空の画像の上と下に続くグラデーション(画像の上端・下端の色に合わせる)
	constexpr Color SKY_TOP_COLOR{ 112, 168, 222, 255 };
	constexpr Color SKY_BOTTOM_COLOR{ 250, 214, 172, 255 };

	/**
	 * @brief 横長の帯を、画面幅に合わせた大きさで描く
	 * @param texture 帯の画像
	 * @param bottom 帯の下端の画面 y 座標
	 * @param widthScale 画面幅に対する帯の幅の倍率
	 * @param offsetX 横のずれ(視差)
	 * @param minHeight 帯の最低の高さ(縦長の画面では幅に合わせると低くなりすぎるので、左右をはみ出させて高さを保つ)
	 */
	void drawBand(const Texture2D& texture, float bottom, float widthScale, float offsetX, float minHeight = 0.0f)
	{
		if (texture.id == 0)
			return;
		const float aspect{ texture.width / static_cast<float>(texture.height) };
		const float height{ std::max(GetScreenWidth() * widthScale / aspect, minHeight) };
		const float width{ height * aspect };
		const Rectangle source{ 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) };
		const Rectangle dest{ (GetScreenWidth() - width) / 2.0f + offsetX, bottom - height, width, height };
		DrawTexturePro(texture, source, dest, Vector2{}, 0.0f, WHITE);
	}

	// 視差の追従の速さ(毎秒)
	constexpr float PARALLAX_FOLLOW_RATE{ 4.0f };
	// 回せないタイルをタップしたときに、回転のバネに与える勢い(度/秒)
	constexpr float BLOCKED_KICK{ 90.0f };
	// ホバーで浮かせる速さ(毎秒)
	constexpr float HOVER_RATE{ 14.0f };
	// タイルのアニメーション(バネで回る・タップで跳ねる・ホバーで浮く)を使うか。
	// 動かすと画像の細かい模様がちらついて見えるので、いったん止めて回転は即座に切り替える。
	// 見た目の演出は、あとでパーティクルやシェーダーで補う
	constexpr bool IS_TILE_ANIMATION_ENABLED{ false };

	// 通電中のタイルの光: タイルより少し大きく、タイルの種類ごとの色で、ゆっくり脈打つ
	constexpr float GLOW_SIZE{ 1.9f };
	constexpr float GLOW_LIFT{ 0.06f };
	constexpr float GLOW_OPACITY{ 0.5f };
	constexpr float GLOW_OPACITY_GOAL{ 0.95f };
	constexpr float GLOW_PULSE_BASE{ 0.82f };
	constexpr float GLOW_PULSE_AMOUNT{ 0.18f };
	constexpr float GLOW_PULSE_HZ{ 0.64f };
	constexpr int GLOW_TEXTURE_SIZE{ 64 };

	/// タイルの種類ごとの光の色(game::board::TileType の順)
	constexpr Color GLOW_COLORS[]{
		{ 0x5a, 0xc8, 0xff, 255 }, // 直線
		{ 0xff, 0xb3, 0x47, 255 }, // 曲がり
		{ 0x7d, 0xff, 0x7a, 255 }, // T字
		{ 0xc8, 0x8b, 0xff, 255 }, // 十字
		{ 0x7f, 0xe3, 0xff, 255 }, // 電源
		{ 0xff, 0xe2, 0x7a, 255 }, // ゴール
		{ 0xff, 0x6b, 0x5f, 255 }, // ロック
		{ 0, 0, 0, 0 },            // 空き(光らない)
	};

	int toIndex(int row, int col)
	{
		return row * game::board::Board::SIZE + col;
	}

	/// 立っている地面の高さ(台座の上か、その外の芝生か)
	float groundHeightAt(float x, float z)
	{
		const float half{ PEDESTAL_SIZE / 2.0f };
		return (std::fabs(x) <= half && std::fabs(z) <= half) ? 0.0f : -PEDESTAL_HEIGHT;
	}

	/// 主人公のポーズごとのコマ送りの速さ(毎秒のコマ数)
	float heroFps(game::hero::HeroPose pose)
	{
		switch (pose)
		{
		case game::hero::HeroPose::Idle: return 4.0f;
		case game::hero::HeroPose::Run: return 12.0f;
		case game::hero::HeroPose::Cheer: return 6.0f;
		default: return 1.0f;
		}
	}

	/// マスごとに決まった見た目違いの番号(同じ種類のタイルが並んでも単調に見えないようにする)
	int tileVariant(int stageIndex, int row, int col)
	{
		return (row * 7 + col * 13 + stageIndex * 5 + row * col) % infrastructure::resource::TILE_VARIANT_COUNT;
	}
} // namespace

namespace infrastructure::render
{
	void WorldRenderer::init(const resource::Assets& assets, const RestoreShader& shader)
	{
		m_assets = &assets;
		m_tileModel = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));
		// DrawModel は BeginShaderMode ではなく material のシェーダーで描かれる
		if (shader.isLoaded())
			m_tileModel.materials[0].shader = shader.getShader();

		Image glowImage{ GenImageGradientRadial(GLOW_TEXTURE_SIZE, GLOW_TEXTURE_SIZE, 0.0f, WHITE, BLANK) };
		m_glowTexture = LoadTextureFromImage(glowImage);
		UnloadImage(glowImage);
		SetTextureFilter(m_glowTexture, TEXTURE_FILTER_BILINEAR);
		m_glowModel = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));
		m_glowModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = m_glowTexture;

		m_camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
		m_camera.fovy = game::data::CAMERA_FOVY_DEG;
		m_camera.projection = CAMERA_PERSPECTIVE;
		updateCamera(Vector3{});
	}

	void WorldRenderer::unload()
	{
		// UnloadModel は material のシェーダーとテクスチャも解放してしまうので、借りているものは既定に戻しておく
		Material& material{ m_tileModel.materials[0] };
		material.shader.id = rlGetShaderIdDefault();
		material.shader.locs = rlGetShaderLocsDefault();
		material.maps[MATERIAL_MAP_ALBEDO].texture.id = rlGetTextureIdDefault();
		UnloadModel(m_tileModel);
		m_tileModel = Model{};
		m_glowModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture.id = rlGetTextureIdDefault();
		UnloadModel(m_glowModel);
		m_glowModel = Model{};
		UnloadTexture(m_glowTexture);
		m_glowTexture = Texture2D{};
	}

	void WorldRenderer::update(float dt, const game::flow::GameFlow& flow, const game::event::GameEventList& events, const game::flow::GameInput& input, Vector3 cameraShake)
	{
		m_time += dt;
		if (IsKeyPressed(KEY_F2))
			m_isShowingAllProps = !m_isShowingAllProps;

		const game::board::Board& board{ flow.getBoard() };
		for (const auto& event : events)
		{
			switch (event.m_type)
			{
			case game::event::GameEventType::TileRotated:
			{
				TileVisual& visual{ m_tileVisuals[toIndex(event.m_row, event.m_col)] };
				visual.m_angle.m_target += 90.0f;
				visual.m_tapScale = game::data::TILE_TAP_SCALE;
				break;
			}
			case game::event::GameEventType::TileBlocked:
			{
				TileVisual& visual{ m_tileVisuals[toIndex(event.m_row, event.m_col)] };
				visual.m_angle.m_velocity += BLOCKED_KICK;
				visual.m_tapScale = game::data::TILE_TAP_SCALE;
				break;
			}
			case game::event::GameEventType::StageStarted:
			case game::event::GameEventType::StageReset: m_needsSnap = true; break;
			default: break;
			}
		}

		if (m_needsSnap)
		{
			snapTiles(board);
			m_needsSnap = false;
		}

		const bool isPlaying{ flow.getPhase() == game::flow::GamePhase::Playing };

		for (int row{}; row < game::board::Board::SIZE; ++row)
		{
			for (int col{}; col < game::board::Board::SIZE; ++col)
			{
				const game::board::Tile& tile{ board.getTile(row, col) };
				TileVisual& visual{ m_tileVisuals[toIndex(row, col)] };
				visual.m_angle.update(dt, game::data::TILE_SPRING_STIFFNESS, game::data::TILE_SPRING_DAMPING);
				visual.m_tapScale = core::approachExp(visual.m_tapScale, 0.0f, game::data::TILE_TAP_DECAY, dt);

				// 電源から遠いタイルほど遅れて色づく(電気が流れていくように見せる)。消えるときは待たずにすぐ消す
				visual.m_poweredTime = tile.m_isPowered ? visual.m_poweredTime + dt : 0.0f;
				const bool isLit{ tile.m_isPowered && visual.m_poweredTime >= tile.m_distance * game::data::POWER_DELAY_PER_STEP };
				const float target{ isLit ? 1.0f : 0.0f };
				const float rate{ target > visual.m_power ? game::data::POWER_RISE_RATE : game::data::POWER_FALL_RATE };
				visual.m_power = core::approachExp(visual.m_power, target, rate, dt);

				const bool isHovered{ isPlaying && row == input.m_hoveredRow && col == input.m_hoveredCol && game::board::isRotatable(tile.m_type) };
				visual.m_hover = core::approachExp(visual.m_hover, isHovered ? 1.0f : 0.0f, HOVER_RATE, dt);

				if constexpr (!IS_TILE_ANIMATION_ENABLED)
				{
					visual.m_angle.snapTo(tile.m_rotation * 90.0f);
					visual.m_tapScale = 0.0f;
					visual.m_hover = 0.0f;
				}
			}
		}

		// 回せるタイルの上では、カーソルを指の形にする
		const bool isOverRotatable{ isPlaying && game::board::Board::isInside(input.m_hoveredRow, input.m_hoveredCol)
			&& game::board::isRotatable(board.getTile(input.m_hoveredRow, input.m_hoveredCol).m_type) };
		SetMouseCursor(isOverRotatable ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

		// せり上がった小物はバネで伸び上がる(少し行き過ぎて戻る)。リセットされたら即座に消す
		for (size_t stage{}; stage < m_propRise.size(); ++stage)
		{
			const int raised{ flow.getRaisedPropCount(static_cast<int>(stage)) };
			for (int i{}; i < game::data::MAX_PROPS_PER_STAGE; ++i)
			{
				core::Spring& rise{ m_propRise[stage][i] };
				if (i < raised)
				{
					rise.m_target = 1.0f;
					rise.update(dt, game::data::PROP_SPRING_STIFFNESS, game::data::PROP_SPRING_DAMPING);
				}
				else
				{
					rise.snapTo(0.0f);
				}
			}
		}

		updateCamera(cameraShake);
	}

	void WorldRenderer::draw(const game::flow::GameFlow& flow, const RestoreShader& shader) const
	{
		ClearBackground(SKY_BOTTOM_COLOR);
		shader.begin();
		drawBackground();
		shader.end();

		BeginMode3D(m_camera);

		shader.begin();
		drawGround();
		shader.end();

		drawTiles(flow.getBoard(), flow.getStageIndex());
		drawGlows(flow.getBoard());

		shader.begin();
		drawProps(flow);
		shader.end();

		EndMode3D();
	}

	void WorldRenderer::drawForegroundLayer(const RestoreShader& shader) const
	{
		shader.begin();
		drawForeground();
		shader.end();
	}

	Vector3 WorldRenderer::cellToWorld(int row, int col)
	{
		constexpr float center{ (game::board::Board::SIZE - 1) / 2.0f };
		return Vector3{ (col - center) * game::data::TILE_SIZE, 0.0f, (row - center) * game::data::TILE_SIZE };
	}

	void WorldRenderer::snapTiles(const game::board::Board& board)
	{
		for (int row{}; row < game::board::Board::SIZE; ++row)
		{
			for (int col{}; col < game::board::Board::SIZE; ++col)
			{
				const game::board::Tile& tile{ board.getTile(row, col) };
				TileVisual& visual{ m_tileVisuals[toIndex(row, col)] };
				visual.m_angle.snapTo(tile.m_rotation * 90.0f);
				visual.m_tapScale = 0.0f;
				visual.m_power = tile.m_isPowered ? 1.0f : 0.0f;
				visual.m_poweredTime = tile.m_isPowered ? 100.0f : 0.0f;
			}
		}
	}

	void WorldRenderer::updateCamera(Vector3 cameraShake)
	{
		// 縦 VIEW_HEIGHT が必ず入る距離と、横 VIEW_WIDTH_MIN が必ず入る距離の遠い方(縦長の画面では横で決まる)
		const float aspect{ static_cast<float>(GetScreenWidth()) / static_cast<float>(std::max(GetScreenHeight(), 1)) };
		const float tanHalf{ std::tan(game::data::CAMERA_FOVY_DEG * DEG2RAD / 2.0f) };
		const float distance{ std::max((game::data::VIEW_HEIGHT / 2.0f) / tanHalf, (game::data::VIEW_WIDTH_MIN / 2.0f) / (tanHalf * aspect)) };

		// ポインターの位置に応じて、カメラを少しずらす(視差で奥行きを感じさせる)
		const Vector2 mouse{ GetMousePosition() };
		const float nx{ std::clamp(mouse.x / GetScreenWidth() * 2.0f - 1.0f, -1.0f, 1.0f) };
		const float ny{ std::clamp(mouse.y / GetScreenHeight() * 2.0f - 1.0f, -1.0f, 1.0f) };
		const float dt{ GetFrameTime() };
		m_parallax.x = core::approachExp(m_parallax.x, nx * game::data::PARALLAX_X, PARALLAX_FOLLOW_RATE, dt);
		m_parallax.y = core::approachExp(m_parallax.y, -ny * game::data::PARALLAX_Y, PARALLAX_FOLLOW_RATE, dt);

		const float pitch{ game::data::CAMERA_PITCH_DEG * DEG2RAD };
		const Vector3 target{ game::data::CAMERA_TARGET_X, game::data::CAMERA_TARGET_Y, game::data::CAMERA_TARGET_Z };
		m_camera.target = Vector3{ target.x + cameraShake.x, target.y + cameraShake.y, target.z + cameraShake.z };
		m_camera.position = Vector3{
			target.x + m_parallax.x + cameraShake.x,
			target.y + distance * std::sin(pitch) + m_parallax.y + cameraShake.y,
			target.z + distance * std::cos(pitch) + cameraShake.z,
		};
	}

	void WorldRenderer::drawGround() const
	{
		// 島の土の部分(上面が芝生の少し下)
		DrawCylinder(Vector3{ 0.0f, -PEDESTAL_HEIGHT - 0.03f - GROUND_THICKNESS, 0.0f }, GROUND_RADIUS, GROUND_BOTTOM_RADIUS, GROUND_THICKNESS, 64, GROUND_COLOR);
		DrawCylinder(Vector3{ 0.0f, -PEDESTAL_HEIGHT - 0.01f, 0.0f }, LAWN_RADIUS, LAWN_RADIUS, 0.02f, 64, LAWN_COLOR);
		DrawCube(Vector3{ 0.0f, -PEDESTAL_HEIGHT / 2.0f, 0.0f }, PEDESTAL_SIZE, PEDESTAL_HEIGHT, PEDESTAL_SIZE, PEDESTAL_COLOR);
		DrawCube(Vector3{ 0.0f, PLATE_THICKNESS / 2.0f, 0.0f }, PLATE_SIZE, PLATE_THICKNESS, PLATE_SIZE, PLATE_COLOR);
	}

	void WorldRenderer::drawBackground() const
	{
		const float screenHeight{ static_cast<float>(GetScreenHeight()) };
		const float shift{ -m_parallax.x / game::data::PARALLAX_X }; // -1〜1

		// 空: 画面全体をグラデーションで塗り、その上に空の画像を描く
		DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), SKY_TOP_COLOR, SKY_BOTTOM_COLOR);
		drawBand(m_assets->getBackground(resource::BackgroundLayer::Sky), screenHeight * SKY_BOTTOM, SKY_SCALE, 0.0f, screenHeight * SKY_MIN_HEIGHT);
		drawBand(m_assets->getBackground(resource::BackgroundLayer::Mountains), screenHeight * MOUNTAINS_BOTTOM, MOUNTAINS_SCALE, shift * MOUNTAINS_PARALLAX,
			screenHeight * MOUNTAINS_MIN_HEIGHT);
		drawBand(m_assets->getBackground(resource::BackgroundLayer::Village), screenHeight * VILLAGE_BOTTOM, VILLAGE_SCALE, shift * VILLAGE_PARALLAX,
			screenHeight * VILLAGE_MIN_HEIGHT);
	}

	void WorldRenderer::drawForeground() const
	{
		// 手前の花と柵は画面の下端に重ねる(箱庭を額縁のように囲む)
		const Texture2D& foreground{ m_assets->getBackground(resource::BackgroundLayer::Foreground) };
		if (foreground.id == 0)
			return;
		const float height{ GetScreenHeight() * FOREGROUND_HEIGHT_RATIO };
		const float widthScale{ height * foreground.width / static_cast<float>(foreground.height) / GetScreenWidth() };
		const float shift{ -m_parallax.x / game::data::PARALLAX_X };
		drawBand(foreground, static_cast<float>(GetScreenHeight()), std::max(widthScale, BG_WIDTH_SCALE), shift * FOREGROUND_PARALLAX);
	}

	void WorldRenderer::drawTiles(const game::board::Board& board, int stageIndex) const
	{
		// タイルの画像はマスより少し大きく(接点が板の外に出る分)、隣のタイルと同じ高さで重なっている。
		// 深度を書き込むと、重なった部分でどちらを手前に描くかが画素ごとに揺れて、カメラが動くたびにちらつく
		// (Z ファイティング)。タイルは深度を書き込まず、描いた順に重ねる
		rlDisableDepthMask();
		for (int row{}; row < game::board::Board::SIZE; ++row)
		{
			for (int col{}; col < game::board::Board::SIZE; ++col)
			{
				const game::board::Tile& tile{ board.getTile(row, col) };
				const TileVisual& visual{ m_tileVisuals[toIndex(row, col)] };
				m_tileModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = m_assets->getTileTexture(stageIndex, tile.m_type, tileVariant(stageIndex, row, col));

				Vector3 position{ cellToWorld(row, col) };
				position.y += TILE_LIFT + game::data::TILE_HOVER_LIFT * visual.m_hover;
				const float scale{ game::data::TILE_SIZE * game::data::TILE_TEXTURE_SCALE
					* (1.0f + visual.m_tapScale + game::data::TILE_HOVER_SCALE * visual.m_hover) };
				// 上から見て時計回り = Y 軸まわりの負の回転
				DrawModelEx(m_tileModel, position, Vector3{ 0.0f, 1.0f, 0.0f }, -visual.m_angle.m_value, Vector3{ scale, 1.0f, scale },
					RestoreShader::makeTint(WHITE, visual.m_power));
			}
		}
		rlEnableDepthMask();
	}

	void WorldRenderer::drawGlows(const game::board::Board& board) const
	{
		const float pulse{ GLOW_PULSE_BASE + GLOW_PULSE_AMOUNT * std::sin(m_time * 2.0f * PI * GLOW_PULSE_HZ) };

		// 光は重ねるほど明るくなる加算合成。奥のものを隠さないよう、深度は書き込まない
		rlDisableDepthMask();
		BeginBlendMode(BLEND_ADDITIVE);
		for (int row{}; row < game::board::Board::SIZE; ++row)
		{
			for (int col{}; col < game::board::Board::SIZE; ++col)
			{
				const game::board::Tile& tile{ board.getTile(row, col) };
				const TileVisual& visual{ m_tileVisuals[toIndex(row, col)] };
				Color color{ GLOW_COLORS[static_cast<size_t>(tile.m_type)] };
				if (color.a == 0 || visual.m_power < 0.01f)
					continue;

				const float opacity{ tile.m_type == game::board::TileType::Goal ? GLOW_OPACITY_GOAL : GLOW_OPACITY };
				color.a = static_cast<unsigned char>(255.0f * std::clamp(visual.m_power * opacity * pulse, 0.0f, 1.0f));
				Vector3 position{ cellToWorld(row, col) };
				position.y += GLOW_LIFT;
				const float size{ game::data::TILE_SIZE * GLOW_SIZE };
				DrawModelEx(m_glowModel, position, Vector3{ 0.0f, 1.0f, 0.0f }, 0.0f, Vector3{ size, 1.0f, size }, color);
			}
		}
		EndBlendMode();
		rlEnableDepthMask();
	}

	void WorldRenderer::drawProps(const game::flow::GameFlow& flow) const
	{
		/**
		 * @brief 立ち絵 1 枚分(小物・主人公)
		 */
		struct Standee
		{
			const Texture2D* m_texture{};
			Vector3 m_foot{};   // 足元の位置
			float m_height{};   // 表示する高さ(ワールド単位)
			bool m_isFlipped{}; // 左右反転するか
		};
		std::vector<Standee> standees;

		for (size_t stage{}; stage < game::data::STAGES.size(); ++stage)
		{
			const game::data::StageDefinition& definition{ game::data::STAGES[stage] };
			for (int i{}; i < definition.m_propCount; ++i)
			{
				// せり上がりの状態は update() で m_propRise に反映済み
				const float rise{ m_isShowingAllProps ? 1.0f : m_propRise[stage][i].m_value };
				if (rise < 0.01f)
					continue;
				const game::data::PropPlacement& placement{ definition.m_props[i] };
				float y{ groundHeightAt(placement.m_x, placement.m_z) };
				if (placement.m_type == game::data::PropType::Balloon)
					y += BALLOON_FLOAT + std::sin(m_time * 1.3f) * BALLOON_BOB;
				standees.push_back(Standee{ &m_assets->getPropTexture(placement.m_type), Vector3{ placement.m_x, y, placement.m_z },
					placement.m_height * rise, placement.m_isFlipped });
			}
		}

		// 主人公: ポーズごとのコマを時間で切り替える。全ポーズを待機の 1 コマ目と同じ倍率で表示する
		const game::hero::HeroState& hero{ flow.getHero() };
		const int frameCount{ m_assets->getHeroFrameCount(hero.getPose()) };
		const Texture2D& reference{ m_assets->getHeroFrame(game::hero::HeroPose::Idle, 0) };
		if (frameCount > 0 && reference.height > 0)
		{
			const int frame{ static_cast<int>(hero.getPoseTime() * heroFps(hero.getPose())) % frameCount };
			const Texture2D& texture{ m_assets->getHeroFrame(hero.getPose(), frame) };
			const float worldPerPixel{ game::data::HERO_IDLE_HEIGHT / reference.height };
			standees.push_back(Standee{ &texture, Vector3{ hero.getX(), groundHeightAt(hero.getX(), hero.getZ()), hero.getZ() },
				texture.height * worldPerPixel, false });
		}

		// 半透明の縁が正しく重なるよう、奥(Z が小さい)から描く
		std::sort(standees.begin(), standees.end(), [](const Standee& a, const Standee& b) { return a.m_foot.z < b.m_foot.z; });

		const float tilt{ BILLBOARD_TILT_DEG * DEG2RAD };
		const Vector3 up{ 0.0f, std::cos(tilt), -std::sin(tilt) };
		for (const Standee& standee : standees)
		{
			const Texture2D& texture{ *standee.m_texture };
			if (texture.id == 0 || standee.m_height <= 0.0f)
				continue;

			const float width{ standee.m_height * texture.width / static_cast<float>(texture.height) };
			Rectangle source{ 0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height) };
			if (standee.m_isFlipped)
			{
				// 左右反転: 右端から左へ読む(幅だけ負にすると画像の外を読んでしまう)
				source.x = source.width;
				source.width = -source.width;
			}
			// 足元(画像の下辺中央)を配置位置に合わせる
			DrawBillboardPro(m_camera, texture, source, standee.m_foot, up, Vector2{ width, standee.m_height }, Vector2{ width / 2.0f, 0.0f }, 0.0f, WHITE);
		}
	}
} // namespace infrastructure::render
