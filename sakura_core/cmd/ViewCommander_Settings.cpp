/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�ݒ�n)�֐��Q

	2012/12/15	ViewCommander.cpp,ViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta, aroka
	Copyright (C) 2006, genta, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "typeprop/DlgTypeList.h"
#include "dlg/DlgFavorite.h"	// �����̊Ǘ�	//@@@ 2003.04.08 MIK
#include "EditApp.h"
#include "util/shell.h"
#include "PropertyManager.h"
#include "util/window.h"


/*! �c�[���o�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� EditWnd::LayoutToolBar(), EditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void ViewCommander::Command_SHOWTOOLBAR(void)
{
	auto& editWnd = GetEditWindow();	// Sep. 10, 2002 genta

	GetDllShareData().common.window.bDispToolBar = (editWnd.m_toolbar.GetToolbarHwnd() == NULL);	// �c�[���o�[�\��
	editWnd.LayoutToolBar();
	editWnd.EndLayoutBars();

	// �S�E�B���h�E�ɕύX��ʒm����B
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::Toolbar,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}


/*! �t�@���N�V�����L�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� EditWnd::LayoutFuncKey(), EditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void ViewCommander::Command_SHOWFUNCKEY(void)
{
	EditWnd& editWnd = GetEditWindow();	// Sep. 10, 2002 genta

	GetDllShareData().common.window.bDispFuncKeyWnd = !editWnd.m_funcKeyWnd.GetHwnd();	// �t�@���N�V�����L�[�\��
	editWnd.LayoutFuncKey();
	editWnd.EndLayoutBars();

	// �S�E�B���h�E�ɕύX��ʒm����B
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::FuncKey,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}


/*! �^�u(�E�B���h�E)�̕\��/��\��

	@author MIK
	@date 2003.06.10 �V�K�쐬
	@date 2006.12.19 ryoji �\���ؑւ� EditWnd::LayoutTabBar(), EditWnd::EndLayoutBars() �ōs���悤�ɕύX
	@date 2007.06.20 ryoji �O���[�vID���Z�b�g
 */
void ViewCommander::Command_SHOWTAB(void)
{
	auto& editWnd = GetEditWindow();	// Sep. 10, 2002 genta

	GetDllShareData().common.tabBar.bDispTabWnd = !editWnd.m_tabWnd.GetHwnd();	// �^�u�o�[�\��
	editWnd.LayoutTabBar();
	editWnd.EndLayoutBars();

	// �܂Ƃ߂�Ƃ��� WS_EX_TOPMOST ��Ԃ𓯊�����	// 2007.05.18 ryoji
	if (GetDllShareData().common.tabBar.bDispTabWnd
		&& !GetDllShareData().common.tabBar.bDispTabWndMultiWin
	) {
		GetEditWindow().WindowTopMost(
			((DWORD)::GetWindowLongPtr(GetEditWindow().GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST)? 1: 2
		);
	}

	// �S�E�B���h�E�ɕύX��ʒm����B
	AppNodeManager::getInstance().ResetGroupId();
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::Tab,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}


/*! �X�e�[�^�X�o�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� EditWnd::LayoutStatusBar(), EditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void ViewCommander::Command_SHOWSTATUSBAR(void)
{
	auto& editWnd = GetEditWindow();	// Sep. 10, 2002 genta

	GetDllShareData().common.window.bDispStatusBar = !editWnd.m_statusBar.GetStatusHwnd();	// �X�e�[�^�X�o�[�\��
	editWnd.LayoutStatusBar();
	editWnd.EndLayoutBars();

	// �S�E�B���h�E�ɕύX��ʒm����B
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::StatusBar,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}

/*! �~�j�}�b�v�̕\��/��\��

	@date 2014.07.14 �V�K�쐬
*/
void ViewCommander::Command_SHOWMINIMAP(void)
{
	auto& editWnd = GetEditWindow();	//	Sep. 10, 2002 genta

	GetDllShareData().common.window.bDispMiniMap = (editWnd.GetMiniMap().GetHwnd() == NULL);
	editWnd.LayoutMiniMap();
	editWnd.EndLayoutBars();

	// �S�E�B���h�E�ɕύX��ʒm����B
	AppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)BarChangeNotifyType::MiniMap,
		(LPARAM)editWnd.GetHwnd(),
		editWnd.GetHwnd()
	);
}


// �^�C�v�ʐݒ�ꗗ
void ViewCommander::Command_TYPE_LIST(void)
{
	DlgTypeList dlgTypeList;
	DlgTypeList::Result result;
	result.documentType = GetDocument().m_docType.GetDocumentType();
	result.bTempChange = true;
	if (dlgTypeList.DoModal(G_AppInstance(), m_view.GetHwnd(), &result)) {
		// Nov. 29, 2000 genta
		// �ꎞ�I�Ȑݒ�K�p�@�\�𖳗���ǉ�
		if (result.bTempChange) {
			HandleCommand(F_CHANGETYPE, true, (LPARAM)result.documentType.GetIndex() + 1, 0, 0, 0);
		}else {
			// �^�C�v�ʐݒ�
			EditApp::getInstance().OpenPropertySheetTypes(-1, result.documentType);
		}
	}
	return;
}


// �^�C�v�ʐݒ�ꎞ�K�p
void ViewCommander::Command_CHANGETYPE(int nTypePlusOne)
{
	TypeConfigNum type = TypeConfigNum(nTypePlusOne - 1);
	auto& doc = GetDocument();
	if (nTypePlusOne == 0) {
		type = doc.m_docType.GetDocumentType();
	}
	if (type.IsValidType() && type.GetIndex() < GetDllShareData().nTypesCount) {
		const TypeConfigMini* pConfig;
		DocTypeManager().GetTypeConfigMini(type, &pConfig);
		doc.m_docType.SetDocumentTypeIdx(pConfig->id, true);
		doc.m_docType.LockDocumentType();
		doc.OnChangeType();
	}
}


// �^�C�v�ʐݒ�
void ViewCommander::Command_OPTION_TYPE(void)
{
	EditApp::getInstance().OpenPropertySheetTypes(-1, GetDocument().m_docType.GetDocumentType());
}


// ���ʐݒ�
void ViewCommander::Command_OPTION(void)
{
	// �ݒ�v���p�e�B�V�[�g �e�X�g�p
	EditApp::getInstance().OpenPropertySheet(-1);
}


// �t�H���g�ݒ�
void ViewCommander::Command_FONT(void)
{
	HWND hwndFrame = GetMainWindow();

	// �t�H���g�ݒ�_�C�A���O
	auto& csView = GetDllShareData().common.view;
	LOGFONT lf = csView.lf;
	INT nPointSize;
#ifdef USE_UNFIXED_FONT
	bool bFixedFont = false;
#else
	bool bFixedFont = true;
#endif
	if (MySelectFont(&lf, &nPointSize, EditWnd::getInstance().m_splitterWnd.GetHwnd(), bFixedFont)) {
		csView.lf = lf;
		csView.nPointSize = nPointSize;

		if (csView.lf.lfPitchAndFamily & FIXED_PITCH) {
			csView.bFontIs_FixedPitch = true;	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
		}else {
			csView.bFontIs_FixedPitch = false;	// ���݂̃t�H���g�͌Œ蕝�t�H���g�łȂ��A��
		}
		// �ݒ�ύX�𔽉f������
		// �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����
		AppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_SAVEEDITSTATE,
			(WPARAM)0, (LPARAM)0, hwndFrame
		);
		AppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0, (LPARAM)PM_CHANGESETTING_FONT, hwndFrame
		);

		// �L�����b�g�̕\��
//		::HideCaret(GetHwnd());
//		::ShowCaret(GetHwnd());

//		// �A�N�e�B�u�ɂ���
//		// �A�N�e�B�u�ɂ���
//		ActivateFrameWindow(hwndFrame);
	}
	return;
}


/*! �t�H���g�T�C�Y�ݒ�
	@param fontSize �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj
	@param shift �t�H���g�T�C�Y���g��or�k�����邽�߂̕ύX��(fontSize=0�̂Ƃ��L��)

	@note TrueType�̂݃T�|�[�g

	@date 2013.04.10 novice �V�K�쐬
*/
void ViewCommander::Command_SETFONTSIZE(int fontSize, int shift, int mode)
{
	// The point sizes recommended by "The Windows Interface: An Application Design Guide", 1/10�|�C���g�P��
	static const INT sizeTable[] = { 8*10, 9*10, 10*10, (INT)(10.5*10), 11*10, 12*10, 14*10, 16*10, 18*10, 20*10, 22*10, 24*10, 26*10, 28*10, 36*10, 48*10, 72*10 };
	auto& csView = GetDllShareData().common.view;
	const LOGFONT& lf = (mode == 0 ? csView.lf
		: GetEditWindow().GetLogfont(mode == 2));
	INT nPointSize;

	// TrueType�̂ݑΉ�
	if (OUT_STROKE_PRECIS != lf.lfOutPrecision) {
		return;
	}

	if (!(0 <= mode && mode <= 2)) {
		return;
	}

	if (fontSize != 0) {
		// �t�H���g�T�C�Y�𒼐ڑI������ꍇ
		nPointSize = t_max(sizeTable[0], t_min(sizeTable[_countof(sizeTable) - 1], fontSize));
	}else if (shift != 0) {
		// ���݂̃t�H���g�ɑ΂��āA�k��or�g�債���t�H���g�I������ꍇ
		nPointSize = (mode == 0 ? csView.nPointSize
			: GetEditWindow().GetFontPointSize(mode == 2));

		// �t�H���g�̊g��or�k�����邽�߂̃T�C�Y����
		for (int i=0; i<_countof(sizeTable); ++i) {
			if (nPointSize <= sizeTable[i]) {
				int index = t_max(0, t_min((int)_countof(sizeTable) - 1, (int)(i + shift)));
				nPointSize = sizeTable[index];
				break;
			}
		}
	}else {
		// �t�H���g�T�C�Y���ς��Ȃ��̂ŏI��
		return;
	}
	// �V�����t�H���g�T�C�Y�ݒ�
	int lfHeight = DpiPointsToPixels(-nPointSize, 10);
	int nTypeIndex = -1;
	auto& doc = GetDocument();
	if (mode == 0) {
		csView.lf.lfHeight = lfHeight;
		csView.nPointSize = nPointSize;
	}else if (mode == 1) {
		TypeConfigNum nDocType = doc.m_docType.GetDocumentType();
		auto type = std::make_unique<TypeConfig>();
		if (!DocTypeManager().GetTypeConfig(nDocType, *type)) {
			// ��̃G���[
			return;
		}
		type->bUseTypeFont = true; // �^�C�v�ʃt�H���g��L���ɂ���
		type->lf = lf;
		type->lf.lfHeight = lfHeight;
		type->nPointSize = nPointSize;
		DocTypeManager().SetTypeConfig(nDocType, *type);
		nTypeIndex = nDocType.GetIndex();
	}else if (mode == 2) {
		doc.m_blfCurTemp = true;
		doc.m_lfCur = lf;
		doc.m_lfCur.lfHeight = lfHeight;
		doc.m_nPointSizeCur = nPointSize;
		doc.m_nPointSizeOrg = GetEditWindow().GetFontPointSize(false);
	}

	HWND hwndFrame = GetMainWindow();

	// �ݒ�ύX�𔽉f������
	// �V���Ƀ^�C�v�ʂ�ꎞ�ݒ肪�L���ɂȂ��Ă��t�H���g���͕ς��Ȃ��̂�SIZE�݂̂̕ύX�ʒm������
	if (mode == 0 || mode == 1) {
		// �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����
		AppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)nTypeIndex,
			(LPARAM)PM_CHANGESETTING_FONTSIZE,
			hwndFrame
		);
	}else if (mode == 2) {
		// ���������X�V
		doc.OnChangeSetting(false);
	}
}


/*! ���݂̃E�B���h�E���Ő܂�Ԃ�

	@date 2002.01.14 YAZAKI ���݂̃E�B���h�E���Ő܂�Ԃ���Ă���Ƃ��́A�ő�l�ɂ���悤��
	@date 2002.04.08 YAZAKI �Ƃ��ǂ��E�B���h�E���Ő܂�Ԃ���Ȃ����Ƃ�����o�O�C���B
	@date 2005.08.14 genta �����ł̐ݒ�͋��ʐݒ�ɔ��f���Ȃ��D
	@date 2005.10.22 aroka ���݂̃E�B���h�E�����ő�l�������^�C�v�̏����l ���g�O���ɂ���

	@note �ύX���鏇����ύX�����Ƃ���EditWnd::InitMenu()���ύX���邱��
	@sa EditWnd::InitMenu()
*/
void ViewCommander::Command_WRAPWINDOWWIDTH(void)	// Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
{
	// Jan. 8, 2006 genta ���菈����m_view.GetWrapMode()�ֈړ�
	EditView::TOGGLE_WRAP_ACTION nWrapMode;
	LayoutInt newKetas;
	
	nWrapMode = m_view.GetWrapMode(&newKetas);
	auto& doc = GetDocument();
	doc.m_nTextWrapMethodCur = TextWrappingMethod::SettingWidth;
	doc.m_bTextWrapMethodCurTemp = (doc.m_nTextWrapMethodCur != m_view.m_pTypeData->nTextWrapMethod);
	if (nWrapMode == EditView::TGWRAP_NONE) {
		return;	// �܂�Ԃ����͌��̂܂�
	}

	GetEditWindow().ChangeLayoutParam(true, doc.m_layoutMgr.GetTabSpace(), newKetas);
	
	// Aug. 14, 2005 genta ���ʐݒ�ւ͔��f�����Ȃ�
//	m_view.m_pTypeData->nMaxLineKetas = m_nViewColNum;

// 2013.12.30 �����Ɉړ����Ȃ��悤��
//	m_view.GetTextArea().SetViewLeftCol(LayoutInt(0));		// �\����̈�ԍ��̌�(0�J�n)

	// �t�H�[�J�X�ړ����̍ĕ`��
	m_view.RedrawAll();
	return;
}


// from ViewCommander_New.cpp
/*!	�����̊Ǘ�(�_�C�A���O)
	@author	MIK
	@date	2003/04/07
*/
void ViewCommander::Command_Favorite(void)
{
	DlgFavorite	dlgFavorite;

	// �_�C�A���O��\������
	if (!dlgFavorite.DoModal(G_AppInstance(), m_view.GetHwnd(), (LPARAM)&GetDocument())) {
		return;
	}

	return;
}


/*!
	@brief �e�L�X�g�̐܂�Ԃ����@��ύX����
	
	@param[in] nWrapMethod �܂�Ԃ����@
		WRAP_NO_TEXT_WRAP  : �܂�Ԃ��Ȃ�
		WRAP_SETTING_WIDTH ; �w�茅�Ő܂�Ԃ�
		WRAP_WINDOW_WIDTH  ; �E�[�Ő܂�Ԃ�
	
	@note �E�B���h�E�����E�ɕ�������Ă���ꍇ�A�����̃E�B���h�E����܂�Ԃ����Ƃ���B
	
	@date 2008.05.31 nasukoji	�V�K�쐬
	@date 2009.08.28 nasukoji	�e�L�X�g�̍ő啝���Z�o����
*/
void ViewCommander::Command_TEXTWRAPMETHOD(TextWrappingMethod nWrapMethod)
{
	auto& doc = GetDocument();

	// ���݂̐ݒ�l�Ɠ����Ȃ牽�����Ȃ�
	if (doc.m_nTextWrapMethodCur == nWrapMethod)
		return;

	int nWidth;

	switch (nWrapMethod) {
	case TextWrappingMethod::NoWrapping:		// �܂�Ԃ��Ȃ�
		nWidth = MAXLINEKETAS;	// �A�v���P�[�V�����̍ő啝�Ő܂�Ԃ�
		break;

	case TextWrappingMethod::SettingWidth:	// �w�茅�Ő܂�Ԃ�
		nWidth = (Int)doc.m_docType.GetDocumentAttribute().nMaxLineKetas;
		break;

	case TextWrappingMethod::WindowWidth:		// �E�[�Ő܂�Ԃ�
		// �E�B���h�E�����E�ɕ�������Ă���ꍇ�͍����̃E�B���h�E�����g�p����
		nWidth = (Int)m_view.ViewColNumToWrapColNum(GetEditWindow().GetView(0).GetTextArea().m_nViewColNum);
		break;

	default:
		return;	// �s���Ȓl�̎��͉������Ȃ�
	}

	doc.m_nTextWrapMethodCur = nWrapMethod;	// �ݒ���L��

	// �܂�Ԃ����@�̈ꎞ�ݒ�K�p�^�ꎞ�ݒ�K�p����	// 2008.06.08 ryoji
	doc.m_bTextWrapMethodCurTemp = (doc.m_docType.GetDocumentAttribute().nTextWrapMethod != nWrapMethod);

	// �܂�Ԃ��ʒu��ύX
	GetEditWindow().ChangeLayoutParam(false, doc.m_layoutMgr.GetTabSpace(), (LayoutInt)nWidth);

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if (doc.m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		doc.m_layoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
		GetEditWindow().RedrawAllViews(nullptr);	// Scroll Bar�̍X�V���K�v�Ȃ̂ōĕ\�������s����
	}else {
		doc.m_layoutMgr.ClearLayoutLineWidth();		// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
}


/*!
	@brief �����J�E���g���@��ύX����
	
	@param[in] nMode �����J�E���g���@
		SelectCountMode::Toggle : �����J�E���g���@���g�O��
		SelectCountMode::ByChar ; �������ŃJ�E���g
		SelectCountMode::ByByte ; �o�C�g���ŃJ�E���g
*/
void ViewCommander::Command_SELECT_COUNT_MODE(int nMode)
{
	// �ݒ�ɂ͕ۑ������AView���Ɏ��t���O��ݒ�
	//BOOL* pbDispSelCountByByte = &GetDllShareData().common.statusBar.bDispSelCountByByte;
	auto& selectCountMode = GetEditWindow().m_nSelectCountMode;

	if (nMode == (int)SelectCountMode::Toggle) {
		// �������̃o�C�g���g�O��
		SelectCountMode nCurrentMode;
		if (selectCountMode == SelectCountMode::Toggle) {
			nCurrentMode = (GetDllShareData().common.statusBar.bDispSelCountByByte ?
								SelectCountMode::ByByte :
								SelectCountMode::ByChar);
		}else {
			nCurrentMode = selectCountMode;
		}
		selectCountMode = (nCurrentMode == SelectCountMode::ByByte ?
								SelectCountMode::ByChar :
								SelectCountMode::ByByte);
	}else if (nMode == (int)SelectCountMode::ByByte || nMode == (int)SelectCountMode::ByChar) {
		selectCountMode = (SelectCountMode)nMode;
	}
}


/*!	@brief ���p���̐ݒ�
	@date Jan. 29, 2005 genta �V�K�쐬
*/
void ViewCommander::Command_SET_QUOTESTRING(const wchar_t* quotestr)
{
	if (!quotestr) {
		return;
	}

	auto& csFormat = GetDllShareData().common.format;
	wcsncpy(csFormat.szInyouKigou, quotestr, _countof(csFormat.szInyouKigou));
	csFormat.szInyouKigou[_countof(csFormat.szInyouKigou) - 1] = L'\0';
}

