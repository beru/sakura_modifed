// 強調キーワード選択ダイアログ

#pragma once

#include "dlg/Dialog.h"
#include "config/maxdata.h" // MAX_KEYWORDSET_PER_TYPE
 
class KeywordSetMgr;

/*
	強調キーワード選択可能数
	1～10個の範囲で指定できる。
	ただし、ソースの修正は必要です。
*/

//	2005.01.13 genta ShareDataの定義と連動させる
const size_t KEYWORD_SELECT_NUM = MAX_KEYWORDSET_PER_TYPE;

class DlgKeywordSelect : public Dialog {
public:
	DlgKeywordSelect();
	~DlgKeywordSelect();
	INT_PTR DoModal(HINSTANCE, HWND, int* pnSet);

protected:

	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	int  GetData(void);
	void SetData(void);
	LPVOID GetHelpIdTable(void);

	int nSet[ KEYWORD_SELECT_NUM ];
	KeywordSetMgr*	pKeywordSetMgr;
};

