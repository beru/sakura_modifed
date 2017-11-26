#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "typeprop/DlgTypeList.h"
#include "dlg/DlgFavorite.h"	// �����̊Ǘ�
#include "EditApp.h"
#include "util/shell.h"
#include "PropertyManager.h"
#include "util/window.h"

// ViewCommander�N���X�̃R�}���h(�ݒ�n)�֐��Q

/*! �c�[���o�[�̕\��/��\�� */
void ViewCommander::Command_ShowToolBar(void)
{
	auto& editWnd = GetEditWindow();

	GetDllShareData().common.window.bDispToolBar = (editWnd.toolbar.GetToolbarHwnd() == NULL);	// �c�[���o�[�\��
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


/*! �t�@���N�V�����L�[�̕\��/��\�� */
void ViewCommander::Command_ShowFuncKey(void)
{
	EditWnd& editWnd = GetEditWindow();

	GetDllShareData().common.window.bDispFuncKeyWnd = !editWnd.funcKeyWnd.GetHwnd();	// �t�@���N�V�����L�[�\��
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


/*! �^�u(�E�B���h�E)�̕\��/��\�� */
void ViewCommander::Command_ShowTab(void)
{
	auto& editWnd = GetEditWindow();

	GetDllShareData().common.tabBar.bDispTabWnd = !editWnd.tabWnd.GetHwnd();	// �^�u�o�[�\��
	editWnd.LayoutTabBar();
	editWnd.EndLayoutBars();

	// �܂Ƃ߂�Ƃ��� WS_EX_TOPMOST ��Ԃ𓯊�����
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


/*! �X�e�[�^�X�o�[�̕\��/��\�� */
void ViewCommander::Command_ShowStatusBar(void)
{
	auto& editWnd = GetEditWindow();

	GetDllShareData().common.window.bDispStatusBar = !editWnd.statusBar.GetStatusHwnd();	// �X�e�[�^�X�o�[�\��
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

/*! �~�j�}�b�v�̕\��/��\�� */
void ViewCommander::Command_ShowMiniMap(void)
{
	auto& editWnd = GetEditWindow();

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
void ViewCommander::Command_Type_List(void)
{
	DlgTypeList dlgTypeList;
	DlgTypeList::Result result;
	result.documentType = GetDocument().docType.GetDocumentType();
	result.bTempChange = true;
	if (dlgTypeList.DoModal(G_AppInstance(), view.GetHwnd(), &result)) {
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
void ViewCommander::Command_ChangeType(int nTypePlusOne)
{
	TypeConfigNum type = TypeConfigNum(nTypePlusOne - 1);
	auto& doc = GetDocument();
	if (nTypePlusOne == 0) {
		type = doc.docType.GetDocumentType();
	}
	if (type.IsValidType() && type.GetIndex() < GetDllShareData().nTypesCount) {
		const TypeConfigMini* pConfig;
		DocTypeManager().GetTypeConfigMini(type, &pConfig);
		doc.docType.SetDocumentTypeIdx(pConfig->id, true);
		doc.docType.LockDocumentType();
		doc.OnChangeType();
	}
}


// �^�C�v�ʐݒ�
void ViewCommander::Command_Option_Type(void)
{
	EditApp::getInstance().OpenPropertySheetTypes(-1, GetDocument().docType.GetDocumentType());
}


// ���ʐݒ�
void ViewCommander::Command_Option(void)
{
	// �ݒ�v���p�e�B�V�[�g �e�X�g�p
	EditApp::getInstance().OpenPropertySheet(-1);
}


// �t�H���g�ݒ�
void ViewCommander::Command_Font(void)
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
	if (MySelectFont(&lf, &nPointSize, EditWnd::getInstance().splitterWnd.GetHwnd(), bFixedFont)) {
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
*/
void ViewCommander::Command_SetFontSize(int fontSize, int shift, int mode)
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
		for (size_t i=0; i<_countof(sizeTable); ++i) {
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
		TypeConfigNum nDocType = doc.docType.GetDocumentType();
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
		doc.blfCurTemp = true;
		doc.lfCur = lf;
		doc.lfCur.lfHeight = lfHeight;
		doc.nPointSizeCur = nPointSize;
		doc.nPointSizeOrg = GetEditWindow().GetFontPointSize(false);
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

	@note �ύX���鏇����ύX�����Ƃ���EditWnd::InitMenu()���ύX���邱��
	@sa EditWnd::InitMenu()
*/
void ViewCommander::Command_WrapWindowWidth(void)
{
	EditView::TOGGLE_WRAP_ACTION nWrapMode;
	int newKetas;
	
	nWrapMode = view.GetWrapMode(&newKetas);
	auto& doc = GetDocument();
	doc.nTextWrapMethodCur = TextWrappingMethod::SettingWidth;
	doc.bTextWrapMethodCurTemp = (doc.nTextWrapMethodCur != view.pTypeData->nTextWrapMethod);
	if (nWrapMode == EditView::TGWRAP_NONE) {
		return;	// �܂�Ԃ����͌��̂܂�
	}

	GetEditWindow().ChangeLayoutParam(true, doc.layoutMgr.GetTabSpace(), newKetas);
	
	// �t�H�[�J�X�ړ����̍ĕ`��
	view.RedrawAll();
	return;
}


/*!	�����̊Ǘ�(�_�C�A���O) */
void ViewCommander::Command_Favorite(void)
{
	DlgFavorite	dlgFavorite;

	// �_�C�A���O��\������
	if (!dlgFavorite.DoModal(G_AppInstance(), view.GetHwnd(), (LPARAM)&GetDocument())) {
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
*/
void ViewCommander::Command_TextWrapMethod(TextWrappingMethod nWrapMethod)
{
	auto& doc = GetDocument();

	// ���݂̐ݒ�l�Ɠ����Ȃ牽�����Ȃ�
	if (doc.nTextWrapMethodCur == nWrapMethod)
		return;

	size_t nWidth;

	switch (nWrapMethod) {
	case TextWrappingMethod::NoWrapping:		// �܂�Ԃ��Ȃ�
		nWidth = MAXLINEKETAS;	// �A�v���P�[�V�����̍ő啝�Ő܂�Ԃ�
		break;

	case TextWrappingMethod::SettingWidth:	// �w�茅�Ő܂�Ԃ�
		nWidth = doc.docType.GetDocumentAttribute().nMaxLineKetas;
		break;

	case TextWrappingMethod::WindowWidth:		// �E�[�Ő܂�Ԃ�
		// �E�B���h�E�����E�ɕ�������Ă���ꍇ�͍����̃E�B���h�E�����g�p����
		nWidth = view.ViewColNumToWrapColNum(GetEditWindow().GetView(0).GetTextArea().nViewColNum);
		break;

	default:
		return;	// �s���Ȓl�̎��͉������Ȃ�
	}

	doc.nTextWrapMethodCur = nWrapMethod;	// �ݒ���L��

	// �܂�Ԃ����@�̈ꎞ�ݒ�K�p�^�ꎞ�ݒ�K�p����
	doc.bTextWrapMethodCurTemp = (doc.docType.GetDocumentAttribute().nTextWrapMethod != nWrapMethod);

	// �܂�Ԃ��ʒu��ύX
	GetEditWindow().ChangeLayoutParam(false, doc.layoutMgr.GetTabSpace(), nWidth);

	// �u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if (doc.nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		doc.layoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
		GetEditWindow().RedrawAllViews(nullptr);	// Scroll Bar�̍X�V���K�v�Ȃ̂ōĕ\�������s����
	}else {
		doc.layoutMgr.ClearLayoutLineWidth();		// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
}


/*!
	@brief �����J�E���g���@��ύX����
	
	@param[in] nMode �����J�E���g���@
		SelectCountMode::Toggle : �����J�E���g���@���g�O��
		SelectCountMode::ByChar ; �������ŃJ�E���g
		SelectCountMode::ByByte ; �o�C�g���ŃJ�E���g
*/
void ViewCommander::Command_Select_Count_Mode(int nMode)
{
	// �ݒ�ɂ͕ۑ������AView���Ɏ��t���O��ݒ�
	//BOOL* pbDispSelCountByByte = &GetDllShareData().common.statusBar.bDispSelCountByByte;
	auto& selectCountMode = GetEditWindow().nSelectCountMode;

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


/*!	@brief ���p���̐ݒ� */
void ViewCommander::Command_Set_QuoteString(const wchar_t* quotestr)
{
	if (!quotestr) {
		return;
	}

	auto& csFormat = GetDllShareData().common.format;
	wcsncpy(csFormat.szInyouKigou, quotestr, _countof(csFormat.szInyouKigou));
	csFormat.szInyouKigou[_countof(csFormat.szInyouKigou) - 1] = L'\0';
}

