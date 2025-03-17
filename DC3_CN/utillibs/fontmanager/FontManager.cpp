#include <iostream>
#include <FontManager.hpp>
#include <console.hpp>

namespace Utils {

    FontManager::FontManager(HWND hWnd)
    {
    }

    auto FontManager::Init(HWND hWnd) -> FontManager&
    {
        this->m_GUI = FontManagerGUI::CreatePtr(hWnd);
        this->m_GUI->Init(22, Utils::FontManagerGUI::NORMAL, L"黑体", 18, 30)
            .Load(".\\cn_Data\\chs_font.dat")
            .OnChanged([&](const Utils::FontManagerGUI* m_this) -> void
            {
                for (auto& [key, font] : this->m_Fonts)
                {
                    auto flag{ static_cast<int32_t>(key & 0xFF000000) };
                    auto size{ static_cast<int32_t>(key & 0x00FFFFFF) };
                    auto base{ static_cast<int32_t>(size - 22) };

                    auto charset{ flag == (0x10 << 24) ? this->UseCharSet : 0x81 };
                    HFONT nFont { m_this->MakeFont(charset, base) };
                    HFONT oFont { font };

                    font = nFont;
                    if (oFont != nullptr)
                    {
                        ::DeleteObject(oFont);
                    }
                }
            });
        return *this;
    }

    auto FontManager::GetGBKFont(int32_t size) -> HFONT
    {
        int32_t key{ size | (0x10 << 24) };
        HFONT font { this->m_Fonts[key] };
        if (nullptr == font)
        {
            auto base{ static_cast<int32_t>(size - 22) };
            this->m_Fonts[key] = this->m_GUI->MakeFont(this->UseCharSet, base);
        }
        return this->m_Fonts[key];
    }

    auto FontManager::GetJISFont(int32_t size) -> HFONT
    {
        int32_t key{ size | (0x20 << 24) };
        HFONT font{ this->m_Fonts[key] };
        if (nullptr == font)
        {
            auto base{ static_cast<int32_t>(size - 22) };
            this->m_Fonts[key] = this->m_GUI->MakeFont(0x81, base);
        }
        return this->m_Fonts[key];
    }

    auto FontManager::GUI() -> FontManagerGUI*
    {
        return this->m_GUI.get();
    }


    auto FontManager::GUIUpdateDisplayState() -> FontManager&
    {
        this->m_GUI->UpdateDisplayState();
        return *this;
    }

    auto FontManager::GUIChooseFont() -> FontManager&
    {
        this->m_GUI->ChooseFont();
        return *this;
    }
}

