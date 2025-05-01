#pragma once
#include <optional>
#include <mutex>
#include <atomic>
#include <functional>
#include <xusb_basic_gdi.hpp>

namespace XSub::GDI
{
    class PlayerWindow
    {
        static auto SafeCheckInstanceCount(bool add, std::function<void(size_t)> callback = nullptr) -> void;
        static auto CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
        static inline auto TIMER_SHOW_WINDOW{ static_cast<WPARAM>(0x01) };
    protected:

        HWND m_Parent{};
        HWND m_That{};
        HDC m_MemDC{};
        HBITMAP m_Bitmap{};

        SIZE  mutable m_Size{};
        POINT mutable m_Point{};
        BLENDFUNCTION mutable m_Blend
        {
            .BlendFlags = AC_SRC_OVER,
            .SourceConstantAlpha = 0x00,
            .AlphaFormat = AC_SRC_ALPHA,
        };

        std::mutex mutable m_Mutex{};
        bool mutable m_IsMessageLoop{};

        auto OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept -> std::optional<LRESULT>;

        auto MakeNewBitmap(LONG width, LONG height) noexcept -> bool;

        auto UpdateLayer(bool lock, uint8_t alpha) noexcept -> bool;

    public:

        static inline const wchar_t ClassName[] { L"XSub_GDI_PlayerWindow_Clazz" };

        virtual ~PlayerWindow() noexcept;

        PlayerWindow(const PlayerWindow&) noexcept = delete;

        PlayerWindow(PlayerWindow&& other) noexcept;

        PlayerWindow(HWND parent, HINSTANCE hInstance = ::GetModuleHandleW(NULL)) noexcept;

        auto operator=(PlayerWindow&& other) noexcept -> PlayerWindow&;

        auto operator=(const PlayerWindow&) noexcept -> PlayerWindow & = delete;

        auto SetParent(HWND parent) noexcept -> void;

        auto SetRect(const RECT& rect, bool update = false) noexcept -> bool;

        auto SetRect(RECT&& rect, bool update = false) noexcept -> bool;

        auto SetSize(LONG width, LONG height, bool update = false) noexcept -> bool;

        auto SetPosition(LONG x, LONG y, bool update = false) noexcept -> bool;

        auto SyncToParentWindow(bool update_layer_bitmap = false, bool force = false) noexcept -> bool;

        auto UpdateLayerBitmap(bool force = false) noexcept -> bool;

        auto UpdateLayer(bool lock = true) const noexcept -> bool;

        auto Show(UINT delay = 200) noexcept -> bool;

        auto Hide() noexcept -> bool;

        auto IsVisible() const noexcept -> bool;

        auto SafeDraw(std::function<bool(HDC, const SIZE&)> do_draw) noexcept -> bool;

        auto Clear(Gdiplus::Color color, bool update_layer = true) noexcept -> void;

        auto Clear(uint32_t color = NULL, bool update_layer = true) noexcept -> void;

        auto MessageLoop(bool loop, bool as_thread = true) noexcept -> void;
    };
}
