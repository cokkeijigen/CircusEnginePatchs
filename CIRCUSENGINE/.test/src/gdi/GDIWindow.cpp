#include <iostream>
#include <GDIWindow.hpp>

namespace GDI {

    auto CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        auto m_this{ static_cast<GDI::Window*>(nullptr) };

        if (uMsg == WM_NCCREATE)
        {
            auto pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
            m_this   = reinterpret_cast<GDI::Window*>(pcs->lpCreateParams);
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, uintptr_t(m_this));
            if (m_this != nullptr) { m_this->m_That = HWND{ hWnd }; }
        }
        else
        {
            m_this = reinterpret_cast<GDI::Window*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }

        if (m_this != nullptr)
        {
            auto reuslt{ m_this->OnMessage(hWnd, uMsg, wParam, lParam) };
            if (reuslt.has_value()) { return LRESULT{ reuslt.value() }; }
        }
        
        return LRESULT{ ::DefWindowProcW(hWnd, uMsg, wParam, lParam) };
    }

    auto Window::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> std::optional<LRESULT>
    {
        return { std::nullopt };
    }

    Window::Window(HWND parent, HINSTANCE hInstance) noexcept
    {
        if (Window::INSTANCE_COUNT == 0)
        {
            WNDCLASSEXW wndclass
            {
              .cbSize{ sizeof(WNDCLASSEXW) },
              .lpfnWndProc{ Window::WndProc },
              .hInstance  { hInstance },
              .lpszClassName{ Window::ClassName },
            };
            ::RegisterClassExW(&wndclass);
        }




        Window::INSTANCE_COUNT++;
    }

    Window::~Window() noexcept
    {
        if (Window::INSTANCE_COUNT == 1)
        {
            ::UnregisterClassW(Window::ClassName, ::GetModuleHandleW(NULL));
        }

        Window::INSTANCE_COUNT--;
        
    }
}
