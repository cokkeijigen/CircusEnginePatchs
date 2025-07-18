#pragma once
#include <detours.h>

namespace Patch {

	struct Hooker
	{
		template<auto Fun>
		inline static decltype(Fun) Call;

		inline static auto Begin() -> void
		{
			::DetourTransactionBegin();
		}

		template<auto Fun>
		inline static auto Add(decltype(Fun) target) -> void
		{
            ::DetourAttach({ &(Hooker::Call<Fun> = {target}) }, Fun);
		}

		template<auto Fun>
		inline static auto Add(const void* target) -> void
		{
			Hooker::Add<Fun>(reinterpret_cast<decltype(Fun)>(target));
		}

		inline static auto Commit() -> void
		{
			::DetourUpdateThread(::GetCurrentThread());
			::DetourTransactionCommit();
		}
	};

    namespace Mem {

        extern auto MemWriteImpl(LPVOID Addr, LPVOID Buf, size_t Size) -> bool;
        extern auto JmpWriteImpl(LPVOID OrgAddr, LPVOID TarAddr, BYTE OP = 0xE9) -> bool;
        extern auto GetBaseAddr() -> PVOID;

        template<class addr_t, class buf_t>
        requires requires(addr_t address, buf_t buffer)
        {
            LPVOID(address) && LPVOID(buffer);
        }
        inline static auto MemWrite(addr_t address, buf_t buffer, size_t size) -> bool
        {
            return MemWriteImpl(LPVOID(address), LPVOID(buffer), size);
        }

        template<class org_t, class tar_t>
        requires requires(org_t org_address, tar_t tar_address)
        {
            LPVOID(org_address) && LPVOID(tar_address);
        }
        inline static auto JmpWrite(org_t org_address, tar_t tar_address, uint8_t op = 0xE9) -> bool
        {
            return JmpWriteImpl(LPVOID(org_address), LPVOID(tar_address), op);
        }
    }

   
}

#ifdef _DEBUG
#define DEBUG_ONLY(...) __VA_ARGS__
#else
#define DEBUG_ONLY(...)
#endif
