#pragma once
#include <FontManagerGUI.hpp>
#include <unordered_map>

namespace Utils {

    class FontManager
    {
       
        std::unique_ptr<FontManagerGUI> m_GUI;
        std::unordered_map<int32_t, HFONT> m_Fonts{};

    public:

        const inline static int DefaultSize{ 22 };

        const inline static int UseCharSet
        {
            ::GetACP() == 936 ? ANSI_CHARSET : 0x86
        };
        
        inline FontManager(){}

        inline FontManager(HWND hWnd)
        {
            this->Init(hWnd);
        };

        auto Init(HWND hWnd) -> FontManager&;

        auto GetGBKFont(int32_t size) -> HFONT;

        auto GetJISFont(int32_t size) -> HFONT;

        auto GUI() -> FontManagerGUI*;

        auto GUIUpdateDisplayState() -> FontManager&;

        auto GUIChooseFont() -> FontManager&;
    };
}
