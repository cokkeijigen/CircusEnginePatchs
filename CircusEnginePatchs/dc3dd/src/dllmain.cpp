#include <iostream>
#include <windows.h>
#include <dc3dd.hpp>
#include <console.hpp>

namespace DC3DD
{
        
    auto DC3DD::ReplacePathA(std::string_view path) -> std::string_view
    {
        static std::string new_path{};
        size_t pos{ path.find_last_of("\\/") };
        if (pos != std::wstring_view::npos)
        {
            new_path = std::string{ ".\\cn_Data" }.append(path.substr(pos));
            DWORD attr { ::GetFileAttributesA(new_path.c_str()) };
            if (attr != INVALID_FILE_ATTRIBUTES)
            {
                DEBUG_ONLY
                ({
                    if (path.ends_with(".mes"))
                    {
                        console::fmt::write<console::cdpg::dDfault, console::txt::dark_yellow>
                        (
                            "[LOAD] %s\n", path.substr(pos + 1).data()
                        );
                    }
                })
                return new_path;
            }

            DEBUG_ONLY
            ({
                if (path.ends_with(".mes"))
                {
                    console::fmt::write("[LOAD] %s\n", path.substr(pos + 1).data());
                }
            })
           
        }
        return {};
    }

    static auto WINAPI CreateFileA(LPCSTR lpFN, DWORD dwDA, DWORD dwSM, LPSECURITY_ATTRIBUTES lpSA, DWORD dwCD, DWORD dwFAA, HANDLE hTF) -> HANDLE
    {
        std::string_view new_path{ DC3DD::ReplacePathA(lpFN) };
        return Patch::Hooker::Call<DC3DD::CreateFileA>(new_path.empty() ? lpFN : new_path.data(), dwDA, dwSM, lpSA, dwCD, dwFAA, hTF);
    }

    static auto WINAPI FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) -> HANDLE
    {
        std::string_view new_path{ DC3DD::ReplacePathA(lpFileName) };
        return Patch::Hooker::Call<DC3DD::FindFirstFileA>(new_path.empty() ? lpFileName : new_path.data(), lpFindFileData);
    }


    static auto WINAPI GetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuf, LPGLYPHMETRICS lpgm, DWORD cjbf, LPVOID pvbf, MAT2* lpmat) -> DWORD
    {
        if (tagTEXTMETRICA lptm{}; ::GetTextMetricsA(hdc, &lptm))
        {
            if (0xA1EC == uChar) // § -> ♪
            {
                HFONT font{ DC3DD::FontManager.GetJISFont(lptm.tmHeight) };
                if (font != nullptr)
                {
                    font = { reinterpret_cast<HFONT>(::SelectObject(hdc, font)) };
                    DWORD result{ ::GetGlyphOutlineW(hdc, L'♪', fuf, lpgm, cjbf, pvbf, lpmat) };
                    static_cast<void>(::SelectObject(hdc, font));
                    return result;
                }
            }

            if (uChar == 0x23) { uChar = 0x20; }

            HFONT font{ DC3DD::FontManager.GetGBKFont(lptm.tmHeight) };
            if (font != nullptr)
            {
                font = { reinterpret_cast<HFONT>(::SelectObject(hdc, font)) };
                DWORD result{ Patch::Hooker::Call<DC3DD::GetGlyphOutlineA>(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat) };
                static_cast<void>(::SelectObject(hdc, font));
                return result;
            }
        }
        return Patch::Hooker::Call<DC3DD::GetGlyphOutlineA>(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat);
    }

    static auto INIT_ALL_PATCH(void) -> void
    {
        Patch::Hooker::Begin();
        Patch::Hooker::Add<DC3DD::CreateFileA>(::CreateFileA);
        Patch::Hooker::Add<DC3DD::FindFirstFileA>(::FindFirstFileA);
        Patch::Hooker::Add<DC3DD::GetGlyphOutlineA>(::GetGlyphOutlineA);
        Patch::Hooker::Add<DC3DD::WndProc>(reinterpret_cast<void*>(0x411AB0));
        Patch::Hooker::Commit();
    }
}

extern "C"
{
    __declspec(dllexport) auto hook(void) -> void {}

    __declspec(dllexport) auto _patch_by_iTsukezigen_(void) -> const char*
    {
        return "https://github.com/cokkeijigen/dc3_cn";
    }

    auto APIENTRY DllMain(HMODULE, DWORD ul_reason_for_call, LPVOID) -> BOOL
    {
        if (DLL_PROCESS_ATTACH == ul_reason_for_call)
        {
            DC3DD::INIT_ALL_PATCH();
        }
        return TRUE;
    }
}
