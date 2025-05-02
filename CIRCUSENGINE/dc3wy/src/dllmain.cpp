#include <iostream>
#include <windows.h>
#include <dc3wy.hpp>
#include <console.hpp>

namespace DC3WY
{
    static auto ReplacePathW(std::wstring_view path) -> std::wstring_view
    {
        static std::wstring new_path{};
        size_t pos{ path.find_last_of(L"\\/") };
        if (pos != std::wstring_view::npos)
        {
            new_path = std::wstring{ L".\\cn_Data" }.append(path.substr(pos));
            DWORD attr { ::GetFileAttributesW(new_path.c_str()) };
            if (attr != INVALID_FILE_ATTRIBUTES)
            {
                if (path.ends_with(L".mes"))
                {
                    console::fmt::write<console::txt::dark_yellow>(L"[LOAD] %s\n", path.substr(pos + 1).data());
                }
                return new_path;
            }
            if (path.ends_with(L".mes"))
            {
                console::fmt::write(L"[LOAD] %s\n", path.substr(pos + 1).data());
            }
        }
        return {};
    }
    
    auto DC3WY::ReplacePathA(std::string_view path) -> std::string_view
    {
        static std::string new_path{};
        size_t pos{ path.find_last_of("\\/") };
        if (pos != std::wstring_view::npos)
        {
            new_path = std::string{ ".\\cn_Data" }.append(path.substr(pos));
            DWORD attr { ::GetFileAttributesA(new_path.c_str()) };
            if (attr != INVALID_FILE_ATTRIBUTES)
            {
                if (path.ends_with(".mes"))
                {
                    console::fmt::write<console::cdpg::dDfault, console::txt::dark_yellow>("[LOAD] %s\n", path.substr(pos + 1).data());
                }
                return new_path;
            }
            if (path.ends_with(".mes"))
            {
                console::fmt::write("[LOAD] %s\n", path.substr(pos + 1).data());
            }
        }
        return {};
    }

    static auto WINAPI CreateFileA(LPCSTR lpFN, DWORD dwDA, DWORD dwSM, LPSECURITY_ATTRIBUTES lpSA, DWORD dwCD, DWORD dwFAA, HANDLE hTF) -> HANDLE
    {
        std::string_view new_path{ DC3WY::ReplacePathA(lpFN) };
        return Patch::Hooker::Call<DC3WY::CreateFileA>(new_path.empty() ? lpFN : new_path.data(), dwDA, dwSM, lpSA, dwCD, dwFAA, hTF);
    }

    static auto WINAPI CreateFileW(LPCWSTR lpFN, DWORD dwDA, DWORD dwSM, LPSECURITY_ATTRIBUTES lpSA, DWORD dwCD, DWORD dwFAA, HANDLE hTF) -> HANDLE
    {
        std::wstring_view new_path{ DC3WY::ReplacePathW(lpFN) };
        return Patch::Hooker::Call<DC3WY::CreateFileW>(new_path.empty() ? lpFN : new_path.data(), dwDA, dwSM, lpSA, dwCD, dwFAA, hTF);
    }

    static auto WINAPI FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) -> HANDLE
    {
        std::string_view new_path{ DC3WY::ReplacePathA(lpFileName) };
        return Patch::Hooker::Call<DC3WY::FindFirstFileA>(new_path.empty() ? lpFileName : new_path.data(), lpFindFileData);
    }
    
    static auto WINAPI GetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuf, LPGLYPHMETRICS lpgm, DWORD cjbf, LPVOID pvbf, MAT2* lpmat) -> DWORD
    {
        if (tagTEXTMETRICA lptm{}; ::GetTextMetricsA(hdc, &lptm))
        {
            if (0xA1EC == uChar) // § -> ♪
            {
                HFONT font{ DC3WY::FontManager.GetJISFont(lptm.tmHeight) };
                if (font != nullptr)
                {
                    font = { reinterpret_cast<HFONT>(::SelectObject(hdc, font)) };
                    DWORD result{ ::GetGlyphOutlineW(hdc, L'♪', fuf, lpgm, cjbf, pvbf, lpmat) };
                    static_cast<void>(::SelectObject(hdc, font));
                    return result;
                }
            }

            if (uChar == 0x23) { uChar = 0x20; }

            HFONT font{ DC3WY::FontManager.GetGBKFont(lptm.tmHeight) };
            if (font != nullptr)
            {
                font = { reinterpret_cast<HFONT>(::SelectObject(hdc, font)) };
                DWORD result{ Patch::Hooker::Call<DC3WY::GetGlyphOutlineA>(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat) };
                static_cast<void>(::SelectObject(hdc, font));
                return result;
            }
        }
        return Patch::Hooker::Call<DC3WY::GetGlyphOutlineA>(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat);
    }

    static auto INIT_ALL_PATCH(void) -> void
    {
        console::make("DEBUG LOG FOR DC3WY");
        Patch::Hooker::Begin();
        Patch::Hooker::Add<DC3WY::CreateFileA>(::CreateFileA);
        //Patch::Hooker::Add<DC3WY::CreateFileW>(::CreateFileW);
        Patch::Hooker::Add<DC3WY::FindFirstFileA>(::FindFirstFileA);
        Patch::Hooker::Add<DC3WY::GetGlyphOutlineA>(::GetGlyphOutlineA);
        Patch::Mem::JmpWrite(0x404BFE, DC3WY::JmpSetNameIconEx);
        Patch::Mem::JmpWrite(0x432490, DC3WY::AudioStop_Hook);
        Patch::Mem::JmpWrite(0x431870, DC3WY::JmpAudioPlayHook);
        Patch::Hooker::Add<DC3WY::WndProc>(reinterpret_cast<void*>(0x40FC20));
        Patch::Hooker::Add<DC3WY::ComPlayVideo_Hook>(reinterpret_cast<void*>(0x444920));
        Patch::Hooker::Add<DC3WY::ComStopVideo_Hook>(reinterpret_cast<void*>(0x444640));
        Patch::Mem::MemWrite(0x49DF58, DC3WY::ChapterTitles, sizeof(DC3WY::ChapterTitles));
        Patch::Hooker::Commit();
    }
}

extern "C"
{
    __declspec(dllexport) auto hook(void) -> void {}

    __declspec(dllexport) auto _patch_by_iTsukezigen_(void) -> const char*
    {
        return { "https://github.com/cokkeijigen/dc3_cn" };
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
