#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <xusb_basic.hpp>

namespace XSub::GDI
{
    class GdiplusStartup
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput{};
        ULONG_PTR gdiplusToken{};
    public:

        GdiplusStartup()
        {
            Gdiplus::GdiplusStartup
            (
                &this->gdiplusToken,
                &this->gdiplusStartupInput,
                NULL
            );
        }

        ~GdiplusStartup()
        {
            Gdiplus::GdiplusShutdown
            (
                this->gdiplusToken
            );
        }

        static auto AutoGdiplusStartup(void) -> void;
    };
}
