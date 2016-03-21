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
		return *m_pEditDoc;
	}
	EditDoc& GetDocument() {
		return *m_pEditDoc;
	}
public:
	// �w�i�Ƀr�b�g�}�b�v���g�p���邩�ǂ���
	// 2010.10.03 �w�i����
	bool IsBkBitmap() const { return m_pEditDoc->m_hBackImg != NULL; }

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
	bool MyGetClipboardData(NativeW&, bool*, bool* = NULL);			// �N���b�v�{�[�h����f�[�^���擾

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
	LayoutInt OnVScroll(int, int);					// �����X�N���[���o�[���b�Z�[�W����
	LayoutInt OnHScroll(int, int);					// �����X�N���[���o�[���b�Z�[�W����
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
		LayoutInt		nLineTo			// [in]     ���I�����郌�C�A�E�g�s�ԍ�
	);

	// ���C�A�E�g�s��1�s�`��
	bool DrawLayoutLine(ColorStrategyInfo& csInfo);

	// �F����
public:
	Color3Setting GetColorIndex(const Layout* pLayout, LayoutYInt nLineNum, int nIndex, ColorStrategyInfo& csInfo, bool bPrev = false);	// �w��ʒu��ColorIndex�̎擾 02/12/13 ai
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
	void DispTextSelected(HDC hdc, LayoutInt nLineNum, const Point& ptXY, LayoutInt nX_Layout);	// �e�L�X�g���]
	void RedrawAll();										// �t�H�[�J�X�ړ����̍ĕ`��
	void Redraw();											// 2001/06/21 asa-o �ĕ`��
	void RedrawLines( LayoutYInt top, LayoutYInt bottom );
	void CaretUnderLineON(bool, bool, bool);				// �J�[�\���s�A���_�[���C����ON
	void CaretUnderLineOFF(bool, bool, bool, bool);			// �J�[�\���s�A���_�[���C����OFF
	bool GetDrawSwitch() const {
		return m_bDrawSWITCH;
	}
	bool SetDrawSwitch(bool b) {
		bool bOld = m_bDrawSWITCH;
		m_bDrawSWITCH = b;
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
	LayoutInt GetWrapOverhang(void) const;								// �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z	// 2008.06.08 ryoji
	LayoutInt ViewColNumToWrapColNum(LayoutInt nViewColNum) const;		//�u�E�[�Ő܂�Ԃ��v�p�Ƀr���[�̌�������܂�Ԃ��������v�Z����	// 2008.06.08 ryoji
	LayoutInt GetRightEdgeForScrollBar(void);							// �X�N���[���o�[����p�ɉE�[���W���擾����		// 2009.08.28 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           IME                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Aug. 25, 2002 genta protected->public�Ɉړ�
	bool IsImeON(void);	// IME ON��	// 2006.12.04 ryoji
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �X�N���[��                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LayoutInt  ScrollAtV(LayoutInt);								// �w���[�s�ʒu�փX�N���[��
	LayoutInt  ScrollAtH(LayoutInt);								// �w�荶�[���ʒu�փX�N���[��
	//	From Here Sep. 11, 2004 genta ����ێ��̓����X�N���[��
	LayoutInt  ScrollByV(LayoutInt vl) {	return ScrollAtV(GetTextArea().GetViewTopLine() + vl);}	// �w��s�X�N���[��
	LayoutInt  ScrollByH(LayoutInt hl) {	return ScrollAtH(GetTextArea().GetViewLeftCol() + hl);}	// �w�茅�X�N���[��
	void ScrollDraw(LayoutInt, LayoutInt, const RECT&, const RECT&, const RECT&);
	void MiniMapRedraw(bool);
public:
	void SyncScrollV(LayoutInt);										// ���������X�N���[��
	void SyncScrollH(LayoutInt);										// ���������X�N���[��

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
	bool IsEmptyArea(LayoutPoint ptFrom, LayoutPoint ptTo = LayoutPoint(LayoutInt(-1), LayoutInt(-1)), bool bSelect = false, bool bBoxSelect = false) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �e�픻��                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool IsCurrentPositionURL(const LayoutPoint& ptCaretPos, LogicRange* pUrlRange, std::wstring* pwstrURL); // �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ�
	bool CheckTripleClick(Point ptMouse);							// �g���v���N���b�N���`�F�b�N����	// 2007.10.02 nasukoji
	
	bool ExecCmd(const TCHAR*, int, const TCHAR*, OutputAdapter* = NULL) ;			// �q�v���Z�X�̕W���o�͂����_�C���N�g����
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
		m_bDragMode = b;
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ҏW                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// �w��ʒu�̎w�蒷�f�[�^�폜
	void DeleteData2(
		const LayoutPoint&	ptCaretPos,
		LogicInt			nDelLen,
		NativeW*			pMem
	);

	// ���݈ʒu�̃f�[�^�폜
	void DeleteData(bool bRedraw);

	// ���݈ʒu�Ƀf�[�^��}��
	void InsertData_CEditView(
		LayoutPoint	ptInsertPos,
		const wchar_t*	pData,
		int				nDataLen,
		LayoutPoint*	pptNewPos,	// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
		bool			bRedraw
	);

	// �f�[�^�u�� �폜&�}���ɂ��g����
	void ReplaceData_CEditView(
		const LayoutRange&	delRange,			// �폜�͈́B���C�A�E�g�P�ʁB
		const wchar_t*		pInsData,			// �}������f�[�^
		LogicInt			nInsDataLen,		// �}������f�[�^�̒���
		bool				bRedraw,
		OpeBlk*				pOpeBlk,
		bool				bFastMode = false,
		const LogicRange*	psDelRangeLogicFast = NULL
	);
	void ReplaceData_CEditView2(
		const LogicRange&	delRange,			// �폜�͈́B���W�b�N�P�ʁB
		const wchar_t*		pInsData,			// �}������f�[�^
		LogicInt			nInsDataLen,		// �}������f�[�^�̒���
		bool				bRedraw,
		OpeBlk*				pOpeBlk,
		bool				bFastMode = false
	);
	bool ReplaceData_CEditView3(
		LayoutRange		delRange,			// �폜�͈́B���C�A�E�g�P�ʁB
		OpeLineData*	pMemCopyOfDeleted,	// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
		OpeLineData*	pInsData,
		bool			bRedraw,
		OpeBlk*			pOpeBlk,
		int				nDelSeq,
		int*			pnInsSeq,
		bool			bFastMode = false,
		const LogicRange*	psDelRangeLogicFast = NULL
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

	int IsSearchString(const StringRef& str, LogicInt, LogicInt*, LogicInt*) const;	// ���݈ʒu������������ɊY�����邩	// 2002.02.08 hor �����ǉ�

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
	bool IsBracket(const wchar_t*, LogicInt, LogicInt);					// ���ʔ��� 03/01/09 ai

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
		bool* pbJumpToSelf = NULL
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
	TOGGLE_WRAP_ACTION GetWrapMode(LayoutInt* newKetas);
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
	TextArea& GetTextArea() { assert(m_pTextArea); return *m_pTextArea; }
	const TextArea& GetTextArea() const { assert(m_pTextArea); return *m_pTextArea; }
	Caret& GetCaret() { assert(m_pCaret); return *m_pCaret; }
	const Caret& GetCaret() const { assert(m_pCaret); return *m_pCaret; }
	Ruler& GetRuler() { assert(m_pRuler); return *m_pRuler; }
	const Ruler& GetRuler() const { assert(m_pRuler); return *m_pRuler; }

	// ��v�����A�N�Z�X
	TextMetrics& GetTextMetrics() { return m_textMetrics; }
	const TextMetrics& GetTextMetrics() const { return m_textMetrics; }
	ViewSelect& GetSelectionInfo() { return m_viewSelect; }
	const ViewSelect& GetSelectionInfo() const { return m_viewSelect; }

	// ��v�I�u�W�F�N�g�A�N�Z�X
	ViewFont& GetFontset() { assert(m_pViewFont); return *m_pViewFont; }
	const ViewFont& GetFontset() const { assert(m_pViewFont); return *m_pViewFont; }

	// ��v�w���p�A�N�Z�X
	const ViewParser& GetParser() const { return m_parser; }
	const TextDrawer& GetTextDrawer() const { return m_textDrawer; }
	ViewCommander& GetCommander() { return m_commander; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       �����o�ϐ��Q                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// �Q��
	EditWnd&			m_editWnd;	// �E�B���h�E
	EditDoc*			m_pEditDoc;	// �h�L�������g
	const TypeConfig*	m_pTypeData;

	// ��v�\�����i
	TextArea*		m_pTextArea;
	Caret*			m_pCaret;
	Ruler*			m_pRuler;

	// ��v����
	TextMetrics		m_textMetrics;
	ViewSelect		m_viewSelect;

	// ��v�I�u�W�F�N�g
	ViewFont*		m_pViewFont;

	// ��v�w���p
	ViewParser		m_parser;
	TextDrawer		m_textDrawer;
	ViewCommander	m_commander;

public:
	// �E�B���h�E
	HWND			m_hwndParent;		// �e�E�B���h�E�n���h��
	HWND			m_hwndVScrollBar;	// �����X�N���[���o�[�E�B���h�E�n���h��
	int				m_nVScrollRate;		// �����X�N���[���o�[�̏k��
	HWND			m_hwndHScrollBar;	// �����X�N���[���o�[�E�B���h�E�n���h��
	HWND			m_hwndSizeBox;		// �T�C�Y�{�b�N�X�E�B���h�E�n���h��
	SplitBoxWnd*	m_pcsbwVSplitBox;	// ���������{�b�N�X
	SplitBoxWnd*	m_pcsbwHSplitBox;	// ���������{�b�N�X
	AutoScrollWnd	m_autoScrollWnd;	// �I�[�g�X�N���[��

public:
	// �`��
	bool			m_bDrawSWITCH;
	COLORREF		m_crBack;				// �e�L�X�g�̔w�i�F			// 2006.12.07 ryoji
	COLORREF		m_crBack2;				// �e�L�X�g�̔w�i(�L�����b�g�p)
	LayoutInt		m_nOldUnderLineY;		// �O���悵���J�[�\���A���_�[���C���̈ʒu 0����=��\��
	LayoutInt		m_nOldUnderLineYBg;
	int				m_nOldUnderLineYMargin;
	int				m_nOldUnderLineYHeight;
	int				m_nOldUnderLineYHeightReal;
	int				m_nOldCursorLineX;		// �O���悵���J�[�\���ʒu�c���̈ʒu // 2007.09.09 Moca
	int				m_nOldCursorVLineWidth;	// �J�[�\���ʒu�c���̑���(px)

public:
	// ��ʃo�b�t�@
	HDC				m_hdcCompatDC;		// �ĕ`��p�R���p�`�u���c�b
	HBITMAP			m_hbmpCompatBMP;	// �ĕ`��p�������a�l�o
	HBITMAP			m_hbmpCompatBMPOld;	// �ĕ`��p�������a�l�o(OLD)
	int				m_nCompatBMPWidth;  // �č��p�������a�l�o�̕�	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	int				m_nCompatBMPHeight; // �č��p�������a�l�o�̍���	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@

public:
	// D&D
	DropTarget*		m_pDropTarget;
	bool			m_bDragMode;					// �I���e�L�X�g�̃h���b�O����
	CLIPFORMAT		m_cfDragData;					// �h���b�O�f�[�^�̃N���b�v�`��	// 2008.06.20 ryoji
	bool			m_bDragBoxData;					// �h���b�O�f�[�^�͋�`��
	LayoutPoint		m_ptCaretPos_DragEnter;			// �h���b�O�J�n���̃J�[�\���ʒu	// 2007.12.09 ryoji
	LayoutInt		m_nCaretPosX_Prev_DragEnter;	// �h���b�O�J�n����X���W�L��	// 2007.12.09 ryoji

	// ����
	LogicPoint		m_ptBracketCaretPos_PHY;	// �O�J�[�\���ʒu�̊��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	LogicPoint		m_ptBracketPairPos_PHY;		// �Ί��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	bool			m_bDrawBracketPairFlag;		// �Ί��ʂ̋����\�����s�Ȃ���						// 03/02/18 ai

	// �}�E�X
	bool			m_bActivateByMouse;		// �}�E�X�ɂ��A�N�e�B�x�[�g	// 2007.10.02 nasukoji
	DWORD			m_dwTripleClickCheck;	// �g���v���N���b�N�`�F�b�N�p����	// 2007.10.02 nasukoji
	Point			m_mouseDownPos;			// �N���b�N���̃}�E�X���W
	int				m_nWheelDelta;			// �z�C�[���ω���
	EFunctionCode	m_eWheelScroll; 		// �X�N���[���̎��
	int				m_nMousePouse;			// �}�E�X��~����
	Point			m_mousePousePos;		// �}�E�X�̒�~�ʒu
	bool			m_bHideMouse;

	int				m_nAutoScrollMode;			// �I�[�g�X�N���[�����[�h
	bool			m_bAutoScrollDragMode;		// �h���b�O���[�h
	Point			m_autoScrollMousePos;		// �I�[�g�X�N���[���̃}�E�X��ʒu
	bool			m_bAutoScrollVertical;		// �����X�N���[����
	bool			m_bAutoScrollHorizontal;	// �����X�N���[����

	// ����
	SearchStringPattern m_searchPattern;
	mutable Bregexp		m_curRegexp;				// �R���p�C���f�[�^
	bool				m_bCurSrchKeyMark;			// ����������̃}�[�N
	bool				m_bCurSearchUpdate;			// �R���p�C���f�[�^�X�V�v��
	int					m_nCurSearchKeySequence;	// �����L�[�V�[�P���X
	std::wstring		m_strCurSearchKey;			// ����������
	SearchOption		m_curSearchOption;			// �����^�u��  �I�v�V����
	LogicPoint			m_ptSrchStartPos_PHY;		// ����/�u���J�n���̃J�[�\���ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	bool				m_bSearch;					// ����/�u���J�n�ʒu��o�^���邩											// 02/06/26 ai
	SearchDirection		m_nISearchDirection;
	int					m_nISearchMode;
	bool				m_bISearchWrap;
	bool				m_bISearchFlagHistory[256];
	int					m_nISearchHistoryCount;
	bool				m_bISearchFirst;
	LayoutRange			m_searchHistory[256];

	// �}�N��
	bool			m_bExecutingKeyMacro;	// �L�[�{�[�h�}�N���̎��s��
	bool			m_bCommandRunning;		// �R�}���h�̎��s��

	// ���͕⊮
	bool			m_bHokan;				//	�⊮�����H���⊮�E�B���h�E���\������Ă��邩�H���ȁH

	// �ҏW
	bool			m_bDoing_UndoRedo;		// Undo, Redo�̎��s����

	// ����Tip�֘A
	DWORD			m_dwTipTimer;			// Tip�N���^�C�}�[
	TipWnd			m_tipWnd;				// Tip�\���E�B���h�E
	POINT			m_poTipCurPos;			// Tip�N�����̃}�E�X�J�[�\���ʒu
	bool			m_bInMenuLoop;			// ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂�
	DicMgr			m_dicMgr;				// �����}�l�[�W��

	TCHAR			m_szComposition[512];	// IMR_DOCUMENTFEED�p���͒�������f�[�^

	// IME
private:
	UINT			m_uMSIMEReconvertMsg;
	UINT			m_uATOKReconvertMsg;
public:
	UINT			m_uWM_MSIME_RECONVERTREQUEST;
private:
	int				m_nLastReconvLine;             // 2002.04.09 minfu �ĕϊ����ۑ��p;
	int				m_nLastReconvIndex;            // 2002.04.09 minfu �ĕϊ����ۑ��p;

public:
	// ATOK��p�ĕϊ���API
	typedef BOOL (WINAPI *FP_ATOK_RECONV)(HIMC, int, PRECONVERTSTRING, DWORD);
	HMODULE			m_hAtokModule;
	FP_ATOK_RECONV	m_AT_ImmSetReconvertString;

	// ���̑�
	AutoMarkMgr*	m_pHistory;			//	Jump����
	RegexKeyword*	m_pRegexKeyword;	//@@@ 2001.11.17 add MIK
	int				m_nMyIndex;			// �������
	Migemo*			m_pMigemo;
	bool			m_bMiniMap;
	bool			m_bMiniMapMouseDown;
	LayoutInt		m_nPageViewTop;
	LayoutInt		m_nPageViewBottom;

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


