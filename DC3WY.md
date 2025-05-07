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
- 为了实现与原版文件共存，因此需要Hook这两个函数，详细可以到官方文档查看：[FindFirstFileA](https://learn.microsoft.com/zh-cn/windows/win32/api/fileapi/nf-fileapi-findfirstfilea)，[CreateFileA](https://learn.microsoft.com/zh-cn/windows/win32/api/fileapi/nf-fileapi-createfilea)
```cpp
Patch::Hooker::Add<DC3WY::CreateFileA>(::CreateFileA);       // 添加Hook
Patch::Hooker::Add<DC3WY::FindFirstFileA>(::FindFirstFileA); // 添加Hook
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
Patch::Hooker::Add<DC3WY::GetGlyphOutlineA>(::GetGlyphOutlineA); // 添加Hook
```
```cpp
static auto WINAPI GetGlyphOutlineA(HDC hdc, UINT uChar, UINT fuf, LPGLYPHMETRICS lpgm, DWORD cjbf, LPVOID pvbf, MAT2* lpmat) -> DWORD
{
    /* 此处暂时省略，后面会与FontManager一起详细讲解 */ 
    return Patch::Hooker::Call<DC3WY::GetGlyphOutlineA>(hdc, uChar, fuf, lpgm, cjbf, pvbf, lpmat);
}
```
### SendMessageA
- 这是一个给窗口发送消息的函数，详细可以到官方文档查看：[SendMessageA](https://learn.microsoft.com/windows/win32/api/winuser/nf-winuser-sendmessagea) <br>那为什么要Hook它呢？那当然是为了更改这个对话框的文本内容。
  ![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_06.png) <br>
- 这个可以搜索字符串`データVer`定位到`sub_40DA40`这个函数
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_07.png) <br>
- 通过`ida`反编译可以一目了然，这个`SendMessageA(DlgItem, 0xCu, 0x104u, lParam);`就是在更改对话框文本内容了。
```cpp
Patch::Hooker::Add<DC3WY::SendMessageA>(::SendMessageA); // 添加Hook
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
    // 为了确保此条消息是更改对话框文本的，还需要判断wParam和DlgCtrlID
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
- 至于这个`バージョン情報`的字符串如何更改，后面会讲到，接着往下看不用急。

![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_06.1.png)

- 当然也可以直接去Hook `sub_40DA40`这个函数来实现修改，我这里偷个懒直接Hook `SendMessageA`了。

## 0x02 一些游戏函数的HOOK

### DC3WY::WndProc <0x40FC20>
- 为什么要Hook这个函数？这个函数是游戏窗口过程函数，在我们需要拿到游戏窗口句柄或者在创建窗口时做一些初始化操作，那么Hook游戏的`WndProc`是最好的选择。
- 这个函数很好找，直接去查找`RegisterClassA`的引用，详细可以到官方文档查看：[RegisterClassA](https://learn.microsoft.com/windows/win32/api/winuser/nf-winuser-registerclassa)
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_08.png) <br>
- 依旧是通过`ida`反编译查看，这个`WndClass.lpfnWndProc = sub_40FC20;` 就是`DC3WY::WndProc`了
```cpp
Patch::Hooker::Add<DC3WY::WndProc>(reinterpret_cast<void*>(0x40FC20)); // 添加Hook
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
- 初始化`Utils::FontManager`，这是我自己写的一个字体选择器GUI，具体实现大家自行查看源码：[utillibs/fontmanager](https://github.com/cokkeijigen/circus_engine_patchs/tree/master/CircusEnginePatchs/utillibs/fontmanager)。

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
    else if (uMsg == WM_SIZE)
    {
        // 当窗口大小改变时需要更新一下显示的状态，针对全屏与窗口模式切换
        if (DC3WY::FontManager.GUI() != nullptr)
        {
            DC3WY::FontManager.GUIUpdateDisplayState();
        }
    }
    else
    {
        /* 其他逻辑…… */
    }
    return Patch::Hooker::Call<DC3WY::WndProc>(hWnd, uMsg, wParam, lParam);
}
```
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_09.png)
- 那么如何引用到游戏中呢？很简单，就是在`GetGlyphOutlineA`中设置就行了，这就是我为什么需要Hook它。
- 在`GetGlyphOutlineA`中可以通过`FontManager::GetJISFont`和`FontManager::GetGBKFont`来获取当前选择字体的`HFONT`对象。
但是这两个函数都是需要一个`size`作为参数的，那么从哪获取呢？答案很简单，直接从第一个参数`HDC hdc`中获取即可。
- 使用`GetTextMetricsA`([详细](https://learn.microsoft.com/windows/win32/api/wingdi/nf-wingdi-gettextmetrics))获取当前字体的`TEXTMETRIC`，
这个`TEXTMETRIC`。
```cpp
tagTEXTMETRICA lptm{};
::GetTextMetricsA(hdc, &lptm);
auto size{ lptm.tmHeight }; // 这个就可以作为size使用
HFONT font{ DC3WY::FontManager.GetGBKFont(size) };
```
- 获取到`HFONT`对象后再通过`SelectObject`([详细](https://learn.microsoft.com/windows/win32/api/wingdi/nf-wingdi-gettextmetrics))来设置`hdc`的新字体。<br>
※ 注意：用完需要再调用`SelectObject`还原回去。为什么要还原回去呢？因为这个游戏使用了多种大小的字体，而`FontManager`会将字体大小作为`key`将`HFONT`存到`map`中，当通过`FontManagerGUI`更改字体大小时，会把这个`map`中的所有字体通过原始大小进行计算，获得缩放比例再更新并创建出与其大小相对新的`HFONT`对象(详细：[Utils::FontManager::m_GUI::OnChanged](https://github.com/cokkeijigen/circus_engine_patchs/blob/master/CircusEnginePatchs/utillibs/fontmanager/FontManager.cpp#L24)、[Utils::FontManagerGUI::MakeFont](https://github.com/cokkeijigen/circus_engine_patchs/blob/master/CircusEnginePatchs/utillibs/fontmanager/FontManagerGUI.cpp#L711))，如果不还原，下次就无法通过大小确定是哪个字体了。
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
- 音符（♪）符号与其他特殊符号的支持这个实现也很简单，只需要将`♪`换成一个GBK编码里存在的字符，例如`§`，然后再在`GetGlyphOutlineA`里替换即可。注意这里使用的是`FontManager::GetJISFont`，其他要替换其他在特殊符号也同理，不多说了。
```cpp
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
```

- 初始化`XSub::GDI::ImageSubPlayer`。这也是我自己写的一个字幕播放器，目前还是个临时方案~~（等我把libass整明白了再继续完善）~~，具体实现大家自行查看源码：[dc3wy/sub](https://github.com/cokkeijigen/circus_engine_patchs/tree/master/CircusEnginePatchs/dc3wy/sub)、[utillibs/xsub](https://github.com/cokkeijigen/circus_engine_patchs/tree/master/CircusEnginePatchs/utillibs/xsub)。

```cpp
XSub::GDI::ImageSubPlayer* SubPlayer{};

auto CALLBACK DC3WY::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (uMsg == WM_CREATE) 
    {
        /* 此处省略…… */
        if (DC3WY::SubPlayer == nullptr)
        {
            // 使用new来创建XSub::GDI::ImageSubPlayer实例
            DC3WY::SubPlayer =
            {
                new XSub::GDI::ImageSubPlayer{ hWnd }
            };
			
            // 设置一个默认字幕位置
            DC3WY::SubPlayer->SetDefualtPoint
            (
                XSub::Point
                {
                    .align{ XSub::Align::Center },
                }
            );
        }
    }
    else if (uMsg == WM_MOVE)
    {
        if (DC3WY::SubPlayer != nullptr)
        {
            // 游戏窗口移动时要更新字幕窗口的位置
            auto x{ static_cast<int>(LOWORD(lParam)) };
            auto y{ static_cast<int>(HIWORD(lParam)) };
            DC3WY::SubPlayer->SetPosition(x, y, true);
        }
    }
	else if(uMsg == WM_SIZE)
    {
        if (DC3WY::SubPlayer != nullptr)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                // 当窗口最小化时隐藏
                DC3WY::SubPlayer->Hide();
                DC3WY::SubPlayer->UpdateLayer();
            }
            else if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
            {
                DC3WY::SubPlayer->Show();
                // 窗口显示或者改变大小时要重新同步
                DC3WY::SubPlayer->SyncToParentWindow(true);
            }
        }
    }
    else
    {
        /* 其他逻辑…… */
    }
    return Patch::Hooker::Call<DC3WY::WndProc>(hWnd, uMsg, wParam, lParam);
}
```

### DC3WY::ComPlayVideo<0x444920>、DC3WY::ComStopVideo<0x444640>

- 这两个是控制OP视频播放与停止的函数，当时在找这个两个函数的时，那过程是相当的抽象，直接说结论，这个游戏使用COM接口来播放视频，直接查找`CoCreateInstance`的引用也能找到，但是太多了，直接上调试器，我有两种方法可以快速定位。

- 首先是使用土方法，直接断文件相关的API看看他是在哪读取的，我这里先试过了`CreateFileA`，不过它并没有使用这个，而是`CreateFileW`，但游戏exe并没有导入这个函数，所以需要从`kernel32.dll`中下断点。<br>

  ![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_10.png)

- 运行直到发现路径是OP视频的为止，然后直接取消断点，然后按下`Alt+F9`运行到用户代码，可以看到上面有个`CoCreateInstance`，这里就是我们要找的函数了。<br>![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_11.png)

- 然后就是第二种方法，那就是直接断`CoCreateInstance`，这个函数普通断点可能停不下来，所以建议使用设置硬件断点（执行）。<br>![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_12.png)

- 这里停下来之后，然后取消断点，接着在堆栈窗口选中顶部的地址按下回车就能来到我们要找的函数了，效果都一样，只不过断`CoCreateInstance`的方式是你前提得知道它用了这个，不然就只能靠断文件相关的API这种图方法找了。

  



# 在写了在写了……
