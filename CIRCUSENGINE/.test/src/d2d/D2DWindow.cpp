#include <iostream>
#include <D2DWindow.hpp>

namespace GUI {

    auto D2DWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        auto m_this{ static_cast<D2DWindow*>(nullptr) };

        if (uMsg == WM_NCCREATE)
        {
            auto pcs = reinterpret_cast<CREATESTRUCT*>(lParam) ;
            m_this   = reinterpret_cast<D2DWindow*>(pcs->lpCreateParams);
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, uintptr_t(m_this));
            m_this->m_hWnd = hWnd;
        }
        else
        {
            m_this = reinterpret_cast<D2DWindow*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }

        if (m_this != nullptr)
        {
            auto reuslt{ m_this->OnMessageHandle(hWnd, uMsg, wParam, lParam) };
            if (reuslt.has_value())
            {
                return reuslt.value();
            }
        }
        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    auto D2DWindow::OnMessageHandle(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept -> std::optional<LRESULT>
    {
        return { std::nullopt };
    }

    D2DWindow::D2DWindow(HWND parent, HINSTANCE hInstance) noexcept
    {
        WNDCLASSEXW d2dwndclass
        {
          .cbSize = sizeof(WNDCLASSEXW),
          .lpfnWndProc = D2DWindow::WndProc,
          .hInstance = hInstance,
          .lpszClassName = L"D2DWND_CLAZZ",
        };
        ::RegisterClassExW(&d2dwndclass);

        ::CreateWindowExW
        (   // 分层窗口 | 鼠标穿透 | 隐藏任务栏图标
            { WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE },
            L"D2DWND_CLAZZ", L"Direct2D_GDI", WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
            nullptr, nullptr, hInstance, nullptr
        );
    }

    auto D2DWindow::UpdateLayer() const -> void
    {

    }

}
