#pragma once

#include "Wnd.h"

struct DllSharedData;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/

#define MAXCOUNTOFVIEW	4

/*!
	@brief �������E�B���h�E�N���X
	
	�S�����E�B���h�E�̊Ǘ��ƕ������̕`����s���B
*/
class SplitterWnd : public Wnd {
public:
	/*
	||  Constructors
	*/
	SplitterWnd();
	~SplitterWnd();
private:
	/*
	||  Attributes & Operations
	*/
	DllSharedData*	pShareData;
	EditWnd*		pEditWnd;
	int				nAllSplitRows;		// �����s��
	int				nAllSplitCols;		// ��������
	int				nVSplitPos;			// ���������ʒu
	int				nHSplitPos;			// ���������ʒu
	HWND			childWndArr[MAXCOUNTOFVIEW];		// �q�E�B���h�E�z��
	int				nChildWndCount;		// �L���Ȏq�E�B���h�E�z��̐�
	HCURSOR			hcurOld;			// ���Ƃ̃}�E�X�J�[�\��
	int				bDragging;			// �����o�[���h���b�O����
	int				nDragPosX;			// �h���b�O�ʒu�w
	int				nDragPosY;			// �h���b�O�ʒu�x
	int				nActivePane;		// �A�N�e�B�u�ȃy�C��
public:
	HWND Create(HINSTANCE, HWND, EditWnd* pEditWnd);	// ������
	void SetChildWndArr(HWND*);	// �q�E�B���h�E�̐ݒ� 
	void DoSplit(int, int);		// �E�B���h�E�̕���
	void SetActivePane(int);	// �A�N�e�B�u�y�C���̐ݒ�
	int GetPrevPane(void);		// �O�̃y�C����Ԃ�
	int GetNextPane(void);		// ���̃y�C����Ԃ�
	int GetFirstPane(void);		// �ŏ��̃y�C����Ԃ�
	int GetLastPane(void);		// �Ō�̃y�C����Ԃ�

	void VSplitOnOff(void);		// �c�����n�m�^�n�e�e
	void HSplitOnOff(void);		// �������n�m�^�n�e�e
	void VHSplitOnOff(void);	// �c�������n�m�^�n�e�e
//	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// �_�C�A���O�̃��b�Z�[�W����
	int GetAllSplitRows() { return nAllSplitRows;}
	int GetAllSplitCols() { return nAllSplitCols;}
protected:
	// ���z�֐�
	virtual LRESULT DispatchEvent_WM_APP(HWND, UINT, WPARAM, LPARAM);	// �A�v���P�[�V������`�̃��b�Z�[�W(WM_APP <= msg <= 0xBFFF)

	// ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q��
	virtual LRESULT OnSize(HWND, UINT, WPARAM, LPARAM);				// �E�B���h�E�T�C�Y�̕ύX����
	virtual LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);			// �`�揈��
	virtual LRESULT OnMouseMove(HWND, UINT, WPARAM, LPARAM);		// �}�E�X�ړ����̏���
	virtual LRESULT OnLButtonDown(HWND, UINT, WPARAM, LPARAM);		// �}�E�X���{�^���������̏���
	virtual LRESULT OnLButtonUp(HWND, UINT, WPARAM, LPARAM);		// �}�E�X���{�^��������̏���
	virtual LRESULT OnLButtonDblClk(HWND, UINT, WPARAM, LPARAM);	// �}�E�X���{�^���_�u���N���b�N���̏���
	/*
	||  �����w���p�֐�
	*/
	void DrawFrame(HDC , RECT*);			// �����t���[���`��
	int HitTestSplitter(int, int);		// �����o�[�ւ̃q�b�g�e�X�g
	void DrawSplitter(int, int, int);	// �����g���b�J�[�̕\��

};



