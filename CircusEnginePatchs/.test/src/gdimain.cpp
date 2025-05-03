#include <iostream>
#include <console.hpp>
#include <GDIWindow.hpp>
#include <thread>
#include <xtime.hpp>
#include <subplayer_gdi.hpp>
#include <xsub/gdi/imgsub_gdi.hpp>
#include <SubManner.hpp>

XSub::GDI::ImageSubPlayer* XSubPlayerWindowGDI{ nullptr };
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
        if (XSubPlayerWindowGDI == nullptr)
        {
            
            XSubPlayerWindowGDI = new XSub::GDI::ImageSubPlayer(hwnd);
            std::thread
            (
                [](void) -> void
                {
                    utils::chilitimer chilitimer{};
                    /*XSub2::GDI2::ImageSub sub
                    {
                         _PROJECT_WORKSPACE
                        L"/dc3wy/sub/271.xsub"
                    };*/

                    XSub::GDI::ImageSubFile SubFile
                    {
                         _PROJECT_WORKSPACE
                         L"/dc3wy/sub/271.xsub"
                    };
                    XSubPlayerWindowGDI->Load(SubFile);
                    //XSubPlayerWindowGDI->Play(23.8f, true);
                    XSubPlayerWindowGDI->Play
                    (
                        { true },
                        [&chilitimer](void) -> float
                        {
                            return { chilitimer.peek() + 23.8f };
                        }
                    );
                    ::Sleep(10000);
                    XSubPlayerWindowGDI->SetDefualtPoint
                    (
                        XSub::Point
                        {
                            .align{ XSub::Align::Center },
                        }
                    );
                    XSubPlayerWindowGDI->UseDefualtPoint(true);
                    ::Sleep(10000);
                    XSubPlayerWindowGDI->Stop(true);
                    XSubPlayerWindowGDI->UnLoad();

                    /*while (true)
                    {
                        auto time{ chilitimer.peek() + 23.8f };
                        XSubPlayerWindowGDI->Update(time);
                        ::Sleep(1);
                    }*/
                }
            ).detach();
        }

  
        break;
    }
    case WM_MOVE:
    {
        auto x{ static_cast<int>(LOWORD(lParam)) };
        auto y{ static_cast<int>(HIWORD(lParam)) };
        //if (gdi != nullptr)
        //{
        //    //gdi->SyncToParentWindow();
        //    gdi->SetPosition(x, y);
        //    //gdi->UpdateLayer();
        //}
        if (XSubPlayerWindowGDI != nullptr)
        {
            XSubPlayerWindowGDI->SetPosition(x, y);
        }
        break;
    }
    case WM_SIZING:
    {
        /*if (gdi != nullptr)
        {
            gdi->SyncToParentWindow();
        }*/
        if (XSubPlayerWindowGDI != nullptr)
        {
            XSubPlayerWindowGDI->SyncToParentWindow(true);
        }
        break;
    }
    case WM_SIZE:
    {
        if (wParam == SIZE_RESTORED)
        {
            if (XSubPlayerWindowGDI != nullptr)
            {
                XSubPlayerWindowGDI->Show();
                XSubPlayerWindowGDI->SyncToParentWindow(true);
            }
            
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
            //gdi->SyncToParentWindow();
            //gdi->UpdateLayerBitmap(true);
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
            if (XSubPlayerWindowGDI != nullptr)
            {
                //console::fmt::write("最大化？\n");
                XSubPlayerWindowGDI->Show();
                XSubPlayerWindowGDI->SyncToParentWindow(true);
            }
        }
        else if (wParam == SIZE_MINIMIZED)
        {
            //console::fmt::write("最小化？\n");
            XSubPlayerWindowGDI->Hide();
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

    RECT rect{ 0, 0, 1024, 576 };
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
