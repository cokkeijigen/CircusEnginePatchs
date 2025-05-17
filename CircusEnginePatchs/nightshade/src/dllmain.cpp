#include <iostream>
#include <windows.h>
#include <patch.hpp>
#include <console.hpp>

namespace NIGHTSHADE
{

    static auto ReplaceCharacter(UINT& uChar) -> bool
    {
        switch (uChar)
        {
        case 0x8441: // Б 
            uChar = L'¿';
            return { true };
        case 0x8443: // Г 
            uChar = L'¡';
            return { true };
        case 0x8444: // Д 
            uChar = L'á';
            return { true };
        case 0x8446: // Ё 
            uChar = L'é';
            return { true };
        case 0x8447: // Ж 
            uChar = L'í';
            return { true };
        case 0x8448: // З 
            uChar = L'ó';
            return { true };
        case 0x8449: // И 
            uChar = L'ú';
            return { true };
        case 0x844A: // Й 
            uChar = L'ü';
            return { true };
        case 0x844B: // К 
            uChar = L'ñ';
            return { true };
        case 0x844C: // Л 
            uChar = L'ç';
            return { true };
        case 0x844D: // М 
            uChar = L'Á';
            return { true };
        case 0x844E: // Н 
            uChar = L'É';
            return { true };
        case 0x844F: // О 
            uChar = L'Í';
            return { true };
        case 0x8450: // П 
            uChar = L'Ó';
            return { true };
        case 0x8451: // Р 
            uChar = L'Ú';
            return { true };
        case 0x8452: // С 
            uChar = L'Ü';
            return { true };
        case 0x8453: // Т 
            uChar = L'Ñ';
            return { true };
        case 0x8454: // У 
            uChar = L'Ç';
            return { true };
        case 0x8455: // Ф 
            uChar = L'º';
            return { true };
        case 0x8456: // Х 
            uChar = L'ª';
            return { true };
        }

        return { false };
    }

    static auto WINAPI GetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuf, LPGLYPHMETRICS lpgm, DWORD cjbf, LPVOID pvbf, MAT2* lpmat) -> DWORD
    {
        //DEBUG_ONLY(console::fmt::write("uChar{ 0x%X }\n", uChar));
        return DWORD
        {
            NIGHTSHADE::ReplaceCharacter(uChar) ?
            ::GetGlyphOutlineW(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat) :
            Patch::Hooker::Call<NIGHTSHADE::GetGlyphOutlineA>(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat)
        };
    }

    static auto INIT_ALL_PATCH(void) -> void
    {
        DEBUG_ONLY(console::make());
        Patch::Hooker::Begin();
        Patch::Hooker::Add<NIGHTSHADE::GetGlyphOutlineA>(::GetGlyphOutlineA);
        Patch::Hooker::Commit();
    }
}

extern "C"
{
    __declspec(dllexport) auto hook(void) -> void {}

    __declspec(dllexport) auto _patch_by_iTsukezigen_(void) -> const char*
    {
        return { "https://github.com/cokkeijigen/circus_engine_patchs/" };
    }

    auto APIENTRY DllMain(HMODULE, DWORD ul_reason_for_call, LPVOID) -> BOOL
    {
        if (DLL_PROCESS_ATTACH == ul_reason_for_call)
        {
            NIGHTSHADE::INIT_ALL_PATCH();
        }
        return TRUE;
    }
}
