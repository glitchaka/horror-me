#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace horror {
// Bitmap atlas baked from the supplied Annapurna SIL Regular and Bold TTFs.
// Only rasterized uppercase glyph pixels are stored, not distributable font files.
class FontAtlas final {
public:
    static const FontAtlas& instance();
    [[nodiscard]] int glyphWidth(char letter,bool bold) const;
    [[nodiscard]] int textWidth(std::string_view text,bool bold) const;
    [[nodiscard]] int height(bool bold) const {return bold?21:14;}
    [[nodiscard]] int alpha(char letter,bool bold,int x,int y) const;
private:
    FontAtlas();
    struct Face {
        std::array<int,64> widths{};
        std::array<std::size_t,64> offsets{};
        std::vector<std::uint8_t> pixels;
    };
    Face regular_,bold_;
    static char normalize(char c);
    static std::vector<std::uint8_t> unpack(std::string_view encoded);
};
