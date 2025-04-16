#pragma once
#include "chapter_title.h"
// 0x7D3EC
const char* characterNames[] = {
	"��",					// ��
	"ɴ��",					// ����
	"ɯ��",					// ����
	"����",					// ����
	"����",					// ����
	"��",					// ��å�
	"��¶¶",				// ������
	"ɼ��",					// ɼ�K
	"����ɯ��",				// ���ꥶ�٥�
	"����",					// ����
	"����",					// ������
	"����",					// ����
	"����",					// �ᥢ��`
	"����",					// ����
	"����",					// ����
	"������",				// ������
	"ӣ",					// ������
	"��",					// ��
	"���»�",				// ���ɥ�`��
	"Ϧ��",					// Ϧ�
	"����",				  	// �椺
	"��¡",				  	// ��¡
	"����",				  	// ����
	"�ļ�",				  	// �ļ�
	"��������",				// ������
	"ɯ��������",			// �������
	"���˵�����",			// ���ˤ���
	"�򿨵�����",			// ��å�����
	"��¶¶������",		   	// ���������
	"ɼ��������",			// ɼ�K����
	"����������",			// ���������
	"����������",			// ��������
	"�����������",			// ���������
	"ӣ������",				// ���������
	"����",				  	// ��
	"����",				  	// �ɉ�
	"��",				   	// ��
	"��",				   	// ��
	"����",				// ���ꥫ
	"С��",				  	// С��
	"��Ү",				  	// ��Ү
	"����",					// ����
	"���ɼ�",				// �ޤ椭
	"������",				// �ʤʤ�
	"�h",				   	// �h
	"��֮"				  	// �x֮
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