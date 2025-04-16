#pragma once
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <d2d1.h>
#include <optional>
#pragma comment(lib, "d2d1.lib")

namespace GUI {

    class D2DWindow
    {
        static auto CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;

        auto OnMessageHandle(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept -> std::optional<LRESULT>;

    protected:

        HWND m_hWnd{};
        ID2D1Factory* m_pID2D1Factory{};
        ID2D1DCRenderTarget* m_pRenderTarget{};
    public:

        D2DWindow(HWND parent, HINSTANCE hInstance = ::GetModuleHandleW(NULL)) noexcept;

        auto UpdateLayer() const -> void;
        auto BeginDraw() const -> void;
        auto EndDraw  () const -> void;

        auto Show() const -> void;
        auto Hide() const -> void;
    };
}
