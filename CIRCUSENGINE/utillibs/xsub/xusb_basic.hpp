#pragma once

namespace XSub
{
    struct XsubHeader
    {
        uint8_t magic[4]{}; // { 'x', 's', 'u', 'b' }
        uint8_t  type[4]{};
        uint16_t width{};
        uint16_t height{};
        uint32_t size{};
    };

    enum Align : uint8_t
    {
        Left   = static_cast<uint8_t>(1 << 0),
        Right  = static_cast<uint8_t>(1 << 1),
        Top    = static_cast<uint8_t>(1 << 2),
        Bottom = static_cast<uint8_t>(1 << 3),
        Center = static_cast<uint8_t>(1 << 4),
        Middle = static_cast<uint8_t>(1 << 5),
    };

    inline auto operator|(Align a, Align b) -> Align
    {
        return Align
        (
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
        );
    }

    #pragma pack(push, 1)
    struct Point
    {
        Align align;
        uint16_t vertical;
        uint16_t horizontal;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct ImageSubEntry
    {
        float start;
        float end;
        uint16_t count;
        struct Entry
        {
            Point point;
            float fadein;
            float fadeout;
            int32_t x;
            int32_t y;
            int32_t width;
            int32_t height;
        } entries[NULL];
    };
    #pragma pack(pop)
}
