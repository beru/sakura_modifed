#pragma once

#include "Wnd.h"
class EditView;

class AutoScrollWnd : public Wnd {
public:
	AutoScrollWnd();
	virtual ~AutoScrollWnd();
	HWND Create(HINSTANCE, HWND , bool, bool, const Point&, EditView*);
	void Close();

private:
	HBITMAP hCenterImg;
	EditView* pView;
protected:
	// 仮想関数

	// 仮想関数 メッセージ処理 詳しくは実装を参照
	LRESULT OnLButtonDown(HWND, UINT, WPARAM, LPARAM);
	LRESULT OnRButtonDown(HWND, UINT, WPARAM, LPARAM);
	LRESULT OnMButtonDown(HWND, UINT, WPARAM, LPARAM);
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);
};

