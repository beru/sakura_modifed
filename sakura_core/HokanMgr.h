#pragma once

// �L�[���[�h�⊮

#include <Windows.h>
#include "dlg/Dialog.h"
#include "util/container.h"


/*! @brief �L�[���[�h�⊮ */
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
	size_t Search(
		POINT*			ppoWin,
		int				nWinHeight,
		int				nColumnWidth,
		const wchar_t*	pszCurWord,
		const TCHAR*	pszHokanFile,
		bool			bHokanLoHiCase,			// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ����
		bool			bHokanByFile,			// �ҏW���f�[�^�������T��
		int				nHokanType,
		bool			bHokanByKeyword,
		NativeW*		pMemHokanWord = nullptr	// �⊮��₪�P�̂Ƃ�����Ɋi�[
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

	void ShowTip();	// �⊮�E�B���h�E�őI�𒆂̒P��ɃL�[���[�h�w���v�̕\��

	static bool AddKouhoUnique(vector_ex<std::wstring>&, const std::wstring&);

	NativeW			memCurWord;
	vector_ex<std::wstring>	vKouho;
	int				nKouhoNum;

	int				nCurKouhoIdx;

	POINT			point;
	int				nWinHeight;
	int				nColumnWidth;
	int				bTimerFlag;

protected:
	/*
	||  �����w���p�֐�
	*/
	LPVOID GetHelpIdTable(void);

};

