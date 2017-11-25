#pragma once

#include "Wnd.h"
#include "util/design_template.h"

class Graphics;
struct EditNode;
struct DllSharedData;

// �^�u�o�[�E�B���h�E
class TabWnd : public Wnd {
public:
	/*
	||  Constructors
	*/
	TabWnd();
	virtual ~TabWnd();

	/*
	|| �����o�֐�
	*/
	HWND Open(HINSTANCE, HWND);		// �E�B���h�E �I�[�v��
	void Close(void);				// �E�B���h�E �N���[�Y
	void TabWindowNotify(WPARAM wParam, LPARAM lParam);
	void Refresh(bool bEnsureVisible = true, bool bRebuild = false);
	void NextGroup(void);			// ���̃O���[�v
	void PrevGroup(void);			// �O�̃O���[�v
	void MoveRight(void);			// �^�u���E�Ɉړ�
	void MoveLeft(void);			// �^�u�����Ɉړ�
	void Separate(void);			// �V�K�O���[�v
	void JoinNext(void);			// ���̃O���[�v�Ɉړ�
	void JoinPrev(void);			// �O�̃O���[�v�Ɉړ�

	LRESULT TabWndDispatchEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT TabListMenu(POINT pt, bool bSel = true, bool bFull = false, bool bOtherGroup = true);	// �^�u�ꗗ���j���[�쐬����	// 2006.03.23 fon

	void SizeBox_ONOFF(bool bSizeBox);
	HWND GetHwndSizeBox() {
		return hwndSizeBox;
	}
	void OnSize() {
		OnSize( GetHwnd(), WM_SIZE, 0, 0 );
	}
	void UpdateStyle();
protected:
	/*
	|| �����w���p�n
	*/
	int FindTabIndexByHWND(HWND hWnd);
	void AdjustWindowPlacement(void);							// �ҏW�E�B���h�E�̈ʒu���킹
	int SetCarmWindowPlacement(HWND hwnd, const WINDOWPLACEMENT* pWndpl);	// �A�N�e�B�u���̏��Ȃ� SetWindowPlacement() �����s����
	void ShowHideWindow(HWND hwnd, BOOL bDisp);
	void HideOtherWindows(HWND hwndExclude);					// ���̕ҏW�E�B���h�E���B��
	void ForceActiveWindow(HWND hwnd);
	void TabWnd_ActivateFrameWindow(HWND hwnd, bool bForce = true);
	HWND GetNextGroupWnd(void);	// ���̃O���[�v�̐擪�E�B���h�E��T��
	HWND GetPrevGroupWnd(void);	// �O�̃O���[�v�̐擪�E�B���h�E��T��
	void GetTabName(EditNode* pEditNode, bool bFull, bool bDupamp, LPTSTR pszName, size_t nLen);	// �^�u���擾����

	// ���z�֐�
	virtual void AfterCreateWindow(void) {}	// �E�B���h�E�쐬��̏���

	// ���z�֐� ���b�Z�[�W����
	virtual LRESULT OnSize(HWND, UINT, WPARAM, LPARAM);				// WM_SIZE����
	virtual LRESULT OnDestroy(HWND, UINT, WPARAM, LPARAM);			// WM_DSESTROY����
	virtual LRESULT OnNotify(HWND, UINT, WPARAM, LPARAM);			// WM_NOTIFY����
	virtual LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);			// WM_PAINT����
	virtual LRESULT OnCaptureChanged(HWND, UINT, WPARAM, LPARAM);	// WM_CAPTURECHANGED ����
	virtual LRESULT OnLButtonDown(HWND, UINT, WPARAM, LPARAM);		// WM_LBUTTONDOWN����
	virtual LRESULT OnLButtonUp(HWND, UINT, WPARAM, LPARAM);		// WM_LBUTTONUP����
	virtual LRESULT OnRButtonDown(HWND, UINT, WPARAM, LPARAM);		// WM_RBUTTONDOWN����
	virtual LRESULT OnLButtonDblClk(HWND, UINT, WPARAM, LPARAM);	// WM_LBUTTONDBLCLK����
	virtual LRESULT OnMouseMove(HWND, UINT, WPARAM, LPARAM);		// WM_MOUSEMOVE����
	virtual LRESULT OnTimer(HWND, UINT, WPARAM, LPARAM);			// WM_TIMER����
	virtual LRESULT OnMeasureItem(HWND, UINT, WPARAM, LPARAM);		// WM_MEASUREITEM����
	virtual LRESULT OnDrawItem(HWND, UINT, WPARAM, LPARAM);			// WM_DRAWITEM����

	// �h���b�O�A���h�h���b�v�Ń^�u�̏����ύX���\��
	// �T�u�N���X������ Tab �ł̃��b�Z�[�W����
	LRESULT OnTabLButtonDown(WPARAM wParam, LPARAM lParam);			// �^�u�� WM_LBUTTONDOWN ����
	LRESULT OnTabLButtonUp(WPARAM wParam, LPARAM lParam);			// �^�u�� WM_LBUTTONUP ����
	LRESULT OnTabMouseMove(WPARAM wParam, LPARAM lParam);			// �^�u�� WM_MOUSEMOVE ����
	LRESULT OnTabTimer(WPARAM wParam, LPARAM lParam);				// �^�u�� WM_TIMER����
	LRESULT OnTabCaptureChanged(WPARAM wParam, LPARAM lParam);		// �^�u�� WM_CAPTURECHANGED ����
	LRESULT OnTabRButtonDown(WPARAM wParam, LPARAM lParam);			// �^�u�� WM_RBUTTONDOWN ����
	LRESULT OnTabRButtonUp(WPARAM wParam, LPARAM lParam);			// �^�u�� WM_RBUTTONUP ����
	LRESULT OnTabMButtonDown(WPARAM wParam, LPARAM lParam);			// �^�u�� WM_MBUTTONDOWN ����
	LRESULT OnTabMButtonUp(WPARAM wParam, LPARAM lParam);			// �^�u�� WM_MBUTTONUP ����
	LRESULT OnTabNotify(WPARAM wParam, LPARAM lParam);				// �^�u�� WM_NOTIFY ����

	// �����⏕�C���^�[�t�F�[�X
	void BreakDrag(void) { if (::GetCapture() == hwndTab) ::ReleaseCapture(); eDragState = DRAG_NONE; nTabCloseCapture = -1; }	// �h���b�O��ԉ�������
	bool ReorderTab(int nSrcTab, int nDstTab);			// �^�u�����ύX����
	void BroadcastRefreshToGroup(void);
	bool SeparateGroup(HWND hwndSrc, HWND hwndDst, POINT ptDrag, POINT ptDrop);	// �^�u��������
	LRESULT ExecTabCommand(int nId, POINTS pts);		// �^�u�� �R�}���h���s����
	void LayoutTab(void);								// �^�u�̃��C�A�E�g��������

	HIMAGELIST InitImageList(void);						// �C���[�W���X�g�̏���������
	int GetImageIndex(EditNode* pNode);					// �C���[�W���X�g�̃C���f�b�N�X�擾����
	HIMAGELIST ImageList_Duplicate(HIMAGELIST himl);	// �C���[�W���X�g�̕�������

	// �^�u�ꗗ��ǉ�
	void DrawBtnBkgnd(HDC hdc, const LPRECT lprcBtn, BOOL bBtnHilighted);	// �{�^���w�i�`�揈��
	void DrawListBtn(Graphics& gr, const LPRECT lprcClient);				// �ꗗ�{�^���`�揈��
	void DrawCloseFigure(Graphics& gr, const RECT &btnRect);				// ����}�[�N�`�揈��
	void DrawCloseBtn(Graphics& gr, const LPRECT lprcClient);				// ����{�^���`�揈��
	void DrawTabCloseBtn(Graphics& gr, const LPRECT lprcClient, bool selected, bool bHover);	// �^�u�����{�^���`�揈��
	void GetListBtnRect(const LPRECT lprcClient, LPRECT lprc);	// �ꗗ�{�^���̋�`�擾����
	void GetCloseBtnRect(const LPRECT lprcClient, LPRECT lprc);	// ����{�^���̋�`�擾����
	void GetTabCloseBtnRect(const LPRECT lprcClient, LPRECT lprc, bool selected);	// �^�u�����{�^���̋�`�擾����

	HFONT CreateMenuFont(void)
	{
		// ���j���[�p�t�H���g�쐬
		NONCLIENTMETRICS	ncm;
		// �ȑO�̃v���b�g�t�H�[���� WINVER >= 0x0600 �Œ�`�����\���̂̃t���T�C�Y��n���Ǝ��s����
		ncm.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0);
		return ::CreateFontIndirect(&ncm.lfMenuFont);
	}

protected:
	enum DragState {
		DRAG_NONE,
		DRAG_CHECK,
		DRAG_DRAG,
	};
	enum CaptureSrc {
		CAPT_NONE,
		CAPT_CLOSE,
	};

	typedef HIMAGELIST (WINAPI *FN_ImageList_Duplicate)(HIMAGELIST himl);

	/*
	|| �����o�ϐ�
	*/
public:
	DllSharedData*	pShareData;			// ���L�f�[�^
	HFONT			hFont;				// �\���p�t�H���g
	HWND			hwndTab;			// �^�u�R���g���[��
	HWND			hwndToolTip;		// �c�[���`�b�v�i�{�^���p�j
	TCHAR			szTextTip[1024];	// �c�[���`�b�v�̃e�L�X�g�i�^�u�p�j
	TabPosition		eTabPosition;		// �^�u�\���ʒu

private:
	DragState	eDragState;				// �h���b�O���
	int			nSrcTab;				// �ړ����^�u
	POINT		ptSrcCursor;			// �h���b�O�J�n�J�[�\���ʒu
	HCURSOR		hDefaultCursor;			// �h���b�O�J�n���̃J�[�\��

	// �^�u�ւ̃A�C�R���\�����\��
	FN_ImageList_Duplicate	realImageList_Duplicate;

	HIMAGELIST	hIml;					// �C���[�W���X�g
	HICON		hIconApp;				// �A�v���P�[�V�����A�C�R��
	HICON		hIconGrep;				// Grep�A�C�R��
	int			iIconApp;				// �A�v���P�[�V�����A�C�R���̃C���f�b�N�X
	int			iIconGrep;				// Grep�A�C�R���̃C���f�b�N�X

	bool		bVisualStyle;			// �r�W���A���X�^�C�����ǂ���
	bool		bHovering;
	bool		bListBtnHilighted;
	bool		bCloseBtnHilighted;		// ����{�^���n�C���C�g���
	CaptureSrc	eCaptureSrc;			// �L���v�`���[��
	bool		bTabSwapped;			// �h���b�O���Ƀ^�u�̓���ւ������������ǂ���
	LONG*		nTabBorderArray;		// �h���b�O�O�̃^�u���E�ʒu�z��
	LOGFONT		lf;						// �\���t�H���g�̓������
	bool		bMultiLine;				// �����s

	// �^�u���̕���{�^���p�ϐ�
	int			nTabHover;				// �}�E�X�J�[�\�����̃^�u�i�����Ƃ���-1�j
	bool		bTabCloseHover;			// �}�E�X�J�[�\�����Ƀ^�u���̕���{�^�������邩
	int			nTabCloseCapture;		// ����{�^�����}�E�X��������Ă���^�u�i�����Ƃ���-1�j

	HWND		hwndSizeBox;
	bool		bSizeBox;

private:
	DISALLOW_COPY_AND_ASSIGN(TabWnd);
};

