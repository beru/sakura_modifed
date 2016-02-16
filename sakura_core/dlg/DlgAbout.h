/*!	@file
	@brief �o�[�W�������_�C�A���O

	@author Norio Nakatani
	@date 1998/05/22 �쐬
	@date 1999/12/05 �č쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief About Box�Ǘ�
	
	DispatchEvent��Ǝ��ɒ�`���邱�ƂŁCDialog�ŃT�|�[�g����Ă��Ȃ�
	���b�Z�[�W��ߑ�����D
*/

class UrlWnd {
public:
	UrlWnd() { m_hWnd = NULL; m_hFont = NULL; m_bHilighted = FALSE; m_pOldProc = NULL; }
	virtual ~UrlWnd() { ; }
	BOOL SetSubclassWindow(HWND hWnd);
	HWND GetHwnd() const { return m_hWnd; }
protected:
	static LRESULT CALLBACK UrlWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
protected:
	HWND m_hWnd;
	HFONT m_hFont;
	BOOL m_bHilighted;
	WNDPROC m_pOldProc;
};

class DlgAbout : public Dialog {
public:
	int DoModal(HINSTANCE, HWND);	// ���[�_���_�C�A���O�̕\��
	// Nov. 7, 2000 genta	�W���ȊO�̃��b�Z�[�W��ߑ�����
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
protected:
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	BOOL OnStnClicked(int);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
private:
	UrlWnd m_UrlUrWnd;
	UrlWnd m_UrlOrgWnd;
};

