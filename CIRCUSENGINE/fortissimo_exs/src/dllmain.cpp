#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <fortissimo_exs.hpp>

extern "C" {

    __declspec(dllexport) auto _patch_by_iTsukezigen_(void) -> void
    {
    }

    auto APIENTRY DllMain(HMODULE, DWORD ul_reason_for_call, LPVOID) -> BOOL
    {
        if (DLL_PROCESS_ATTACH == ul_reason_for_call)
        {
            FORTISSIMO_EXS::INIT_ALL_PATCH();
        }
        return TRUE;
    }
}
