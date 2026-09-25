#pragma once

// 画面に出す文章はここにまとめる(ステージ名とヒント文は game/data/Stages.h)。
// フォント読み込み時にここと Stages.h の文字を集めてアトラスに焼き込むため、
// ここに無い漢字を描くと「?」になる(infrastructure::resource::Assets 参照)。
namespace infrastructure::ui
{
	constexpr const char* TEXT_TITLE{ "かんせいのパレット" };
	constexpr const char* TEXT_SUBTITLE{ "KANSEI PALETTE" };
	constexpr const char* TEXT_CAPTION{ "せかいの いろを うしなった ひとの ものがたり" };
	constexpr const char* TEXT_START{ "はじめる" };
	constexpr const char* TEXT_ENDING_SUBTITLE{ "ぜんぶ、いろづいた。" };
	constexpr const char* TEXT_ENDING_CAPTION{ "主人公は、はじめて世界の色を見た。" };
	constexpr const char* TEXT_RETRY{ "もういちど あそぶ" };
	constexpr const char* TEXT_NEXT{ "つぎへ" };
	constexpr const char* TEXT_RESET{ "やりなおす (R)" };
	constexpr const char* TEXT_SOUND_ON{ "音 ON" };
	constexpr const char* TEXT_SOUND_OFF{ "音 OFF" };
	constexpr const char* TEXT_GOAL{ "ゴール" };
	constexpr const char* TEXT_COLOR_RETURNED{ "が もどった" };
	constexpr const char* TEXT_COLORS_LEFT{ "あと %d つの いろ" };

	/// フォントに焼き込む文字をまとめたもの(上の文章をすべて並べる)
	constexpr const char* TEXT_ALL_UI{
		"かんせいのパレット KANSEI PALETTE せかいの いろを うしなった ひとの ものがたり はじめる "
		"ぜんぶ、いろづいた。主人公は、はじめて世界の色を見た。もういちど あそぶ つぎへ やりなおす (R) "
		"音 ON OFF ゴール が もどった あと つの いろ STAGE 0123456789 /"
	};
} // namespace infrastructure::ui
