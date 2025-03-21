#include <iostream>
#include <D2DWindow.hpp>

// 注意 UpdateLayeredWindow 后不会自动发送WM_PAINT消息
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_NCHITTEST)
    {
        return HTCAPTION;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int main3()
{
    // 窗口类定义
    WNDCLASSEXW wcx = { 0 };
    wcx.cbSize = sizeof(wcx);
    wcx.lpfnWndProc = WindowProc;
    wcx.lpszClassName = L"Test Class";
    wcx.hInstance = GetModuleHandleW(nullptr);

    // 注册窗口类
    ::RegisterClassExW(&wcx);

    //创建窗口
    DWORD ex_style_normal = WS_EX_LAYERED;                                                // 正常
    DWORD ex_style_hide = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE;               // 置顶，隐藏任务栏图标
    DWORD ex_style_mouse_penetration = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST; // 置顶，鼠标穿透
    HWND hwnd = ::CreateWindowExW
    (
        ex_style_hide, wcx.lpszClassName, L"Direct2D_GDI", WS_POPUP,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        nullptr, nullptr, wcx.hInstance, nullptr
    );

    // 显示窗口
    ShowWindow(hwnd, SW_NORMAL);
 /*   D2DTestWindow a(hwnd);
    a.BeginDraw();
    a.sg_pRenderTarget->Clear(D2D1::ColorF(0.2f, 0.3f, 0.5f, 0.5f));
    a.EndDraw();
    a.Update();*/

    // 消息循环
    MSG msg = { 0 };
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }

    // 清理 Direct2D 资源
    //sg_pRenderTarget->Release();
    //sg_pID2D1Factory->Release();

    // 清理 GDI 资源
    //SelectObject(h_mem_dc, h_bitmap_old);
    //DeleteObject(h_bitmap);
    //ReleaseDC(hwnd, h_mem_dc);

    // 注销窗口类
    UnregisterClassW(wcx.lpszClassName, wcx.hInstance);
    return 0;
}
