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
	// ���z�֐�

	// ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q��
	LRESULT OnLButtonDown(HWND, UINT, WPARAM, LPARAM);
	LRESULT OnRButtonDown(HWND, UINT, WPARAM, LPARAM);
	LRESULT OnMButtonDown(HWND, UINT, WPARAM, LPARAM);
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);
};

