#include "StdAfx.h"
#include "DocFileOperation.h"
#include "DocVisitor.h"
#include "EditDoc.h"

#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "dlg/DlgOpenFile.h"
#include "_main/AppMode.h" 
#include "_main/ControlTray.h"
#include "EditApp.h"
#include "window/EditWnd.h"
#include "uiparts/WaitCursor.h"
#include "util/window.h"
#include "env/DllSharedData.h"
#include "env/SakuraEnvironment.h"
#include "plugin/Plugin.h"
#include "plugin/JackManager.h"
#include "timer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���b�N                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool DocFileOperation::_ToDoLock() const
{
	// �t�@�C�����J���Ă��Ȃ�
	if (!doc.docFile.GetFilePathClass().IsValidPath()) {
		return false;
	}

	// �r���[���[�h
	if (AppMode::getInstance().IsViewMode()) {
		return false;
	}

	// �r���ݒ�
	if (GetDllShareData().common.file.nFileShareMode == FileShareMode::NonExclusive) {
		return false;
	}
	return true;
}

void DocFileOperation::DoFileLock(bool bMsg)
{
	if (this->_ToDoLock()) {
		doc.docFile.FileLock(GetDllShareData().common.file.nFileShareMode, bMsg);
	}
}

void DocFileOperation::DoFileUnlock()
{
	doc.docFile.FileUnlock();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ���[�hUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�u�t�@�C�����J���v�_�C�A���O
bool DocFileOperation::OpenFileDialog(
	HWND			hwndParent,		// [in]
	const TCHAR*	pszOpenFolder,	// [in]     NULL�ȊO���w�肷��Ə����t�H���_���w��ł���
	LoadInfo*		pLoadInfo,		// [in/out] ���[�h���
	std::vector<std::tstring>&	files
	)
{
	// �A�N�e�B�u�ɂ���
	ActivateFrameWindow(hwndParent);

	// �t�@�C���I�[�v���_�C�A���O��\��
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		hwndParent,
		_T("*.*"),
		pszOpenFolder ? pszOpenFolder : SakuraEnvironment::GetDlgInitialDir().c_str(),	// �����t�H���_
		MruFile().GetPathList(),														// MRU���X�g�̃t�@�C���̃��X�g
		MruFolder().GetPathList()														// OPENFOLDER���X�g�̃t�@�C���̃��X�g
	);
	return dlgOpenFile.DoModalOpenDlg( pLoadInfo, &files );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ���[�h�t���[                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool DocFileOperation::DoLoadFlow(LoadInfo* pLoadInfo)
{
	LoadResultType eLoadResult = LoadResultType::Failure;

	try {
		// ���[�h�O�`�F�b�N
		if (doc.NotifyCheckLoad(pLoadInfo) == CallbackResultType::Interrupt) {
			throw FlowInterruption();
		}

		// ���[�h����
		doc.NotifyBeforeLoad(pLoadInfo);				// �O����
		eLoadResult = doc.NotifyLoad(*pLoadInfo);	// �{����
		doc.NotifyAfterLoad(*pLoadInfo);				// �㏈��
	}catch (FlowInterruption) {
		eLoadResult = LoadResultType::Interrupt;
	}catch (...) {
		// �\�����ʗ�O�����������ꍇ�� NotifyFinalLoad �͕K���ĂԁI
		doc.NotifyFinalLoad(LoadResultType::Failure);
		throw;
	}
	
	// �ŏI����
	doc.NotifyFinalLoad(eLoadResult);

	return eLoadResult == LoadResultType::OK;
}

// �t�@�C�����J��
bool DocFileOperation::FileLoad(
	LoadInfo* pLoadInfo		// [in/out]
	)
{
	Timer t;

	bool bRet = DoLoadFlow(pLoadInfo);
	// �I�[�v���㎩�����s�}�N�������s����
	if (bRet) {
		doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);

		// �v���O�C���FDocumentOpen�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}
	}

	TRACE(L"FileLoad time %f\n", t.ElapsedSecond());

	return bRet;
}

// �t�@�C�����J���i�������s�}�N�������s���Ȃ��j
bool DocFileOperation::FileLoadWithoutAutoMacro(
	LoadInfo* pLoadInfo		// [in/out]
	)
{
	return DoLoadFlow(pLoadInfo);
}

// ����t�@�C���̍ăI�[�v��
void DocFileOperation::ReloadCurrentFile(
	EncodingType nCharCode		// [in] �����R�[�h���
	)
{
	auto& activeView = doc.pEditWnd->GetActiveView();

	// �v���O�C���FDocumentClose�C�x���g���s
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(activeView, params);
	}

	auto& caret = activeView.GetCaret();
	if (!fexist(doc.docFile.GetFilePath())) {
		// �t�@�C�������݂��Ȃ�
		doc.docFile.SetCodeSet(nCharCode,  CodeTypeName(nCharCode).IsBomDefOn());
		// �J�[�\���ʒu�\�����X�V����
		caret.ShowCaretPosInfo();
		return;
	}

	auto& textArea = activeView.GetTextArea();
	// �J�[�\���ʒu�ۑ�
	size_t nViewTopLine = textArea.GetViewTopLine();	// �\����̈�ԏ�̍s(0�J�n)
	size_t nViewLeftCol = textArea.GetViewLeftCol();	// �\����̈�ԍ��̌�(0�J�n)
	Point	ptCaretPosXY = caret.GetCaretLayoutPos();

	// ���[�h
	LoadInfo loadInfo;
	loadInfo.filePath = doc.docFile.GetFilePath();
	loadInfo.eCharCode = nCharCode;
	loadInfo.bViewMode = AppMode::getInstance().IsViewMode();
	loadInfo.bWritableNoMsg = !doc.IsEditable(); // ���łɕҏW�ł��Ȃ���ԂȂ�t�@�C�����b�N�̃��b�Z�[�W��\�����Ȃ�
	loadInfo.bRequestReload = true;
	bool bRet = this->DoLoadFlow(&loadInfo);

	// �J�[�\���ʒu���� (�������ł̓I�v�V�����̃J�[�\���ʒu�����i�����s�P�ʁj���w�肳��Ă��Ȃ��ꍇ�ł���������)
	if (ptCaretPosXY.y < (int)doc.layoutMgr.GetLineCount()) {
		textArea.SetViewTopLine(nViewTopLine);
		textArea.SetViewLeftCol(nViewLeftCol);
	}
	caret.MoveCursorProperly(ptCaretPosXY, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

	// �I�[�v���㎩�����s�}�N�������s����
	if (bRet) {
		doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);
		// �v���O�C���FDocumentOpen�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �Z�[�uUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �u�t�@�C������t���ĕۑ��v�_�C�A���O */
bool DocFileOperation::SaveFileDialog(
	SaveInfo*	pSaveInfo	// [out]
	)
{
	// �g���q�w��
	// �ꎞ�K�p��g���q�Ȃ��̏ꍇ�̊g���q���^�C�v�ʐݒ肩�玝���Ă���
	TCHAR szDefaultWildCard[_MAX_PATH + 10];	// ���[�U�[�w��g���q
	{
		LPCTSTR	szExt;
		const TypeConfig& type = doc.docType.GetDocumentAttribute();
		// �t�@�C���p�X�������ꍇ�� *.txt �Ƃ���
		if (!this->doc.docFile.GetFilePathClass().IsValidPath()) {
			szExt = _T("");
		}else {
			szExt = this->doc.docFile.GetFilePathClass().GetExt();
		}
		if (type.nIdx == 0) {
			// ��{
			if (szExt[0] == _T('\0')) { 
				// �t�@�C���p�X�������܂��͊g���q�Ȃ�
				_tcscpy(szDefaultWildCard, _T("*.txt"));
			}else {
				// �g���q����
				_tcscpy(szDefaultWildCard, _T("*"));
				_tcscat(szDefaultWildCard, szExt);
			}
		}else {
			szDefaultWildCard[0] = _T('\0'); 
			DocTypeManager::ConvertTypesExtToDlgExt(type.szTypeExts, szExt, szDefaultWildCard);
		}

		if (!this->doc.docFile.GetFilePathClass().IsValidPath()) {
			//�u�V�K����ۑ����͑S�t�@�C���\���v�I�v�V����
			if (GetDllShareData().common.file.bNoFilterSaveNew)
				_tcscat(szDefaultWildCard, _T(";*.*"));	// �S�t�@�C���\��
		}else {
			//�u�V�K�ȊO����ۑ����͑S�t�@�C���\���v�I�v�V����
			if (GetDllShareData().common.file.bNoFilterSaveFile)
				_tcscat(szDefaultWildCard, _T(";*.*"));	// �S�t�@�C���\��
		}
	}
	// ����ɁA����ԍ���t����
	if (pSaveInfo->filePath[0] == _T('\0')) {
		const EditNode* node = AppNodeManager::getInstance().GetEditNode(doc.pEditWnd->GetHwnd());
		if (0 < node->nId) {
			TCHAR szText[16];
			auto_sprintf(szText, _T("%d"), node->nId);
			auto_strcpy(pSaveInfo->filePath, LS(STR_NO_TITLE2));	// ����
			auto_strcat(pSaveInfo->filePath, szText);
		}
	}

	// �_�C�A���O��\��
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		EditWnd::getInstance().GetHwnd(),
		szDefaultWildCard,
		SakuraEnvironment::GetDlgInitialDir().c_str(),	// �����t�H���_
		MruFile().GetPathList(),	// �ŋ߂̃t�@�C��
		MruFolder().GetPathList()	// �ŋ߂̃t�H���_
	);
	return dlgOpenFile.DoModalSaveDlg(pSaveInfo, pSaveInfo->eCharCode == CODE_CODEMAX);
}

//�u�t�@�C������t���ĕۑ��v�_�C�A���O
bool DocFileOperation::SaveFileDialog(LPTSTR szPath)
{
	SaveInfo saveInfo;
	saveInfo.filePath = szPath;
	saveInfo.eCharCode = CODE_CODEMAX; //###�g���b�L�[
	bool bRet = SaveFileDialog(&saveInfo);
	_tcscpy_s(szPath, _MAX_PATH, saveInfo.filePath);
	return bRet;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �Z�[�u�t���[                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool DocFileOperation::DoSaveFlow(SaveInfo* pSaveInfo)
{
	SaveResultType eSaveResult = SaveResultType::Failure;
	try {
		// �I�v�V�����F���ύX�ł��㏑�����邩
		// ### ���ύX�Ȃ�㏑�����Ȃ��Ŕ����鏈���͂ǂ� CDocListener �� OnCheckSave() �����O��
		// ### �i�ۑ����邩�ǂ����₢���킹���肷������O�Ɂj���؂����ƂȂ̂ŁA
		// ### �X�}�[�g����Ȃ��H��������Ȃ����ǁA�Ƃ肠���������ɔz�u���Ă���
		if (!GetDllShareData().common.file.bEnableUnmodifiedOverwrite) {
			// �㏑���̏ꍇ
			if (pSaveInfo->bOverwriteMode) {
				// ���ύX�̏ꍇ�͌x�������o���A�I��
				if (!doc.docEditor.IsModified() &&
					pSaveInfo->eol == EolType::None &&	// �����s�R�[�h�w��ۑ������N�G�X�g���ꂽ�ꍇ�́A�u�ύX�����������́v�Ƃ݂Ȃ�
					!pSaveInfo->bChgCodeSet
				) {		// �����R�[�h�Z�b�g�̕ύX���L�����ꍇ�́A�u�ύX�����������́v�Ƃ݂Ȃ�
					EditApp::getInstance().soundSet.NeedlessToSaveBeep();
					throw FlowInterruption();
				}
			}
		}

		// �Z�[�u�O�`�F�b�N
		if (doc.NotifyCheckSave(pSaveInfo) == CallbackResultType::Interrupt) {
			throw FlowInterruption();
		}

		// �Z�[�u�O���܂�����
		if (doc.NotifyPreBeforeSave(pSaveInfo) == CallbackResultType::Interrupt) {
			throw FlowInterruption();
		}

		// �ۑ��O�������s�}�N�������s����
		doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnSave, pSaveInfo->filePath);

		// �v���O�C���FDocumentBeforeSave�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_BEFORE_SAVE, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}

		if (!pSaveInfo->bOverwriteMode) {	// �㏑���łȂ���ΑO�����̃N���[�Y�C�x���g���Ă�
			// �v���O�C���FDocumentClose�C�x���g���s
			plugs.clear();
			JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
			}
		}

		// �Z�[�u����
		doc.NotifyBeforeSave(*pSaveInfo);	// �O����
		doc.NotifySave(*pSaveInfo);			// �{����
		doc.NotifyAfterSave(*pSaveInfo);	// �㏈��

		// �v���O�C���FDocumentAfterSave�C�x���g���s
		plugs.clear();
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_AFTER_SAVE, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}

		// ����
		eSaveResult = SaveResultType::OK; //###��
	}catch (FlowInterruption) {
		eSaveResult = SaveResultType::Interrupt;
	}catch (...) {
		// �\�����ʗ�O�����������ꍇ�� NotifyFinalSave �͕K���ĂԁI
		doc.NotifyFinalSave(SaveResultType::Failure);
		throw;
	}

	// �ŏI����
	doc.NotifyFinalSave(eSaveResult);

	return eSaveResult == SaveResultType::OK;
}


/*! �㏑���ۑ�

	@return �ۑ����s��ꂽor�ۑ��s�v�̂��߉����s��Ȃ������Ƃ���true��Ԃ�
*/
bool DocFileOperation::FileSave()
{
	// �t�@�C�������w�肳��Ă��Ȃ��ꍇ�́u���O��t���ĕۑ��v�̃t���[�֑J��
	if (!doc.docFile.GetFilePathClass().IsValidPath()) {
		return FileSaveAs();
	}

	// �Z�[�u���
	SaveInfo saveInfo;
	doc.GetSaveInfo(&saveInfo);
	saveInfo.eol = EolType::None;			// ���s�R�[�h���ϊ�
	saveInfo.bOverwriteMode = true;	// �㏑���v��

	// �㏑������
	return doc.docFileOperation.DoSaveFlow(&saveInfo);
}


/*! ���O��t���ĕۑ��t���[ */
bool DocFileOperation::FileSaveAs(
	const wchar_t* filename,
	EncodingType eCodeType,
	EolType eEolType,
	bool bDialog
	)
{
	// �Z�[�u���
	SaveInfo saveInfo;
	doc.GetSaveInfo(&saveInfo);
	saveInfo.eol = EolType::None; // �����l�͕ϊ����Ȃ�
	if (filename) {
		// �_�C�A���O�Ȃ��ۑ��A�܂��̓}�N���̈�������
		saveInfo.filePath = to_tchar(filename);
		if (EolType::None <= eEolType && eEolType < EolType::CodeMax) {
			saveInfo.eol = eEolType;
		}
		if (IsValidCodeType(eCodeType) && eCodeType != saveInfo.eCharCode) {
			saveInfo.eCharCode = eCodeType;
			saveInfo.bBomExist = CodeTypeName(eCodeType).IsBomDefOn();
		}
	}
	if (bDialog) {
		if (!filename && AppMode::getInstance().IsViewMode()) {
			saveInfo.filePath = _T(""); //���ǂݍ��ݐ�p���[�h�̂Ƃ��̓t�@�C�������w�肵�Ȃ�
		}

		// �_�C�A���O�\��
		if (!SaveFileDialog(&saveInfo)) {
			return false;
		}
	}

	// �Z�[�u����
	if (DoSaveFlow(&saveInfo)) {
		// �I�[�v���㎩�����s�}�N�������s����iANSI�łł͂����ōă��[�h���s���������s�}�N�������s�����j
		// ��Ď��� Patches#1550557 �ɁA�u���O��t���ĕۑ��v�ŃI�[�v���㎩�����s�}�N�������s����邱�Ƃ̐���ɂ��ċc�_�̌o�܂���
		//   ���h�t�@�C�����ɉ����ĕ\����ω�������}�N���Ƃ���z�肷��ƁA����͂���ł����悤�Ɏv���܂��B�h
		doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);

		// �v���O�C���FDocumentOpen�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
		}

		return true;
	}

	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �N���[�Y                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	����(����)�B
	���[�U�L�����Z�����쓙�ɂ��N���[�Y����Ȃ������ꍇ�� false ��Ԃ��B
*/
bool DocFileOperation::FileClose()
{
	// �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s
	if (!doc.OnFileClose(false)) {
		return false;
	}

	// �v���O�C���FDocumentClose�C�x���g���s
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
	}

	// �����f�[�^�̃N���A
	doc.InitDoc();

	// �S�r���[�̏�����
	doc.InitAllView();

	doc.SetCurDirNotitle();

	// ����ԍ��擾
	AppNodeManager::getInstance().GetNoNameNumber(doc.pEditWnd->GetHwnd());

	// �e�E�B���h�E�̃^�C�g�����X�V
	doc.pEditWnd->UpdateCaption();

	// �I�[�v���㎩�����s�}�N�������s����
	doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ���ĊJ�� */
void DocFileOperation::FileCloseOpen(const LoadInfo& argLoadInfo)
{
	// �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s
	if (!doc.OnFileClose(false)) {
		return;
	}

	// �v���O�C���FDocumentClose�C�x���g���s
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
	}

	// �t�@�C�����w�肪�����ꍇ�̓_�C�A���O�œ��͂�����
	LoadInfo loadInfo = argLoadInfo;
	if (loadInfo.filePath.Length() == 0) {
		std::vector<std::tstring> files;
		if (!OpenFileDialog(EditWnd::getInstance().GetHwnd(), NULL, &loadInfo, files)) {
			return;
		}
		loadInfo.filePath = files[0].c_str();
		// ���̃t�@�C���͐V�K�E�B���h�E
		size_t nSize = files.size();
		for (size_t i=1; i<nSize; ++i) {
			LoadInfo filesLoadInfo = loadInfo;
			filesLoadInfo.filePath = files[i].c_str();
			ControlTray::OpenNewEditor(
				G_AppInstance(),
				EditWnd::getInstance().GetHwnd(),
				filesLoadInfo,
				NULL,
				true
			);
		}
	}

	// �����f�[�^�̃N���A
	doc.InitDoc();

	// �S�r���[�̏�����
	doc.InitAllView();

	// �J��
	FileLoadWithoutAutoMacro(&loadInfo);

	if (!doc.docFile.GetFilePathClass().IsValidPath()) {
		doc.SetCurDirNotitle();
		AppNodeManager::getInstance().GetNoNameNumber(doc.pEditWnd->GetHwnd());
	}

	// �e�E�B���h�E�̃^�C�g�����X�V
	doc.pEditWnd->UpdateCaption();

	// �I�[�v���㎩�����s�}�N�������s����
	// �����[�h���ĂȂ��Ă�(����)�ɂ͕ύX�ς�
	doc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);

	// �v���O�C���FDocumentOpen�C�x���g���s
	plugs.clear();
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(doc.pEditWnd->GetActiveView(), params);
	}
}


