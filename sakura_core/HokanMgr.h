#pragma once

// キーワード補完

#include <Windows.h>
#include "dlg/Dialog.h"
#include "util/container.h"


/*! @brief キーワード補完 */
class HokanMgr : public Dialog {
public:
	/*
	||  Constructors
	*/
	HokanMgr();
	~HokanMgr();

	HWND DoModeless(HINSTANCE, HWND, LPARAM); // モードレスダイアログの表示
	void Hide(void);
	// 初期化
	size_t Search(
		POINT*			ppoWin,
		int				nWinHeight,
		int				nColumnWidth,
		const wchar_t*	pszCurWord,
		const TCHAR*	pszHokanFile,
		bool			bHokanLoHiCase,			// 入力補完機能：英大文字小文字を同一視する
		bool			bHokanByFile,			// 編集中データから候補を探す
		int				nHokanType,
		bool			bHokanByKeyword,
		NativeW*		pMemHokanWord = nullptr	// 補完候補が１つのときこれに格納
	);
	void HokanSearchByKeyword(
		const wchar_t*	pszCurWord,
		bool 			bHokanLoHiCase,
		vector_ex<std::wstring>& 	vKouho
	);
//	void SetCurKouhoStr(void);
	BOOL DoHokan(int);
	void ChangeView(LPARAM); // モードレス時：対象となるビューの変更


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

	void ShowTip();	// 補完ウィンドウで選択中の単語にキーワードヘルプの表示

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
	||  実装ヘルパ関数
	*/
	LPVOID GetHelpIdTable(void);

};

