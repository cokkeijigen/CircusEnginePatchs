#pragma once
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <d2d1.h>
#include <optional>
#pragma comment(lib, "d2d1.lib")
//#include <console.hpp>

class D2DTestWindow
{
public:

    HDC h_mem_dc{};
    ID2D1Factory* m_ID2D1Factory{};
    ID2D1DCRenderTarget* m_RenderTarget{};
    HWND hwnd{}, m_Parent{};
    HBITMAP h_bitmap_old{};
    HBITMAP h_bitmap{};
    ID2D1SolidColorBrush* m_brush{};

    auto MakeD2DColor(uint32_t uiRGBA) -> D2D1::ColorF
    {
        auto r{ (float)((uiRGBA & 0xFF000000) >> 0x18) / 255.0f };
        auto g{ (float)((uiRGBA & 0x00FF0000) >> 0x10) / 255.0f };
        auto b{ (float)((uiRGBA & 0x0000FF00) >> 0x08) / 255.0f };
        auto a{ (float)((uiRGBA & 0x000000FF) >> 0x00) / 255.0f };
        return D2D1::ColorF{ r, g, b, a };
    }

    inline D2DTestWindow(HWND hwnd, HWND parent)
    {
        this->m_Parent = parent;
        this->hwnd = hwnd;
        HDC h_dc = GetDC(hwnd);
        this->h_mem_dc = CreateCompatibleDC(h_dc);
        this->h_bitmap = CreateCompatibleBitmap(h_dc, 1280, 720);

        // 选入 HBITMAP
        this->h_bitmap_old = (HBITMAP)SelectObject(h_mem_dc, h_bitmap);
        ReleaseDC(hwnd, h_dc);

        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &this->m_ID2D1Factory);

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties
        (
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        );
        this->m_ID2D1Factory->CreateDCRenderTarget(&props, &this->m_RenderTarget);
        this->m_RenderTarget->CreateSolidColorBrush({}, &m_brush);
        RECT rc = { 0 };
        GetClientRect(hwnd, &rc);
        this->m_RenderTarget->BindDC(h_mem_dc, &rc);
    }

    inline ~D2DTestWindow()
    {
        this->m_RenderTarget->Release();
        this->m_ID2D1Factory->Release();
        this->m_brush->Release();
        SelectObject(this->h_mem_dc, this->h_bitmap_old);
        DeleteObject(this->h_bitmap);
        ReleaseDC(this->hwnd, this->h_mem_dc);
    }

    inline void BeginDraw()
    {
        this->m_RenderTarget->BeginDraw();
    }

    inline void DrawCircle(float x, float y, float radius, float strokeWidth, D2D1::ColorF color)
    {
        this->m_brush->SetColor(color);
        this->m_RenderTarget->DrawEllipse
        (
            D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius),
            this->m_brush, strokeWidth
        );
    }

    inline void EndDraw()
    {
        this->m_RenderTarget->EndDraw();
    }

    inline void Update()
    {
     /*   RECT rect{ 0, 0, 1280, 720 };
        ::AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
        int width{ rect.right - rect.left };
        int height{ rect.bottom - rect.top };*/

        SIZE draw_size = { 1280, 720 };

        HDC h_screen_dc = GetDC(NULL);

        BLENDFUNCTION blend = { 0 };          // 混合结构体，用于设置混合参数
        blend.BlendOp = 0;
        blend.BlendOp = AC_SRC_OVER;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        POINT mem_pos = { 0, 0 };             // 设置贴图在内存DC的位置，与 draw_size 值之和不能大于图形分辨率，否则不显示
        //SetWindowPos(this->hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        int x{ CW_USEDEFAULT }, y{ CW_USEDEFAULT };
        {
            RECT rect{};
            bool success
            {
                ::GetWindowRect(this->m_Parent, &rect) ||
                ::GetWindowRect(::GetDesktopWindow(), &rect)
            };
            if (success)
            {
                x = ((rect.left + rect.right) / 2) - (draw_size.cx / 2);
                y = ((rect.top + rect.bottom) / 2) - (draw_size.cy / 2);
            }
        }

        POINT wnd_pos = { (LONG)x, (LONG)y }; // 设置窗口在桌面（屏幕上）的位置

        //console::fmt::write("pos: x -> %d y -> %d\n", x, y);
        BOOL is_update = UpdateLayeredWindow  // 贴图
        (
            this->hwnd,            // <- Window HWND
            h_screen_dc,     // <- Screen DC
            &wnd_pos,        // <- 层在屏幕上的
            &draw_size,      // <- 贴图大小
            this->h_mem_dc,        // <- 需要MemDC，由CreateCompatibleDC获得
            &mem_pos,        // <- 兼容DC层位置
            NULL,            // <- 色键，可以用来去除指定的颜色，但是效果拉跨
            &blend,          // <- 混合结构，设置混合参数
            ULW_ALPHA        // <- 设置 blend 的时候使用
            //ULW_COLORKEY   // <- 设置 色键 的时候使用
        );
    }
};
