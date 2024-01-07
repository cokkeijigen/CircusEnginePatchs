#include <iostream>
#include <thread>
#include <windows.h>
#include <dsound.h>
#include "dc3wy.h"

namespace Dc3wy::subtitle {

    intptr_t SubWndProcPtr = NULL;
    using WndProc = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
    static WndProc MainWndProc = NULL;
    static HWND hStaticText  = NULL;
    static HWND SubtitleWnd  = NULL;
    static double playedTime = 0.0l;
    static IDirectSoundBuffer* pDsBuffer;

    static LRESULT CALLBACK SubWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_CREATE) { 
        
        }
        if (message == 114514) {
       
        }
        else {
            return MainWndProc(hWnd, message, wParam, lParam);
        }
    }
    
    static void init(intptr_t base) {
        Dc3wy::subtitle::SubWndProcPtr = (intptr_t)SubWndProc;
        Dc3wy::subtitle::MainWndProc = (WndProc)(base + 0x0FC20);
    }

    static void run() {
        if(!pDsBuffer) return;
        WAVEFORMATEX format{};
        pDsBuffer->GetFormat(&format, sizeof(WAVEFORMATEX), NULL);
        // 计算采样率和帧大小
        DOUBLE samplesPerSecond = (DOUBLE)format.nSamplesPerSec;
        DOUBLE bytesPerSample = (DOUBLE)format.wBitsPerSample / 8;
        DOUBLE channels = (DOUBLE)format.nChannels;
        while (pDsBuffer) {
            // 获取当前播放位置
            DWORD playCursor = 0x00, writeCursor = 0x00;
            pDsBuffer->GetCurrentPosition(&playCursor, &writeCursor);
            DOUBLE playedSamples = (playCursor * 8) / (channels * bytesPerSample);
            playedTime = playedSamples / samplesPerSecond / 10;
            //PostMessageA(hStaticText, 114514, NULL, NULL);
            printf("\r　PlayedTime: %f", playedTime);
            //std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        printf("\n\n");
    }

    static void test() {
        std::thread(subtitle::run).detach();
    }

    void destroy() {

    }
}

namespace Dc3wy {
    
    using Sub32000 = DWORD(__thiscall*)(VOID*, DWORD);
    static Sub32000 dsBufferRelease = NULL;
    static char* current_audio_name = NULL;
    static DWORD* pDsBufferArray = NULL;
    static DWORD dword_a95a4 = NULL;

    struct hook {
        
        using PDSB = IDirectSoundBuffer*;
        
        static int __stdcall audio_play(int a1, int i) {
            /*printf(
                "[audio_play] v1: 0x%02X v2: 0x%02X file: %s\n", 
                a1, a2, current_audio_name
            );*/
            if (i == 0x01) {
                DWORD Buf = *(DWORD*)dword_a95a4;
                *(DWORD*)(Buf + 0x136C) = a1;
            }
            if (PDSB pDSB = (PDSB)pDsBufferArray[i]) {
                if (!strncmp("music", current_audio_name, 0x05)) {
                    printf("Current: [%s]\n", current_audio_name);
                    subtitle::pDsBuffer = pDSB;
                    subtitle::test();
                }
                return pDSB->Play(0x00, 0x00, a1 != 0x00);
            }
            return NULL;
        }

        int __thiscall audio_stop(int i) {
            DWORD Buf = *(DWORD*)dword_a95a4;
            if ((unsigned int)(i - 0x01) <= 0x03 && Buf) {
                *((BYTE*)Buf + 0x20 * i + 0x4D10) = 0x00;
            }
            ((DWORD*)this)[0x05 * i + 0x15] = 0x00;
            if (PDSB pDSB = (PDSB)pDsBufferArray[i]) {
                if (pDSB == subtitle::pDsBuffer) {
                    subtitle::pDsBuffer = nullptr;
                }
                pDSB->SetCurrentPosition(0x00);
                pDSB->Stop();
                return dsBufferRelease(this, i);
            }
            return NULL;
        }
    };

    __declspec(naked) void jmp_audio_play_hook() {
        __asm {
            push eax
            mov eax, dword ptr ss:[esp + 0x130]
            mov current_audio_name, eax
            mov eax, dword ptr ss:[esp]
            add esp, 0x04
            jmp hook::audio_play
        }
    }

    __declspec(naked) void jmp_audio_stop_hook() {
        __asm jmp hook::audio_stop
    }


    void jmp_hook_init(intptr_t base) {
        Dc3wy::dsBufferRelease = (Sub32000)(base + 0x32000);
        Dc3wy::pDsBufferArray  = (DWORD*)(base + 0xA95EC);
        Dc3wy::dword_a95a4 = base + 0xA95A4;
        subtitle::init(base);
    }
    
}

