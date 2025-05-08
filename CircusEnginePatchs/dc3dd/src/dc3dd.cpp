#include <iostream>
#include <dc3dd.hpp>

namespace DC3DD
{
    Utils::FontManager DC3DD::FontManager{};

    auto CALLBACK DC3DD::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
    {

        if (uMsg == WM_CREATE)
        {
            ::SetWindowTextW(hWnd, DC3DD::TitleName);
            HMENU system_menu{ ::GetSystemMenu(hWnd, FALSE) };
            if (system_menu != nullptr)
            {
                ::AppendMenuW(system_menu, MF_UNCHECKED, 0x114514, L"更改字体");
            }

            if (!DC3DD::FontManager.IsInit())
            {
                DC3DD::FontManager.Init(hWnd);
            }
        }
        else if (uMsg == WM_SYSCOMMAND)
        {
            if (wParam == 0x114514)
            {
                if (DC3DD::FontManager.GUI() != nullptr)
                {
                    DC3DD::FontManager.GUIChooseFont();
                }
            }
        }
        return { Patch::Hooker::Call<DC3DD::WndProc>(hWnd, uMsg, wParam, lParam) };
    }
}
