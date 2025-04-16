#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <iostream>
#include <vector>
#include "detours.h"
#pragma comment(lib, "detours.lib")
using namespace std;

extern "C" __declspec(dllexport) void handle(void) {}

DWORD BaseAddr = (DWORD)GetModuleHandle(NULL);
HMODULE _gdi32 = GetModuleHandle(L"gdi32.dll");
HMODULE _user32 = GetModuleHandle(L"user32.dll");
HMODULE _kernel32 = GetModuleHandle(L"kernel32.dll");

typedef HWND(WINAPI* _CreateWindowExA)(
	DWORD dwExStyle,
	LPCTSTR lpClassName,
	LPCTSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam);
_CreateWindowExA oldCreateWindowExA;

typedef HANDLE(WINAPI* _CreateFileA)(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
	);
_CreateFileA oldCreateFileA;

typedef HANDLE(WINAPI* _CreateFileW)(
	 LPCWSTR lpFileName,
	 DWORD dwDesiredAccess,
	 DWORD dwShareMode,
     LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	 DWORD dwCreationDisposition,
	 DWORD dwFlagsAndAttributes,
	 HANDLE hTemplateFile
	);
_CreateFileW oldCreateFileW;

typedef HANDLE(WINAPI* _FindFirstFileA)(
	LPCSTR lpFileName,
	LPWIN32_FIND_DATAA lpFindFileData
	);
_FindFirstFileA oldFindFirstFileA;

typedef HFONT(WINAPI* _MessageBoxA)(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_ UINT uType
);
_MessageBoxA oldMessageBoxA;

typedef DWORD(WINAPI* _GetGlyphOutlineA)(
	_In_ HDC hdc,
	_In_ UINT uChar,
	_In_ UINT fuFormat,
	_Out_ LPGLYPHMETRICS lpgm,
	_In_ DWORD cjBuffer,
	_Out_writes_bytes_opt_(cjBuffer) LPVOID pvBuffer,
	_In_ CONST MAT2* lpmat2);
_GetGlyphOutlineA oldGetGlyphOutlineA;

typedef LRESULT(WINAPI* _SendMessageA)(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_Pre_maybenull_ _Post_valid_ WPARAM wParam,
	_Pre_maybenull_ _Post_valid_ LPARAM lParam);
_SendMessageA oldSendMessageA;

void ReplacePathA(string* path){
	string fileName(path->substr(path->find_last_of("\\") + 1));
	string newPath;
	if (
		GetFileAttributesA((newPath.assign("cn_Data\\" + fileName)).c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributesA((newPath.assign("cn_Data\\SYSTEM\\" + fileName)).c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributesA((newPath.assign("cn_Data\\MOVIE\\" + fileName)).c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributesA((newPath.assign("cn_Data\\MES\\" + fileName)).c_str()) != INVALID_FILE_ATTRIBUTES
	)	path->assign(newPath);
}

void ReplacePathW(wstring * path) {
	wstring fileName(path->substr(path->find_last_of(L"\\") + 1));
	wstring newPath;
	if (
		GetFileAttributesW((newPath.assign(L"cn_Data\\" + fileName)).c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributesW((newPath.assign(L"cn_Data\\SYSTEM\\" + fileName)).c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributesW((newPath.assign(L"cn_Data\\MOVIE\\" + fileName)).c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributesW((newPath.assign(L"cn_Data\\MES\\" + fileName)).c_str()) != INVALID_FILE_ATTRIBUTES
	)	path->assign(newPath);
}

struct Dc3Font {
	HFONT jisFont = nullptr;
	HFONT chsFont = nullptr;
	int size = 0;
};
vector<Dc3Font*> FontVector;
Dc3Font* GetFontStruct(int size) {
	for (auto iter = FontVector.begin(); iter != FontVector.end(); iter++)
		if ((*iter)->size == size) return *iter;
	Dc3Font* font = new Dc3Font;
	font->size = size;
	font->chsFont = CreateFontA(
		size, size / 2, 0, 0, 0, 0, 0, 0, 0x86, 4, 0x20, 4, 4, "黑体"
	);
	FontVector.push_back(font);
	return font;
}

bool OverWriteAdress(const char* adress, const char* buf, int len) {
	DWORD oldati1, oldati2;
	if (VirtualProtect((LPVOID)adress, len, PAGE_EXECUTE_READWRITE, &oldati1)) {
		SIZE_T wirtebyte = 0;
		WriteProcessMemory(INVALID_HANDLE_VALUE, (LPVOID)adress, buf, len, &wirtebyte);
		VirtualProtect((LPVOID)adress, len, oldati1, &oldati2);
		if (len != wirtebyte) return false;
		else return true;
	}
	return false;
}

void make_console()
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
}

