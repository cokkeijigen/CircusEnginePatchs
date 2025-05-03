#include <iostream>
#include <dc3dd.hpp>

namespace DC3DD
{
    auto DC3DD::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
    {


        return { Patch::Hooker::Call<DC3DD::WndProc>(hWnd, uMsg, wParam, lParam) };
    }
}
