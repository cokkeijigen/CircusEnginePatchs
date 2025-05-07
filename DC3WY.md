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
- 为了实现与原版文件共存，因此需要HOOK这两个函数
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
# 在写了在写了……
