// �����L�[���[�h�I���_�C�A���O

#pragma once

#include "dlg/Dialog.h"
#include "config/maxdata.h" // MAX_KEYWORDSET_PER_TYPE
 
class KeywordSetMgr;

/*
	�����L�[���[�h�I���\��
	1�`10�͈̔͂Ŏw��ł���B
	�������A�\�[�X�̏C���͕K�v�ł��B
*/

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

