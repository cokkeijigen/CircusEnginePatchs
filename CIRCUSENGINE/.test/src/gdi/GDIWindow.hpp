#pragma once
#include <windows.h>
#include <optional>
#include <atomic>


namespace GDI {

    class Window
    {
        static inline auto INSTANCE_COUNT{ std::atomic<size_t>(0) };
        static auto CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
    protected:
        
        HWND m_Parent{};
        HWND m_That{};

        auto OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> std::optional<LRESULT>;
    public:

        static inline wchar_t ClassName[]
        {
            L"GDI_Window_Clazz"
        };

        ~Window() noexcept;
        Window(HWND parent, HINSTANCE hInstance = ::GetModuleHandleW(NULL)) noexcept;
    };
}
