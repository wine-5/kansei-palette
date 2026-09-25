#pragma once

#include "raylib.h"

// 日本語フォントを読み込む。
//
// raylib は DxLib と違い、フォント読み込み時に「使う文字」を指定し、その文字だけを
// テクスチャ(アトラス)に焼き込む。指定していない文字は描画すると「?」になる。
//
// 読み込む文字 = ASCII + 全角記号 + ひらがな + カタカナ + usedText に含まれる全文字
// 漢字は数が多いので、ゲーム中で表示する文章をまとめて usedText に渡すこと。
//
// fontSize はアトラスに焼き込むピクセルサイズ。DrawTextEx でこれより大きく描くとぼやけるので、
// 一番大きく表示するサイズで読み込み、小さい文字は縮小して描くのがおすすめ。
Font LoadJapaneseFont(const char* path, int fontSize, const char* usedText);
