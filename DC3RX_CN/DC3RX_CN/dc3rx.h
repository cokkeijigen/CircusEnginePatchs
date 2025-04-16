#pragma once
#include "chapter_title.h"
// 0x7D3EC
const char* characterNames[] = {
	"葵",					// 葵
	"纱罗",					// さら
	"莎拉",					// サラ
	"姬乃",					// 姫乃
	"立夏",					// 立夏
	"莉卡",					// リッカ
	"夏露露",				// シャルル
	"杉并",					// 杉並
	"伊丽莎白",				// エリザベス
	"吉尔",					// ジル
	"伊恩",					// イアン
	"耕助",					// 耕助
	"玛丽",					// メアリー
	"美琴",					// 美琴
	"美夏",					// 美夏
	"瑠璃香",				// 瑠璃香
	"樱",					// さくら
	"巴",					// 巴
	"艾德华",				// エドワード
	"夕阳",					// 夕陽
	"柚子",				  	// ゆず
	"清隆",				  	// 清隆
	"艾德",				  	// エト
	"四季",				  	// 四季
	"葵的声音",				// 葵の声
	"莎拉的声音",			// サラの声
	"姬乃的声音",			// 姫乃の声
	"莉卡的声音",			// リッカの声
	"夏露露的声音",		   	// シャルルの声
	"杉并的声音",			// 杉並の声
	"伊恩的声音",			// イアンの声
	"耕助的声音",			// 耕助の声
	"瑠璃香的声音",			// 瑠璃香の声
	"樱的声音",				// さくらの声
	"音姬",				  	// 音姫
	"由梦",				  	// 由夢
	"茜",				   	// 茜
	"杏",				   	// 杏
	"艾莉卡",				// エリカ
	"小恋",				  	// 小恋
	"麻耶",				  	// 麻耶
	"美夏",					// 美夏
	"真由纪",				// まゆき
	"奈奈香",				// ななか
	"渉",				   	// 渉
	"义之"				  	// 義之
};

void _dataInit() {
	BYTE ncount = 0;
	DWORD start = BaseAddr + 0x7D3EC;
	DWORD endpos = BaseAddr + 0x7D55C;
	for (DWORD addr = start; addr < endpos; addr += 8)
		memcpy((void*)addr,  (void*)&characterNames[ncount++], 4);
	memcpy((void*)(BaseAddr + 0x884A8), chapterTitles, sizeof(chapterTitles));
}

char* getTitle(FILE* chsinfo_fp) {
	fseek(chsinfo_fp, 0L, 0);
	char* title = new char[0xff];
	const char* _title = "title=";
	int ch;
	unsigned char i = 0;
	bool can_read = false;
	while (EOF != (ch = fgetc(chsinfo_fp))) {
		if (!can_read && (char)ch == _title[i]) {
			if (i == 5) {
				can_read = true;
				i = 0;
				continue;
			}
			i++;
		}
		else if (!can_read && ch == 0x20 && i != 5) {
			i = 0;
		}
		else if (can_read) {
			if (ch == 0x0a || i == 0xfe) break;
			if (i == 0 && ch == 0x20) continue;
			title[i++] = (char)ch;
		}
	}
	title[i] = '\0';
	return title;
}

char* getVerMSG(FILE* chsinfo_fp) {
	fseek(chsinfo_fp, 0L, 0);
	char* msg = new char[0xff];
	const char* _msg = "msg=";
	int ch;
	unsigned char i = 0;
	bool can_read = false;
	while (EOF != (ch = fgetc(chsinfo_fp))) {
		if (!can_read && (char)ch == _msg[i]) {
			if (i == 3) {
				can_read = true;
				i = 0;
				continue;
			}
			i++;
		}
		else if (!can_read && ch == 0x20 && i != 3) {
			i = 0;
		}
		else if (can_read) {
			if (ch == 0x0a || i == 0xfe) break;
			if (i == 0 && ch == 0x20) continue;
			if (ch == 0x5c) ch = (int)'\n';
			msg[i++] = (char)ch;
		}
	}
	msg[i] = '\0';
	return msg;
}