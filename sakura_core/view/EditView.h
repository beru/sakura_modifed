/*!	@file
	@brief �����E�B���h�E�̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, asa-o, MIK, hor, Misaka, Stonee, YAZAKI
	Copyright (C) 2002, genta, hor, YAZAKI, Azumaiya, KK, novice, minfu, ai, aroka, MIK
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, Moca, novice, Kazika, isearch
	Copyright (C) 2005, genta, Moca, MIK, ryoji, maru
	Copyright (C) 2006, genta, aroka, fon, yukihane, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#pragma once

#include <Windows.h>
#include <ObjIdl.h>  // LPDATAOBJECT
#include <ShellAPI.h>  // HDROP
#include "TextMetrics.h"
#include "TextDrawer.h"
#include "TextArea.h"
#include "Caret.h"
#include "ViewCalc.h" // parent
#include "EditView_Paint.h"	// parent
#include "ViewParser.h"
#include "ViewSelect.h"
#include "SearchAgent.h"
#include "view/colors/EColorIndexType.h"
#include "window/TipWnd.h"
#include "window/AutoScrollWnd.h"
#include "DicMgr.h"
//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
#include "extmodule/Bregexp.h"
#include "Eol.h"				// EolType
#include "cmd/ViewCommander.h"
#include "mfclike/MyWnd.h"		// parent
#include "doc/DocListener.h"	// parent
#include "basis/SakuraBasis.h"	// LogicInt, LayoutInt
#include "util/container.h"		// vector_ex
#include "util/design_template.h"

class ViewFont;
class Ruler;
class DropTarget; /// 2002/2/3 aroka �w�b�_�y�ʉ�
class OpeBlk;///
class SplitBoxWnd;///
class RegexKeyword;///
class AutoMarkMgr; /// 2002/2/3 aroka �w�b�_�y�ʉ� to here
class EditDoc;	//	2002/5/13 YAZAKI �w�b�_�y�ʉ�
class Layout;	//	2002/5/13 YAZAKI �w�b�_�y�ʉ�
class Migemo;	// 2004.09.14 isearch
struct ColorStrategyInfo;
struct Color3Setting;
class OutputAdapter;

// struct DispPos; //	�N����include���Ă܂�
// class CColorStrategy;	// �N����include���Ă܂�
class Color_Found;

#ifndef IDM_COPYDICINFO
#define IDM_COPYDICINFO 2000
#endif
#ifndef IDM_JUMPDICT
#define IDM_JUMPDICT 2001	// 2006.04.10 fon
#endif

///	�}�E�X����R�}���h�����s���ꂽ�ꍇ�̏�ʃr�b�g
///	@date 2006.05.19 genta
const int CMD_FROM_MOUSE = 2;


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �����E�B���h�E�̊Ǘ�
	
	1�̕����E�B���h�E�ɂ�1��EditDoc�I�u�W�F�N�g�����蓖�Ă��A
	1��EditDoc�I�u�W�F�N�g�ɂ��A4��CEditViwe�I�u�W�F�N�g�����蓖�Ă���B
	�E�B���h�E���b�Z�[�W�̏����A�R�}���h���b�Z�[�W�̏����A
	��ʕ\���Ȃǂ��s���B
	
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
*/
// 2007.08.25 kobake �����Ԋu�z��̋@�\��TextMetrics�Ɉړ�
// 2007.10.02 kobake Command_TRIM2��CConvert�Ɉړ�

class EditView :
	public ViewCalc, //$$ ���ꂪ�e�N���X�ł���K�v�͖������A���̃N���X�̃��\�b�h�Ăяo���������̂ŁA�b��I�ɐe�N���X�Ƃ���B
	public EditView_Paint,
	public MyWnd,
	public DocListenerEx
{
public:
	const EditDoc& GetDocument() const {
		return *pEditDoc;
	}
	EditDoc& GetDocument() {
		return *pEditDoc;
	}
public:
	// �w�i�Ƀr�b�g�}�b�v���g�p���邩�ǂ���
	// 2010.10.03 �w�i����
	bool IsBkBitmap() const { return pEditDoc->hBackImg != NULL; }

public:
	EditView& GetEditView() override {
		return *this;
	}
	const EditView& GetEditView() const override {
		return *this;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����Ɣj��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// Constructors
	EditView(EditWnd& editWnd);
	~EditView();
	void Close();
	// �������n�����o�֐�
	BOOL Create(
		HWND		hwndParent,	// �e
		EditDoc&	editDoc,	// �Q�Ƃ���h�L�������g
		int			nMyIndex,	// �r���[�̃C���f�b�N�X
		BOOL		bShow,		// �쐬���ɕ\�����邩�ǂ���
		bool		bMiniMap
	);
	void CopyViewStatus(EditView*) const;					// �����̕\����Ԃ𑼂̃r���[�ɃR�s�[

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �N���b�v�{�[�h                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// �擾
	bool MyGetClipboardData(NativeW&, bool*, bool* = nullptr);			// �N���b�v�{�[�h����f�[�^���擾

	// �ݒ�
	bool MySetClipboardData(const ACHAR*, int, bool bColumnSelect, bool = false);	// �N���b�v�{�[�h�Ƀf�[�^��ݒ�
	bool MySetClipboardData(const WCHAR*, int, bool bColumnSelect, bool = false);	// �N���b�v�{�[�h�Ƀf�[�^��ݒ�

	// ���p
	void CopyCurLine(bool bAddCRLFWhenCopy, EolType neweol, bool bEnableLineModePaste);	// �J�[�\���s���N���b�v�{�[�h�ɃR�s�[����	// 2007.10.08 ryoji
	void CopySelectedAllLines(const wchar_t*, bool);			// �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[����


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �C�x���g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// �h�L�������g�C�x���g
	void OnAfterLoad(const LoadInfo& loadInfo);
	// ���b�Z�[�W�f�B�X�p�b�`��
	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	//
	void OnChangeSetting();								// �ݒ�ύX�𔽉f������
	void OnPaint(HDC, PAINTSTRUCT *, BOOL);				// �ʏ�̕`�揈��
	void OnPaint2( HDC, PAINTSTRUCT *, BOOL );			// �ʏ�̕`�揈��
	void DrawBackImage(HDC hdc, RECT& rcPaint, HDC hdcBgImg);
	void OnTimer(HWND, UINT, UINT_PTR, DWORD);
	// �E�B���h�E
	void OnSize(int, int);								// �E�B���h�E�T�C�Y�̕ύX����
	void OnMove(int, int, int, int);
	// �t�H�[�J�X
	void OnSetFocus(void);
	void OnKillFocus(void);
	// �X�N���[��
	int OnVScroll(int, int);					// �����X�N���[���o�[���b�Z�[�W����
	int OnHScroll(int, int);					// �����X�N���[���o�[���b�Z�[�W����
	// �}�E�X
	void OnLBUTTONDOWN(WPARAM, int, int);			// �}�E�X���{�^������
	void OnMOUSEMOVE(WPARAM, int, int);				// �}�E�X�ړ��̃��b�Z�[�W����
	void OnLBUTTONUP(WPARAM, int, int);				// �}�E�X���{�^���J���̃��b�Z�[�W����
	void OnLBUTTONDBLCLK(WPARAM, int , int);		// �}�E�X���{�^���_�u���N���b�N
	void OnRBUTTONDOWN(WPARAM, int, int);			// �}�E�X�E�{�^������
	void OnRBUTTONUP(WPARAM, int, int);				// �}�E�X�E�{�^���J��
	void OnMBUTTONDOWN(WPARAM, int, int);			// �}�E�X���{�^������
	void OnMBUTTONUP(WPARAM, int, int);				// �}�E�X���{�^���J��
	void OnXLBUTTONDOWN(WPARAM, int, int);			// �}�E�X�T�C�h�{�^��1����
	void OnXLBUTTONUP(WPARAM, int, int);			// �}�E�X�T�C�h�{�^��1�J��		// 2009.01.17 nasukoji
	void OnXRBUTTONDOWN(WPARAM, int, int);			// �}�E�X�T�C�h�{�^��2����
	void OnXRBUTTONUP(WPARAM, int, int);			// �}�E�X�T�C�h�{�^��2�J��		// 2009.01.17 nasukoji
	LRESULT OnMOUSEWHEEL(WPARAM, LPARAM);			// �����}�E�X�z�C�[���̃��b�Z�[�W����
	LRESULT OnMOUSEHWHEEL(WPARAM, LPARAM);			// �����}�E�X�z�C�[���̃��b�Z�[�W����
	LRESULT OnMOUSEWHEEL2(WPARAM, LPARAM, bool, EFunctionCode);		// �}�E�X�z�C�[���̃��b�Z�[�W����
	bool IsSpecialScrollMode(int);					// �L�[�E�}�E�X�{�^����Ԃ��X�N���[�����[�h�𔻒肷��		// 2009.01.17 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2006.05.14 Moca  �݊�BMP�ɂ���ʃo�b�t�@
	// 2007.09.30 genta CompatibleDC����֐�
protected:
	// ���W�b�N�s��1�s�`��
	bool DrawLogicLine(
		HDC				hdc,			// [in]     ���Ώ�
		DispPos*		pDispPos,		// [in/out] �`�悷��ӏ��A�`�挳�\�[�X
		int				nLineTo			// [in]     ���I�����郌�C�A�E�g�s�ԍ�
	);

	// ���C�A�E�g�s��1�s�`��
	bool DrawLayoutLine(ColorStrategyInfo& csInfo);

	// �F����
public:
	Color3Setting GetColorIndex(const Layout* pLayout, int nLineNum, int nIndex, ColorStrategyInfo& csInfo, bool bPrev = false);	// �w��ʒu��ColorIndex�̎擾 02/12/13 ai
	void SetCurrentColor(Graphics& gr, EColorIndexType, EColorIndexType, EColorIndexType);
	COLORREF GetTextColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2);
	COLORREF GetBackColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2);

	// ��ʃo�b�t�@
protected:
	bool CreateOrUpdateCompatibleBitmap(int cx, int cy);	// ������BMP���쐬�܂��͍X�V
	void UseCompatibleDC(BOOL fCache);
public:
	void DeleteCompatibleBitmap();							// ������BMP���폜

public:
	void DispTextSelected(HDC hdc, int nLineNum, const Point& ptXY, int nX_Layout);	// �e�L�X�g���]
	void RedrawAll();										// �t�H�[�J�X�ړ����̍ĕ`��
	void Redraw();											// 2001/06/21 asa-o �ĕ`��
	void RedrawLines(int top, int bottom);
	void CaretUnderLineON(bool, bool, bool);				// �J�[�\���s�A���_�[���C����ON
	void CaretUnderLineOFF(bool, bool, bool, bool);			// �J�[�\���s�A���_�[���C����OFF
	bool GetDrawSwitch() const {
		return bDrawSwitch;
	}
	bool SetDrawSwitch(bool b) {
		bool bOld = bDrawSwitch;
		bDrawSwitch = b;
		return bOld;
	}
	bool IsDrawCursorVLinePos(int);
	void DrawBracketCursorLine(bool);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �X�N���[��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AdjustScrollBars();											// �X�N���[���o�[�̏�Ԃ��X�V����
	BOOL CreateScrollBar();												// �X�N���[���o�[�쐬	// 2006.12.19 ryoji
	void DestroyScrollBar();											// �X�N���[���o�[�j��	// 2006.12.19 ryoji
	int GetWrapOverhang(void) const;								// �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z	// 2008.06.08 ryoji
	int ViewColNumToWrapColNum(int nViewColNum) const;		//�u�E�[�Ő܂�Ԃ��v�p�Ƀr���[�̌�������܂�Ԃ��������v�Z����	// 2008.06.08 ryoji
	int GetRightEdgeForScrollBar(void);							// �X�N���[���o�[����p�ɉE�[���W���擾����		// 2009.08.28 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           IME                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Aug. 25, 2002 genta protected->public�Ɉړ�
	bool IsImeON(void);	// IME ON��	// 2006.12.04 ryoji
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �X�N���[��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int  ScrollAtV(int);								// �w���[�s�ʒu�փX�N���[��
	int  ScrollAtH(int);								// �w�荶�[���ʒu�փX�N���[��
	//	From Here Sep. 11, 2004 genta ����ێ��̓����X�N���[��
	int  ScrollByV(int vl) { return ScrollAtV((int)GetTextArea().GetViewTopLine() + vl); }	// �w��s�X�N���[��
	int  ScrollByH(int hl) { return ScrollAtH((int)GetTextArea().GetViewLeftCol() + hl); }	// �w�茅�X�N���[��
	void ScrollDraw(int, int, const RECT&, const RECT&, const RECT&);
	void MiniMapRedraw(bool);
public:
	void SyncScrollV(int);										// ���������X�N���[��
	void SyncScrollH(int);										// ���������X�N���[��

	void SetBracketPairPos(bool);										// �Ί��ʂ̋����\���ʒu�ݒ� 03/02/18 ai

	void AutoScrollEnter();
	void AutoScrollExit();
	void AutoScrollMove(Point& point);
	void AutoScrollOnTimer();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �ߋ��̈�Y                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void SetIMECompFormPos(void);								// IME�ҏW�G���A�̈ʒu��ύX
	void SetIMECompFormFont(void);								// IME�ҏW�G���A�̕\���t�H���g��ύX
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �e�L�X�g�I��                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 2002/01/19 novice public�����ɕύX
	bool GetSelectedDataSimple(NativeW&);// �I��͈͂̃f�[�^���擾
	bool GetSelectedDataOne(NativeW& memBuf, int nMaxLen);
	bool GetSelectedData(NativeW*, bool, const wchar_t*, bool, bool bAddCRLFWhenCopy, EolType neweol = EolType::Unknown);	// �I��͈͂̃f�[�^���擾
	int IsCurrentPositionSelected(LayoutPoint ptCaretPos);					// �w��J�[�\���ʒu���I���G���A���ɂ��邩
	int IsCurrentPositionSelectedTEST(const LayoutPoint& ptCaretPos, const LayoutRange& select) const; // �w��J�[�\���ʒu���I���G���A���ɂ��邩
	// 2006.07.09 genta �s���w��ɂ��J�[�\���ړ�(�I��̈���l��)
	void MoveCursorSelecting(LayoutPoint ptWk_CaretPos, bool bSelect, int = _CARETMARGINRATE);
	void ConvSelectedArea(EFunctionCode);									// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	// �w��ʒu�܂��͎w��͈͂��e�L�X�g�̑��݂��Ȃ��G���A���`�F�b�N����	// 2008.08.03 nasukoji
	bool IsEmptyArea(LayoutPoint ptFrom, LayoutPoint ptTo = LayoutPoint(-1, -1), bool bSelect = false, bool bBoxSelect = false) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �e�픻��                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool IsCurrentPositionURL(const LayoutPoint& ptCaretPos, LogicRange* pUrlRange, std::wstring* pwstrURL); // �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ�
	bool CheckTripleClick(Point ptMouse);							// �g���v���N���b�N���`�F�b�N����	// 2007.10.02 nasukoji
	
	bool ExecCmd(const TCHAR*, int, const TCHAR*, OutputAdapter* = nullptr) ;			// �q�v���Z�X�̕W���o�͂����_�C���N�g����
	void AddToCmdArr(const TCHAR*);
	bool ChangeCurRegexp(bool bRedrawIfChanged = true);			// 2002.01.16 hor ���K�\���̌����p�^�[����K�v�ɉ����čX�V����(���C�u�������g�p�ł��Ȃ��Ƃ��� false ��Ԃ�)
	void SendStatusMessage(const TCHAR* msg);					// 2002.01.26 hor �����^�u���^�u�b�N�}�[�N�������̏�Ԃ��X�e�[�^�X�o�[�ɕ\������
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode, bool bDocumentFeed = false);	// �ĕϊ��p�\���̂�ݒ肷�� 2002.04.09 minfu
	LRESULT SetSelectionFromReonvert(const RECONVERTSTRING* pReconv, bool bUnicode);				// �ĕϊ��p�\���̂̏������ɑI��͈͂�ύX���� 2002.04.09 minfu

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           D&D                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public: // �e�X�g�p�ɃA�N�Z�X������ύX
	// IDropTarget����
	STDMETHODIMP DragEnter(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragLeave(void);
	STDMETHODIMP Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHODIMP PostMyDropFiles(LPDATAOBJECT pDataObject);		// �Ǝ��h���b�v�t�@�C�����b�Z�[�W���|�X�g����	// 2008.06.20 ryoji
	void OnMyDropFiles(HDROP hDrop);								// �Ǝ��h���b�v�t�@�C�����b�Z�[�W����			// 2008.06.20 ryoji
	CLIPFORMAT GetAvailableClipFormat(LPDATAOBJECT pDataObject);
	DWORD TranslateDropEffect(CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect);
	bool IsDragSource(void);

	void _SetDragMode(bool b) {
		bDragMode = b;
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ҏW                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// �w��ʒu�̎w�蒷�f�[�^�폜
	void DeleteData2(
		const LayoutPoint&	ptCaretPos,
		int					nDelLen,
		NativeW*			pMem
	);

	// ���݈ʒu�̃f�[�^�폜
	void DeleteData(bool bRedraw);

	// ���݈ʒu�Ƀf�[�^��}��
	void InsertData_CEditView(
		LayoutPoint	ptInsertPos,
		const wchar_t*	pData,
		size_t			nDataLen,
		LayoutPoint*	pptNewPos,	// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
		bool			bRedraw
	);

	// �f�[�^�u�� �폜&�}���ɂ��g����
	void ReplaceData_CEditView(
		const LayoutRange&	delRange,			// �폜�͈́B���C�A�E�g�P�ʁB
		const wchar_t*		pInsData,			// �}������f�[�^
		size_t				nInsDataLen,		// �}������f�[�^�̒���
		bool				bRedraw,
		OpeBlk*				pOpeBlk,
		bool				bFastMode = false,
		const LogicRange*	psDelRangeLogicFast = nullptr
	);
	void ReplaceData_CEditView2(
		const LogicRange&	delRange,			// �폜�͈́B���W�b�N�P�ʁB
		const wchar_t*		pInsData,			// �}������f�[�^
		size_t				nInsDataLen,		// �}������f�[�^�̒���
		bool				bRedraw,
		OpeBlk*				pOpeBlk,
		bool				bFastMode = false
	);
	bool ReplaceData_CEditView3(
		LayoutRange		delRange,			// �폜�͈́B���C�A�E�g�P�ʁB
		OpeLineData*	pMemCopyOfDeleted,	// �폜���ꂽ�f�[�^�̃R�s�[(nullptr�\)
		OpeLineData*	pInsData,
		bool			bRedraw,
		OpeBlk*			pOpeBlk,
		int				nDelSeq,
		int*			pnInsSeq,
		bool			bFastMode = false,
		const LogicRange*	psDelRangeLogicFast = nullptr
	);
	void RTrimPrevLine(void);		// 2005.10.11 ryoji �O�̍s�ɂ��閖���̋󔒂��폜

	//	Oct. 2, 2005 genta �}�����[�h�̐ݒ�E�擾
	bool IsInsMode() const;
	void SetInsMode(bool);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 2004.10.13 �C���N�������^���T�[�`�֌W
	void TranslateCommand_isearch(EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM&);
	bool ProcessCommand_isearch(int, bool, LPARAM, LPARAM, LPARAM, LPARAM);

	//	Jan. 10, 2005 genta HandleCommand����grep�֘A�����𕪗�
	void TranslateCommand_grep(EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM&);

	//	Jan. 10, 2005 �C���N�������^���T�[�`
	bool IsISearchEnabled(int nCommand) const;

	bool KeySearchCore(const NativeW* pMemCurText);	// 2006.04.10 fon
	bool MiniMapCursorLineTip(POINT* po, RECT* rc, bool* pbHide);

	/*!	EditView::KeywordHelpSearchDict�̃R�[�����w��p���[�J��ID
		@date 2006.04.10 fon �V�K�쐬
	*/
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	// EditView::OnTimer
		LID_SKH_POPUPMENU_R = 2,	// EditView::CreatePopUpMenu_R
	};
	BOOL KeywordHelpSearchDict(LID_SKH nID, POINT* po, RECT* rc);	// 2006.04.10 fon

	int IsSearchString(const StringRef& str, int, int*, int*) const;	// ���݈ʒu������������ɊY�����邩	// 2002.02.08 hor �����ǉ�

	void GetCurrentTextForSearch(NativeW&, bool bStripMaxPath = true, bool bTrimSpaceTab = false);			// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	bool GetCurrentTextForSearchDlg(NativeW&, bool bGetHistory = false);		// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾�i�_�C�A���O�p�j 2006.08.23 ryoji

private:
	// �C���N�������^���T�[�` 
	// 2004.10.24 isearch migemo
	void ISearchEnter(int mode, SearchDirection direction);
	void ISearchExit();
	void ISearchExec(DWORD wChar);
	void ISearchExec(LPCWSTR pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(NativeT* msg) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Jun. 16, 2000 genta
	bool  SearchBracket(const LayoutPoint& ptPos, LayoutPoint* pptLayoutNew, int* mode);	// �Ί��ʂ̌���		// mode�̒ǉ� 02/09/18 ai
	bool  SearchBracketForward(LogicPoint ptPos, LayoutPoint* pptLayoutNew,
						const wchar_t* upChar, const wchar_t* dnChar, int mode);	//	�Ί��ʂ̑O������	// mode�̒ǉ� 02/09/19 ai
	bool  SearchBracketBackward(LogicPoint ptPos, LayoutPoint* pptLayoutNew,
						const wchar_t* dnChar, const wchar_t* upChar, int mode);	//	�Ί��ʂ̌������	// mode�̒ǉ� 02/09/19 ai
	void DrawBracketPair(bool);								// �Ί��ʂ̋����\�� 02/09/18 ai
	bool IsBracket(const wchar_t*, int, int);					// ���ʔ��� 03/01/09 ai

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �⊮                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// �x��
	//	Jan. 10, 2005 genta HandleCommand����⊮�֘A�����𕪗�
	void PreprocessCommand_hokan(int nCommand);
	void PostprocessCommand_hokan(void);

	// �⊮�E�B���h�E��\������BCtrl+Space��A�����̓���/�폜���ɌĂяo����܂��B YAZAKI 2002/03/11
	void ShowHokanMgr(NativeW& memData, bool bAutoDecided);

	int HokanSearchByFile(const wchar_t*, bool, vector_ex<std::wstring>&, int); // 2003.06.25 Moca


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �W�����v                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//@@@ 2003.04.13 MIK, Apr. 21, 2003 genta bClose�ǉ�
	//	Feb. 17, 2007 genta ���΃p�X�̊�f�B���N�g���w����ǉ�
	bool TagJumpSub(
		const TCHAR* pszJumpToFile,
		Point ptJumpTo,
		bool bClose = false,
		bool bRelFromIni = false,
		bool* pbJumpToSelf = nullptr
	);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ���j���[                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int	CreatePopUpMenu_R(void);		// �|�b�v�A�b�v���j���[(�E�N���b�N)
	int	CreatePopUpMenuSub(HMENU hMenu, int nMenuIdx, int* pParentMenus);		// �|�b�v�A�b�v���j���[



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           DIFF                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AnalyzeDiffInfo(const char*, int);	// DIFF���̉��	//@@@ 2002.05.25 MIK
	bool MakeDiffTmpFile(TCHAR*, HWND, EncodingType, bool);		// DIFF�ꎞ�t�@�C���쐬	//@@@ 2002.05.28 MIK	// 2005.10.29 maru
	bool MakeDiffTmpFile2(TCHAR*, const TCHAR*, EncodingType, EncodingType);
	void ViewDiffInfo(const TCHAR*, const TCHAR*, int, bool);		// DIFF�����\��		// 2005.10.29 maru

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	// ���ݍs�𗚗��ɒǉ�����


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          ���̑�                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool OPEN_ExtFromtoExt(bool, bool, const TCHAR* [], const TCHAR* [], int, int, const TCHAR*); // �w��g���q�̃t�@�C���ɑΉ�����t�@�C�����J���⏕�֐� // 2003.08.12 Moca
	//	Jan.  8, 2006 genta �܂�Ԃ��g�O�����씻��
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode(int* newKetas);
	void SmartIndent_CPP(wchar_t);				// C/C++�X�}�[�g�C���f���g����
	// �R�}���h����
	void SetFont(void);							// �t�H���g�̕ύX
	void SplitBoxOnOff(bool, bool, bool);		// �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e

//	2001/06/18 asa-o
	bool  ShowKeywordHelp(POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin);	// �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��
	void SetUndoBuffer(bool bPaintLineNumber = false);			// �A���h�D�o�b�t�@�̏���
	HWND StartProgress();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �A�N�Z�T                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ��v�\�����i�A�N�Z�X
	TextArea& GetTextArea() { assert(pTextArea); return *pTextArea; }
	const TextArea& GetTextArea() const { assert(pTextArea); return *pTextArea; }
	Caret& GetCaret() { assert(pCaret); return *pCaret; }
	const Caret& GetCaret() const { assert(pCaret); return *pCaret; }
	Ruler& GetRuler() { assert(pRuler); return *pRuler; }
	const Ruler& GetRuler() const { assert(pRuler); return *pRuler; }

	// ��v�����A�N�Z�X
	TextMetrics& GetTextMetrics() { return textMetrics; }
	const TextMetrics& GetTextMetrics() const { return textMetrics; }
	ViewSelect& GetSelectionInfo() { return viewSelect; }
	const ViewSelect& GetSelectionInfo() const { return viewSelect; }

	// ��v�I�u�W�F�N�g�A�N�Z�X
	ViewFont& GetFontset() { assert(pViewFont); return *pViewFont; }
	const ViewFont& GetFontset() const { assert(pViewFont); return *pViewFont; }

	// ��v�w���p�A�N�Z�X
	const ViewParser& GetParser() const { return parser; }
	const TextDrawer& GetTextDrawer() const { return textDrawer; }
	ViewCommander& GetCommander() { return commander; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �����o�ϐ��Q                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// �Q��
	EditWnd&			editWnd;	// �E�B���h�E
	EditDoc*			pEditDoc;	// �h�L�������g
	const TypeConfig*	pTypeData;

	// ��v�\�����i
	TextArea*		pTextArea;
	Caret*			pCaret;
	Ruler*			pRuler;

	// ��v����
	TextMetrics		textMetrics;
	ViewSelect		viewSelect;

	// ��v�I�u�W�F�N�g
	ViewFont*		pViewFont;

	// ��v�w���p
	ViewParser		parser;
	TextDrawer		textDrawer;
	ViewCommander	commander;

public:
	// �E�B���h�E
	HWND			hwndParent;			// �e�E�B���h�E�n���h��
	HWND			hwndVScrollBar;		// �����X�N���[���o�[�E�B���h�E�n���h��
	int				nVScrollRate;		// �����X�N���[���o�[�̏k��
	HWND			hwndHScrollBar;		// �����X�N���[���o�[�E�B���h�E�n���h��
	HWND			hwndSizeBox;		// �T�C�Y�{�b�N�X�E�B���h�E�n���h��
	SplitBoxWnd*	pcsbwVSplitBox;		// ���������{�b�N�X
	SplitBoxWnd*	pcsbwHSplitBox;		// ���������{�b�N�X
	AutoScrollWnd	autoScrollWnd;		// �I�[�g�X�N���[��

public:
	// �`��
	bool			bDrawSwitch;
	COLORREF		crBack;				// �e�L�X�g�̔w�i�F			// 2006.12.07 ryoji
	COLORREF		crBack2;			// �e�L�X�g�̔w�i(�L�����b�g�p)
	int				nOldUnderLineY;		// �O���悵���J�[�\���A���_�[���C���̈ʒu 0����=��\��
	int				nOldUnderLineYBg;
	int				nOldUnderLineYMargin;
	int				nOldUnderLineYHeight;
	int				nOldUnderLineYHeightReal;
	int				nOldCursorLineX;		// �O���悵���J�[�\���ʒu�c���̈ʒu // 2007.09.09 Moca
	int				nOldCursorVLineWidth;	// �J�[�\���ʒu�c���̑���(px)

public:
	// ��ʃo�b�t�@
	HDC				hdcCompatDC;		// �ĕ`��p�R���p�`�u���c�b
	HBITMAP			hbmpCompatBMP;		// �ĕ`��p�������a�l�o
	HBITMAP			hbmpCompatBMPOld;	// �ĕ`��p�������a�l�o(OLD)
	int				nCompatBMPWidth;	// �č��p�������a�l�o�̕�	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	int				nCompatBMPHeight;	// �č��p�������a�l�o�̍���	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@

public:
	// D&D
	DropTarget*		pDropTarget;
	bool			bDragMode;					// �I���e�L�X�g�̃h���b�O����
	CLIPFORMAT		cfDragData;					// �h���b�O�f�[�^�̃N���b�v�`��	// 2008.06.20 ryoji
	bool			bDragBoxData;				// �h���b�O�f�[�^�͋�`��
	LayoutPoint		ptCaretPos_DragEnter;		// �h���b�O�J�n���̃J�[�\���ʒu	// 2007.12.09 ryoji
	int				nCaretPosX_Prev_DragEnter;	// �h���b�O�J�n����X���W�L��	// 2007.12.09 ryoji

	// ����
	LogicPoint		ptBracketCaretPos_PHY;		// �O�J�[�\���ʒu�̊��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	LogicPoint		ptBracketPairPos_PHY;		// �Ί��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	bool			bDrawBracketPairFlag;		// �Ί��ʂ̋����\�����s�Ȃ���						// 03/02/18 ai

	// �}�E�X
	bool			bActivateByMouse;		// �}�E�X�ɂ��A�N�e�B�x�[�g	// 2007.10.02 nasukoji
	DWORD			dwTripleClickCheck;		// �g���v���N���b�N�`�F�b�N�p����	// 2007.10.02 nasukoji
	Point			mouseDownPos;			// �N���b�N���̃}�E�X���W
	int				nWheelDelta;			// �z�C�[���ω���
	EFunctionCode	eWheelScroll; 			// �X�N���[���̎��
	int				nMousePouse;			// �}�E�X��~����
	Point			mousePousePos;			// �}�E�X�̒�~�ʒu
	bool			bHideMouse;

	int				nAutoScrollMode;			// �I�[�g�X�N���[�����[�h
	bool			bAutoScrollDragMode;		// �h���b�O���[�h
	Point			autoScrollMousePos;			// �I�[�g�X�N���[���̃}�E�X��ʒu
	bool			bAutoScrollVertical;		// �����X�N���[����
	bool			bAutoScrollHorizontal;		// �����X�N���[����

	// ����
	SearchStringPattern searchPattern;
	mutable Bregexp		curRegexp;				// �R���p�C���f�[�^
	bool				bCurSrchKeyMark;		// ����������̃}�[�N
	bool				bCurSearchUpdate;		// �R���p�C���f�[�^�X�V�v��
	int					nCurSearchKeySequence;	// �����L�[�V�[�P���X
	std::wstring		strCurSearchKey;		// ����������
	SearchOption		curSearchOption;		// �����^�u��  �I�v�V����
	LogicPoint			ptSrchStartPos_PHY;		// ����/�u���J�n���̃J�[�\���ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	bool				bSearch;				// ����/�u���J�n�ʒu��o�^���邩											// 02/06/26 ai
	SearchDirection		nISearchDirection;
	int					nISearchMode;
	bool				bISearchWrap;
	bool				bISearchFlagHistory[256];
	int					nISearchHistoryCount;
	bool				bISearchFirst;
	LayoutRange			searchHistory[256];

	// �}�N��
	bool			bExecutingKeyMacro;		// �L�[�{�[�h�}�N���̎��s��
	bool			bCommandRunning;		// �R�}���h�̎��s��

	// ���͕⊮
	bool			bHokan;					//	�⊮�����H���⊮�E�B���h�E���\������Ă��邩�H���ȁH

	// �ҏW
	bool			bDoing_UndoRedo;		// Undo, Redo�̎��s����

	// ����Tip�֘A
	DWORD			dwTipTimer;				// Tip�N���^�C�}�[
	TipWnd			tipWnd;					// Tip�\���E�B���h�E
	POINT			poTipCurPos;			// Tip�N�����̃}�E�X�J�[�\���ʒu
	bool			bInMenuLoop;			// ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂�
	DicMgr			dicMgr;					// �����}�l�[�W��

	TCHAR			szComposition[512];		// IMR_DOCUMENTFEED�p���͒�������f�[�^

	// IME
private:
	UINT			uMSIMEReconvertMsg;
	UINT			uATOKReconvertMsg;
public:
	UINT			uWM_MSIME_RECONVERTREQUEST;
private:
	int				nLastReconvLine;		// 2002.04.09 minfu �ĕϊ����ۑ��p;
	int				nLastReconvIndex;		// 2002.04.09 minfu �ĕϊ����ۑ��p;

public:
	// ATOK��p�ĕϊ���API
	typedef BOOL (WINAPI *FP_ATOK_RECONV)(HIMC, int, PRECONVERTSTRING, DWORD);
	HMODULE			hAtokModule;
	FP_ATOK_RECONV	AT_ImmSetReconvertString;

	// ���̑�
	AutoMarkMgr*	pHistory;			//	Jump����
	RegexKeyword*	pRegexKeyword;		//@@@ 2001.11.17 add MIK
	int				nMyIndex;			// �������
	Migemo*			pMigemo;
	bool			bMiniMap;
	bool			bMiniMapMouseDown;
	int				nPageViewTop;
	int				nPageViewBottom;

private:
	DISALLOW_COPY_AND_ASSIGN(EditView);
};

class OutputAdapter
{
public:
	OutputAdapter(){};
	virtual  ~OutputAdapter(){};

	virtual bool OutputW(const WCHAR* pBuf, int size = -1) = 0;
	virtual bool OutputA(const ACHAR* pBuf, int size = -1) = 0;
	virtual bool IsEnableRunningDlg(){ return true; }
	virtual bool IsActiveDebugWindow(){ return true; }
};


