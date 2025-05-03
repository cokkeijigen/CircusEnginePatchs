#include <iostream>
#include <mutex>
#include <xusb_basic_gdi.hpp>

namespace XSub::GDI
{
    auto GdiplusStartup::AutoGdiplusStartup(void) -> void
    {
        static auto Mutex{ std::mutex{} };
        static std::unique_ptr<GdiplusStartup> p_GdiplusStartup{ nullptr };
        std::lock_guard<std::mutex> lock(Mutex);
        if (p_GdiplusStartup == nullptr)
        {
            p_GdiplusStartup = std::make_unique<GdiplusStartup>();
        }
    }
}
