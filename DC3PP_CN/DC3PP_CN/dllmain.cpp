/**********************************
	Ever tried. Ever failed.
	No matter. Try again.
	Fail again. Fail better.
 **********************************/

#include "framework.h"

typedef HFONT(WINAPI* fnCreateFontA)(
	int nHeight, // logical height of font height
	int nWidth, // logical average character width
	int nEscapement, // angle of escapement
	int nOrientation, // base-line orientation angle
	int fnWeight, // font weight
	DWORD fdwItalic, // italic attribute flag
	DWORD fdwUnderline, // underline attribute flag
	DWORD fdwStrikeOut, // strikeout attribute flag
	DWORD fdwCharSet, // character set identifier
	DWORD fdwOutputPrecision, // output precision
	DWORD fdwClipPrecision, // clipping precision
	DWORD fdwQuality, // output quality
	DWORD fdwPitchAndFamily, // pitch and family
	LPCSTR lpszFace // pointer to typeface name string
	);
fnCreateFontA CreateFontAOLD;
HFONT WINAPI CreateFontAEx(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
	fdwCharSet = 0x86;
	return CreateFontAOLD(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, "黑体");
}

PVOID g_pOldCreateWindowExA = CreateWindowExA;
typedef HWND(WINAPI* pfuncCreateWindowExA)(
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
HWND WINAPI NewCreateWindowExA(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	const char* szWndName = "【-COKEZIGE汉化组-】Da Capo Ⅲ Platinum Partner - Ver.1.00";

	return ((pfuncCreateWindowExA)g_pOldCreateWindowExA)(dwExStyle, lpClassName, (LPCTSTR)szWndName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

LPWSTR ctowJIS(char* str)
{
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(932, 0, str, -1, NULL, 0); //计算长度
	LPWSTR out = new wchar_t[dwMinSize];
	MultiByteToWideChar(932, 0, str, -1, out, dwMinSize);//转换
	return out;
}

char* wtocGBK(LPCTSTR str)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(936, NULL, str, -1, NULL, 0, NULL, FALSE); //计算长度
	char* out = new char[dwMinSize];
	WideCharToMultiByte(936, NULL, str, -1, out, dwMinSize, NULL, FALSE);//转换
	return out;
}



//log
wchar_t szTitle[] = L"提示";
typedef int (WINAPI* fnMessageboxA)(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType
	);
fnMessageboxA MessageBoxAOLD;
int WINAPI MessageBoxAEx(_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType)
{
	char* Pstr = wtocGBK(ctowJIS((char*)lpText));
	if (strcmp(Pstr, "終了しますか？") == 0)
	{
		strcpy(Pstr, "结束游戏吗？");
	}
	return MessageBoxAOLD(hWnd, Pstr, wtocGBK(szTitle), uType);
}

PVOID g_pOldCreateFileA = CreateFileA;
typedef HANDLE(WINAPI* PfuncCreateFileA)(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
	);


PVOID g_pOldFindFirstFileA = FindFirstFileA;
typedef HANDLE(WINAPI* PfuncFindFirstFileA)(
	LPCSTR lpFileName,
	LPWIN32_FIND_DATAA lpFindFileData
	);

PVOID g_pOldAppendMenuA = AppendMenuA;
typedef BOOL(WINAPI* PfuncAppendMenuA) (HMENU hMenu, UINT uFlags, UINT_PTR uIDNewltem, LPCTSTR lpNewltem);

PVOID  g_pOldSendMessageA = SendMessageA;
typedef LRESULT(WINAPI* PfuncSendMessageA)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM IParam);

//======================================
void ReplacePath(string *path) {
	string nname = "cn_Data\\" + path->substr(path->find_last_of("\\") + 1);
	if (GetFileAttributesA(nname.c_str()) != INVALID_FILE_ATTRIBUTES) {
		printf("ReadFileFromKaze:%s\n",nname.c_str());
		path->assign(nname);
	}
}

HANDLE WINAPI NewCreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile)
{
	string newName(lpFileName);
	ReplacePath(&newName);

	return ((PfuncCreateFileA)(g_pOldCreateFileA))(
		newName.c_str(),
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}

HANDLE WINAPI NewFindFirstFileA(
	LPCSTR lpFileName,
	LPWIN32_FIND_DATAA lpFindFileData) {

	string newName(lpFileName);
	ReplacePath(&newName);

	return ((PfuncFindFirstFileA)(g_pOldFindFirstFileA))(
		newName.c_str(),
		lpFindFileData);
}

BOOL WINAPI NewAppendMenuA(HMENU hMenu,UINT uFlags,UINT_PTR uIDNewltem,LPCTSTR lpNewltem) {
	//printf("%s", lpNewltem);
	if (strcmp((const char*)lpNewltem, "僶乕僕儑儞忣曬") == 0) {
		return ((PfuncAppendMenuA)(g_pOldAppendMenuA)) (hMenu, uFlags, uIDNewltem, (LPCTSTR)"版本信息");
	}
	else return ((PfuncAppendMenuA)(g_pOldAppendMenuA)) (hMenu,uFlags,uIDNewltem,lpNewltem);
}

LRESULT WINAPI NewSendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM IParam) {
	const char *oldptr = (const char*)IParam;
	if (oldptr && strlen(oldptr) == 77 ) {
		if (oldptr[0] == 0x44 && oldptr[1] == 0x2e && oldptr[2] == 0x43) {
			oldptr = "～初音岛Ⅲ·白金伙伴～ Ver.1.00\n资源版本 Ver.1.00";
			IParam = (LPARAM)oldptr;
		}
	}
	
	return ((PfuncSendMessageA)(g_pOldSendMessageA))(hWnd, Msg, wParam, IParam);
}

void Init()
{

	CreateFontAOLD = (fnCreateFontA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontA");
	MessageBoxAOLD = (fnMessageboxA)GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxA");

	DetourTransactionBegin();
	DetourAttach((void**)& CreateFontAOLD, CreateFontAEx);
	DetourAttach(&g_pOldCreateWindowExA, NewCreateWindowExA);
	DetourAttach(&g_pOldCreateFileA, NewCreateFileA);
	DetourAttach(&g_pOldFindFirstFileA, NewFindFirstFileA);
	DetourAttach(&g_pOldAppendMenuA, NewAppendMenuA);
	DetourAttach(&g_pOldSendMessageA, NewSendMessageA);
	DetourAttach((void**)& MessageBoxAOLD, MessageBoxAEx);

	if (DetourTransactionCommit() != NOERROR) { MessageBox(NULL, L"INIT FUNCTION ERROR", L"System", MB_OK | MB_ICONERROR); }
	else { MessageBox(NULL, L"本补丁由【-COKEZIGE汉化组-】制作\n禁止一切录播直播和商业行为", L"警告", MB_OK | MB_ICONINFORMATION); }
}


const char* SaveTitleCHS[][2]{
{ "DC3PP_0518_FREE_AOI", "与葵的通话" },
{ "DC3PP_0518_FREE_HMN", "和姬乃通电话" },
{ "DC3PP_0518_FREE_RCC", "与立夏的通话" },
{ "DC3PP_0518_FREE_SRA", "与纱罗的通话" },
{ "DC3PP_0518_FREE_SRR", "和夏露露的对话" },
{ "DC3PP_1_0516_E1_PRO", "编辑会议！" },
{ "DC3PP_1_0516_F1_PRO", "露露姐的危险按摩" },
{ "DC3PP_1_0517_A1_PRO", "非官方新闻部的挑战书" },
{ "DC3PP_1_0517_C1_PRO", "意想不到的帮手" },
{ "DC3PP_1_0517_E1_PRO", "接力赛成员的决定！" },
{ "DC3PP_1_0518_A1_PRO", "和立夏的个人课程？" },
{ "DC3PP_1_0518_B1_PRO", "江户川姐弟的力量关系" },
{ "DC3PP_1_0518_D1_PRO", "纠纷的三人四脚" },
{ "DC3PP_1_0518_E1_PRO", "充满爱意的汤" },
{ "DC3PP_1_0518_F1_FRE", "给谁打电话好呢？" },
{ "DC3PP_1_0519_A1_PRO", "体育祭，开幕！" },
{ "DC3PP_1_0519_B1_PRO", "陷入白热化的三人四脚比赛！" },
{ "DC3PP_1_0519_C1_PRO", "梦幻的菜单" },
{ "DC3PP_1_0519_E1_PRO", "给谁加油呢？" },
{ "DC3PP_1_0519_E2_PRO", "珍藏的秘策" },
{ "DC3PP_1_0519_E3_PRO", "决战之时" },
{ "DC3PP_1_0519_F1_PRO", "庆功宴" },
{ "DC3PP_2_0628_E1_PRO", "暑假活动计划" },
{ "DC3PP_2_0628_F1_PRO", "波澜汹涌的学习会" },
{ "DC3PP_2_0629_B1_PRO", "迎接期末考试" },
{ "DC3PP_2_0630_C1_PRO", "四季的安排是？" },
{ "DC3PP_2_0701_A1_PRO", "最后的坚持" },
{ "DC3PP_2_0701_E1_PRO", "关于旅行的讨论" },
{ "DC3PP_2_0722_B1_PRO", "暑假将至" },
{ "DC3PP_2_0722_E1_PRO", "诱惑的泳衣秀" },
{ "DC3PP_2_0723_A1_PRO", "旅行开始了" },
{ "DC3PP_2_0723_B1_PRO", "身旁的立夏" },
{ "DC3PP_2_0723_C1_PRO", "泳池边的女神们" },
{ "DC3PP_2_0723_E1_PRO", "令人怀念的味道" },
{ "DC3PP_2_0723_F1_PRO", "澡堂的相约" },
{ "DC3PP_2_0723_F20_PRO", "恋爱叠叠高" },
{ "DC3PP_2_0723_F21_PRO", "恋爱叠叠高：白" },
{ "DC3PP_2_0723_F22_PRO", "恋爱叠叠高：中间的粉色木块" },
{ "DC3PP_2_0723_F23_PRO", "恋爱叠叠高：下面的粉色木块" },
{ "DC3PP_2_0723_F30_PRO", "ＬＯＶＥ　ＫＩＳＳ" },
{ "DC3PP_2_0724_B1_PRO", "寻宝" },
{ "DC3PP_2_0724_D2_PRO", "宝箱的里面是？" },
{ "DC3PP_2_0724_E1_PRO", "夏日合宿的尾声" },
{ "DC3PP_2_0724_F2_AOI", "夜晚的幽会·葵" },
{ "DC3PP_2_0724_F2_HMN", "夜晚的幽会·姬乃" },
{ "DC3PP_2_0724_F2_PRO", "夜晚的幽会" },
{ "DC3PP_2_0724_F2_RCC", "夜晚的幽会·立夏" },
{ "DC3PP_2_0724_F2_SRA", "夜晚的幽会·纱罗" },
{ "DC3PP_2_0724_F2_SRR", "夜晚的幽会·夏露露" },
{ "DC3PP_2_0725_B1_PRO", "夏日的回忆" },
{ "DC3PP_3_0922_E1_PRO", "我也想去嘛！" },
{ "DC3PP_3_0922_F1_CMN", "超能力者露露姐" },
{ "DC3PP_3_0923_A1_CMN", "出发的早晨" },
{ "DC3PP_3_0923_B1_CMN", "在机场" },
{ "DC3PP_3_0923_D10_CMN", "旅伴" },
{ "DC3PP_3_0923_D11_HMN", "哥哥的计划是？" },
{ "DC3PP_3_0923_D12_SRA", "我是来观光的" },
{ "DC3PP_3_0924_B1_CMN", "时光荏苒" },
{ "DC3PP_3_0924_D1_CMN", "抓住机会！" },
{ "DC3PP_3_0924_F1_CMN", "第１个教训" },
{ "DC3PP_3_0924_F2_CMN", "关于四季……" },
{ "DC3PP_3_0925_A1_CMN", "自由活动的计划" },
{ "DC3PP_3_0925_B1_CMN", "哈罗兹" },
{ "DC3PP_3_0925_F1_CMN", "错过时令的百物语" },
{ "DC3PP_3_0926_A1_CMN", "旧克里沙利斯宅" },
{ "DC3PP_3_0926_B10_CMN", "一人一半" },
{ "DC3PP_3_0926_B11_HMN", "一人一半：姬乃" },
{ "DC3PP_3_0926_B12_SRA", "一人一半：纱罗" },
{ "DC3PP_3_0926_F1_CMN", "决意之夜" },
{ "DC3PP_3_0927_B10_CMN", "让人担心的那个孩子" },
{ "DC3PP_3_0927_B10_HMN", "让人担心的姬乃" },
{ "DC3PP_3_0927_B10_SRA", "让人担心的纱罗" },
{ "DC3PP_3_0927_B11_HMN", "姬乃就是姬乃" },
{ "DC3PP_3_0927_B12_SRA", "纱罗就是纱罗" },
{ "DC3PP_3_0930_A11_HMN", "老妈的味道" },
{ "DC3PP_3_0930_A12_SRA", "只要有学长在就好" },
{ "DC3PP_3_1001_A11_HMN", "今后亦然" },
{ "DC3PP_3_1001_A12_SRA", "茜色的告白" },
{ "DC3PP_3_1101_E1_PRO", "说到文化祭的节目的话？" },
{ "DC3PP_3_1101_E2_CMN", "自己播下的忙碌种子" },
{ "DC3PP_3_1101_F1_RCC", "命中注定之两人的下学" },
{ "DC3PP_3_1101_F2_SRR", "露露姐亲手做的危险料理" },
{ "DC3PP_3_1102_A1_AOI", "命中注定的两人的上学路" },
{ "DC3PP_3_1102_B1_CMN", "为了让文化祭热闹起来" },
{ "DC3PP_3_1102_C1_CMN", "立夏的选美出场决心" },
{ "DC3PP_3_1102_D1_CMN", "通向文化祭的漫长准备之路" },
{ "DC3PP_3_1102_D2_CMN", "学生会的帮手？" },
{ "DC3PP_3_1102_F10_CMN", "深夜发来的短信" },
{ "DC3PP_3_1102_F11_RCC", "立夏与三明治" },
{ "DC3PP_3_1102_F12_SRR", "露露姐与奇迹？的饭团" },
{ "DC3PP_3_1102_F13_AOI", "小葵与青春的味道" },
{ "DC3PP_3_1103_A1_CMN", "男人的浪漫？" },
{ "DC3PP_3_1103_B1_CMN", "文化祭开始！" },
{ "DC3PP_3_1103_B2_RCC", "睡脸与心动的责任" },
{ "DC3PP_3_1103_C1_SRR", "私人服务" },
{ "DC3PP_3_1103_D1_CMN", "非官方新闻部开始行动" },
{ "DC3PP_3_1103_D2_CMN", "对圣诞老人的请求" },
{ "DC3PP_3_1103_D3_AOI", "情感丰富的小葵" },
{ "DC3PP_3_1103_E1_CMN", "选美比赛正式开始！" },
{ "DC3PP_3_1103_F10_CMN", "最重要的人" },
{ "DC3PP_3_1103_F11_RCC", "用心跳传达的心意" },
{ "DC3PP_3_1103_F12_SRR", "并非作为堂姐弟……" },
{ "DC3PP_3_1103_F13_AOI", "再浪漫一些" },
{ "DC3PP_4_1213_E1_RCC", "圣诞节将近" },
{ "DC3PP_4_1213_E1_SRR", "卿卿我我的新婚夫妇" },
{ "DC3PP_4_1213_E2_RCC", "生硬的两人" },
{ "DC3PP_4_1213_F1_RCC", "对立夏的思念" },
{ "DC3PP_4_1213_F1_SRR", "成为恋人的伊始" },
{ "DC3PP_4_1213_F2_SRR", "小隆的女朋友" },
{ "DC3PP_4_1214_A1_RCC", "昨天的会议" },
{ "DC3PP_4_1214_A1_SRR", "一路平安的约定" },
{ "DC3PP_4_1214_D1_RCC", "意外敏锐的耕助" },
{ "DC3PP_4_1214_E1_RCC", "身不由己的日常生活" },
{ "DC3PP_4_1214_E1_SRR", "圣诞派对和心不在焉的露露姐" },
{ "DC3PP_4_1214_F1_SRR", "世代继承的心意" },
{ "DC3PP_4_1215_A1_SRR", "圣诞节想要什么？" },
{ "DC3PP_4_1215_D1_SRR", "雪村李与编织玩偶" },
{ "DC3PP_4_1215_E1_SRA", "突然的来信" },
{ "DC3PP_4_1215_F1_SRA", "第一次留宿" },
{ "DC3PP_4_1215_F1_SRR", "圣诞活动的建议" },
{ "DC3PP_4_1215_F2_SRA", "学长的颜色……" },
{ "DC3PP_4_1216_A1_SRA", "意外的监护人" },
{ "DC3PP_4_1216_A1_SRR", "和雪村的悄悄话" },
{ "DC3PP_4_1216_C1_SRR", "公主和仆人" },
{ "DC3PP_4_1216_D1_SRA", "解谜迷宫的幕后" },
{ "DC3PP_4_1216_E1_SRA", "被伙伴祝福的幸福" },
{ "DC3PP_4_1216_F1_SRR", "一起洗澡！？" },
{ "DC3PP_4_1217_B1_SRR", "初次约会" },
{ "DC3PP_4_1217_D1_SRR", "露露姐告诉我的景色" },
{ "DC3PP_4_1217_E1_RCC", "特刊的企划会议" },
{ "DC3PP_4_1217_E2_RCC", "修习的成果" },
{ "DC3PP_4_1217_F1_RCC", "关于魔法" },
{ "DC3PP_4_1218_A1_RCC", "倒计时" },
{ "DC3PP_4_1218_B1_HMN", "抢手" },
{ "DC3PP_4_1218_D1_SRR", "圣诞派对的隐藏活动！？" },
{ "DC3PP_4_1218_E1_HMN", "编辑会议" },
{ "DC3PP_4_1218_E1_SRR", "一波未平一波又起" },
{ "DC3PP_4_1218_E2_HMN", "教室里的两人" },
{ "DC3PP_4_1218_F1_SRR", "还有一件事想做" },
{ "DC3PP_4_1218_F2_SRR", "前世的夏露露的故事" },
{ "DC3PP_4_1219_A1_HMN", "又冷，又羞，上学路" },
{ "DC3PP_4_1219_B1_HMN", "请节哀……" },
{ "DC3PP_4_1219_D1_SRR", "永不枯萎樱花树的所赠之物" },
{ "DC3PP_4_1219_E1_SRA", "没有出口的迷宫" },
{ "DC3PP_4_1219_F1_HMN", "跟姬乃小姐约好！" },
{ "DC3PP_4_1219_F1_SRA", "直面父亲的决心" },
{ "DC3PP_4_1219_F1_SRR", "人偶剧剧本的修改版" },
{ "DC3PP_4_1220_A1_SRA", "名为准备的战场" },
{ "DC3PP_4_1220_A1_SRR", "人偶剧剧本，完成！" },
{ "DC3PP_4_1220_B1_HMN", "联谊的邀请" },
{ "DC3PP_4_1220_D1_SRR", "可靠的援军参战！" },
{ "DC3PP_4_1220_E1_HMN", "小矮人的鞋店" },
{ "DC3PP_4_1220_E1_SRR", "开始行动，官方新闻部" },
{ "DC3PP_4_1220_F1_HMN", "ＩＬＯＶＥＹＯＵ？！" },
{ "DC3PP_4_1221_A1_SRR", "弥足珍贵的午休" },
{ "DC3PP_4_1221_B1_HMN", "进行中的圣诞派对准备" },
{ "DC3PP_4_1221_E1_HMN", "紧急集合！" },
{ "DC3PP_4_1221_E1_SRR", "开始做小道具吧！" },
{ "DC3PP_4_1221_F1_SRR", "快乐而吵闹的回家路" },
{ "DC3PP_4_1222_A1_HMN", "滑啊滑，滑不动" },
{ "DC3PP_4_1222_A1_SRR", "进展如何？" },
{ "DC3PP_4_1222_B1_AOI", "小葵来也" },
{ "DC3PP_4_1222_B1_HMN", "最后冲刺" },
{ "DC3PP_4_1222_B1_SRR", "密室逃脱，完成！" },
{ "DC3PP_4_1222_C1_AOI", "寒风中令人悸动不已的便当" },
{ "DC3PP_4_1222_E1_AOI", "定期会议……不对？" },
{ "DC3PP_4_1222_F1_HMN", "再坦率一点" },
{ "DC3PP_4_1222_F1_SRR", "更显意外的援手" },
{ "DC3PP_4_1223_A1_AOI", "喜欢巨乳萝莉？" },
{ "DC3PP_4_1223_A1_SRA", "圣诞派对开始！" },
{ "DC3PP_4_1223_B1_AOI", "欢迎光临，小葵" },
{ "DC3PP_4_1223_B1_HMN", "圣诞派对开幕！" },
{ "DC3PP_4_1223_B1_SRR", "圣诞派对约会" },
{ "DC3PP_4_1223_B2_HMN", "欢迎诸位光临" },
{ "DC3PP_4_1223_D1_HMN", "明天陪着你" },
{ "DC3PP_4_1223_D1_SRA", "和恋人度过的特别时光" },
{ "DC3PP_4_1223_D1_SRR", "各自的圣诞派对" },
{ "DC3PP_4_1223_E1_RCC", "前夜祭" },
{ "DC3PP_4_1223_E1_SRR", "为明天做准备装饰教室" },
{ "DC3PP_4_1223_F1_AOI", "与勤劳少女的归途" },
{ "DC3PP_4_1223_F1_SRR", "突击！　立夏的房间" },
{ "DC3PP_4_1224_A1_SRA", "最初的礼物" },
{ "DC3PP_4_1224_B1_AOI", "最佳情侣？！" },
{ "DC3PP_4_1224_B1_HMN", "和姬乃在一起" },
{ "DC3PP_4_1224_B1_RCC", "圣诞派对" },
{ "DC3PP_4_1224_B2_AOI", "和小葵的圣诞派对约会" },
{ "DC3PP_4_1224_D1_SRA", "纱罗参加选美比赛！？" },
{ "DC3PP_4_1224_D1_SRR", "露露姐＋圣诞装扮＝？" },
{ "DC3PP_4_1224_D2_SRR", "紧急出场、选美比赛主持人！" },
{ "DC3PP_4_1224_E1_SRR", "孩子们露出笑容" },
{ "DC3PP_4_1224_E2_SRR", "夏露露的祈愿故事" },
{ "DC3PP_4_1224_F1_AOI", "恋人的证明" },
{ "DC3PP_4_1224_F1_HMN", "飘雪之中，念者惟君" },
{ "DC3PP_4_1224_F1_RCC", "接吻前的几厘米" },
{ "DC3PP_4_1224_F1_SRA", "新婚夫妇一样的生活" },
{ "DC3PP_4_1224_F1_SRR", "辛苦大家了！" },
{ "DC3PP_4_1224_F2_AOI", "雪花纷飞的平安夜" },
{ "DC3PP_4_1224_F2_HMN", "我想知晓" },
{ "DC3PP_4_1224_F2_SRR", "礼物就是――我?哦" },
{ "DC3PP_4_1224_F3_SRR", "白色圣诞节" },
{ "DC3PP_4_1225_A1_AOI", "有点缺油" },
{ "DC3PP_4_1225_A1_HMN", "Ｗｈｏ　ａｍ　Ｉ？！" },
{ "DC3PP_4_1225_A1_SRA", "在房间里卿卿我我" },
{ "DC3PP_4_1225_B1_HMN", "我变成姬乃，姬乃变成我" },
{ "DC3PP_4_1225_F1_AOI", "照看公主殿下" },
{ "DC3PP_4_1225_F1_HMN", "女体的神秘" },
{ "DC3PP_4_1225_F1_SRA", "说服的方法" },
{ "DC3PP_4_1226_A1_AOI", "饿肚子的露露姐" },
{ "DC3PP_4_1226_A1_HMN", "清早的葛木家" },
{ "DC3PP_4_1226_B1_RCC", "寒假也要社团活动" },
{ "DC3PP_4_1226_C1_HMN", "双方的事" },
{ "DC3PP_4_1226_D1_HMN", "美琴的直觉" },
{ "DC3PP_4_1226_D1_RCC", "调查ｏｒ约会" },
{ "DC3PP_4_1226_F1_AOI", "切勿硬撑" },
{ "DC3PP_4_1226_F1_RCC", "有什么事情正在发生" },
{ "DC3PP_4_1227_A1_AOI", "陪我睡觉吧" },
{ "DC3PP_4_1227_B1_HMN", "哥哥眼中的景色" },
{ "DC3PP_4_1227_C1_HMN", "巧克力香蕉间吻！" },
{ "DC3PP_4_1227_E1_AOI", "刺激的更衣" },
{ "DC3PP_4_1227_F1_AOI", "喂食" },
{ "DC3PP_4_1227_F1_HMN", "重要的回忆" },
{ "DC3PP_4_1228_A1_AOI", "一起去看医生吧" },
{ "DC3PP_4_1228_B1_AOI", "在超市购物" },
{ "DC3PP_4_1228_B1_HMN", "小葵的追问" },
{ "DC3PP_4_1228_D1_HMN", "纱罗的洞察" },
{ "DC3PP_4_1228_E1_HMN", "被立夏看透" },
{ "DC3PP_4_1228_F1_AOI", "今天的主菜" },
{ "DC3PP_4_1229_A1_AOI", "恶化的身体状况" },
{ "DC3PP_4_1229_B1_AOI", "能依靠的人是……" },
{ "DC3PP_4_1229_B1_HMN", "樱的建议" },
{ "DC3PP_4_1229_B1_RCC", "寻找类似的案例" },
{ "DC3PP_4_1229_E1_AOI", "残酷的假说" },
{ "DC3PP_4_1229_E1_HMN", "更深、更深……" },
{ "DC3PP_4_1229_E1_RCC", "不枯樱的调查" },
{ "DC3PP_4_1229_F1_AOI", "为了救命" },
{ "DC3PP_4_1229_F1_HMN", "在一张被子里" },
{ "DC3PP_4_1229_F1_SRA", "稍微提早的除夕荞麦面" },
{ "DC3PP_4_1230_A1_AOI", "笑容之下" },
{ "DC3PP_4_1230_A1_HMN", "这次才是真的" },
{ "DC3PP_4_1230_B1_SRA", "纱罗的老家" },
{ "DC3PP_4_1230_E1_AOI", "大家的探望" },
{ "DC3PP_4_1230_F1_HMN", "洗后背啦！" },
{ "DC3PP_4_1230_F1_SRA", "瑠川家的一家团聚" },
{ "DC3PP_4_1230_F2_SRA", "理想的夫妻形象" },
{ "DC3PP_4_1231_A1_AOI", "早上和小葵泡澡" },
{ "DC3PP_4_1231_A1_SRA", "残酷的真相" },
{ "DC3PP_4_1231_B1_AOI", "来做大扫除吧" },
{ "DC3PP_4_1231_B1_HMN", "二人独处的除夕" },
{ "DC3PP_4_1231_D1_RCC", "跨年在我家?" },
{ "DC3PP_4_1231_E1_RCC", "谜之少女" },
{ "DC3PP_4_1231_E1_SRA", "真相的坦白" },
{ "DC3PP_4_1231_F1_AOI", "两个人跨年" },
{ "DC3PP_4_1231_F1_HMN", "特别的跨年" },
{ "DC3PP_4_1231_F1_SRA", "除夕的升空焰火" },
{ "DC3PP_4_1231_F2_HMN", "在元旦日出中起誓" },
{ "DC3PP_4_1301_A1_SRA", "瑠川家度过的元旦" },
{ "DC3PP_4_1301_B1_RCC", "一年之计在于？" },
{ "DC3PP_4_1301_E1_RCC", "正义的魔法使１" },
{ "DC3PP_4_1301_E1_SRA", "能为父亲做的事" },
{ "DC3PP_4_1301_F1_AOI", "与小葵的新年参拜约会" },
{ "DC3PP_4_1301_F1_SRA", "投接球邀请方法" },
{ "DC3PP_4_1301_F2_AOI", "留下两人的回忆" },
{ "DC3PP_4_1302_A1_SRA", "托付的球" },
{ "DC3PP_4_1302_B1_AOI", "来写新年贺卡吧" },
{ "DC3PP_4_1302_B1_RCC", "睡不够的正月" },
{ "DC3PP_4_1302_C1_SRA", "一片光明的人生设计" },
{ "DC3PP_4_1302_D1_RCC", "冷淡的立夏" },
{ "DC3PP_4_1302_E1_AOI", "寄达不到的肖像画" },
{ "DC3PP_4_1303_B1_RCC", "迟来的新年参拜" },
{ "DC3PP_4_1304_B1_AOI", "珍贵的笑容" },
{ "DC3PP_4_1305_A1_SRA", "我回来了，初音岛" },
{ "DC3PP_4_1305_B1_RCC", "不管什么事都提立夏" },
{ "DC3PP_4_1308_B1_RCC", "纪念的吻" },
{ "DC3PP_4_1308_F1_RCC", "思念你" },
{ "DC3PP_4_1309_B1_AOI", "为了明天" },
{ "DC3PP_4_1309_D1_RCC", "温暖的地方" },
{ "DC3PP_4_1309_F1_RCC", "肌肤之亲" },
{ "DC3PP_4_1310_B1_AOI", "　伴随着快乐回忆的是。" },
{ "DC3PP_4_1310_F1_AOI", "永别了……" },
{ "DC3PP_4_1310_F2_AOI", "最喜欢又最重要的……" },
{ "DC3PP_4_1311_A1_RCC", "不安的风" },
{ "DC3PP_4_1311_A1_SRA", "让父亲安心的办法" },
{ "DC3PP_4_1311_B1_AOI", "忘记不了的思念" },
{ "DC3PP_4_1311_B1_RCC", "风暴前的宁静" },
{ "DC3PP_4_1311_B2_AOI", "祝福之钟" },
{ "DC3PP_4_1311_E1_RCC", "忘却之海" },
{ "DC3PP_4_1311_E1_SRA", "为了参加送别赛" },
{ "DC3PP_4_1311_F1_RCC", "自觉" },
{ "DC3PP_4_1312_B10_RCC", "寻找那个少女" },
{ "DC3PP_4_1312_B11_RCC", "和柚一起搜索" },
{ "DC3PP_4_1312_B12_RCC", "和夕阳一起搜索" },
{ "DC3PP_4_1312_E1_RCC", "正义的魔法使２" },
{ "DC3PP_4_1312_E1_SRA", "靠理论和努力来弥补的运动神经" },
{ "DC3PP_4_1312_E2_RCC", "打错的电话" },
{ "DC3PP_4_1313_F1_RCC", "立夏的心思１" },
{ "DC3PP_4_1313_F2_RCC", "立夏的心思２" },
{ "DC3PP_4_1316_A1_RCC", "一个大决心" },
{ "DC3PP_4_1316_E1_RCC", "百年的思念" },
{ "DC3PP_4_1401_E1_SRA", "纱罗擅长的领域" },
{ "DC3PP_4_1414_E1_SRA", "情人节?礼物" },
{ "DC3PP_4_1420_E1_SRA", "努力的成果" },
{ "DC3PP_4_1500_A1_RCC", "直到永远" },
{ "DC3PP_4_1513_E1_SRA", "白色情人节?礼物" },
{ "DC3PP_4_1515_A1_SRA", "狂欢的毕业典礼" },
{ "DC3PP_4_1515_D1_SRA", "送别比赛，打响！" },
{ "DC3PP_4_1515_E1_SRA", "想要传达的话语" },
{ "DC3PP_4_9999_A1_SRA", "幸福的家庭计划" }
};

void CopyBlockCHS()
{
	memcpy((void*)(BaseAddr + 0x88460), SaveTitleCHS, sizeof(SaveTitleCHS));
}

/**********************************************************************************************************************/

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//make_console();
		CopyBlockCHS();
		Init();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void dummy(void) {
	return;
}