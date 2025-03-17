#pragma once
#include <FontManagerGUI.hpp>
#include <unordered_map>

namespace Utils {

    class FontManager
    {
       
        std::unique_ptr<FontManagerGUI> m_GUI;
        std::unordered_map<int32_t, HFONT> m_Fonts{};

    public:

        const inline static int UseCharSet
        {
            ::GetACP() == 936 ? ANSI_CHARSET : 0x86
        };

        FontManager() : FontManager(NULL) {}

        FontManager(HWND hWnd);

        auto Init(HWND hWnd) -> FontManager&;

        auto GetGBKFont(int32_t size) -> HFONT;

        auto GetJISFont(int32_t size) -> HFONT;

        auto GUI() -> FontManagerGUI*;

        auto GUIUpdateDisplayState() -> FontManager&;

        auto GUIChooseFont() -> FontManager&;
    };
}
