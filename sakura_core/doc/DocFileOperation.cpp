/*
	Copyright (C) 2008, kobake

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
#include "env/DLLSHAREDATA.h"
#include "env/SakuraEnvironment.h"
#include "plugin/Plugin.h"
#include "plugin/JackManager.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���b�N                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool DocFileOperation::_ToDoLock() const
{
	// �t�@�C�����J���Ă��Ȃ�
	if (!m_pDocRef->m_docFile.GetFilePathClass().IsValidPath()) {
		return false;
	}

	// �r���[���[�h
	if (AppMode::getInstance()->IsViewMode()) {
		return false;
	}

	// �r���ݒ�
	if (GetDllShareData().m_common.m_file.m_nFileShareMode == SHAREMODE_NOT_EXCLUSIVE) {
		return false;
	}
	return true;
}

void DocFileOperation::DoFileLock(bool bMsg)
{
	if (this->_ToDoLock()) {
		m_pDocRef->m_docFile.FileLock(GetDllShareData().m_common.m_file.m_nFileShareMode, bMsg);
	}
}

void DocFileOperation::DoFileUnlock()
{
	m_pDocRef->m_docFile.FileUnlock();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ���[�hUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�u�t�@�C�����J���v�_�C�A���O
// Mar. 30, 2003 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
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
		MRUFile().GetPathList(),														// MRU���X�g�̃t�@�C���̃��X�g
		MRUFolder().GetPathList()														// OPENFOLDER���X�g�̃t�@�C���̃��X�g
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
		if (m_pDocRef->NotifyCheckLoad(pLoadInfo) == CallbackResultType::Interrupt) {
			throw FlowInterruption();
		}

		// ���[�h����
		m_pDocRef->NotifyBeforeLoad(pLoadInfo);			// �O����
		eLoadResult = m_pDocRef->NotifyLoad(*pLoadInfo);	// �{����
		m_pDocRef->NotifyAfterLoad(*pLoadInfo);			// �㏈��
	}catch (FlowInterruption) {
		eLoadResult = LoadResultType::Interrupt;
	}catch (...) {
		// �\�����ʗ�O�����������ꍇ�� NotifyFinalLoad �͕K���ĂԁI
		m_pDocRef->NotifyFinalLoad(LoadResultType::Failure);
		throw;
	}
	
	// �ŏI����
	m_pDocRef->NotifyFinalLoad(eLoadResult);

	return eLoadResult == LoadResultType::OK;
}

// �t�@�C�����J��
bool DocFileOperation::FileLoad(
	LoadInfo* pLoadInfo		// [in/out]
	)
{
	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);

	bool bRet = DoLoadFlow(pLoadInfo);
	// 2006.09.01 ryoji �I�[�v���㎩�����s�}�N�������s����
	if (bRet) {
		m_pDocRef->RunAutoMacro(GetDllShareData().m_common.m_macro.m_nMacroOnOpened);

		// �v���O�C���FDocumentOpen�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(&m_pDocRef->m_pEditWnd->GetActiveView(), params);
		}
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	LONGLONG diff = now.QuadPart - start.QuadPart;
	TCHAR buff[32];
	swprintf(buff, L"load time %f\n", diff / (double)freq.QuadPart);
	OutputDebugString(buff);

	return bRet;
}

// �t�@�C�����J���i�������s�}�N�������s���Ȃ��j
// 2009.08.11 ryoji FileLoad�ւ̃p�����[�^�ǉ��ɂ��Ă�������ANSI�łƐ������Ƃ�₷���̂œ��ʂ͕ʊ֐��ɂ��Ă���
bool DocFileOperation::FileLoadWithoutAutoMacro(
	LoadInfo* pLoadInfo		// [in/out]
	)
{
	return DoLoadFlow(pLoadInfo);
}

// ����t�@�C���̍ăI�[�v��
void DocFileOperation::ReloadCurrentFile(
	ECodeType nCharCode		// [in] �����R�[�h���
	)
{
	auto& activeView = m_pDocRef->m_pEditWnd->GetActiveView();

	// �v���O�C���FDocumentClose�C�x���g���s
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(&activeView, params);
	}

	auto& caret = activeView.GetCaret();
	if (!fexist(m_pDocRef->m_docFile.GetFilePath())) {
		// �t�@�C�������݂��Ȃ�
		// Jul. 26, 2003 ryoji BOM��W���ݒ��	// IsBomDefOn�g�p 2013/5/17	Uchi
		m_pDocRef->m_docFile.SetCodeSet(nCharCode,  CodeTypeName(nCharCode).IsBomDefOn());
		// �J�[�\���ʒu�\�����X�V����	// 2008.07.22 ryoji
		caret.ShowCaretPosInfo();
		return;
	}

	auto& textArea = activeView.GetTextArea();
	// �J�[�\���ʒu�ۑ�
	LayoutInt		nViewTopLine = textArea.GetViewTopLine();	// �\����̈�ԏ�̍s(0�J�n)
	LayoutInt		nViewLeftCol = textArea.GetViewLeftCol();	// �\����̈�ԍ��̌�(0�J�n)
	LayoutPoint	ptCaretPosXY = caret.GetCaretLayoutPos();

	// ���[�h
	LoadInfo loadInfo;
	loadInfo.filePath = m_pDocRef->m_docFile.GetFilePath();
	loadInfo.eCharCode = nCharCode;
	loadInfo.bViewMode = AppMode::getInstance()->IsViewMode(); // 2014.06.13 IsEditable->IsViewMode�ɖ߂��B������ bForceNoMsg��ǉ�
	loadInfo.bWritableNoMsg = !m_pDocRef->IsEditable(); // ���łɕҏW�ł��Ȃ���ԂȂ�t�@�C�����b�N�̃��b�Z�[�W��\�����Ȃ�
	loadInfo.bRequestReload = true;
	bool bRet = this->DoLoadFlow(&loadInfo);

	// �J�[�\���ʒu���� (�������ł̓I�v�V�����̃J�[�\���ʒu�����i�����s�P�ʁj���w�肳��Ă��Ȃ��ꍇ�ł���������)
	// 2007.08.23 ryoji �\���̈敜��
	if (ptCaretPosXY.GetY2() < m_pDocRef->m_layoutMgr.GetLineCount()) {
		textArea.SetViewTopLine(nViewTopLine);
		textArea.SetViewLeftCol(nViewLeftCol);
	}
	caret.MoveCursorProperly(ptCaretPosXY, true);	// 2007.08.23 ryoji MoveCursor()->MoveCursorProperly()
	caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

	// 2006.09.01 ryoji �I�[�v���㎩�����s�}�N�������s����
	if (bRet) {
		m_pDocRef->RunAutoMacro(GetDllShareData().m_common.m_macro.m_nMacroOnOpened);
		// �v���O�C���FDocumentOpen�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(&m_pDocRef->m_pEditWnd->GetActiveView(), params);
		}
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �Z�[�uUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �u�t�@�C������t���ĕۑ��v�_�C�A���O
	@date 2001.02.09 genta	���s�R�[�h�����������ǉ�
	@date 2003.03.30 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
	@date 2003.07.20 ryoji	BOM�̗L�������������ǉ�
	@date 2006.11.10 ryoji	���[�U�[�w��̊g���q���󋵈ˑ��ŕω�������
*/
bool DocFileOperation::SaveFileDialog(
	SaveInfo*	pSaveInfo	// [out]
	)
{
	// �g���q�w��
	// �ꎞ�K�p��g���q�Ȃ��̏ꍇ�̊g���q���^�C�v�ʐݒ肩�玝���Ă���
	// 2008/6/14 �傫������ Uchi
	TCHAR szDefaultWildCard[_MAX_PATH + 10];	// ���[�U�[�w��g���q
	{
		LPCTSTR	szExt;
		const TypeConfig& type = m_pDocRef->m_docType.GetDocumentAttribute();
		// �t�@�C���p�X�������ꍇ�� *.txt �Ƃ���
		if (!this->m_pDocRef->m_docFile.GetFilePathClass().IsValidPath()) {
			szExt = _T("");
		}else {
			szExt = this->m_pDocRef->m_docFile.GetFilePathClass().GetExt();
		}
		if (type.m_nIdx == 0) {
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
			DocTypeManager::ConvertTypesExtToDlgExt(type.m_szTypeExts, szExt, szDefaultWildCard);
		}

		if (!this->m_pDocRef->m_docFile.GetFilePathClass().IsValidPath()) {
			//�u�V�K����ۑ����͑S�t�@�C���\���v�I�v�V����	// 2008/6/15 �o�O�t�B�b�N�X Uchi
			if (GetDllShareData().m_common.m_file.m_bNoFilterSaveNew)
				_tcscat(szDefaultWildCard, _T(";*.*"));	// �S�t�@�C���\��
		}else {
			//�u�V�K�ȊO����ۑ����͑S�t�@�C���\���v�I�v�V����
			if (GetDllShareData().m_common.m_file.m_bNoFilterSaveFile)
				_tcscat(szDefaultWildCard, _T(";*.*"));	// �S�t�@�C���\��
		}
	}
	// ����ɁA����ԍ���t����
	if (pSaveInfo->filePath[0] == _T('\0')) {
		const EditNode* node = AppNodeManager::getInstance()->GetEditNode(m_pDocRef->m_pEditWnd->GetHwnd());
		if (0 < node->m_nId) {
			TCHAR szText[16];
			auto_sprintf(szText, _T("%d"), node->m_nId);
			auto_strcpy(pSaveInfo->filePath, LS(STR_NO_TITLE2));	// ����
			auto_strcat(pSaveInfo->filePath, szText);
		}
	}

	// �_�C�A���O��\��
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		G_AppInstance(),
		EditWnd::getInstance()->GetHwnd(),
		szDefaultWildCard,
		SakuraEnvironment::GetDlgInitialDir().c_str(),	// �����t�H���_
		MRUFile().GetPathList(),	// �ŋ߂̃t�@�C��
		MRUFolder().GetPathList()	// �ŋ߂̃t�H���_
	);
	return dlgOpenFile.DoModalSaveDlg( pSaveInfo, pSaveInfo->eCharCode == CODE_CODEMAX );
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
		// 2009.04.12 ryoji CSaveAgent::OnCheckSave()����ړ�
		// ### ���ύX�Ȃ�㏑�����Ȃ��Ŕ����鏈���͂ǂ� CDocListener �� OnCheckSave() �����O��
		// ### �i�ۑ����邩�ǂ����₢���킹���肷������O�Ɂj���؂����ƂȂ̂ŁA
		// ### �X�}�[�g����Ȃ��H��������Ȃ����ǁA�Ƃ肠���������ɔz�u���Ă���
		if (!GetDllShareData().m_common.m_file.m_bEnableUnmodifiedOverwrite) {
			// �㏑���̏ꍇ
			if (pSaveInfo->bOverwriteMode) {
				// ���ύX�̏ꍇ�͌x�������o���A�I��
				if (!m_pDocRef->m_docEditor.IsModified() &&
					pSaveInfo->eol == EolType::None &&	// �����s�R�[�h�w��ۑ������N�G�X�g���ꂽ�ꍇ�́A�u�ύX�����������́v�Ƃ݂Ȃ�
					!pSaveInfo->bChgCodeSet
				) {		// �����R�[�h�Z�b�g�̕ύX���L�����ꍇ�́A�u�ύX�����������́v�Ƃ݂Ȃ�
					EditApp::getInstance()->m_soundSet.NeedlessToSaveBeep();
					throw FlowInterruption();
				}
			}
		}

		// �Z�[�u�O�`�F�b�N
		if (m_pDocRef->NotifyCheckSave(pSaveInfo) == CallbackResultType::Interrupt) {
			throw FlowInterruption();
		}

		// �Z�[�u�O���܂�����
		if (m_pDocRef->NotifyPreBeforeSave(pSaveInfo) == CallbackResultType::Interrupt) {
			throw FlowInterruption();
		}

		// 2006.09.01 ryoji �ۑ��O�������s�}�N�������s����
		m_pDocRef->RunAutoMacro(GetDllShareData().m_common.m_macro.m_nMacroOnSave, pSaveInfo->filePath);

		// �v���O�C���FDocumentBeforeSave�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_BEFORE_SAVE, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(&m_pDocRef->m_pEditWnd->GetActiveView(), params);
		}

		if (!pSaveInfo->bOverwriteMode) {	// �㏑���łȂ���ΑO�����̃N���[�Y�C�x���g���Ă�
			// �v���O�C���FDocumentClose�C�x���g���s
			plugs.clear();
			JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(&m_pDocRef->m_pEditWnd->GetActiveView(), params);
			}
		}

		// �Z�[�u����
		m_pDocRef->NotifyBeforeSave(*pSaveInfo);	// �O����
		m_pDocRef->NotifySave(*pSaveInfo);			// �{����
		m_pDocRef->NotifyAfterSave(*pSaveInfo);	// �㏈��

		// �v���O�C���FDocumentAfterSave�C�x���g���s
		plugs.clear();
		JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_AFTER_SAVE, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(&m_pDocRef->m_pEditWnd->GetActiveView(), params);
		}

		// ����
		eSaveResult = SaveResultType::OK; //###��
	}catch (FlowInterruption) {
		eSaveResult = SaveResultType::Interrupt;
	}catch (...) {
		// �\�����ʗ�O�����������ꍇ�� NotifyFinalSave �͕K���ĂԁI
		m_pDocRef->NotifyFinalSave(SaveResultType::Failure);
		throw;
	}

	// �ŏI����
	m_pDocRef->NotifyFinalSave(eSaveResult);

	return eSaveResult == SaveResultType::OK;
}


/*! �㏑���ۑ�

	@return �ۑ����s��ꂽor�ۑ��s�v�̂��߉����s��Ȃ������Ƃ���true��Ԃ�

	@date 2004.06.05 genta  �r���[���[�h�̃`�F�b�N��CEditDoc����㏑���ۑ������Ɉړ�
	@date 2006.12.30 ryoji  CEditView::Command_FILESAVE()���珈���{�̂�؂�o��
	@date 2008.03.20 kobake �߂�l�̎d�l���`
*/
bool DocFileOperation::FileSave()
{
	// �t�@�C�������w�肳��Ă��Ȃ��ꍇ�́u���O��t���ĕۑ��v�̃t���[�֑J��
	if (!m_pDocRef->m_docFile.GetFilePathClass().IsValidPath()) {
		return FileSaveAs();
	}

	// �Z�[�u���
	SaveInfo saveInfo;
	m_pDocRef->GetSaveInfo(&saveInfo);
	saveInfo.eol = EolType::None;			// ���s�R�[�h���ϊ�
	saveInfo.bOverwriteMode = true;	// �㏑���v��

	// �㏑������
	return m_pDocRef->m_docFileOperation.DoSaveFlow(&saveInfo);
}


/*! ���O��t���ĕۑ��t���[

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS_DIALOG()���珈���{�̂�؂�o��
*/
bool DocFileOperation::FileSaveAs(
	const WCHAR* filename,
	ECodeType eCodeType,
	EolType eEolType,
	bool bDialog
	)
{
	// �Z�[�u���
	SaveInfo saveInfo;
	m_pDocRef->GetSaveInfo(&saveInfo);
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
		if (!filename && AppMode::getInstance()->IsViewMode()) {
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
		m_pDocRef->RunAutoMacro(GetDllShareData().m_common.m_macro.m_nMacroOnOpened);

		// �v���O�C���FDocumentOpen�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(&m_pDocRef->m_pEditWnd->GetActiveView(), params);
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

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()���珈���{�̂�؂�o��
*/
bool DocFileOperation::FileClose()
{
	// �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s
	if (!m_pDocRef->OnFileClose(false)) {
		return false;
	}

	// �v���O�C���FDocumentClose�C�x���g���s
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(&m_pDocRef->m_pEditWnd->GetActiveView(), params);
	}

	// �����f�[�^�̃N���A
	m_pDocRef->InitDoc();

	// �S�r���[�̏�����
	m_pDocRef->InitAllView();

	m_pDocRef->SetCurDirNotitle();

	// ����ԍ��擾
	AppNodeManager::getInstance()->GetNoNameNumber(m_pDocRef->m_pEditWnd->GetHwnd());

	// �e�E�B���h�E�̃^�C�g�����X�V
	m_pDocRef->m_pEditWnd->UpdateCaption();

	// 2006.09.01 ryoji �I�[�v���㎩�����s�}�N�������s����
	m_pDocRef->RunAutoMacro(GetDllShareData().m_common.m_macro.m_nMacroOnOpened);

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ���ĊJ��
	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()���珈���{�̂�؂�o��
*/
void DocFileOperation::FileCloseOpen(const LoadInfo& argLoadInfo)
{
	// �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s
	if (!m_pDocRef->OnFileClose(false)) {
		return;
	}

	// �v���O�C���FDocumentClose�C�x���g���s
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(&m_pDocRef->m_pEditWnd->GetActiveView(), params);
	}

	// �t�@�C�����w�肪�����ꍇ�̓_�C�A���O�œ��͂�����
	LoadInfo loadInfo = argLoadInfo;
	if (loadInfo.filePath.Length() == 0) {
		std::vector<std::tstring> files;
		if (!OpenFileDialog(EditWnd::getInstance()->GetHwnd(), NULL, &loadInfo, files)) {
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
				EditWnd::getInstance()->GetHwnd(),
				filesLoadInfo,
				NULL,
				true
			);
		}
	}

	// �����f�[�^�̃N���A
	m_pDocRef->InitDoc();

	// �S�r���[�̏�����
	m_pDocRef->InitAllView();

	// �J��
	FileLoadWithoutAutoMacro(&loadInfo);

	if (!m_pDocRef->m_docFile.GetFilePathClass().IsValidPath()) {
		m_pDocRef->SetCurDirNotitle();
		AppNodeManager::getInstance()->GetNoNameNumber(m_pDocRef->m_pEditWnd->GetHwnd());
	}

	// �e�E�B���h�E�̃^�C�g�����X�V
	m_pDocRef->m_pEditWnd->UpdateCaption();

	// �I�[�v���㎩�����s�}�N�������s����
	// �����[�h���ĂȂ��Ă�(����)�ɂ͕ύX�ς�
	m_pDocRef->RunAutoMacro(GetDllShareData().m_common.m_macro.m_nMacroOnOpened);

	// �v���O�C���FDocumentOpen�C�x���g���s
	plugs.clear();
	JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(&m_pDocRef->m_pEditWnd->GetActiveView(), params);
	}
}


