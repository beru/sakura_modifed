/*!	@file
	@brief ����v���r���[�Ǘ��N���X
*/
#pragma once

#include <Windows.h>
#include "basis/SakuraBasis.h"
#include "util/design_template.h"
#include "Print.h"

class ColorStrategy;
class ColorStrategyPool;
class DlgCancel;
class EditWnd;
class Layout;
class LayoutMgr;

class PrintPreview {
// �����o�֐��錾
public:
	/*
	||  �R���X�g���N�^
	*/
	PrintPreview(class EditWnd& parentWnd);
	~PrintPreview();
	
	/*
	||	�C�x���g
	*/
	//	Window Messages
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);	// �`�揈��
	LRESULT OnSize(WPARAM, LPARAM);				// WM_SIZE ����
	LRESULT OnVScroll(WPARAM wParam, LPARAM lParam);
	LRESULT OnHScroll(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);

	//	User Messages
	void OnChangeSetting();
	void OnChangePrintSetting(void);
	void OnPreviewGoPage(int nPage);	// �v���r���[ �y�[�W�w��
	void OnPreviewGoPreviousPage() { OnPreviewGoPage(nCurPageNum - 1); }		//	�O�̃y�[�W��
	void OnPreviewGoNextPage() { OnPreviewGoPage(nCurPageNum + 1); }		//	�O�̃y�[�W��
	void OnPreviewGoDirectPage(void);
	void OnPreviewZoom(BOOL bZoomUp);
	void OnPrint(void);	// ������s
	bool OnPrintPageSetting(void);
	void OnCheckAntialias(void);

	/*
	||	�R���g���[��
	*/
	//	�X�N���[���o�[
	void InitPreviewScrollBar(void);
	
	//	PrintPreview�o�[�i��ʏ㕔�̃R���g���[���j
	void CreatePrintPreviewControls(void);
	void DestroyPrintPreviewControls(void);

	void SetFocusToPrintPreviewBar(void);
	HWND GetPrintPreviewBarHANDLE(void) { return hwndPrintPreviewBar;	}
	HWND GetPrintPreviewBarHANDLE_Safe() const { if (!this) return NULL; else return hwndPrintPreviewBar; } // this��NULL�ł����s�ł���ŁB2007.10.29 kobake
	
	//	PrintPreview�o�[�̃��b�Z�[�W�����B
	//	�܂�PrintPreviewBar_DlgProc�Ƀ��b�Z�[�W���͂��ADispatchEvent_PPB�ɓ]������d�g��
	static INT_PTR CALLBACK PrintPreviewBar_DlgProc(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);
	INT_PTR DispatchEvent_PPB(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam 		// second message parameter
	);

protected:
	/*
	||	�`��B
	||	DrawXXXXX()�́A���݂̃t�H���g�𔼊p�t�H���g�ɐݒ肵�Ă���Ăяo�����ƁB
	||	�܂��ADrawXXXXX()���甲���Ă����Ƃ��́A���p�t�H���g�ɐݒ肳��Ă��邱�Ƃ����҂��Ă悢�B
	||	�t�H���g�́A���p�t�H���g�ƑS�p�t�H���g�����Ȃ����Ƃ����҂��Ă悢�B
	*/
	void DrawHeaderFooter(HDC hdc, const Rect& rect , bool bHeader);
	ColorStrategy* DrawPageTextFirst(int nPageNum);
	ColorStrategy* DrawPageText(HDC, int, int, int nPageNum, DlgCancel*, ColorStrategy* pStrategyStart);

	// ����^�v���r���[ �s�`��
	ColorStrategy* Print_DrawLine(
		HDC				hdc,
		POINT			ptDraw,		// �`����W�BHDC�����P�ʁB
		const wchar_t*	pLine,
		size_t			nDocLineLen,
		size_t			nLineStart,
		size_t			nLineLen,
		size_t			nIndent,	// �܂�Ԃ��C���f���g����
		const Layout*	pLayout = nullptr,	// �F�t�pLayout
		ColorStrategy*	pStrategyStart = nullptr
	);

	// ����^�v���r���[ �u���b�N�`��
	void Print_DrawBlock(
		HDC				hdc,
		POINT			ptDraw,		// �`����W�BHDC�����P�ʁB
		const wchar_t*	pPhysicalLine,
		int				nBlockLen,
		int				nKind,		// 0:���p, 1:�S�p
		const Layout*	pLayout,	// �F�ݒ�pLayout
		int				nColorIndex,
		int				nBgnPhysical,
		int				nLayoutX,
		int				nDx,
		const int*		pDxArray
	);

	// �w�胍�W�b�N�ʒu��ColorStrategy���擾
	ColorStrategy* GetColorStrategy(
		const StringRef&	stringLine,
		size_t				iLogic,
		ColorStrategy*		pStrategy,
		bool&				bChange
	);

	// ����p�t�H���g���쐬����
	void CreateFonts(HDC hdc);
	// ����p�t�H���g��j������
	void DestroyFonts();

public:
	//	�t�H���g��
	static int CALLBACK MyEnumFontFamProc(
		ENUMLOGFONT*	pelf,		// pointer to logical-font data
		NEWTEXTMETRIC*	pntm,		// pointer to physical-font data
		int				nFontType,	// type of font
		LPARAM			lParam 		// address of application-defined data
	);

	/*
	||	�A�N�Z�T
	*/
	void SetPrintSetting(PrintSetting* pPrintSetting) {
		sPrintSetting = *pPrintSetting;
		this->pPrintSetting = &sPrintSetting;
		this->pPrintSettingOrg = pPrintSetting;
	}
	BOOL GetDefaultPrinterInfo() { return print.GetDefaultPrinter(&pPrintSetting->mdmDevMode); }
	int  GetCurPageNum() { return nCurPageNum; }	// ���݂̃y�[�W
	int  GetAllPageNum() { return nAllPageNum; }	// ���݂̃y�[�W
	
	/*
	||	�w�b�_�E�t�b�^
	*/
	void SetHeader(char* pszWork[]);	//	&f�Ȃǂ�o�^
	void SetFooter(char* pszWork[]);	//	&p/&P�Ȃǂ�o�^

protected:
	void SetPreviewFontHan(const LOGFONT* lf);
	void SetPreviewFontZen(const LOGFONT* lf);

// �����o�ϐ��錾
public:
	// none

protected:
	EditWnd&		parentWnd;	//	�e��EditEnd�B

	HDC				hdcCompatDC;		// �ĕ`��p�R���p�`�u��DC
	HBITMAP			hbmpCompatBMP;		// �ĕ`��p������BMP
	HBITMAP			hbmpCompatBMPOld;	// �ĕ`��p������BMP(OLD)
	int				nbmpCompatScale;	// BMP�̉�ʂ�10(COMPAT_BMP_BASE)�s�N�Z�����������BMP�̃s�N�Z����

	//	�R���g���[������p
	//	����o�[
	HWND			hwndPrintPreviewBar;	// ����v���r���[ ����o�[
	//	�X�N���[���o�[
	int				nPreviewVScrollPos;	// ����v���r���[�F�X�N���[���ʒu�c
	int				nPreviewHScrollPos;	// ����v���r���[�F�X�N���[���ʒu��
	BOOL			SCROLLBAR_HORZ;
	BOOL			SCROLLBAR_VERT;
	HWND			hwndVScrollBar;		// �����X�N���[���o�[�E�B���h�E�n���h��
	HWND			hwndHScrollBar;		// �����X�N���[���o�[�E�B���h�E�n���h��
	//	�T�C�Y�{�b�N�X
	HWND			hwndSizeBox;		// �T�C�Y�{�b�N�X�E�B���h�E�n���h��
	BOOL			sizeBoxCanMove;		// �T�C�Y�{�b�N�X�E�B���h�E�n���h���𓮂����邩�ǂ���

	//	�\��
	int				nPreview_Zoom;	// ����v���r���[�F�{��

	//	����ʒu�����肷�邽�߂̕ϐ�
	int				nPreview_ViewWidth;			// ����v���r���[�F�r���[��(�s�N�Z��)
	int				nPreview_ViewHeight;		// ����v���r���[�F�r���[����(�s�N�Z��)
	int				nPreview_ViewMarginLeft;	// ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��)
	int				nPreview_ViewMarginTop;		// ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��)
	short			nPreview_PaperAllWidth;		// �p����(1/10mm�P��)
	short			nPreview_PaperAllHeight;	// �p������(1/10mm�P��)
	short			nPreview_PaperWidth;		// �p������L����(1/10mm�P��)
	short			nPreview_PaperHeight;		// �p������L������(1/10mm�P��)
	short			nPreview_PaperOffsetLeft;	// �p���]�����[(1/10mm�P��)
	short			nPreview_PaperOffsetTop;	// �p���]����[(1/10mm�P��)
	int				bPreview_EnableColumns;		// �󎚉\����/�y�[�W
	int				bPreview_EnableLines;		// �󎚉\�s��/�y�[�W
	int				nPreview_LineNumberColumns;	// �s�ԍ��G���A�̕��i�������j
	WORD			nAllPageNum;				// �S�y�[�W��
	WORD			nCurPageNum;				// ���݂̃y�[�W

	PrintSetting*	pPrintSetting;				// ���݂̈���ݒ�(�L���b�V���ւ̃|�C���^)
	PrintSetting*	pPrintSettingOrg;			// ���݂̈���ݒ�(���L�f�[�^)
	PrintSetting	sPrintSetting;				// ���݂̈���ݒ�(�L���b�V��)
	LOGFONT			lfPreviewHan;				// �v���r���[�p�t�H���g
	LOGFONT			lfPreviewZen;				// �v���r���[�p�t�H���g

	HFONT			hFontHan;					// ����p���p�t�H���g�n���h��
	HFONT			hFontHan_b;					// ����p���p�t�H���g�n���h�� ����
	HFONT			hFontHan_u;					// ����p���p�t�H���g�n���h�� ����
	HFONT			hFontHan_bu;				// ����p���p�t�H���g�n���h�� �����A����
	HFONT			hFontZen;					// ����p�S�p�t�H���g�n���h��
	HFONT			hFontZen_b;					// ����p�S�p�t�H���g�n���h�� ����
	HFONT			hFontZen_u;					// ����p�S�p�t�H���g�n���h�� ����
	HFONT			hFontZen_bu;				// ����p�S�p�t�H���g�n���h�� �����A����
	int				nAscentHan;					// ���p�����̃A�Z���g�i������/����C������̍����j
	int				nAscentZen;					// �S�p�����̃A�Z���g�i������/����C������̍����j

	ColorStrategyPool*	pool;					// �F��`�Ǘ����

public:
	class LayoutMgr*	pLayoutMgr_Print;		// ����p�̃��C�A�E�g�Ǘ����
protected:
	TypeConfig typePrint;

	// �v���r���[����o�Ă����݂̃v�����^�����L�����Ă�����悤��static�ɂ��� 2003.05.02 ����� 
	static Print	print;						// ���݂̃v�����^���

	bool			bLockSetting;				// �ݒ�̃��b�N
	bool			bDemandUpdateSetting;		// �ݒ�̍X�V�v��

private:
	DISALLOW_COPY_AND_ASSIGN(PrintPreview);
};

