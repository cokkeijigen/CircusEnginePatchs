#include "framework.h"
#include "dc3rx.h"

string szWndName ("");
string verMSG("");

HWND WINAPI NewCreateWindowExA(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, 
    int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam){
    if (strlen(szWndName.c_str()) > 5) lpWindowName = (LPCTSTR)szWndName.c_str();
    return oldCreateWindowExA(dwExStyle, lpClassName, lpWindowName,
        dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam
    );
}

HANDLE WINAPI NewCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile){
    string newName(lpFileName);
    ReplacePathA(&newName);
    return oldCreateFileA(
        newName.c_str(),  dwDesiredAccess, dwShareMode, lpSecurityAttributes,
        dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile
    );
}

HANDLE WINAPI newCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,HANDLE hTemplateFile) {
    wstring newName(lpFileName);
    ReplacePathW(&newName);
    return oldCreateFileW(
            newName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes,
            dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile
    );
};

HANDLE WINAPI NewFindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) {
    string newName(lpFileName);
    ReplacePathA(&newName);
    return oldFindFirstFileA(newName.c_str(), lpFindFileData);
}

HANDLE WINAPI newMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {

    if (lpText && strstr(lpText, "僙乕僽椞堟偑偁傝傑偣傫") && strstr(lpText, "傪嶌惉偟偰傛傠偟偄偱偡偐丠")){
        unsigned char len = strlen(lpText);
        string nwlpText(lpText);
        nwlpText.assign(nwlpText.substr(24, len - 50));
        nwlpText.insert(0, "找不到存档目录\n\n是否在");
        nwlpText.append("创建目录？");
        return oldMessageBoxA(hWnd, nwlpText.c_str(), "首次启动", uType);
    }
    else if (lpText && !strcmp(lpText, "惓忢偵梀傇偨傔偵偼丄惓偟偔僀儞僗僩乕儖偟偰偔偩偝偄"))
        return oldMessageBoxA(hWnd, "如需正常游玩，请正确启动游戏！", "警告", uType);

    else if (lpText && !strcmp(lpText, "廔椆偟傑偡偐丠"))
        return oldMessageBoxA(hWnd, "要结束游戏吗？", "提示", uType);

    return oldMessageBoxA(hWnd, lpText, lpCaption, uType);
}

DWORD WINAPI newGetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuFormat, LPGLYPHMETRICS lpgm,
    DWORD cjBuffer, LPVOID pvBuffer, MAT2* lpmat) {
    tagTEXTMETRICA lptm;
    LONG tmHeight;
    GetTextMetricsA(hdc, &lptm);
    tmHeight = lptm.tmHeight;
    Dc3Font* font = GetFontStruct(tmHeight);
    if (uChar == 0xA1EC) {
        uChar = 0x81F4; // 日文音符
        if (!font->jisFont) 
            font->jisFont = CreateFontA(
                tmHeight, tmHeight / 2, 0, 0, 0, 0, 0, 0, 0x80, 4, 0x20, 4, 4, "黑体"
            );
        SelectObject(hdc, font->jisFont);
    }
    else SelectObject(hdc, font->chsFont);
    if (uChar == 0x23) uChar = 0x20;        // 半角空格
    if (uChar == 0x8140) uChar = 0xA1A1;    // 全角空格
    return oldGetGlyphOutlineA(hdc, uChar, fuFormat, lpgm, cjBuffer, pvBuffer, lpmat);
}

LRESULT WINAPI newSendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM IParam) {
    const char* oldptr = (const char*)IParam;
    if(oldptr && strlen(verMSG.c_str()) > 5 && strstr(oldptr, "丂僨乕僞") &&
        strstr(oldptr, "D.C.嘨 R X-rated 乣僟丒僇乕億嘨 傾乕儖乣 X-rated")) {
        IParam = (LPARAM)verMSG.c_str();
    }
    return oldSendMessageA(hWnd, Msg, wParam, IParam);
}

bool initComplete() {

    const char* chsinfo = "cn_Data\\_chinese_info.ini";
    if (GetFileAttributesA(chsinfo) == INVALID_FILE_ATTRIBUTES)
        return false;
    FILE * chsinfo_fp = fopen(chsinfo, "r");
    if (chsinfo_fp == NULL) return false;
    else {
        szWndName.append(getTitle(chsinfo_fp));
        verMSG.append(getVerMSG(chsinfo_fp));
        fclose(chsinfo_fp);
    }

    if (_gdi32 == NULL || _user32 == NULL || _kernel32 == NULL || BaseAddr == NULL)
        return false;
    
    oldCreateWindowExA = (_CreateWindowExA)GetProcAddress(_user32, "CreateWindowExA");
    oldCreateFileA = (_CreateFileA)GetProcAddress(_kernel32, "CreateFileA");
    oldCreateFileW = (_CreateFileW)GetProcAddress(_kernel32, "CreateFileW");
    oldFindFirstFileA = (_FindFirstFileA)GetProcAddress(_kernel32, "FindFirstFileA");
    oldMessageBoxA = (_MessageBoxA)GetProcAddress(_user32, "MessageBoxA");
    oldGetGlyphOutlineA = (_GetGlyphOutlineA)GetProcAddress(_gdi32, "GetGlyphOutlineA");
    oldSendMessageA = (_SendMessageA)GetProcAddress(_user32, "SendMessageA");

    if (oldCreateWindowExA == NULL || oldFindFirstFileA == NULL 
        || oldMessageBoxA == NULL  || oldCreateFileA == NULL 
        || oldCreateFileW == NULL  || oldGetGlyphOutlineA == NULL
        || oldSendMessageA == NULL
        ) return false;

    return true;
}

void initHookLoader() {
    //make_console();
    if (initComplete()) { 
        _dataInit();
        DetourTransactionBegin();
        DetourAttach(&oldCreateWindowExA, NewCreateWindowExA);
        DetourAttach(&oldCreateFileA, NewCreateFileA);
        DetourAttach(&oldCreateFileW, newCreateFileW);
        DetourAttach(&oldFindFirstFileA, NewFindFirstFileA);
        DetourAttach((void**)&oldMessageBoxA, newMessageBoxA);
        DetourAttach((void**)&oldGetGlyphOutlineA, newGetGlyphOutlineA);
        DetourAttach((void**)&oldSendMessageA, newSendMessageA);
        DetourUpdateThread(GetCurrentThread());
        DetourTransactionCommit();
        return;
    }
    MessageBoxW(NULL, L"补丁初始化失败！", L"警告", MB_OK | MB_ICONINFORMATION);
    exit(0);
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            initHookLoader();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
