#include <iostream>
#include <console.hpp>
#include <GDIWindow.hpp>
#include <SubManner.hpp>
#include <thread>
#include <xtime.hpp>

GDI::Window* gdi{ nullptr };

static void DrawRoundedRectangle(Gdiplus::Graphics& graphics, Gdiplus::Rect rect, int cornerRadius)
{
    Gdiplus::GraphicsPath path{};
    Gdiplus::SolidBrush brush((Gdiplus::Color::Black & 0xFFFFFF) | 0x70 << 24);
    path.AddArc(rect.X, rect.Y, cornerRadius, cornerRadius, 180, 90);
    path.AddArc(rect.GetRight() - cornerRadius, rect.Y, cornerRadius, cornerRadius, 270, 90);
    path.AddArc(rect.GetRight() - cornerRadius, rect.GetBottom() - cornerRadius, cornerRadius, cornerRadius, 0, 90);
    path.AddArc(rect.X, rect.GetBottom() - cornerRadius, cornerRadius, cornerRadius, 90, 90);
    path.CloseFigure();
    graphics.FillPath(&brush, &path);
}

static auto CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{

    switch (uMsg)
    {
    case WM_CLOSE: {
        ::ExitProcess(0);
        break;
    }
    case WM_CREATE:
    {
        if (gdi == nullptr)
        {
            gdi = new GDI::Window(hwnd);

            /*constexpr auto filePath
            {
                _PROJECT_WORKSPACE
                L"/dc3wy/sub/271/271_CHN_01_JP.png"
            };*/

            {

                //static Gdiplus::Image* image{ nullptr };

                //if (image == nullptr)
                //{
                //    image = Gdiplus::Image::FromFile(filePath);
                //}

                //if (image != nullptr)
                //{
                //    auto width{ image->GetWidth() };
                //    auto height{ image->GetHeight() };
                //    Gdiplus::Graphics graphics(gdi->m_MemDC);
                //    graphics.Clear(Gdiplus::Color(0x90, 0, 0, 0));

                //    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

                //    Gdiplus::ColorMatrix colorMatrix{
                //       1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                //       0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                //       0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                //       0.0f, 0.0f, 0.0f, 0.8f, 0.0f,
                //       0.0f, 0.0f, 0.0f, 0.0f, 1.0f
                //    };
                //    Gdiplus::ImageAttributes imgAttr{};
                //    imgAttr.SetColorMatrix(&colorMatrix,
                //        Gdiplus::ColorMatrixFlagsDefault,
                //        Gdiplus::ColorAdjustTypeBitmap);
                //    Gdiplus::Rect rect(0, 0, width, height);

                //    graphics.DrawImage(image, rect, 0, 0, width, height, Gdiplus::UnitPixel, &imgAttr);
                //}
            }
            {
                std::thread
                (
                    [](void) -> void
                    {
                        auto get_fade_alpha
                        {
                            [](float duration, float startTime, float curtime) -> float
                            {
                                float elapsedTime = curtime - startTime;

                                if (elapsedTime <= 0.0f) return 0.0f;
                                if (elapsedTime >= duration) return 255.0f;
                                float progress = elapsedTime / duration;
                                return progress * 255.0f;
                            }
                        };

                        constexpr auto filePath
                        {
                            _PROJECT_WORKSPACE
                            L"/dc3wy/sub/271.xsub"
                        };


                        utils::chilitimer chilitimer{};
                        XSub::GDI::ImageSub sub{ filePath };

                        while (true)
                        {
                            auto time{ chilitimer.peek() };
                            auto is_draw{ sub.Draw(time, gdi->m_MemDC, gdi->m_Size) };
                            console::fmt::write("is_draw{ %s }\n",is_draw ? "true": "false");
                            if (is_draw)
                            {
                                gdi->UpdateLayer();
                            }
                            ::Sleep(1);
                        }
                    }
                ).detach();
                
            }
            //Gdiplus::Graphics graphics(gdi->m_MemDC);
            //DrawRoundedRectangle(graphics, { 0, 0, gdi->m_Size.cx, gdi->m_Size.cy }, 1);
           
        }
        break;
    }
    case WM_MOVE:
    {
        auto x{ static_cast<int>(LOWORD(lParam)) };
        auto y{ static_cast<int>(HIWORD(lParam)) };
        if (gdi != nullptr)
        {
            //gdi->SyncToParentWindow();
            gdi->SetPosition(x, y);
            //gdi->UpdateLayer();
        }
        break;
    }
    case WM_SIZING:
    {
        auto rect{ reinterpret_cast<RECT*>(lParam) };
        if (gdi != nullptr)
        {
            gdi->SyncToParentWindow();
        }
        break;
    }
    case WM_SIZE:
    {
        if (wParam == SIZE_RESTORED)
        {
            if (gdi != nullptr)
            {
               /* gdi->UpdateLayerBitmap();
                Gdiplus::Graphics graphics(gdi->m_MemDC);
                graphics.Clear(Gdiplus::Color(0, 0, 0, 0));
                DrawRoundedRectangle(graphics, { 0, 0, gdi->m_Size.cx, gdi->m_Size.cy }, 1);
                gdi->UpdateLayer();
                BITMAP bitmap{};
                auto get_object_result = int
                {
                    ::GetObjectW(gdi->m_Bitmap, sizeof(BITMAP), &bitmap)
                };
                console::fmt::write
                (
                    "bmp{ %d, %d } cx{ %d } cy{ %d }\n",
                    bitmap.bmWidth, bitmap.bmHeight,
                    gdi->m_Size.cx, gdi->m_Size.cy
                );*/
            }
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
           /* gdi->SyncToParentWindow(true);
            Gdiplus::Graphics graphics(gdi->m_MemDC);
            graphics.Clear(Gdiplus::Color(0, 0, 0, 0));
            DrawRoundedRectangle(graphics, { 0, 0, gdi->m_Size.cx, gdi->m_Size.cy }, 1);
            gdi->UpdateLayer();
            BITMAP bitmap{};
            auto get_object_result = int
            {
                ::GetObjectW(gdi->m_Bitmap, sizeof(BITMAP), &bitmap)
            };
            console::fmt::write
            (
                "bmp{ %d, %d } cx{ %d } cy{ %d }\n",
                bitmap.bmWidth, bitmap.bmHeight,
                gdi->m_Size.cx, gdi->m_Size.cy
            );*/
        }
        else
        {
            console::fmt::write("wParam{ %d }\n", wParam);
        }
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        /*auto hdc{ ::BeginPaint(hwnd, &ps) };
        Gdiplus::Graphics graphics(hdc);
        Gdiplus::SolidBrush brush{ Gdiplus::Color::Blue };
        graphics.FillRectangle(&brush, 50, 50, 200, 100);*/
        ::EndPaint(hwnd, &ps);
        break;
    }
    }
    return { ::DefWindowProcW(hwnd, uMsg, wParam, lParam) };
}

auto main(int, char**) -> int
{
    console::make();
    console::writeline("hello gdimain");

    WNDCLASSEX windowclass
    {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = MainWndProc,
        .hInstance = ::GetModuleHandleW(NULL),
        .hbrBackground = HBRUSH(COLOR_WINDOW),
        .lpszClassName = L"by_iTsukezigen_clazz",
    };
    ::RegisterClassExW(&windowclass);

    RECT rect{ 0, 0, 1280, 720 };
    ::AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
    int width{ rect.right - rect.left };
    int height{ rect.bottom - rect.top };
    int screenWidth{ ::GetSystemMetrics(SM_CXSCREEN) };
    int screenHeight{ ::GetSystemMetrics(SM_CYSCREEN) };
    int x{ (screenWidth - width) / 2 };
    int y{ (screenHeight - height) / 2 };

    auto windowHandle = HWND
    {
        ::CreateWindowExW
        (
            WS_EX_OVERLAPPEDWINDOW, windowclass.lpszClassName,
            L"Hello World!", WS_OVERLAPPEDWINDOW,
            x, y, width, height, NULL, NULL,
            windowclass.hInstance, NULL
        )
    };
    ::ShowWindow(windowHandle, SW_NORMAL);

    MSG message{};
    while (::GetMessageW(&message, NULL, 0, 0))
    {
        ::TranslateMessage(&message);
        ::DispatchMessageW(&message);
    }
    return { NULL };
}
