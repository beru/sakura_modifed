#pragma once

class SplitBoxWnd;

#include "Wnd.h"


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 分割ボックスウィンドウクラス
*/
class SplitBoxWnd : public Wnd {
public:
	/*
	||  Constructors
	*/
	SplitBoxWnd();
	virtual ~SplitBoxWnd();
	HWND Create(HINSTANCE, HWND, int);
	
	static void Draw3dRect(HDC, int, int, int, int, COLORREF, COLORREF);
	static void FillSolidRect(HDC, int, int, int, int, COLORREF);
	
//	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// メッセージディスパッチャ
	
private:
	int	bVertical;	// 垂直分割ボックスか
	int	nDragPosY;
	int	nDragPosX;
protected:
	// 仮想関数
	
	// 仮想関数 メッセージ処理 詳しくは実装を参照
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);			// 描画処理
	LRESULT OnLButtonDown(HWND, UINT, WPARAM, LPARAM);		// WM_LBUTTONDOWN
	LRESULT OnMouseMove(HWND, UINT, WPARAM, LPARAM);		// WM_MOUSEMOVE
	LRESULT OnLButtonUp(HWND, UINT, WPARAM, LPARAM);		// WM_LBUTTONUP
	LRESULT OnLButtonDblClk(HWND, UINT, WPARAM, LPARAM);	// WM_LBUTTONDBLCLK


};

