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
#include "CLoadAgent.h"
#include "CReadManager.h"
#include "_main/CAppMode.h"
#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "env/CDocTypeManager.h"
#include "env/CShareData.h"
#include "doc/CEditDoc.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "uiparts/CVisualProgress.h"
#include "util/file.h"

CallbackResultType LoadAgent::OnCheckLoad(LoadInfo* pLoadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// �����[�h�v���̏ꍇ�́A�p���B
	if (pLoadInfo->bRequestReload) {
		goto next;
	}

	// �t�H���_���w�肳�ꂽ�ꍇ�́u�t�@�C�����J���v�_�C�A���O��\�����A���ۂ̃t�@�C�����͂𑣂�
	if (IsDirectory(pLoadInfo->filePath)) {
		std::vector<std::tstring> files;
		LoadInfo loadInfo(_T(""), CODE_AUTODETECT, false);
		bool bDlgResult = pDoc->m_docFileOperation.OpenFileDialog(
			EditWnd::getInstance()->GetHwnd(),
			pLoadInfo->filePath,	// �w�肳�ꂽ�t�H���_
			&loadInfo,
			files
		);
		if (!bDlgResult) {
			return CallbackResultType::Interrupt; // �L�����Z�����ꂽ�ꍇ�͒��f
		}
		size_t nSize = files.size();
		if (0 < nSize) {
			loadInfo.filePath = files[0].c_str();
			// ���̃t�@�C���͐V�K�E�B���h�E
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
		*pLoadInfo = loadInfo;
	}

	// ���̃E�B���h�E�Ŋ��ɊJ����Ă���ꍇ�́A������A�N�e�B�u�ɂ���
	HWND hWndOwner;
	if (ShareData::getInstance()->ActiveAlreadyOpenedWindow(pLoadInfo->filePath, &hWndOwner, pLoadInfo->eCharCode)) {
		pLoadInfo->bOpened = true;
		return CallbackResultType::Interrupt;
	}

	// ���݂̃E�B���h�E�ɑ΂��ăt�@�C����ǂݍ��߂Ȃ��ꍇ�́A�V���ȃE�B���h�E���J���A�����Ƀt�@�C����ǂݍ��܂���
	if (!pDoc->IsAcceptLoad()) {
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			EditWnd::getInstance()->GetHwnd(),
			*pLoadInfo
		);
		return CallbackResultType::Interrupt;
	}

next:
	auto& csFile = GetDllShareData().m_common.m_file;
	// �I�v�V�����F�J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������
	if (csFile.GetAlertIfFileNotExist()) {
		if (!fexist(pLoadInfo->filePath)) {
			InfoBeep();
			//	Feb. 15, 2003 genta Popup�E�B���h�E��\�����Ȃ��悤�ɁD
			//	�����ŃX�e�[�^�X���b�Z�[�W���g���Ă���ʂɕ\������Ȃ��D
			TopInfoMessage(
				EditWnd::getInstance()->GetHwnd(),
				LS(STR_NOT_EXSIST_SAVE),	// Mar. 24, 2001 jepro �኱�C��
				pLoadInfo->filePath.GetBufferPointer()
			);
		}
	}

	// �ǂݎ��\�`�F�b�N
	do {
		File file(pLoadInfo->filePath.c_str());

		// �t�@�C�������݂��Ȃ��ꍇ�̓`�F�b�N�ȗ�
		if (!file.IsFileExist()) {
			break;
		}

		// ���b�N�͈ꎞ�I�ɉ������ă`�F�b�N����i�`�F�b�N�����Ɍ�߂�ł��Ȃ��Ƃ���܂Ői�߂�����S�j
		// �� ���b�N���Ă��Ă��A�N�Z�X���̕ύX�ɂ���ēǂݎ��Ȃ��Ȃ��Ă��邱�Ƃ�����
		bool bLock = (pLoadInfo->IsSamePath(pDoc->m_docFile.GetFilePath()) && pDoc->m_docFile.IsFileLocking());
		if (bLock) {
			pDoc->m_docFileOperation.DoFileUnlock();
		}

		// �`�F�b�N
		if (!file.IsFileReadable()) {
			if (bLock) {
				pDoc->m_docFileOperation.DoFileLock(false);
			}
			ErrorMessage(
				EditWnd::getInstance()->GetHwnd(),
				LS(STR_LOADAGENT_ERR_OPEN),
				pLoadInfo->filePath.c_str()
			);
			return CallbackResultType::Interrupt; // �t�@�C�������݂��Ă���̂ɓǂݎ��Ȃ��ꍇ�͒��f
		}
		if (bLock) {
			pDoc->m_docFileOperation.DoFileLock(false);
		}
	}while (false);	//	1�񂵂��ʂ�Ȃ�. break�ł����܂Ŕ��

	// �t�@�C���T�C�Y�`�F�b�N
	if (csFile.m_bAlertIfLargeFile) {
		WIN32_FIND_DATA wfd;
		HANDLE nFind = ::FindFirstFile( pLoadInfo->filePath.c_str(), &wfd );
		if (nFind != INVALID_HANDLE_VALUE) {
			::FindClose( nFind );
			LARGE_INTEGER nFileSize;
			nFileSize.HighPart = wfd.nFileSizeHigh;
			nFileSize.LowPart = wfd.nFileSizeLow;
			// csFile.m_nAlertFileSize ��MB�P��
			if ((nFileSize.QuadPart>>20) >= (csFile.m_nAlertFileSize)) {
				int nRet = MYMESSAGEBOX( EditWnd::getInstance()->GetHwnd(),
					MB_ICONQUESTION | MB_YESNO | MB_TOPMOST,
					GSTR_APPNAME,
					LS(STR_LOADAGENT_BIG_FILE),
					csFile.m_nAlertFileSize );
				if (nRet != IDYES) {
					return CallbackResultType::Interrupt;
				}
			}
		}
	}

	return CallbackResultType::Continue;
}

void LoadAgent::OnBeforeLoad(LoadInfo* pLoadInfo)
{
}

LoadResultType LoadAgent::OnLoad(const LoadInfo& loadInfo)
{
	LoadResultType eRet = LoadResultType::OK;
	EditDoc* pDoc = GetListeningDoc();

	// �����f�[�^�̃N���A
	pDoc->InitDoc(); //$$

	// �p�X���m��
	pDoc->SetFilePathAndIcon( loadInfo.filePath );

	// ������ʊm��
	pDoc->m_docType.SetDocumentType( loadInfo.nType, true );
	pDoc->m_pEditWnd->m_pViewFontMiniMap->UpdateFont(&pDoc->m_pEditWnd->GetLogfont());
	InitCharWidthCache( pDoc->m_pEditWnd->m_pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache( CharWidthFontMode::Edit, pDoc->m_pEditWnd->GetLogfontCacheMode() );
	InitCharWidthCache( pDoc->m_pEditWnd->GetLogfont() );
	pDoc->m_pEditWnd->m_pViewFont->UpdateFont(&pDoc->m_pEditWnd->GetLogfont());

	// �N���Ɠ����ɓǂޏꍇ�͗\�߃A�E�g���C����͉�ʂ�z�u���Ă���
	// �i�t�@�C���ǂݍ��݊J�n�ƂƂ��Ƀr���[���\�������̂ŁA���ƂŔz�u����Ɖ�ʂ̂�������傫���́j
	if (!pDoc->m_pEditWnd->m_dlgFuncList.m_bEditWndReady) {
		pDoc->m_pEditWnd->m_dlgFuncList.Refresh();
		HWND hEditWnd = pDoc->m_pEditWnd->GetHwnd();
		if (!::IsIconic( hEditWnd ) && pDoc->m_pEditWnd->m_dlgFuncList.GetHwnd()) {
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::SendMessage( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
	}

	// �t�@�C�������݂���ꍇ�̓t�@�C����ǂ�
	if (fexist(loadInfo.filePath)) {
		// CDocLineMgr�̍\��
		ReadManager reader;
		ProgressSubject* pOld = EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(&reader);
		CodeConvertResult eReadResult = reader.ReadFile_To_CDocLineMgr(
			&pDoc->m_docLineMgr,
			loadInfo,
			&pDoc->m_docFile.m_fileInfo
		);
		if (eReadResult == CodeConvertResult::LoseSome) {
			eRet = LoadResultType::LoseSome;
		}
		EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(pOld);
	}else {
		// ���݂��Ȃ��Ƃ����h�L�������g�ɕ����R�[�h�𔽉f����
		const TypeConfig& types = pDoc->m_docType.GetDocumentAttribute();
		pDoc->m_docFile.SetCodeSet( loadInfo.eCharCode, 
			( loadInfo.eCharCode == types.m_encoding.m_eDefaultCodetype ) ?
				types.m_encoding.m_bDefaultBom : CodeTypeName( loadInfo.eCharCode ).IsBomDefOn() );
	}

	// ���C�A�E�g���̕ύX
	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINEKETAS�ŏ���������
	// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
	const TypeConfig& ref = pDoc->m_docType.GetDocumentAttribute();
	LayoutInt nMaxLineKetas = ref.m_nMaxLineKetas;
	if (ref.m_nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}

	ProgressSubject* pOld = EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(&pDoc->m_layoutMgr);
	pDoc->m_layoutMgr.SetLayoutInfo( true, ref, ref.m_nTabSpace, nMaxLineKetas );
	pDoc->m_pEditWnd->ClearViewCaretPosInfo();
	
	EditApp::getInstance()->m_pVisualProgress->ProgressListener::Listen(pOld);

	return eRet;
}


void LoadAgent::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// �e�E�B���h�E�̃^�C�g�����X�V
	pDoc->m_pEditWnd->UpdateCaption();

	// -- -- �� InitAllView�ł���Ă����� -- -- //	// 2009.08.28 nasukoji	CEditView::OnAfterLoad()���炱���Ɉړ�
	pDoc->m_nCommandExecNum = 0;

	// �e�L�X�g�̐܂�Ԃ����@��������
	pDoc->m_nTextWrapMethodCur = pDoc->m_docType.GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	pDoc->m_bTextWrapMethodCurTemp = false;													// �ꎞ�ݒ�K�p��������
	pDoc->m_blfCurTemp = false;
	pDoc->m_bTabSpaceCurTemp = false;

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if (pDoc->m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		pDoc->m_layoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	}else {
		pDoc->m_layoutMgr.ClearLayoutLineWidth();		// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
}


void LoadAgent::OnFinalLoad(LoadResultType eLoadResult)
{
	EditDoc* pDoc = GetListeningDoc();

	if (eLoadResult == LoadResultType::Failure) {
		pDoc->SetFilePathAndIcon(_T(""));
		pDoc->m_docFile.SetBomDefoult();
	}
	if (eLoadResult == LoadResultType::LoseSome) {
		AppMode::getInstance()->SetViewMode(true);
	}

	// �ĕ`�� $$�s��
	// EditWnd::getInstance()->GetActiveView().SetDrawSwitch(true);
	bool bDraw = EditWnd::getInstance()->GetActiveView().GetDrawSwitch();
	if (bDraw) {
		EditWnd::getInstance()->Views_RedrawAll(); // �r���[�ĕ`��
		InvalidateRect( EditWnd::getInstance()->GetHwnd(), NULL, TRUE );
	}
	Caret& caret = EditWnd::getInstance()->GetActiveView().GetCaret();
	caret.MoveCursor(caret.GetCaretLayoutPos(), true);
	EditWnd::getInstance()->GetActiveView().AdjustScrollBars();
}

