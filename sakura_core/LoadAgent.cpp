#include "StdAfx.h"
#include "LoadAgent.h"
#include "ReadManager.h"
#include "_main/AppMode.h"
#include "_main/ControlTray.h"
#include "EditApp.h"
#include "env/DocTypeManager.h"
#include "env/ShareData.h"
#include "doc/EditDoc.h"
#include "view/EditView.h"
#include "window/EditWnd.h"
#include "uiparts/VisualProgress.h"
#include "util/fileUtil.h"

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
		bool bDlgResult = pDoc->docFileOperation.OpenFileDialog(
			EditWnd::getInstance().GetHwnd(),
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
					EditWnd::getInstance().GetHwnd(),
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
	if (ShareData::getInstance().ActiveAlreadyOpenedWindow(pLoadInfo->filePath, &hWndOwner, pLoadInfo->eCharCode)) {
		pLoadInfo->bOpened = true;
		return CallbackResultType::Interrupt;
	}

	// ���݂̃E�B���h�E�ɑ΂��ăt�@�C����ǂݍ��߂Ȃ��ꍇ�́A�V���ȃE�B���h�E���J���A�����Ƀt�@�C����ǂݍ��܂���
	if (!pDoc->IsAcceptLoad()) {
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			EditWnd::getInstance().GetHwnd(),
			*pLoadInfo
		);
		return CallbackResultType::Interrupt;
	}

next:
	auto& csFile = GetDllShareData().common.file;
	// �I�v�V�����F�J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������
	if (csFile.GetAlertIfFileNotExist()) {
		if (!fexist(pLoadInfo->filePath)) {
			InfoBeep();
			//	Popup�E�B���h�E��\�����Ȃ��悤�ɁD
			//	�����ŃX�e�[�^�X���b�Z�[�W���g���Ă���ʂɕ\������Ȃ��D
			TopInfoMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_NOT_EXSIST_SAVE),
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
		bool bLock = (pLoadInfo->IsSamePath(pDoc->docFile.GetFilePath()) && pDoc->docFile.IsFileLocking());
		if (bLock) {
			pDoc->docFileOperation.DoFileUnlock();
		}

		// �`�F�b�N
		if (!file.IsFileReadable()) {
			if (bLock) {
				pDoc->docFileOperation.DoFileLock(false);
			}
			ErrorMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_LOADAGENT_ERR_OPEN),
				pLoadInfo->filePath.c_str()
			);
			return CallbackResultType::Interrupt; // �t�@�C�������݂��Ă���̂ɓǂݎ��Ȃ��ꍇ�͒��f
		}
		if (bLock) {
			pDoc->docFileOperation.DoFileLock(false);
		}
	}while (false);	//	1�񂵂��ʂ�Ȃ�. break�ł����܂Ŕ��

	// �t�@�C���T�C�Y�`�F�b�N
	if (csFile.bAlertIfLargeFile) {
		WIN32_FIND_DATA wfd;
		HANDLE nFind = ::FindFirstFile( pLoadInfo->filePath.c_str(), &wfd );
		if (nFind != INVALID_HANDLE_VALUE) {
			::FindClose( nFind );
			LARGE_INTEGER nFileSize;
			nFileSize.HighPart = wfd.nFileSizeHigh;
			nFileSize.LowPart = wfd.nFileSizeLow;
			// csFile.nAlertFileSize ��MB�P��
			if ((nFileSize.QuadPart>>20) >= (csFile.nAlertFileSize)) {
				int nRet = MYMESSAGEBOX( EditWnd::getInstance().GetHwnd(),
					MB_ICONQUESTION | MB_YESNO | MB_TOPMOST,
					GSTR_APPNAME,
					LS(STR_LOADAGENT_BIG_FILE),
					csFile.nAlertFileSize );
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
	pDoc->SetFilePathAndIcon(loadInfo.filePath);

	// ������ʊm��
	pDoc->docType.SetDocumentType(loadInfo.nType, true);
	pDoc->pEditWnd->pViewFontMiniMap->UpdateFont(&pDoc->pEditWnd->GetLogfont());
	InitCharWidthCache(pDoc->pEditWnd->pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap);
	SelectCharWidthCache(CharWidthFontMode::Edit, pDoc->pEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(pDoc->pEditWnd->GetLogfont());
	pDoc->pEditWnd->pViewFont->UpdateFont(&pDoc->pEditWnd->GetLogfont());

	// �N���Ɠ����ɓǂޏꍇ�͗\�߃A�E�g���C����͉�ʂ�z�u���Ă���
	// �i�t�@�C���ǂݍ��݊J�n�ƂƂ��Ƀr���[���\�������̂ŁA���ƂŔz�u����Ɖ�ʂ̂�������傫���́j
	if (!pDoc->pEditWnd->dlgFuncList.bEditWndReady) {
		pDoc->pEditWnd->dlgFuncList.Refresh();
		HWND hEditWnd = pDoc->pEditWnd->GetHwnd();
		if (!::IsIconic( hEditWnd ) && pDoc->pEditWnd->dlgFuncList.GetHwnd()) {
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::SendMessage( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
	}

	// �t�@�C�������݂���ꍇ�̓t�@�C����ǂ�
	if (fexist(loadInfo.filePath)) {
		// CDocLineMgr�̍\��
		ReadManager reader;
		ProgressSubject* pOld = EditApp::getInstance().pVisualProgress->ProgressListener::Listen(&reader);
		CodeConvertResult eReadResult = reader.ReadFile_To_CDocLineMgr(
			pDoc->docLineMgr,
			loadInfo,
			&pDoc->docFile.fileInfo
		);
		if (eReadResult == CodeConvertResult::LoseSome) {
			eRet = LoadResultType::LoseSome;
		}
		EditApp::getInstance().pVisualProgress->ProgressListener::Listen(pOld);
	}else {
		// ���݂��Ȃ��Ƃ����h�L�������g�ɕ����R�[�h�𔽉f����
		const TypeConfig& types = pDoc->docType.GetDocumentAttribute();
		pDoc->docFile.SetCodeSet( loadInfo.eCharCode, 
			( loadInfo.eCharCode == types.encoding.eDefaultCodetype ) ?
				types.encoding.bDefaultBom : CodeTypeName( loadInfo.eCharCode ).IsBomDefOn() );
	}

	// ���C�A�E�g���̕ύX
	// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINEKETAS�ŏ���������
	// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
	const TypeConfig& ref = pDoc->docType.GetDocumentAttribute();
	size_t nMaxLineKetas = ref.nMaxLineKetas;
	if (ref.nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}

	ProgressSubject* pOld = EditApp::getInstance().pVisualProgress->ProgressListener::Listen(&pDoc->layoutMgr);
	pDoc->layoutMgr.SetLayoutInfo(true, ref, ref.nTabSpace, nMaxLineKetas);
	pDoc->pEditWnd->ClearViewCaretPosInfo();
	
	EditApp::getInstance().pVisualProgress->ProgressListener::Listen(pOld);

	return eRet;
}


void LoadAgent::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// �e�E�B���h�E�̃^�C�g�����X�V
	pDoc->pEditWnd->UpdateCaption();

	// -- -- �� InitAllView�ł���Ă����� -- --
	pDoc->nCommandExecNum = 0;

	// �e�L�X�g�̐܂�Ԃ����@��������
	pDoc->nTextWrapMethodCur = pDoc->docType.GetDocumentAttribute().nTextWrapMethod;	// �܂�Ԃ����@
	pDoc->bTextWrapMethodCurTemp = false;													// �ꎞ�ݒ�K�p��������
	pDoc->blfCurTemp = false;
	pDoc->bTabSpaceCurTemp = false;

	// �u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if (pDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		pDoc->layoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	}else {
		pDoc->layoutMgr.ClearLayoutLineWidth();		// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
}


void LoadAgent::OnFinalLoad(LoadResultType eLoadResult)
{
	EditDoc* pDoc = GetListeningDoc();

	if (eLoadResult == LoadResultType::Failure) {
		pDoc->SetFilePathAndIcon(_T(""));
		pDoc->docFile.SetBomDefoult();
	}
	if (eLoadResult == LoadResultType::LoseSome) {
		AppMode::getInstance().SetViewMode(true);
	}

	// �ĕ`�� $$�s��
	auto& editWnd = EditWnd::getInstance();
	// editWnd.GetActiveView().SetDrawSwitch(true);
	bool bDraw = editWnd.GetActiveView().GetDrawSwitch();
	if (bDraw) {
		editWnd.Views_RedrawAll(); // �r���[�ĕ`��
		InvalidateRect(editWnd.GetHwnd(), nullptr, TRUE);
	}
	Caret& caret = editWnd.GetActiveView().GetCaret();
	caret.MoveCursor(caret.GetCaretLayoutPos(), true);
	editWnd.GetActiveView().AdjustScrollBars();
}

