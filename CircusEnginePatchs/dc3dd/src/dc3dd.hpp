#include <windows.h>
#include <dsound.h>
#include <patch.hpp>
#include <fontmanager.hpp>
#include <imgsub_gdi.hpp>

namespace DC3DD
{
    extern auto CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM) -> LRESULT;

    extern auto ReplacePathA(std::string_view path) -> std::string_view;

    extern Utils::FontManager FontManager;

    static constexpr inline wchar_t TitleName[]
    {
        L"【COKEZIGE STUDIO】Da Capo Ⅲ Dream Days - CHS Beta.0.5"
        L" ※仅供学习交流使用，禁止一切直播录播和商用行为※"
    };
}
