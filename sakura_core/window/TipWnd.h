#pragma once

class TipWnd;

#include "Wnd.h"
#include "mem/Memory.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class TipWnd : public Wnd {
public:
	/*
	||  Constructors
	*/
	TipWnd();
	~TipWnd();
	void Create(HINSTANCE, HWND);	// 初期化

	/*
	||  Attributes & Operations
	*/
	void Show(int, int, const TCHAR*, RECT* pRect = nullptr);	// Tipを表示
	void Hide(void);	// Tipを消す
	void GetWindowSize(LPRECT pRect);		// ウィンドウのサイズを得る

	void ChangeFont(LOGFONT* lf) {
		if (hFont) {
			::DeleteObject(hFont);
		}
		hFont = ::CreateFontIndirect(lf);
	}

protected:
	HFONT		hFont;

public:
	NativeW		key;			// キーの内容データ
	bool		KeyWasHit;		// キーがヒットしたか
	int			nSearchLine;	// 辞書のヒット行
	int			nSearchDict;	// ヒット辞書番号

	NativeT		info;			// Tipの内容データ
	bool		bAlignLeft;		// 右側揃えでチップを表示

protected:
	/*
	||  実装ヘルパ関数
	*/
	void ComputeWindowSize(HDC, HFONT, const TCHAR*, RECT*);	// ウィンドウのサイズを決める
	void DrawTipText(HDC, HFONT, const TCHAR*);	// ウィンドウのテキストを表示

	// 仮想関数
	virtual void AfterCreateWindow(void);

	// 仮想関数 メッセージ処理 詳しくは実装を参照
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);	// 描画処理
};


