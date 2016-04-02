/*!	@file
	@brief �L�[���[�h�⊮

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o
	Copyright (C) 2003, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "dlg/Dialog.h"
#include "util/container.h"


/*! @brief �L�[���[�h�⊮

	@date 2003.06.25 Moca �t�@�C��������̕⊮�@�\��ǉ�
*/
class HokanMgr : public Dialog {
public:
	/*
	||  Constructors
	*/
	HokanMgr();
	~HokanMgr();

	HWND DoModeless(HINSTANCE, HWND, LPARAM); // ���[�h���X�_�C�A���O�̕\��
	void Hide(void);
	// ������
	int Search(
		POINT*			ppoWin,
		int				nWinHeight,
		int				nColumnWidth,
		const wchar_t*	pszCurWord,
		const TCHAR*	pszHokanFile,
		bool			bHokanLoHiCase,			// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� 2001/06/19 asa-o
		bool			bHokanByFile,			// �ҏW���f�[�^�������T���B 2003.06.23 Moca
		int				nHokanType,
		bool			bHokanByKeyword,
		NativeW*		pMemHokanWord = nullptr	// �⊮��₪�P�̂Ƃ�����Ɋi�[ 2001/06/19 asa-o
	);
	void HokanSearchByKeyword(
		const wchar_t*	pszCurWord,
		bool 			bHokanLoHiCase,
		vector_ex<std::wstring>& 	vKouho
	);
//	void SetCurKouhoStr(void);
	BOOL DoHokan(int);
	void ChangeView(LPARAM); // ���[�h���X���F�ΏۂƂȂ�r���[�̕ύX


	BOOL OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	BOOL OnDestroy(void);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnBnClicked(int wID);
	BOOL OnKeyDown(WPARAM wParam, LPARAM lParam);
	BOOL OnLbnSelChange(HWND hwndCtl, int wID);
	BOOL OnLbnDblclk(int wID);
	BOOL OnKillFocus(WPARAM wParam, LPARAM lParam);
//	int OnVKeyToItem(WPARAM wParam, LPARAM lParam);
//	int OnCharToItem(WPARAM wParam, LPARAM lParam);

	int KeyProc(WPARAM, LPARAM);

//	2001/06/18 asa-o
	void ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v�̕\��

	static bool AddKouhoUnique(vector_ex<std::wstring>&, const std::wstring&);

	NativeW			m_memCurWord;
	vector_ex<std::wstring>	m_vKouho;
	int				m_nKouhoNum;

	int				m_nCurKouhoIdx;

	POINT			m_point;
	int				m_nWinHeight;
	int				m_nColumnWidth;
	int				m_bTimerFlag;

protected:
	/*
	||  �����w���p�֐�
	*/
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

};

