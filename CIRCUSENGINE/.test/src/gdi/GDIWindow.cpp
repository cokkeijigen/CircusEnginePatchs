#include <iostream>
#include <GDIWindow.hpp>
#include <thread>
namespace GDI {

    auto GDI::GdiplusStartup(void) -> void
    {
        static std::unique_ptr<GDI::Window::GdiplusStartup> GdiplusStartup{ nullptr };
        if (GdiplusStartup == nullptr)
        {
            GdiplusStartup = std::make_unique<GDI::Window::GdiplusStartup>();
        }
    }

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
            auto&& reuslt{ m_this->OnMessage(hWnd, uMsg, wParam, lParam) };
            if (reuslt.has_value()) { return LRESULT{ reuslt.value() }; }
        }
        
        return LRESULT{ ::DefWindowProcW(hWnd, uMsg, wParam, lParam) };
    }

    auto Window::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept -> std::optional<LRESULT>
    {
        if (uMsg == WM_CREATE)
        {
            this->SyncToParentWindow(true);
        }
        return { std::nullopt };
    }

    Window::Window(HWND parent, HINSTANCE hInstance) noexcept: m_ScreenDC(::GetDC(NULL)), m_Parent(parent)
    {
        GDI::GdiplusStartup();
        if (++Window::INSTANCE_COUNT == 0x01)
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
        
        auto hWnd = HWND
        {
            ::CreateWindowExW
            (
                // 分层窗口 | 鼠标穿透 | 隐藏任务栏图标
                { WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE },
                { reinterpret_cast<LPCWSTR>(Window::ClassName) },
                { reinterpret_cast<LPCWSTR>(L"GDIWindow") },
                { WS_CHILD | WS_VISIBLE | WS_POPUP },
                { 0x00000000 },
                { 0x00000000 },
                { 0x00000000 },
                { 0x00000000 },
                { parent     },
                { nullptr    },
                { hInstance  },
                { this       }
            )
        };

        if (this->m_That == nullptr)
        {
            this->m_That = { hWnd };
        }

        if (parent != nullptr)
        {
            ::SetForegroundWindow(parent);
        }
    }

    Window::~Window() noexcept
    {
        if (--Window::INSTANCE_COUNT == 0x00)
        {
            ::UnregisterClassW
            (
                { Window::ClassName },
                { ::GetModuleHandleW(NULL) }
            );
        }
    }

    Window::Window(Window&& other) noexcept
    {
        this->operator=(std::forward<Window&&>(other));
    }
    
    Window& Window::operator=(Window&& other) noexcept
    {
        ++Window::INSTANCE_COUNT;
        auto _this { reinterpret_cast<uint8_t*>(this  ) };
        auto _other{ reinterpret_cast<uint8_t*>(&other) };
        for (size_t i = 0; i < sizeof(Window); i++)
        {
            _this [i] = { _other[i] };
            _other[i] = { 0x00 };
        }
        if (this->m_That != nullptr)
        {
            ::SetWindowLongPtrW
            (
                { this->m_That   },
                { GWLP_USERDATA  },
                { intptr_t(this) }
            );
        }
        return { *this };
    }

    auto Window::Init(LONG width, LONG height, LONG x, LONG y) noexcept -> void
    {
        this->SetPosition(x, y);
        this->SetSize(width, height);
        this->UpdateLayerBitmap(true);
    }

    auto Window::IsWindowVisible() const noexcept -> bool
    {
        if (this->m_That != nullptr)
        {
            return { static_cast<bool>(::IsWindowVisible(this->m_That))};
        }
        return { false };
    }

    auto Window::SetRect(const RECT& rect, bool update) noexcept -> bool
    {
        this->m_Size = SIZE
        {
            .cx = { rect.right - rect.left },
            .cy = { rect.bottom - rect.top }
        };
        this->m_Point = POINT
        {
            .x = { rect.left },
            .y = { rect.top  }
        };
        return bool
        {
            update ?
            this->UpdateLayer() : true
        };
    }

    auto Window::SetRect(RECT&& rect, bool update) noexcept -> bool
    {
        return { this->SetRect(rect, update) };
    }

    auto Window::SetSize(LONG width, LONG height, bool update) noexcept -> bool
    {
        this->m_Size = SIZE{ width, height };
        return bool
        {
            update ?
            this->UpdateLayer() : true
        };
    }

    auto Window::SetPosition(LONG x, LONG y, bool update) noexcept -> bool
    {
        this->m_Point = POINT{ x, y };
        return bool
        {
            update ?
            this->UpdateLayer() : true
        };
    }

    auto Window::Show() const noexcept -> bool
    {
        if (this->m_That != nullptr)
        {
            auto result{ ::ShowWindow(this->m_That, SW_SHOW) };
            return { static_cast<bool>(result) };
        }
    }

    auto Window::Hide() const noexcept -> bool
    {
        if (this->m_That != nullptr)
        {
            auto result{ ::ShowWindow(this->m_That, SW_HIDE) };
            return { static_cast<bool>(result) };
        }
        return { false };
    }

    auto Window::SyncToParentWindow(bool update_layer_bitmap, bool force) noexcept -> bool
    {
        RECT rect{};
        auto get_client_rect = BOOL
        {
            ::GetClientRect(this->m_Parent, &rect)
        };
        if (static_cast<bool>(get_client_rect))
        {
            this->m_Size = SIZE
            {
                { rect.right - rect.left },
                { rect.bottom - rect.top }
            };
        }
        else
        {
            return { false };
        }
        auto get_window_rect = BOOL
        {
            ::GetWindowRect(this->m_Parent, &rect)
        };
        if (static_cast<bool>(get_client_rect))
        {
            const int& width { this->m_Size.cx };
            const int& height{ this->m_Size.cy };
            static const auto captionHeight = int
            {
                ::GetSystemMetrics(SM_CYCAPTION)
            };
            this->m_Point = POINT
            {
                .x = int{ ((rect.left + rect.right) / 2) - (width / 2) },
                .y = int
                {
                    (((rect.top + rect.bottom) / 2) - (height / 2)) +
                    ((captionHeight + 1) / 2)
                }
            };
            if (update_layer_bitmap)
            {
                return { this->UpdateLayerBitmap(force) };
            }
            return { true };
        }
        return { false };
    }

    auto Window::UpdateLayerBitmap(bool force) noexcept -> bool
    {
        if (this->m_MemDC == nullptr)
        {
            this->m_MemDC = { ::CreateCompatibleDC(NULL) };
            if(this->m_MemDC == nullptr) return { false };
        }

        if (!force && this->m_Bitmap != nullptr)
        {
            BITMAP bitmap{};
            auto get_object_result = int
            {
                ::GetObjectW(this->m_Bitmap, sizeof(BITMAP), &bitmap)
            };
            if (static_cast<bool>(get_object_result))
            {
                bool no_update_required
                {
                    bitmap.bmWidth  >= this->m_Size.cx &&
                    bitmap.bmHeight >= this->m_Size.cy
                };
                if (no_update_required)
                {
                    return { true };
                }
            }
        }

        auto info = BITMAPINFO
        {
            .bmiHeader
            {
                .biSize{ sizeof(BITMAPINFO::bmiHeader) },
                .biWidth { 2560 },
                .biHeight{ 1440 },
                /*.biWidth { this->m_Size.cx },
                .biHeight{ this->m_Size.cy },*/
                .biPlanes{ 1 },
                .biBitCount{ 32 },
                .biCompression{ BI_RGB },
                .biSizeImage{ 0 }
            }
        };

        void* bytes{ nullptr };
        auto bmp = HBITMAP
        {
            ::CreateDIBSection
            (
                { NULL  },
                { &info },
                { DIB_RGB_COLORS },
                { &bytes },
                { NULL },
                { 0 }
            )
        };

        if (bmp == nullptr)
        {
            return { false };
        }

        ::SelectObject(this->m_MemDC, bmp);
        if (this->m_Bitmap != nullptr)
        {
            ::DeleteObject(this->m_Bitmap);
        }
        this->m_Bitmap = { bmp };
        return { true };
    }

    auto Window::UpdateLayer() const noexcept -> bool
    {
        if (this->m_That == nullptr)
        {
            return { false };
        }

        if (this->m_MemDC == nullptr)
        {
            return { false };
        }

        POINT pos{};
        auto is_update = BOOL
        {
            ::UpdateLayeredWindow
            (
                { this->m_That },
                { NULL },
                { &this->m_Point },
                { &this->m_Size  },
                { this->m_MemDC  },
                { &pos },
                { NULL },
                { &this->m_Blend },
                { ULW_ALPHA }
            )
        };
        return { static_cast<bool>(is_update) };
    }
    
}
