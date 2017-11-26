#pragma once

class TipWnd;

#include "Wnd.h"
#include "mem/Memory.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class TipWnd : public Wnd {
public:
	/*
	||  Constructors
	*/
	TipWnd();
	~TipWnd();
	void Create(HINSTANCE, HWND);	// ������

	/*
	||  Attributes & Operations
	*/
	void Show(int, int, const TCHAR*, RECT* pRect = nullptr);	// Tip��\��
	void Hide(void);	// Tip������
	void GetWindowSize(LPRECT pRect);		// �E�B���h�E�̃T�C�Y�𓾂�

	void ChangeFont(LOGFONT* lf) {
		if (hFont) {
			::DeleteObject(hFont);
		}
		hFont = ::CreateFontIndirect(lf);
	}

protected:
	HFONT		hFont;

public:
	NativeW		key;			// �L�[�̓��e�f�[�^
	bool		KeyWasHit;		// �L�[���q�b�g������
	int			nSearchLine;	// �����̃q�b�g�s
	int			nSearchDict;	// �q�b�g�����ԍ�

	NativeT		info;			// Tip�̓��e�f�[�^
	bool		bAlignLeft;		// �E�������Ń`�b�v��\��

protected:
	/*
	||  �����w���p�֐�
	*/
	void ComputeWindowSize(HDC, HFONT, const TCHAR*, RECT*);	// �E�B���h�E�̃T�C�Y�����߂�
	void DrawTipText(HDC, HFONT, const TCHAR*);	// �E�B���h�E�̃e�L�X�g��\��

	// ���z�֐�
	virtual void AfterCreateWindow(void);

	// ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q��
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);	// �`�揈��
};


