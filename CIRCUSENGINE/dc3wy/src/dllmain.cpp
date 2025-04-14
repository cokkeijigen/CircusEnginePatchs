#include <windows.h>
#include <dc3wy.hpp>

extern "C" {
    
    __declspec(dllexport) auto hook(void) -> void {}

    __declspec(dllexport) auto _patch_by_iTsukezigen_(void) -> const char*
    {
        return "https://github.com/cokkeijigen/dc3_cn";
    }

    auto APIENTRY DllMain(HMODULE, DWORD ul_reason_for_call, LPVOID) -> BOOL
    {
        if (DLL_PROCESS_ATTACH == ul_reason_for_call)
        {
            DC3WY::INIT_ALL_PATCH();
        }
        return TRUE;
    }
}
