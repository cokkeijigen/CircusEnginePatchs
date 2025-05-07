# DC3WY 中文本地化笔记

| 工具 |说明 |
|:---|:---|
| [MesTextTool](https://github.com/cokkeijigen/MesTextTool) | Mes文本的提取导入 |
|[sagilio::GARbro](https://github.com/sagilio/GARbro)| *.CRX图片类型封装转换|
| [x32dbg](https://x64dbg.com/) | 静态动态调试分析 |
| IDA 9 PRO | 静态逆向分析 |
| CFF Explorer| 修改IAT |

## 0x00 去除DVD和KEY验证
### 首先是DVD验证，这可以直接搜索字符`DVD`串或者断`MessageBoxA`就能找到位置。 <br>
- 可以发现上面有个跳转是调到下面弹出验证消息的逻辑，直接`nop`掉跳转即可
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_00.png) <br>
### 其次是Key验证弹窗，这个可以搜字符串`InstKey`找到，但是存在多个位置，不好判断具体是哪个调用了，所以直接上`x32dbg`，这里直接断`DialogBoxParamA`<br>
- 此时找到`sub_40EA30`函数里调用了`DialogBoxParamA`：
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_01.png) <br>
- 可以发现这个函数有多个引用<br>
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_02.png) <br>
- 所以还需要继续下断点查找是哪里调用了
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_03.png) <br>
- 最终来到`0x425F80`这个地址，可以发现这个是一个循环一直调用着`sub_40EA30`，不难猜出通过验证的逻辑肯定是走`jmp dc3wy.429434`
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_04.png) <br>
- 所以改起来就简单了，直接把`call <dc3wy.40EA30>`改成`jmp dc3wy.429434`，~~其余的nop就行了(强迫症)~~ 即可
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_05.png) <br>

## 0x01 一些系统API的HOOK
### CreateFileA与FindFirstFileA
- 为了实现与原版文件共存，因此需要HOOK这两个函数，api详细可以到官方文档查看：
[CreateFileA](https://learn.microsoft.com/zh-cn/windows/win32/api/fileapi/nf-fileapi-createfilea)，
[FindFirstFileA](https://learn.microsoft.com/zh-cn/windows/win32/api/fileapi/nf-fileapi-findfirstfilea)
```cpp
Patch::Hooker::Add<DC3WY::CreateFileA>(::CreateFileA);
Patch::Hooker::Add<DC3WY::FindFirstFileA>(::FindFirstFileA);
```
- 这里声明一个辅助函数`ReplacePathA`用于查找并替换文件
```cpp
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
            return new_path;
        }
    }
    return {};
}

  static auto WINAPI CreateFileA(LPCSTR lpFN, DWORD dwDA, DWORD dwSM, LPSECURITY_ATTRIBUTES lpSA, DWORD dwCD, DWORD dwFAA, HANDLE hTF) -> HANDLE
  {
      std::string_view new_path{ DC3WY::ReplacePathA(lpFN) };
      return Patch::Hooker::Call<DC3WY::CreateFileA>(new_path.empty() ? lpFN : new_path.data(), dwDA, dwSM, lpSA, dwCD, dwFAA, hTF);
  }

  static auto WINAPI FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) -> HANDLE
  {
      std::string_view new_path{ DC3WY::ReplacePathA(lpFileName) };
      return Patch::Hooker::Call<DC3WY::FindFirstFileA>(new_path.empty() ? lpFileName : new_path.data(), lpFindFileData);
  }
```

### GetGlyphOutlineA
- 这个主要是用来更改游戏字体与文本编码
```cpp
Patch::Hooker::Add<DC3WY::GetGlyphOutlineA>(::GetGlyphOutlineA);
```
```cpp
static auto WINAPI GetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuf, LPGLYPHMETRICS lpgm, DWORD cjbf, LPVOID pvbf, MAT2* lpmat) -> DWORD
{
    /* 此处省略，后面会与FontManager一起详细讲解 */ 
    return Patch::Hooker::Call<DC3WY::GetGlyphOutlineA>(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat);
}
```
### 
### SendMessageA
- 这是一个给窗口发送消息的函数，api详细可以到官方文档查看：[SendMessageA](https://learn.microsoft.com/windows/win32/api/winuser/nf-winuser-sendmessagea) <br>
- 那为什么要Hook它呢？那当然是为了更改这个对话框的文本内容。
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_06.png) <br>
- 这个可以搜索字符串`データVer`定位到`sub_40DA40`这个函数
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_07.png) <br>
- 通过`ida`反编译可以一目了然，这个`SendMessageA(DlgItem, 0xCu, 0x104u, lParam);`就是在更改对话框文本内容了。
```cpp
Patch::Hooker::Add<DC3WY::SendMessageA>(::SendMessageA);
```
```cpp
static constexpr inline wchar_t PatchDesc[]
{
    L"本补丁由【COKEZIGE STUDIO】制作并免费发布\n\n"
    L"仅供学习交流使用，禁止一切直播录播和商用行为\n\n"
    L"补丁源代码已开源至GitHub\n\n"
    L"https://github.com/cokkeijigen/dc3_cn"
};

static auto WINAPI SendMessageA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (uMsg == 0xCu && wParam == 0x104u)
    {
        auto ctr_id{ ::GetDlgCtrlID(hWnd) };
        if (ctr_id == 1005)
        {
            auto result
            {
                ::SendMessageW
                (
                    { hWnd }, { 0x0000000Cu },
                    { sizeof(DC3WY::PatchDesc) / sizeof(wchar_t) },
                    { reinterpret_cast<LPARAM>(DC3WY::PatchDesc) }
                )
            };
            return { result };
        }
    }
    return Patch::Hooker::Call<DC3WY::SendMessageA>(hWnd, uMsg, wParam, lParam);
}
```
- 当然也可以直接去Hook `sub_40DA40`这个函数来实现修改，我这里偷个懒直接Hook `SendMessageA`了。

## 0x02 一些游戏函数的HOOK

### DC3WY::WndProc <0x40FC20>
- 为什么要Hook这个函数？这个函数是游戏窗口过程函数，在我们需要拿到游戏窗口句柄或者在游戏创建窗口时做一些初始化操作，那么Hook游戏的`WndProc`是最好的选择。
- 这个函数很好找，直接去查找`RegisterClassA`的引用，api详细可以到官方文档查看：
[RegisterClassA](https://learn.microsoft.com/windows/win32/api/winuser/nf-winuser-registerclassa)
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_08.png) <br>
- 依旧是通过`ida`反编译查看，这个`WndClass.lpfnWndProc = sub_40FC20;` 就是`DC3WY::WndProc`了
```cpp
Patch::Hooker::Add<DC3WY::WndProc>(reinterpret_cast<void*>(0x40FC20));
```
```cpp
static constexpr inline wchar_t TitleName[]
{
    L"【COKEZIGE STUDIO】Da Capo Ⅲ With You - CHS Ver.1.00"
    L" ※仅供学习交流使用，禁止一切直播录播和商用行为※" 
};
```
```cpp
auto CALLBACK DC3WY::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (uMsg == WM_CREATE) 
    {
        // 设置窗口标题
        ::SetWindowTextW(hWnd, DC3WY::TitleName);
        /* 一些初始化操作，此处省略…… */
    }
    else
    {
        /* 其他逻辑…… */
    }
    return Patch::Hooker::Call<DC3WY::WndProc>(hWnd, uMsg, wParam, lParam);
}
```
- 初始化`Utils::FontManager`，这是我自己写的一个字体选择器GUI，详细：
[utillibs/fontmanager](https://github.com/cokkeijigen/circus_engine_patchs/tree/master/CircusEnginePatchs/utillibs/fontmanager)。

```cpp

Utils::FontManager DC3WY::FontManager{};

auto CALLBACK DC3WY::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (uMsg == WM_CREATE) 
    {
        // 设置窗口标题
        ::SetWindowTextW(hWnd, DC3WY::TitleName);

        HMENU SystemMenu{ ::GetSystemMenu(hWnd, FALSE) };
        if (SystemMenu != nullptr)
        {
            // 向系统菜单添加 “更改字体” 选项，标识为0x114514
            ::AppendMenuW(SystemMenu, MF_UNCHECKED, 0x114514, L"更改字体");
        }
        if (!DC3WY::FontManager.IsInit())
        {
            DC3WY::FontManager.Init(hWnd);
        }

    }
    else if (uMsg == WM_SYSCOMMAND)
    {
        if (wParam == 0x114514)
        {
            if (DC3WY::FontManager.GUI() != nullptr)
            {
                // 显示选择字体窗口
                DC3WY::FontManager.GUIChooseFont();
            }
            return TRUE;
        }
    }
    else
    {
        /* 其他逻辑…… */
    }
    return Patch::Hooker::Call<DC3WY::WndProc>(hWnd, uMsg, wParam, lParam);
}
```
- 应用字体需要在`GetGlyphOutlineA`里，先通过`FontManager::GetJISFont`或者`FontManager::GetGBKFont`获取当前选择的字体的`HFONT`对象，<br>
这两个函数的参数是需要一个`szie`，那么这个`size`从哪来？很简单，从第一个参数`HDC hdc`中获取。<br>
只需要调用`GetTextMetricsA`([详细](https://learn.microsoft.com/windows/win32/api/wingdi/nf-wingdi-gettextmetrics))
```cpp
static auto WINAPI GetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuf, LPGLYPHMETRICS lpgm, DWORD cjbf, LPVOID pvbf, MAT2* lpmat) -> DWORD
{
    if (tagTEXTMETRICA lptm{}; ::GetTextMetricsA(hdc, &lptm))
    {
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
```
# 在写了在写了……
