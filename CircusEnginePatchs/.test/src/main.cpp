#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <console.hpp>
#include <windows.h>
#include <FontManager.hpp>

static std::unique_ptr<Utils::FontManagerGUI> gui{};

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg)
    {
    case WM_CLOSE: {
        ExitProcess(0);
        break;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == uint16_t(6666))
        {
            gui->ChooseFont();
        }
        break;
    }

    case WM_CREATE:
    {
        gui = Utils::FontManagerGUI::CreatePtr(static_cast<HWND>(hwnd));
        gui->Init(Utils::FontManager::DefaultSize, Utils::FontManagerGUI::NORMAL, L"黑体", 18, 30)
            .Load(".\\font.dat").OnChanged(
                [&](const Utils::FontManagerGUI* m_this) -> void
                {
                    console::writeline("更改了");
                }
            );
        break;
    }
    default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return TRUE;
}

auto main(int, char**)  -> int
{


    console::make();
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
    HWND hwndButton = CreateWindow(L"BUTTON", L"Test",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        (width - 100) / 2, (height - 100) / 2, 100, 100,
        mainHWND, HMENU(6666), GetModuleHandleA(NULL), NULL
    );
    ::ShowWindow(mainHWND, SW_NORMAL);
    MSG msg{};
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return {};
}

#include <SubManner.hpp>
#include <GDIWindow.hpp>



auto main2(int, char**) -> int
{





    /*console::make();*/

   /* console::make();
    constexpr auto filePath
    {
        _PROJECT_WORKSPACE
        L"/dc3wy/sub/271.xsub"
    };
    GDI::GdiplusStartup();
    XSub::GDI::ImageSub sub2(filePath);
    console::write("");*/

    return {};
}
