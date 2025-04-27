#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <optional>
#include <atomic>

namespace GDI {

    extern auto GdiplusStartup(void) -> void;

    class Window
    {
        static auto inline INSTANCE_COUNT{ std::atomic<size_t>(0) };
        static auto CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
    protected:
        
    public:

        class GdiplusStartup
        {
            Gdiplus::GdiplusStartupInput gdiplusStartupInput{};
            ULONG_PTR gdiplusToken{};
        public:

            inline GdiplusStartup()
            {
                Gdiplus::GdiplusStartup
                (
                    &gdiplusToken,
                    &gdiplusStartupInput,
                    NULL
                );
            }
            inline ~GdiplusStartup()
            {
                Gdiplus::GdiplusShutdown
                (
                    gdiplusToken
                );
            }
        };

        HWND m_Parent{};
        HWND m_That{};
        HDC m_MemDC{};
        HDC m_ScreenDC{};
        HBITMAP m_Bitmap{};

        SIZE  mutable m_Size{};
        POINT mutable m_Point{};
        BLENDFUNCTION mutable m_Blend
        {
            .BlendFlags = AC_SRC_OVER,
            .SourceConstantAlpha = 255,
            .AlphaFormat = AC_SRC_ALPHA,
        };

        auto OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept -> std::optional<LRESULT>;

        static inline const wchar_t ClassName[]
        {
            L"GDI_Window_Clazz"
        };

        ~Window() noexcept;
        Window(const Window&) = delete;
        Window(Window&& other) noexcept;

        Window(HWND parent, HINSTANCE hInstance = ::GetModuleHandleW(NULL)) noexcept;

        auto operator=(Window&& other) noexcept -> Window&;
        auto operator=(const Window& ) noexcept -> Window& = delete;

        auto IsWindowVisible() const noexcept -> bool;

        auto Init(LONG width, LONG height, LONG x = 0, LONG y = 0) noexcept -> void;

        auto SetRect(const RECT& rect, bool update = false) noexcept -> bool;

        auto SetRect(RECT&& rect, bool update = false) noexcept -> bool;

        auto SetSize(LONG width, LONG height, bool update = false) noexcept -> bool;

        auto SetPosition(LONG x, LONG y, bool update = false) noexcept -> bool;

        auto SyncToParentWindow(bool update_layer_bitmap = false, bool force = false) noexcept -> bool;

        auto UpdateLayerBitmap (bool force  = false) noexcept -> bool;

        auto UpdateLayer() const noexcept -> bool;

        auto Show() const noexcept -> bool;

        auto Hide() const noexcept -> bool;
    };
}
