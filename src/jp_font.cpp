#include "jp_font.h"

#include <algorithm>
#include <vector>

namespace {

void AddRange(std::vector<int>& codepoints, int first, int last) {
    for (int c = first; c <= last; ++c) codepoints.push_back(c);
}

}  // namespace

Font LoadJapaneseFont(const char* path, int fontSize, const char* usedText) {
    std::vector<int> codepoints;
    AddRange(codepoints, 0x0020, 0x007E);  // ASCII
    AddRange(codepoints, 0x3000, 0x303F);  // 全角スペース・句読点・括弧(「」『』【】など)
    AddRange(codepoints, 0x3041, 0x3096);  // ひらがな
    AddRange(codepoints, 0x30A0, 0x30FF);  // カタカナ(長音符「ー」・中黒「・」含む)
    AddRange(codepoints, 0xFF01, 0xFF5E);  // 全角英数・記号(！？（）など)

    // 表示する文章から文字を拾う(漢字はここで入る)。UTF-8 → コードポイント配列に変換される。
    int textCount = 0;
    int* textCodepoints = LoadCodepoints(usedText, &textCount);
    codepoints.insert(codepoints.end(), textCodepoints, textCodepoints + textCount);
    UnloadCodepoints(textCodepoints);

    // 重複を除く(アトラスが無駄に大きくなるのを防ぐ)
    std::sort(codepoints.begin(), codepoints.end());
    codepoints.erase(std::unique(codepoints.begin(), codepoints.end()), codepoints.end());
    codepoints.erase(std::remove_if(codepoints.begin(), codepoints.end(),
                                    [](int c) { return c < 0x20; }),  // 改行などの制御文字は不要
                     codepoints.end());

    Font font = LoadFontEx(path, fontSize, codepoints.data(), static_cast<int>(codepoints.size()));
    // 拡大縮小して描いたときにジャギーが目立たないようにする
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    return font;
}
