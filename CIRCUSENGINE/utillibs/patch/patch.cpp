#include <iostream>
#include <windows.h>
#include <patch.hpp>

namespace Patch::Mem {

    auto MemWriteImpl(LPVOID Addr, LPVOID Buf, size_t Size) -> bool
    {
        DWORD  Protect{ NULL };
        SIZE_T Written{ NULL };
        if (::VirtualProtect(Addr, Size, PAGE_EXECUTE_READWRITE, &Protect))
        {
            static_cast<void>(WriteProcessMemory(INVALID_HANDLE_VALUE, Addr, Buf, Size, &Written));
            static_cast<void>(VirtualProtect(Addr, Size, Protect, &Protect));
            return Size == Written;
        }
        return false;
    }

    auto JmpWriteImpl(LPVOID OrgAddr, LPVOID TarAddr, BYTE OP) -> bool
    {
        BYTE Jmp[5] = { OP, 0x0, 0x0, 0x0, 0x0 };
        auto Tar { reinterpret_cast<DWORD>(TarAddr) };
        auto Org { reinterpret_cast<DWORD>(OrgAddr) };
        auto Addr{ static_cast<DWORD>(Tar - Org - 5) };
        *reinterpret_cast<DWORD*>(Jmp + 1) = Addr;
        return MemWriteImpl(OrgAddr, &Jmp, 5);
    }

    auto GetBaseAddr() -> PVOID
    {
        static PVOID BaseAddr
        {
            ::GetModuleHandleW(NULL)
        };
        
        return BaseAddr;
    }
}
