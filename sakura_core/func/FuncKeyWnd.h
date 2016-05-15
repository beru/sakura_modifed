/*!	@file
	@brief �t�@���N�V�����L�[�E�B���h�E

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#pragma once

#include "window/Wnd.h"
#include "env/DllSharedData.h"

struct DllSharedData;
class EditDoc; // 2002/2/10 aroka

// �t�@���N�V�����L�[�E�B���h�E
// @date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
class FuncKeyWnd : public Wnd {
public:
	/*
	||  Constructors
	*/
	FuncKeyWnd();
	virtual ~FuncKeyWnd();
	/*
	|| �����o�֐�
	*/
	HWND Open(HINSTANCE, HWND, EditDoc*, bool);	// �E�B���h�E �I�[�v��
	void Close(void);	// �E�B���h�E �N���[�Y
	void SizeBox_ONOFF(bool);	// �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ�
	void Timer_ONOFF(bool); // �X�V�̊J�n�^��~ 20060126 aroka
	/*
	|| �����o�ϐ�
	*/
private:
	// 20060126 aroka ���ׂ�Private�ɂ��āA�����������ɍ��킹�ĕ��בւ�
	EditDoc*		pEditDoc;
	DllSharedData&	shareData;
	int				nCurrentKeyState;
	WCHAR			szFuncNameArr[12][256];
	HWND			hwndButtonArr[12];
	HFONT			hFont;	// �\���p�t�H���g
	bool			bSizeBox;
	HWND			hwndSizeBox;
	int				nTimerCount;
	int				nButtonGroupNum;	// Open�ŏ�����
	EFunctionCode	nFuncCodeArr[12];	// Open->CreateButtons�ŏ�����
protected:
	/*
	|| �����w���p�n
	*/
	void CreateButtons(void);	// �{�^���̐���
	int CalcButtonSize(void);	// �{�^���̃T�C�Y���v�Z
	
	// ���z�֐�
	virtual void AfterCreateWindow(void) {}	// �E�B���h�E�쐬��̏���	// 2007.03.13 ryoji �������Ȃ�
	
	// ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q��
	virtual LRESULT OnTimer(HWND, UINT, WPARAM, LPARAM);	// WM_TIMER�^�C�}�[�̏���
	virtual LRESULT OnCommand(HWND, UINT, WPARAM, LPARAM);	// WM_COMMAND����
	virtual LRESULT OnSize(HWND, UINT, WPARAM, LPARAM);		// WM_SIZE����
	virtual LRESULT OnDestroy(HWND, UINT, WPARAM, LPARAM);	// WM_DESTROY����
};

