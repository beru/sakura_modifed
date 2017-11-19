/*!	@file
	@brief �t�@���N�V�����L�[�E�B���h�E
*/
#pragma once

#include "window/Wnd.h"
#include "env/DllSharedData.h"

struct DllSharedData;
class EditDoc;

// �t�@���N�V�����L�[�E�B���h�E
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
	void Timer_ONOFF(bool); // �X�V�̊J�n�^��~
	/*
	|| �����o�ϐ�
	*/
private:
	// ���ׂ�Private�ɂ��āA�����������ɍ��킹�ĕ��בւ�
	EditDoc*		pEditDoc;
	DllSharedData&	shareData;
	int				nCurrentKeyState;
	wchar_t			szFuncNameArr[12][256];
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
	virtual void AfterCreateWindow(void) {}	// �E�B���h�E�쐬��̏���
	
	// ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q��
	virtual LRESULT OnTimer(HWND, UINT, WPARAM, LPARAM);	// WM_TIMER�^�C�}�[�̏���
	virtual LRESULT OnCommand(HWND, UINT, WPARAM, LPARAM);	// WM_COMMAND����
	virtual LRESULT OnSize(HWND, UINT, WPARAM, LPARAM);		// WM_SIZE����
	virtual LRESULT OnDestroy(HWND, UINT, WPARAM, LPARAM);	// WM_DESTROY����
};

