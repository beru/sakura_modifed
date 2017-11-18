#pragma once

class SplitBoxWnd;

#include "Wnd.h"


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �����{�b�N�X�E�B���h�E�N���X
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
	
//	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// ���b�Z�[�W�f�B�X�p�b�`��
	
private:
	int	bVertical;	// ���������{�b�N�X��
	int	nDragPosY;
	int	nDragPosX;
protected:
	// ���z�֐�
	
	// ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q��
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);			// �`�揈��
	LRESULT OnLButtonDown(HWND, UINT, WPARAM, LPARAM);		// WM_LBUTTONDOWN
	LRESULT OnMouseMove(HWND, UINT, WPARAM, LPARAM);		// WM_MOUSEMOVE
	LRESULT OnLButtonUp(HWND, UINT, WPARAM, LPARAM);		// WM_LBUTTONUP
	LRESULT OnLButtonDblClk(HWND, UINT, WPARAM, LPARAM);	// WM_LBUTTONDBLCLK


};

