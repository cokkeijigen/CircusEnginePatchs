#include <iostream>
#include <dc3wy.hpp>
#include <console.hpp>
#include <xtime.hpp>

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

    Utils::FontManager DC3WY::FontManager{};
    XSub::GDI::ImageSubPlayer* SubPlayer{};
    static IDirectSoundBuffer* CurrentPlayingBuffer{};
    static bool IsOpMoviePlaying{ false };

    auto CALLBACK DC3WY::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
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
                    { 0x1919810 },
                    { L"Backlog显示全部图标" }
                );
            }
            if (!DC3WY::FontManager.IsInit())
            {
                DC3WY::FontManager.Init(hWnd);
            }

            if (DC3WY::SubPlayer == nullptr)
            {
                DC3WY::SubPlayer =
                {
                    new XSub::GDI::ImageSubPlayer{ hWnd }
                };

                DC3WY::SubPlayer->SetDefualtPoint
                (
                    XSub::Point
                    {
                        .align{ XSub::Align::Center },
                    }
                );
            }
        }
        else if (uMsg == WM_SYSCOMMAND)
        {
            if (wParam == 0x114514)
            {
                if (DC3WY::FontManager.GUI() != nullptr)
                {
                    if (DC3WY::IsOpMoviePlaying)
                    {
                        ::MessageBoxW
                        (
                            { hWnd },
                            { L"当前无法更改字体！" },
                            { L"WARNING" },
                            { MB_OK }
                        );
                        return FALSE;
                    }
                    DC3WY::FontManager.GUIChooseFont();
                }
                return TRUE;
            }
            if (wParam == 0x1919810)
            {
                HMENU SystemMenu{ ::GetSystemMenu(hWnd, FALSE) };
                if (SystemMenu == nullptr)
                {
                    return FALSE;
                }
                auto mii = MENUITEMINFO
                {
                   .cbSize{ sizeof(MENUITEMINFO) },
                   .fMask{ MIIM_STATE }
                };
                ::GetMenuItemInfoW(SystemMenu, 0x1919810, FALSE, &mii);
                ::ModifyMenuW
                (
                    { SystemMenu },
                    { 0x1919810 },
                    {
                        mii.fState & MF_CHECKED ?
                        static_cast<UINT>(MF_UNCHECKED) :
                        static_cast<UINT>(MF_CHECKED)
                    },
                    { 0x1919810 },
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
        else if (uMsg == WM_MOVE)
        {
            if (DC3WY::SubPlayer != nullptr)
            {
                auto x{ static_cast<int>(LOWORD(lParam)) };
                auto y{ static_cast<int>(HIWORD(lParam)) };
                DC3WY::SubPlayer->SetPosition(x, y, true);
            }
        }
        else if (uMsg == WM_SIZE)
        {
            if (DC3WY::FontManager.GUI() != nullptr)
            {
                DC3WY::FontManager.GUIUpdateDisplayState();
            }

            if (DC3WY::SubPlayer != nullptr)
            {
                if (wParam == SIZE_MINIMIZED)
                {
                    DC3WY::SubPlayer->Hide();
                    DC3WY::SubPlayer->UpdateLayer();
                }
                else if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
                {
                    DC3WY::SubPlayer->Show();
                    DC3WY::SubPlayer->SyncToParentWindow(true);
                }

                auto size{ DC3WY::SubPlayer->GetCurrentSize() };
                DEBUG_ONLY(console::fmt::write("Size{ .cx=%d .cy=%d }\n", size.cx, size.cy));
            }
        }
        else if (uMsg == WM_SIZING)
        {
            if (DC3WY::SubPlayer != nullptr)
            {
                DC3WY::SubPlayer->SyncToParentWindow(true);
            }
        }
        return Patch::Hooker::Call<DC3WY::WndProc>(hWnd, uMsg, wParam, lParam);
    }

    static auto LoadXSubAndPlayIfExist(std::string_view file, bool play) -> bool
    {
        if (DC3WY::SubPlayer == nullptr)
        {
            return { false };
        }

        std::string path{ ".\\cn_Data\\" };

        auto pos{ file.rfind("\\") };
        if (pos != std::string_view::npos)
        {
            path.append(file.substr(pos + 1)).append(".xsub");
        }
        else
        {
            path.append(file).append(".xsub");
        }

        bool is_load { DC3WY::SubPlayer->Load(path) };
        if (is_load)
        {
            DEBUG_ONLY
            ({
                console::fmt::write<console::cdpg::dDfault, console::txt::dark_yellow>
                (
                    "[LOAD] %s\n", path.data()
                );
            })
            if (play) { DC3WY::SubPlayer->Play(); }
        }
        return { is_load };
    }

    static auto __fastcall AudioRelease(int32_t* m_this, int32_t, uint32_t index) -> DWORD
    {
        auto RawCall{ reinterpret_cast<decltype(DC3WY::AudioRelease)*>(0x432000) };
        return RawCall(m_this, NULL, index);
    }

    auto __fastcall DC3WY::AudioStop_Hook(int32_t* m_this, int32_t, uint32_t index) -> DWORD
    {
        auto UnknownPtr{ reinterpret_cast<uintptr_t*>(0x4A95A4) };
        auto DirectSoundBuffers{ reinterpret_cast<IDirectSoundBuffer**>(0x4A95EC) };

        m_this[0x05 * index + 0x15] = 0x00;
        if (index - 0x01 <= 0x03 && *UnknownPtr != NULL)
        {
            auto uint8_ptr{ reinterpret_cast<uint8_t*>(*UnknownPtr) };
            *(uint8_ptr + (0x20 * index) + 0x4D10) = 0x00;
        }

        auto buffer{ DirectSoundBuffers[index] };
        if (buffer != nullptr)
        {
            auto is_current_playing_buffer
            {
                DC3WY::CurrentPlayingBuffer != nullptr &&
                buffer == DC3WY::CurrentPlayingBuffer
            };
            if (is_current_playing_buffer)
            {
                DC3WY::SubPlayer->Stop();
                DC3WY::SubPlayer->UnLoad();
                DC3WY::CurrentPlayingBuffer = { nullptr };
            }
            buffer->SetCurrentPosition(0x00);
            buffer->Stop();
            auto result{ DC3WY::AudioRelease(m_this, NULL, index) };
            return { result };
        }
        return { NULL };
    }
    

    static auto __stdcall AudioPlay_Hook(const char* file, uint32_t flag, uint32_t index) -> int
    {
        auto UnknownPtr{ reinterpret_cast<uintptr_t*>(0x4A95A4) };
        auto DirectSoundBuffers{ reinterpret_cast<IDirectSoundBuffer**>(0x4A95EC) };
        if (index == 0x01)
        {
            *reinterpret_cast<uint32_t*>(*UnknownPtr + 0x136C) = flag;
        }
        auto buffer{ DirectSoundBuffers[index] };
        if (buffer != nullptr)
        {
            auto result{ buffer->Play(0x00, 0x00, flag != 0x00) };
            if (DC3WY::CurrentPlayingBuffer == nullptr)
            {
                auto is_load{ DC3WY::LoadXSubAndPlayIfExist(file, true) };
                if (is_load)
                {
                    DC3WY::CurrentPlayingBuffer = { buffer };
                }
            }
            return { result };
        }
        return { NULL };
    }

    auto DC3WY::ComPlayVideo_Hook(void) -> int32_t
    {
        std::string_view movie_file_path{ reinterpret_cast<const char*>(0x4E65F8) };
        if (!movie_file_path.empty())
        {
            std::string_view new_path{ DC3WY::ReplacePathA(movie_file_path) };
            if (!new_path.empty())
            {
                auto dest{ const_cast<char*>(movie_file_path.data()) };
                std::copy(new_path.begin(), new_path.end(), dest);
                dest[new_path.size()] = {};
            }
            else
            {
                auto pos{ movie_file_path.rfind("\\") };
                if (pos != std::string_view::npos)
                {
                    auto name{ movie_file_path.substr(pos + 1) };
                    if (name.size() >= 7)
                    {
                        auto is_gop_mpg
                        {
                            (name[0] == 'g' || name[0] == 'G') &&
                            (name[1] == 'o' || name[1] == 'O') &&
                            (name[2] == 'p' || name[2] == 'P') &&
                            (name[3] == '.') &&
                            (name[4] == 'm' || name[4] == 'M') &&
                            (name[5] == 'p' || name[5] == 'P') &&
                            (name[6] == 'g' || name[6] == 'G')
                        };
                        if (is_gop_mpg) // gop.mpg
                        {
                            DC3WY::LoadXSubAndPlayIfExist("274", false);
                        }
                        else if(name.size() == 8)
                        {
                            auto is_op_0_mpg
                            {
                                (name[0] == 'o' || name[0] == 'O') &&
                                (name[1] == 'p' || name[1] == 'P') &&
                                (name[2] == '0' && name[4] == '.') &&
                                (name[5] == 'm' || name[5] == 'M') &&
                                (name[6] == 'p' || name[6] == 'P') &&
                                (name[7] == 'g' || name[7] == 'G')
                            };
                            if (is_op_0_mpg)
                            {
                                if (name[3] == '1') // op01.mpg
                                {
                                    DC3WY::LoadXSubAndPlayIfExist("270", false);
                                }
                                else if (name[3] == '2') // op02.mpg
                                {
                                    DC3WY::LoadXSubAndPlayIfExist("272", false);
                                }
                            }
                        }
                    }
                }
            }

            DEBUG_ONLY(console::fmt::write("[DC3WY::ComPlayVideo_Hook] %s\n", movie_file_path.data()));
        }
        auto result{ Patch::Hooker::Call<DC3WY::ComPlayVideo_Hook>() };

        DC3WY::IsOpMoviePlaying = { result >= 0 };

        if (DC3WY::IsOpMoviePlaying && DC3WY::SubPlayer != nullptr)
        {
            auto is_load { DC3WY::SubPlayer->IsLoad() };
            if (is_load)
            {
                DC3WY::SubPlayer->UseDefualtAlign(true);
                DC3WY::SubPlayer->UseDefualtHorizontal();
                DC3WY::SubPlayer->Play();
            };
        }

        if (DC3WY::FontManager.GUI() != nullptr)
        {
            DC3WY::FontManager.GUI()->HideWindow();
        }
        
        return { result };
    }

    auto DC3WY::ComStopVideo_Hook(void) -> int32_t
    {
        if (DC3WY::SubPlayer != nullptr)
        {
            DC3WY::SubPlayer->Stop();
            DC3WY::SubPlayer->UnLoad();
            DC3WY::SubPlayer->UnuseDefualtPoint();
        }
        DC3WY::IsOpMoviePlaying = { false };
        DEBUG_ONLY(console::fmt::write("[DC3WY::ComStopVideo]\n"));
        auto result{ Patch::Hooker::Call<DC3WY::ComStopVideo_Hook>() };
        return { result };
    }

    static auto __stdcall SetNameIconEx(const char* name, int& line, int& row) -> BOOL
    {
        if (DC3WY::EnableBacklogAllIcon)
        {
            std::string_view _name{ name };
            if (_name.size() != 0)
            {
                line = 3;
                row  = 4;
                return { static_cast<BOOL>(true) };
            }
        }
        return { static_cast<BOOL>(false) };
    }

    __declspec(naked) auto DC3WY::JmpAudioPlayHook(void) -> void
     {
        __asm
        {
            sub esp, 0x04
            mov eax, dword ptr ss:[esp+0x04]  // ret addr
            mov dword ptr ss:[esp], eax
            mov eax, dword ptr ss:[esp+0x130] // file name
            mov dword ptr ss:[esp+0x04], eax
            jmp DC3WY::AudioPlay_Hook
        }
     }

    __declspec(naked) auto DC3WY::JmpSetNameIconEx(void) -> void
    {
        __asm
        {
            sub esp, 0x08                     // 使用栈内存来存放存放line和row的结果
            mov dword ptr ss:[esp], 0x00      // int row  = 0x00;
            mov dword ptr ss:[esp+0x04], 0x00 // int line = 0x00;
            lea eax, dword ptr ss:[esp]
            push eax                          // push &row
            lea eax, dword ptr ss:[esp+0x08]
            push eax                          // push &line
            lea eax, dword ptr ss:[esp+0x4C] 
            push eax                          // push name
            call DC3WY::SetNameIconEx         // DC3WY::SetNameIconEx(name, &line, &row)
            test eax, eax                     
            jnz _succeed                      // if(result) goto _succeed
            add esp, 0x08                     // 恢复堆栈（释放变量内存）
            mov dl, byte ptr ds:[0x004795BA]  // 搬运原地址的指令
            mov eax, 0x00404C04               // 跳转回去原来的地址
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
}
