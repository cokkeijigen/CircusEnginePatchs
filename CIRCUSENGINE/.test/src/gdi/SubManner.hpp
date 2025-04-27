#pragma once
#include <vector>
#include <span>
#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>

namespace Sub
{
    



    class ImageSub
    {
    public:
        HBITMAP m_Bitmap{};
        HDC     m_MemDC {};
        LONG m_Width {};
        LONG m_Height{};

        ImageSub(std::wstring_view path, HDC memDC = ::CreateCompatibleDC(NULL)) noexcept;

        auto Draw(HDC destHDC, POINT pos, uint8_t alpha) const noexcept -> bool;
    };
}

namespace XSub {

    enum Align: uint8_t
    {
        Left   = static_cast<uint8_t>(1 << 0),
        Right  = static_cast<uint8_t>(1 << 1),
        Top    = static_cast<uint8_t>(1 << 2),
        Bottom = static_cast<uint8_t>(1 << 3),
        Center = static_cast<uint8_t>(1 << 4),
        Middle = static_cast<uint8_t>(1 << 5),
    };

    inline Align operator|(Align a, Align b)
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
    struct ImageEntry
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

    struct Header
    {
        uint8_t magic[4]{};
        uint8_t type [4]{};
        uint16_t width {};
        uint16_t height{};
        uint32_t size{};
    };

    namespace GDI
    {
        class ImageSub
        {
        public:
            std::span<uint8_t>       m_RawEntries{};
            std::vector<ImageEntry*> m_SubEntries{};

            Header* m_Header{};
            HBITMAP m_Bitmap{};
            HDC     m_MemDC{};

            ImageSub(std::wstring_view path, HDC memDC = ::CreateCompatibleDC(NULL));
            auto Draw(float time, HDC dest) noexcept -> bool;
        };
    }
}
