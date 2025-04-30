#include <iostream>
#include <subplayer_gdi.hpp>

namespace XSub::GDI
{

    auto PlayerWindow::AutoGdiplusStartup(void) -> void
    {
        static std::unique_ptr<PlayerWindow::GdiplusStartup> GdiplusStartup{ nullptr };
        if (GdiplusStartup == nullptr)
        {
            GdiplusStartup = std::make_unique<PlayerWindow::GdiplusStartup>();
        }
    }

    auto PlayerWindow::SafeCheckInstanceCount(bool add, std::function<void(size_t)> callback) -> void
    {
        static auto CHECK_MUTEX{ std::mutex{} };
        static auto INSTANCE_COUNT{ std::atomic<size_t>(0) };
        std::lock_guard<std::mutex> lock(CHECK_MUTEX);
        if (callback != nullptr)
        {
            callback(add ? ++INSTANCE_COUNT : --INSTANCE_COUNT);
        }
    }

    auto PlayerWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        auto m_this{ static_cast<PlayerWindow*>(nullptr) };

        if (uMsg == WM_NCCREATE)
        {
            auto pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
            m_this = reinterpret_cast<PlayerWindow*>(pcs->lpCreateParams);
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, uintptr_t(m_this));
            if (m_this != nullptr) { m_this->m_That = HWND{ hWnd }; }
        }
        else
        {
            m_this = reinterpret_cast<PlayerWindow*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }

        if (m_this != nullptr)
        {
            const auto& reuslt{ m_this->OnMessage(hWnd, uMsg, wParam, lParam) };
            if (reuslt.has_value()) { return LRESULT{ reuslt.value() }; }
        }

        return LRESULT{ ::DefWindowProcW(hWnd, uMsg, wParam, lParam) };
    }

    auto PlayerWindow::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept -> std::optional<LRESULT>
    {
        if (uMsg == WM_CREATE)
        {
            this->SyncToParentWindow(false, true);
            auto screenWidth { ::GetSystemMetrics(SM_CXSCREEN) };
            auto screenHeight{ ::GetSystemMetrics(SM_CYSCREEN) };
            this->MakeNewBitmap(screenWidth, screenHeight);
        }
        return { std::nullopt };;
    }

    PlayerWindow::~PlayerWindow() noexcept
    {
        PlayerWindow::SafeCheckInstanceCount
        (
            { false },
            [](size_t count) -> void {
                if (count != 0) { return; }
                ::UnregisterClassW
                (
                    { PlayerWindow::ClassName },
                    { ::GetModuleHandleW(NULL) }
                );
            }
        );
    }

    PlayerWindow::PlayerWindow(HWND parent, HINSTANCE hInstance) noexcept : m_Parent(parent)
    {
        PlayerWindow::AutoGdiplusStartup();
        PlayerWindow::SafeCheckInstanceCount
        (
            { true },
            [&hInstance](size_t count) -> void
            {
                if (count != 1) { return; }
                WNDCLASSEXW wndclass
                {
                  .cbSize{ sizeof(WNDCLASSEXW) },
                  .lpfnWndProc{ PlayerWindow::WndProc },
                  .hInstance  { hInstance },
                  .lpszClassName{ PlayerWindow::ClassName },
                };
                ::RegisterClassExW(&wndclass);
            }
        );

        auto hWnd = HWND
        {
            ::CreateWindowExW
            (
                // 分层窗口 | 鼠标穿透 | 隐藏任务栏图标
                { WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE },
                { reinterpret_cast<LPCWSTR>(PlayerWindow::ClassName) },
                { reinterpret_cast<LPCWSTR>(L"PlayerWindowGDI") },
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

    auto PlayerWindow::operator=(PlayerWindow&& other) noexcept -> PlayerWindow&
    {
        PlayerWindow::SafeCheckInstanceCount(true);
        this->m_Mutex.lock();
        {
            auto _this { reinterpret_cast<uint8_t*>(this  ) };
            auto _other{ reinterpret_cast<uint8_t*>(&other) };
            for (size_t i{ 0 }; i < sizeof(PlayerWindow); i++)
            {
                _this[i]  = { _other[i] };
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
        }
        this->m_Mutex.unlock();
        return { *this };
    }

    PlayerWindow::PlayerWindow(PlayerWindow&& other) noexcept
    {
        this->operator=(std::forward<PlayerWindow&&>(other));
    }

    auto PlayerWindow::SetParent(HWND parent) noexcept -> void
    {
        std::lock_guard<std::mutex> lock(this->m_Mutex);
        this->m_Parent = { parent };
    }

    auto PlayerWindow::SetRect(const RECT& rect, bool update) noexcept -> bool
    {
        this->m_Mutex.lock();
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
        this->m_Mutex.unlock();
        return bool
        {
            update ?
            this->UpdateLayer() : true
        };
    }

    auto PlayerWindow::SetRect(RECT&& rect, bool update) noexcept -> bool
    {
        return { this->SetRect(rect, update) };
    }

    auto PlayerWindow::SetSize(LONG width, LONG height, bool update) noexcept -> bool
    {
        this->m_Mutex.lock();
        this->m_Size = SIZE{ width, height };
        this->m_Mutex.unlock();
        return bool
        {
            update ?
            this->UpdateLayer() : true
        };
    }

    auto PlayerWindow::SetPosition(LONG x, LONG y, bool update) noexcept -> bool
    {
        this->m_Mutex.lock();
        this->m_Point = POINT{ x, y };
        this->m_Mutex.unlock();
        return bool
        {
            update ?
            this->UpdateLayer() : true
        };
    }

    auto PlayerWindow::Show() const noexcept -> bool
    {
        if (this->m_That != nullptr)
        {
            auto result{ ::ShowWindow(this->m_That, SW_SHOW) };
            return { static_cast<bool>(result) };
        }
        return { false };
    }

    auto PlayerWindow::Hide() const noexcept -> bool
    {
        if (this->m_That != nullptr)
        {
            auto result{ ::ShowWindow(this->m_That, SW_HIDE) };
            return { static_cast<bool>(result) };
        }
        return { false };
    }

    auto PlayerWindow::SyncToParentWindow(bool update_layer_bitmap, bool force) noexcept -> bool
    {
        if (force)
        {
            this->m_Mutex.lock();
        }
        else if (!this->m_Mutex.try_lock())
        {
            return { false };
        }

        RECT rect{};
        BOOL get_client_rect
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
            this->m_Mutex.unlock();
            return { false };
        }

        BOOL get_window_rect
        {
            ::GetWindowRect(this->m_Parent, &rect)
        };
        if (static_cast<bool>(get_window_rect))
        {
            const int& width { this->m_Size.cx };
            const int& height{ this->m_Size.cy };
            static const int captionHeight
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
            this->m_Mutex.unlock();

            if (update_layer_bitmap)
            {
                return { this->UpdateLayerBitmap(force) };
            }
            return { true };
        }
        else
        {
            this->m_Mutex.unlock();
            return { false };
        }
    }
    auto PlayerWindow::UpdateLayerBitmap(bool force) noexcept -> bool
    {
        if (force)
        {
            this->m_Mutex.lock();
        }
        else if (!this->m_Mutex.try_lock())
        {
            return { false };
        }

        if (this->m_Bitmap != nullptr)
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
                    this->m_Mutex.unlock();
                    return { true };
                }
            }
        }
        this->m_Mutex.unlock();
        return { this->MakeNewBitmap(this->m_Size.cx, this->m_Size.cy) };
    }

    auto PlayerWindow::MakeNewBitmap(LONG width, LONG height) noexcept -> bool
    {
        this->m_Mutex.lock();
        if (this->m_MemDC == nullptr)
        {
            this->m_MemDC = { ::CreateCompatibleDC(NULL) };
            if (this->m_MemDC == nullptr)
            {
                this->m_Mutex.unlock();
                return { false };
            }
        }
        this->m_Mutex.unlock();

        BITMAPINFO info
        {
            .bmiHeader
            {
                .biSize{ sizeof(BITMAPINFO::bmiHeader) },
                .biWidth { width  },
                .biHeight{ height },
                .biPlanes{ 1 },
                .biBitCount{ 32 },
                .biCompression{ BI_RGB },
                .biSizeImage{ 0 }
            }
        };

        auto bytes{ static_cast<void*>(nullptr) };
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
        if (bmp == nullptr) { return { false }; }

        this->m_Mutex.lock();
        ::SelectObject(this->m_MemDC, bmp);
        if (this->m_Bitmap != nullptr)
        {
            ::DeleteObject(this->m_Bitmap);
        }
        this->m_Bitmap = { bmp };
        this->m_Mutex.unlock();
        return { true };
    }

    auto PlayerWindow::UpdateLayer(bool lock) const noexcept -> bool
    {
        if (lock) { this->m_Mutex.lock(); }

        if (this->m_That == nullptr)
        {
            if (lock) { this->m_Mutex.unlock(); }
            return { false };
        }

        if (this->m_MemDC == nullptr)
        {
            if (lock) { this->m_Mutex.unlock(); }
            return { false };
        }

        auto pos{ POINT{} };
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
        if (lock) { this->m_Mutex.unlock(); }
        return { static_cast<bool>(is_update) };
    }

    auto PlayerWindow::SafeDraw(std::function<bool(HDC, const SIZE&)> do_draw) noexcept -> void
    {
        this->m_Mutex.lock();
        bool is_draw { do_draw(this->m_MemDC, this->m_Size) };
        if (is_draw) { this->UpdateLayer(false); }
        this->m_Mutex.unlock();
    }

};
