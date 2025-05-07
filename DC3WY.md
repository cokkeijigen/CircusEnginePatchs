# DC3WY 中文本地化笔记

| 工具 |说明 |
|:---|:---|
| [MesTextTool](https://github.com/cokkeijigen/MesTextTool) | Mes文本的提取导入 |
|[sagilio::GARbro](https://github.com/sagilio/GARbro)| *.CRX图片类型封装转换|
| [x32dbg](https://x64dbg.com/) | 静态动态调试分析 |
| IDA 9 PRO | 静态逆向分析 |
| CFF Explorer| 修改IAT |

## 0x00 去除DVD和KEY验证
首先是DVD验证，这可以直接搜索字符`DVD`串或者断`MessageBoxA`就能找到位置。 <br>
可以发现上面有个跳转是调到下面弹出验证消息的逻辑，直接`nop`掉跳转即可
![Image_text](https://raw.githubusercontent.com/cokkeijigen/circus_engine_patchs/master/Pictures/img_dc3wy_note_00.png)
