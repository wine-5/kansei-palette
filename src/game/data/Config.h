#pragma once

// 調整用の数値はすべてここに集める。
// 値はプロトタイプの実装値。実際に見て微調整する。
namespace game::data
{
	// --- 画面 ---
	constexpr int SCREEN_WIDTH{ 1280 }; // 基準の解像度
	constexpr int SCREEN_HEIGHT{ 720 };

	// --- カメラ ---
	constexpr float CAMERA_FOVY_DEG{ 32.0f };  // 縦の視野角
	constexpr float CAMERA_PITCH_DEG{ 50.0f }; // 俯角
	constexpr float CAMERA_TARGET_X{ 0.0f };   // 注視点
	constexpr float CAMERA_TARGET_Y{ 0.5f };
	constexpr float CAMERA_TARGET_Z{ -0.2f };
	constexpr float VIEW_WIDTH_MIN{ 8.4f };    // 見せる横幅(ワールド単位)の下限・上限。縦長の画面でも盤面の左の主人公まで入る幅
	constexpr float VIEW_WIDTH_MAX{ 9.6f };
	constexpr float VIEW_HEIGHT{ 8.6f };       // 見せる縦幅
	constexpr float PARALLAX_X{ 0.55f };       // ポインター位置による視差の最大量
	constexpr float PARALLAX_Y{ 0.25f };

	// --- タイル ---
	constexpr float TILE_SIZE{ 1.0f };           // 1 マスの大きさ
	constexpr float TILE_TEXTURE_SCALE{ 1.16f };  // 接点が板の外に出るため、画像はマスより大きく貼る(tools/slice_sheet.py の TILE_PLATE_SCALE と同じ)
	constexpr float TILE_SPRING_STIFFNESS{ 260.0f };
	constexpr float TILE_SPRING_DAMPING{ 23.0f };
	constexpr float TILE_TAP_SCALE{ 0.07f };      // タップ時に一瞬大きくなる割合
	constexpr float TILE_TAP_DECAY{ 9.0f };
	constexpr float TILE_HOVER_LIFT{ 0.05f };
	constexpr float TILE_HOVER_SCALE{ 0.045f };

	// --- 通電 ---
	constexpr float POWER_DELAY_PER_STEP{ 0.085f }; // 電源からの距離 1 あたりの色づきの遅れ(秒)
	constexpr float POWER_RISE_RATE{ 9.0f };
	constexpr float POWER_FALL_RATE{ 16.0f };

	// --- 色の復元 ---
	constexpr float RESTORE_RATE{ 1.7f }; // 復元度が目標へ近づく速さ(毎秒)

	// --- クリア演出のタイムライン(秒) ---
	constexpr float CLEAR_RESTORE_START{ 0.45f }; // 色の復元と小物のせり上がりを始める
	constexpr float CLEAR_PROP_INTERVAL{ 0.12f }; // 小物が順にせり上がる間隔
	constexpr float CLEAR_CARD_TIME{ 2.7f };      // クリアカード(最終ステージはエンディングへ)
	constexpr float ENDING_DELAY{ 0.5f };         // 最終ステージのフラッシュからエンディングまで
	constexpr float PROP_SPRING_STIFFNESS{ 190.0f };
	constexpr float PROP_SPRING_DAMPING{ 13.0f };

	// --- 主人公 ---
	constexpr float HERO_IDLE_THINK_TIME{ 14.0f };   // 操作がないと考え込むまでの秒数
	constexpr float HERO_THINK_REPEAT_TIME{ 6.0f };  // 考え込んだ後、再び考え込むまでの秒数
	constexpr float HERO_THINK_DURATION{ 3.2f };
	constexpr float HERO_SWEAT_DURATION{ 1.1f };
	constexpr float HERO_DIZZY_DURATION{ 0.9f };
	constexpr float HERO_RUN_SPEED{ 5.4f };          // 入場時の移動速度(毎秒)
	constexpr float HERO_WALK_SPEED{ 2.6f };         // つながった道を歩く速さ(毎秒)
	constexpr float HERO_RUN_START_X{ -9.0f };       // 入場の走り始め
	constexpr float HERO_TITLE_X{ -3.9f };           // タイトル・エンディングでの立ち位置(盤面の左)
	constexpr float HERO_TITLE_Z{ 1.4f };
	constexpr float HERO_IDLE_HEIGHT{ 1.34f };       // 待機ポーズの高さ(ワールド単位)。全ポーズを同じ倍率で表示する

	// --- 星くず ---
	constexpr int PARTICLE_MAX{ 700 }; // Web で重ければ 400 に減らす
	constexpr float PARTICLE_GRAVITY{ 5.2f };
	constexpr float PARTICLE_BOUNCE{ 0.35f };
} // namespace game::data
