#include <iostream>
#include <windows.h>
#include <dc3wy.hpp>
#include <patch.hpp>
#include <console.hpp>
#include <fontmanager.hpp>

namespace DC3WY {

    constexpr inline auto EnableBacklogAllIconFile
    {
        ".\\cn_Data\\EnableBacklogAllIcon"
    };

    static bool EnableBacklogAllIcon
    {
        ::GetFileAttributesA(EnableBacklogAllIconFile)
        != INVALID_FILE_ATTRIBUTES
    };

    static Utils::FontManager FontManager{};

    static auto CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
    {
        if (uMsg == WM_CREATE) 
        {
            ::SetWindowTextW(hWnd, DC3WY::TitleName);
            HMENU SystemMenu{ ::GetSystemMenu(hWnd, FALSE) };
            if (SystemMenu != nullptr)
            {
                ::AppendMenuW(SystemMenu, MF_UNCHECKED, 0x114514, L"更改字体");
                ::AppendMenuW
                (
                    { SystemMenu },
                    {
                         DC3WY::EnableBacklogAllIcon ?
                         static_cast<UINT>(MF_CHECKED) :
                         static_cast<UINT>(MF_UNCHECKED)
                    },
                    { 0x1919 },
                    { L"Backlog显示全部图标" }
                );
            }
            if (DC3WY::FontManager.GUI() == nullptr)
            {
                DC3WY::FontManager.Init(hWnd);
            }
        }
        else if (uMsg == WM_SIZE && DC3WY::FontManager.GUI() != nullptr)
        {
            DC3WY::FontManager.GUIUpdateDisplayState();
        }
        else if (uMsg == WM_SYSCOMMAND)
        {
            if (wParam == 0x114514)
            {
                if (DC3WY::FontManager.GUI() != nullptr)
                {
                    DC3WY::FontManager.GUIChooseFont();
                }
                return TRUE;
            }
            if (wParam == 0x1919)
            {
                HMENU SystemMenu{ GetSystemMenu(hWnd, FALSE) };
                if (SystemMenu == nullptr)
                {
                    return FALSE;
                }
                auto mii = MENUITEMINFO
                {
                   .cbSize = sizeof(MENUITEMINFO),
                   .fMask = MIIM_STATE
                };
                ::GetMenuItemInfoW(SystemMenu, 0x1919, FALSE, &mii);
                ::ModifyMenuW
                (
                    { SystemMenu },
                    { 0x1919 },
                    {
                        mii.fState & MF_CHECKED ?
                        static_cast<UINT>(MF_UNCHECKED) :
                        static_cast<UINT>(MF_CHECKED)
                    },
                    { 0x1919 },
                    { L"Backlog显示全部图标" }
                );

                if (mii.fState & MF_CHECKED)
                {
                    ::DeleteFileA(DC3WY::EnableBacklogAllIconFile);
                    DC3WY::EnableBacklogAllIcon = { false };
                }
                else
                {
                    auto hFile = HANDLE
                    {
                        ::CreateFileA
                        (
                            { DC3WY::EnableBacklogAllIconFile },
                            { GENERIC_WRITE },
                            { NULL },
                            { NULL },
                            { CREATE_ALWAYS },
                            { FILE_ATTRIBUTE_NORMAL },
                            { NULL }
                        )
                    };
                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        ::CloseHandle(hFile);
                    }
                    DC3WY::EnableBacklogAllIcon = { true };
                }
                return TRUE;
            }
        }
        return Patch::Hooker::Call<DC3WY::WndProc>(hWnd, uMsg, wParam, lParam);
    }

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
    
    static auto ReplacePathA(std::string_view path) -> std::string_view
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
        std::string_view new_path { DC3WY::ReplacePathA(lpFN) };
        return Patch::Hooker::Call<DC3WY::CreateFileA>(new_path.empty()? lpFN : new_path.data(), dwDA, dwSM, lpSA, dwCD, dwFAA, hTF);
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

    static auto __stdcall SetNameIconEx(const char* name, int& line, int& row) -> int
    {
        if (DC3WY::EnableBacklogAllIcon)
        {
            std::string_view _name{ name };
            if (_name.size() != 0)
            {
                line = 3;
                row  = 4;
                return { static_cast<int>(true) };
            }
        }
        return { static_cast<int>(false) };
    }

    static __declspec(naked) auto JmpSetNameIconEx(void) -> void
    {
        __asm
        {
            sub esp, 0x08                     // 使用栈内存来存放存放line和row的结果
            mov dword ptr ss:[esp], 0x00      // 初始化为0
            mov dword ptr ss:[esp+0x04], 0x00 // 初始化为0
            lea eax, dword ptr ss:[esp]         
            push eax // push 用于存放row的栈内存地址
            lea eax, dword ptr ss:[esp+0x08]
            push eax // push 用于存放line的栈内存地址
            lea eax, dword ptr ss:[esp+0x4C]
            push eax // push 角色名字
            call DC3WY::SetNameIconEx
            test eax, eax
            jnz _succeed
            add esp, 0x08
            mov dl, byte ptr ds:[0x004795BA]
            mov eax, 0x00404C04
            jmp eax
        }
    _succeed:
        __asm
        {
            mov edi, dword ptr ss:[esp]    // row
            mov eax, dword ptr ss:[esp+4]  // line
            add esp, 0x08
            mov dword ptr ss:[esp+0x10], eax
            mov eax, 0x00404D2E
            jmp eax
        }
    }

	auto DC3WY::INIT_ALL_PATCH(void) -> void
	{
		console::make("DEBUG LOG FOR DC3WY");
        Patch::Hooker::Begin();
        Patch::Hooker::Add<DC3WY::CreateFileA>(::CreateFileA);
        Patch::Hooker::Add<DC3WY::CreateFileW>(::CreateFileW);
        Patch::Hooker::Add<DC3WY::FindFirstFileA>(::FindFirstFileA);
        Patch::Hooker::Add<DC3WY::GetGlyphOutlineA>(::GetGlyphOutlineA);
        Patch::Hooker::Add<DC3WY::WndProc>(reinterpret_cast<void*>(0x40FC20));
        Patch::Mem::JmpWrite(0x404BFE, DC3WY::JmpSetNameIconEx);
        Patch::Mem::MemWrite(0x49DF58, DC3WY::ChapterTitles, sizeof(DC3WY::ChapterTitles));
        Patch::Hooker::Commit();
	}

}
