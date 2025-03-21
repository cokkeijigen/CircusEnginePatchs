//#define UNICODE
#include <iostream>
//#include <console.hpp>
#include <Graphis.hpp>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi")
#include <versionhelpers.h>
#include <D2DWindowBase.hpp>

#include <thread>
static Graphis* g_pGraphis{ nullptr };

static HWND g_MainhWnd{};
static HWND g_GraphishWnd{};
static D2DTestWindow* d2d{};

static auto CALLBACK GraphisWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        //console::writeline("GraphisWndProc");
        //d2d->BeginDraw();
        //d2d->sg_pRenderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
        //for (size_t i = 0; i < 100; i++)
        //{
        //    d2d->DrawCircle
        //    (
        //        std::rand() % 1280, std::rand() % 720, std::rand() % 100,
        //        { (std::rand() % 10) * 1.0f },
        //        {
        //            (std::rand() % 100) / 100.0f,
        //            (std::rand() % 100) / 100.0f,
        //            (std::rand() % 100) / 100.0f,
        //            255.0f
        //        }
        //    );
        //}
        //d2d->EndDraw();
        if (::d2d) ::d2d->Update();
        break;
    }
    case WM_CREATE:
    {
        if (!d2d)
        {
            d2d = new D2DTestWindow(hwnd, g_MainhWnd);
            d2d->BeginDraw();
            d2d->m_RenderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
            for (size_t i = 0; i < 100; i++)
            {
                d2d->DrawCircle
                (
                    std::rand() % 1280, std::rand() % 720, std::rand() % 100,
                    { (std::rand() % 10) * 1.0f },
                    {
                        (std::rand() % 100) / 100.0f,
                        (std::rand() % 100) / 100.0f,
                        (std::rand() % 100) / 100.0f,
                        255.0f
                    }
                );
            }
            d2d->EndDraw();
        }
        g_GraphishWnd = hwnd;
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static auto CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{

    switch (uMsg)
    {
    case WM_CLOSE: {
        ExitProcess(0);
        break;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == uint16_t(6666)) {
           
        }
        break;
    }
    case WM_SIZE:
    case WM_MOVE:
    {
        if (g_GraphishWnd)
        {
            ::SendMessageW(g_GraphishWnd, WM_PAINT, wParam, lParam);
            //::InvalidateRect(g_GraphishWnd, NULL, TRUE);
        }
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        HDC hdc{ ::BeginPaint(hwnd, &ps) };
        
       
        ::EndPaint(hwnd, &ps);
        break;
    }
    case WM_CREATE:
    {
        auto call = [hwnd]()
            {
                g_MainhWnd = hwnd;
                RECT rect{ 0, 0, 1280, 720 };
                ::AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
                int width{ rect.right - rect.left };
                int height{ rect.bottom - rect.top };

                int screenWidth{ ::GetSystemMetrics(SM_CXSCREEN) };
                int screenHeight{ ::GetSystemMetrics(SM_CYSCREEN) };
                int x{ (screenWidth - width) / 2 };
                int y{ (screenHeight - height) / 2 };
                auto d2d = ::CreateWindowExW
                (
                    WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
                    L"D2DWND_CLAZZ", L"Direct2D_GDI", WS_POPUP,
                    x, y, width, height,
                    g_MainhWnd, nullptr, ::GetModuleHandleW(NULL), nullptr
                );
                ::ShowWindow(d2d, SW_SHOW);
                MSG message{};
                while (message.message != WM_QUIT)
                {
                    if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
                    {
                        ::DispatchMessage(&message);
                    }
                    else
                    {
                        //if(::d2d) ::d2d->Update();
                    }

                }
            };
        std::thread(call).detach();
        
        break;
    }
    default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return TRUE;
}


static auto CreateTestWindow() -> void
{
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


    WNDCLASSEXW d2dwndclass
    {
      .cbSize = sizeof(WNDCLASSEXW),
      .lpfnWndProc = GraphisWndProc,
      .hInstance = windowclass.hInstance,
      .lpszClassName = L"D2DWND_CLAZZ",
    };

    ::RegisterClassExW(&d2dwndclass);


    RECT rect{ 0, 0, 1280, 720 };
    ::AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
    int width { rect.right - rect.left };
    int height{ rect.bottom - rect.top };
    int screenWidth{ ::GetSystemMetrics(SM_CXSCREEN) };
    int screenHeight{ ::GetSystemMetrics(SM_CYSCREEN) };
    int x{ (screenWidth  - width) / 2  };
    int y{ (screenHeight - height) / 2 };

    auto windowHandle = HWND
    {
        ::CreateWindowExW
        (
            WS_EX_OVERLAPPEDWINDOW, windowclass.lpszClassName,
            L"Hello DirectX!", WS_OVERLAPPEDWINDOW,
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

    if (g_pGraphis != nullptr)
    {
        delete g_pGraphis;
        g_pGraphis = nullptr;
    }
}

auto main(int, char**) -> int
{
    //console::make();
    CreateTestWindow();
    return 0;
}


auto main2(int, char**) -> int
{


    //console::make();
    WNDCLASSEX wndcls{};
    wndcls.cbSize = sizeof(WNDCLASSEX);
    wndcls.style = CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = MainWndProc;
    wndcls.lpszClassName = L"iTsukezigen";
    wndcls.hbrBackground = GetSysColorBrush(COLOR_WINDOW + 1);
    ::RegisterClassExW(&wndcls);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int width = 1280;
    int height = 720;
    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;

    HWND mainHWND = ::CreateWindowExW(
        WS_EX_LTRREADING, wndcls.lpszClassName,
        L"Hello Wolrd", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
        x, y, width, height, NULL, NULL, GetModuleHandleA(NULL),
        NULL
    );

    

    /*HWND hwndButton = CreateWindow(L"BUTTON", L"Test",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        (width - 100) / 2, (height - 100) / 2, 100, 50,
        mainHWND, HMENU(6666), GetModuleHandleA(NULL), NULL
    );*/
    ::ShowWindow(mainHWND, SW_NORMAL);
    MSG msg{};
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_pGraphis != nullptr)
    {
        delete g_pGraphis;
        g_pGraphis = nullptr;
    }

    return 0;
}

