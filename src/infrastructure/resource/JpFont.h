#pragma once
#include "raylib.h"

namespace infrastructure::resource
{
	/**
	 * @brief 日本語フォントを読み込む
	 * @details raylib は DxLib と違い、フォント読み込み時に「使う文字」を指定し、その文字だけを
	 *          テクスチャ(アトラス)に焼き込む。指定していない文字は描画すると「?」になる。
	 *          読み込む文字 = ASCII + 全角記号 + ひらがな + カタカナ + usedText に含まれる全文字。
	 *          漢字は数が多いので、ゲーム中で表示する文章をまとめて usedText に渡すこと。
	 *          fontSize より大きく描くとぼやけるので、一番大きく表示するサイズで読み込む。
	 *          InitWindow() の後に呼ぶこと。
	 * @param path フォントファイルのパス
	 * @param fontSize アトラスに焼き込むピクセルサイズ
	 * @param usedText 画面に表示する文章(UTF-8)。ここに含まれる漢字が読み込まれる
	 * @return 読み込んだフォント。不要になったら UnloadFont() で解放する
	 */
	Font loadJapaneseFont(const char* path, int fontSize, const char* usedText);
} // namespace infrastructure::resource
