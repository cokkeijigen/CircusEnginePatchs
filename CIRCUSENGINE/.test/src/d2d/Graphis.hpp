#pragma once
#include <windows.h>
#include <d2d1.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

class Graphis
{
   

    static auto MakeD2DColor(uint32_t uiRGBA) -> D2D1::ColorF
    {
        auto r{ (float)((uiRGBA & 0xFF000000) >> 0x18) / 255.0f };
        auto g{ (float)((uiRGBA & 0x00FF0000) >> 0x10) / 255.0f };
        auto b{ (float)((uiRGBA & 0x0000FF00) >> 0x08) / 255.0f };
        auto a{ (float)((uiRGBA & 0x000000FF) >> 0x00) / 255.0f };
        return D2D1::ColorF{ r, g, b, a };
    }
public:

    ID2D1Factory* m_pFactory{};
    ID2D1DCRenderTarget* m_pRenderRarget{};

    inline Graphis()
    {
    }

    inline ~Graphis()
    {
        if (this->m_pFactory != nullptr)
        {
            this->m_pFactory->Release();
            this->m_pFactory = nullptr;
        }
        
        if (this->m_pRenderRarget != nullptr)
        {
            this->m_pRenderRarget->Release();
            this->m_pRenderRarget = nullptr;
        }

    }
    HDC h_mem_dc{};

    inline auto Init(HWND windowHandle) -> bool
    {

        // 创建内存DC
        HDC h_dc = GetDC(windowHandle);
        h_mem_dc = CreateCompatibleDC(h_dc);

        // 在内存DC创建 HBITMAP
        HBITMAP h_bitmap = CreateCompatibleBitmap(h_dc, 1280, 720);

        // 选入 HBITMAP
        HBITMAP h_bitmap_old = (HBITMAP)SelectObject(h_mem_dc, h_bitmap);
        ReleaseDC(windowHandle, h_dc);



       

        
        HRESULT factory_hr
        {
            ::D2D1CreateFactory
            (
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                &this->m_pFactory
            )
        };

        if (factory_hr != S_OK)
        {
            return false;
        }






        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties
        (
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        );
        this->m_pFactory->CreateDCRenderTarget(&props, &this->m_pRenderRarget);

        RECT rc = { 0 };
        GetClientRect(windowHandle, &rc);
        this->m_pRenderRarget->BindDC(h_mem_dc, &rc);

       


        /*RECT rect{};
        ::GetClientRect(windowHandle, &rect);

        HRESULT target_hr
        {
            this->m_pFactory->CreateHwndRenderTarget
            (
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties
                (
                    windowHandle,
                    D2D1::SizeU(rect.right, rect.bottom)
                ),
                &this->m_pRenderRarget
            )
        };*/

        //return bool { target_hr == S_OK };
        return bool { true };
    }

    inline auto BeginDraw() -> void
    {
        this->m_pRenderRarget->BeginDraw();
    }

    inline auto EndDraw() -> void
    {
        this->m_pRenderRarget->EndDraw();
    }

    static inline uint32_t MakeRGBA(uint8_t uR, uint8_t uG, uint8_t uB, uint8_t uA)
    {
        return ((uint32_t)uR << 0x18) | ((uint32_t)uG << 0x10) | ((uint32_t)uB << 0x08) | ((uint32_t)uA << 0x00);
    }

    inline auto ClearScreen(uint32_t color = 0) -> void
    {
        this->m_pRenderRarget->Clear(MakeD2DColor(color));
    }

    inline auto ClearScreen(D2D1::ColorF color) -> void
    {
        this->m_pRenderRarget->Clear(color);
    }

    inline auto DrawCircle(float x, float y, float radius, uint32_t color) -> void
    {
        ID2D1SolidColorBrush* brush{};
        this->m_pRenderRarget->CreateSolidColorBrush(MakeD2DColor(color), &brush);

        this->m_pRenderRarget->DrawEllipse
        (
            D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius),
            brush, 3.0f
        );

        brush->Release();
    }
};
