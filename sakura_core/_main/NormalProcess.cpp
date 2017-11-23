/*!	@file
	@brief �G�f�B�^�v���Z�X�N���X
*/

#include "StdAfx.h"
#include "NormalProcess.h"
#include "CommandLine.h"
#include "ControlTray.h"
#include "window/EditWnd.h"
#include "GrepAgent.h"
#include "doc/EditDoc.h"
#include "doc/logic/DocLine.h"
#include "debug/RunningTimer.h"
#include "util/window.h"
#include "util/fileUtil.h"
#include "plugin/PluginManager.h"
#include "plugin/JackManager.h"
#include "AppMode.h"
#include "env/DocTypeManager.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

NormalProcess::NormalProcess(HINSTANCE hInstance, LPCTSTR lpCmdLine)
	:
	Process(hInstance, lpCmdLine),
	pEditApp(nullptr)
{
}

NormalProcess::~NormalProcess()
{
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �v���Z�X�n���h��                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief �G�f�B�^�v���Z�X������������
	
	EditWnd���쐬����B
*/
bool NormalProcess::InitializeProcess()
{
	MY_RUNNINGTIMER(runningTimer, "NormalProcess::Init");

	// �v���Z�X�������̖ڈ�
	HANDLE hMutex = _GetInitializeMutex();
	if (!hMutex) {
		return false;
	}

	// ���L������������������
	if (!Process::InitializeProcess()) {
		return false;
	}

	// �����I������
	SelectLang::ChangeLang(GetDllShareData().common.window.szLanguageDll);

	// �R�}���h���C���I�v�V����
	bool bViewMode = false;
	
	auto& cmdLine = CommandLine::getInstance();
	// �R�}���h���C���Ŏ󂯎�����t�@�C�����J����Ă���ꍇ��
	// ���̕ҏW�E�B���h�E���A�N�e�B�u�ɂ���
	EditInfo fi = cmdLine.GetEditInfo();
	if (fi.szPath[0] != _T('\0')) {
		// MRU����J�[�\���ʒu�𕜌����鑀���CEditDoc::FileLoad��
		// �s����̂ł����ł͕K�v�Ȃ��D

		// �w��t�@�C�����J����Ă��邩���ׂ�
		// �����R�[�h���قȂ�Ƃ��̓��[�j���O���o���悤��
		HWND hwndOwner;
		if (GetShareData().ActiveAlreadyOpenedWindow(fi.szPath, &hwndOwner, fi.nCharCode)) {
			// �J�[�\���ʒu�������Ɏw�肳��Ă�����w��ʒu�ɃW�����v
			if (fi.ptCursor.y >= 0) {	// �s�̎w�肪���邩
				Point& pt = GetDllShareData().workBuffer.logicPoint;
				if (fi.ptCursor.x < 0) {
					// ���̎w�肪�����ꍇ
					::SendMessage(hwndOwner, MYWM_GETCARETPOS, 0, 0);
				}else {
					pt.x = fi.ptCursor.x;
				}
				pt.y = fi.ptCursor.y;
				::SendMessage(hwndOwner, MYWM_SETCARETPOS, 0, 0);
			}
			// �A�N�e�B�u�ɂ���
			ActivateFrameWindow(hwndOwner);
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			return false;
		}
	}


	// �v���O�C���ǂݍ���
	MY_TRACETIME(runningTimer, "Before Init Jack");
	// �W���b�N������
	JackManager::getInstance();
	MY_TRACETIME(runningTimer, "After Init Jack");

	MY_TRACETIME(runningTimer, "Before Load Plugins");
	// �v���O�C���ǂݍ���
	PluginManager::getInstance().LoadAllPlugin();
	MY_TRACETIME(runningTimer, "After Load Plugins");

	// �G�f�B�^�A�v���P�[�V�������쐬
	// �O���[�vID���擾
	int nGroupId = cmdLine.GetGroupId();
	if (GetDllShareData().common.tabBar.bNewWindow && nGroupId == -1) {
		nGroupId = AppNodeManager::getInstance().GetFreeGroupId();
	}
	// CEditApp���쐬
	pEditApp = &EditApp::getInstance();
	pEditApp->Create(GetProcessInstance(), nGroupId);
	EditWnd* pEditWnd = pEditApp->GetEditWindow();
	auto& activeView = pEditWnd->GetActiveView();
	if (!pEditWnd->GetHwnd()) {
		::ReleaseMutex(hMutex);
		::CloseHandle(hMutex);
		return false;	// EditWnd::Create()���s�̂��ߏI��
	}

	// �R�}���h���C���̉��
	bool bDebugMode = cmdLine.IsDebugMode();
	bool bGrepMode = cmdLine.IsGrepMode();
	bool bGrepDlg = cmdLine.IsGrepDlg();

	MY_TRACETIME(runningTimer, "CheckFile");

	// -1: SetDocumentTypeWhenCreate �ł̋����w��Ȃ�
	const TypeConfigNum nType = (fi.szDocType[0] == '\0' ? TypeConfigNum(-1) : DocTypeManager().GetDocumentTypeOfExt(fi.szDocType));

	auto& editDoc = pEditWnd->GetDocument();
	if (bDebugMode) {
		// �f�o�b�O���j�^���[�h�ɐݒ�
		editDoc.SetCurDirNotitle();
		AppMode::getInstance().SetDebugModeON();
		if (!AppMode::getInstance().IsDebugMode()) {
			// �f�o�b�O�ł͂Ȃ���(����)
			AppNodeManager::getInstance().GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
		}
		// �A�E�g�v�b�g�p�^�C�v�ʐݒ�
		pEditWnd->SetDocumentTypeWhenCreate(fi.nCharCode, false, nType);
		pEditWnd->dlgFuncList.Refresh();	// �A�E�g���C����\������
	}else if (bGrepMode) {
		// GREP
		pEditWnd->SetDocumentTypeWhenCreate(fi.nCharCode, false, nType);
		pEditWnd->dlgFuncList.Refresh();	// �A�E�g���C����\�ߕ\�����Ă���
		HWND hEditWnd = pEditWnd->GetHwnd();
		if (!::IsIconic(hEditWnd) && pEditWnd->dlgFuncList.GetHwnd()) {
			RECT rc;
			::GetClientRect(hEditWnd, &rc);
			::SendMessage(hEditWnd, WM_SIZE, ::IsZoomed(hEditWnd) ? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
		}
		GrepInfo gi = cmdLine.GetGrepInfo();
		if (!bGrepDlg) {
			SetMainWindow(pEditWnd->GetHwnd());
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			this->pEditApp->pGrepAgent->DoGrep(
				activeView,
				gi.bGrepReplace,
				&gi.mGrepKey,
				&gi.mGrepRep,
				&gi.mGrepFile,
				&gi.mGrepFolder,
				gi.bGrepCurFolder,
				gi.bGrepSubFolder,
				gi.bGrepStdout,
				gi.bGrepHeader,
				gi.grepSearchOption,
				gi.charEncoding,
				gi.nGrepOutputLineType,
				gi.nGrepOutputStyle,
				gi.bGrepOutputFileOnly,
				gi.bGrepOutputBaseFolder,
				gi.bGrepSeparateFolder,
				gi.bGrepPaste,
				gi.bGrepBackup
			);
			pEditWnd->dlgFuncList.Refresh();	// �A�E�g���C�����ĉ�͂���
		}else {
			AppNodeManager::getInstance().GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
			
			if (gi.mGrepKey.GetStringLength() < _MAX_PATH) {
				SearchKeywordManager().AddToSearchKeys(gi.mGrepKey.GetStringPtr());
			}
			if (gi.mGrepFile.GetStringLength() < _MAX_PATH) {
				SearchKeywordManager().AddToGrepFiles(gi.mGrepFile.GetStringPtr());
			}
			NativeT memGrepFolder = gi.mGrepFolder;
			if (gi.mGrepFolder.GetStringLength() < _MAX_PATH) {
				SearchKeywordManager().AddToGrepFolders(gi.mGrepFolder.GetStringPtr());
				// �w��Ȃ��̏ꍇ�̓J�����g�t�H���_�ɂ���
				if (memGrepFolder.GetStringLength() == 0) {
					TCHAR szCurDir[_MAX_PATH];
					::GetCurrentDirectory(_countof(szCurDir), szCurDir);
					memGrepFolder.SetString(szCurDir);
				}
			}
			auto& csSearch = GetDllShareData().common.search;
			csSearch.bGrepSubFolder = gi.bGrepSubFolder;
			csSearch.searchOption = gi.grepSearchOption;
			csSearch.nGrepCharSet = gi.charEncoding;
			csSearch.nGrepOutputLineType = gi.nGrepOutputLineType;
			csSearch.nGrepOutputStyle = gi.nGrepOutputStyle;
			SetMainWindow(pEditWnd->GetHwnd());
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			hMutex = NULL;
			
			pEditWnd->dlgGrep.strText = gi.mGrepKey.GetStringPtr();		// ����������
			pEditWnd->dlgGrep.bSetText = true;
			int nSize = _countof2(pEditWnd->dlgGrep.szFile);
			_tcsncpy(pEditWnd->dlgGrep.szFile, gi.mGrepFile.GetStringPtr(), nSize);	// �����t�@�C��
			pEditWnd->dlgGrep.szFile[nSize - 1] = _T('\0');
			nSize = _countof2(pEditWnd->dlgGrep.szFolder);
			_tcsncpy(pEditWnd->dlgGrep.szFolder, memGrepFolder.GetStringPtr(), nSize);	// �����t�H���_
			pEditWnd->dlgGrep.szFolder[nSize - 1] = _T('\0');

			INT_PTR nRet = pEditWnd->dlgGrep.DoModal(GetProcessInstance(), pEditWnd->GetHwnd(), NULL);
			if (nRet != FALSE) {
				activeView.GetCommander().HandleCommand(F_GREP, true, 0, 0, 0, 0);
			}else {
				// ������Grep�łȂ�
				editDoc.SetCurDirNotitle();
			}
			pEditWnd->dlgFuncList.Refresh();	// �A�E�g���C�����ĉ�͂���
		}

		// �v���O�C���FEditorStart�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_EDITOR_START, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(activeView, params);
		}

		// �v���O�C���FDocumentOpen�C�x���g���s
		plugs.clear();
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(activeView, params);
		}

		if (!bGrepDlg && gi.bGrepStdout) {
			// �����I��
			PostMessage( pEditWnd->GetHwnd(), MYWM_CLOSE, PM_CLOSE_GREPNOCONFIRM | PM_CLOSE_EXIT, (LPARAM)NULL );
		}

		return true;
	}else {
		bViewMode = cmdLine.IsViewMode();
		if (fi.szPath[0] != _T('\0')) {
			// �����^�C�v�w��
			pEditWnd->OpenDocumentWhenStart(
				LoadInfo(
					fi.szPath,
					fi.nCharCode,
					bViewMode,
					nType
				)
			);
			// �ǂݍ��ݒ��f���āu(����)�v�ɂȂ������i���v���Z�X����̃��b�N�Ȃǁj���I�v�V�����w���L���ɂ���
			// Note. fi.nCharCode �ŕ����R�[�h�������w�肳��Ă��Ă��A�ǂݍ��ݒ��f���Ȃ��ꍇ�͕ʂ̕����R�[�h���I������邱�Ƃ�����B
			//       �ȑO�́u(����)�v�ɂȂ�Ȃ��ꍇ�ł��������� SetDocumentTypeWhenCreate() ���Ă�ł������A
			//       �u�O��ƈقȂ镶���R�[�h�v�̖₢���킹�őO��̕����R�[�h���I�����ꂽ�ꍇ�ɂ��������Ȃ��Ă����B
			if (!editDoc.docFile.GetFilePathClass().IsValidPath()) {
				// �ǂݍ��ݒ��f���āu(����)�v�ɂȂ���
				// ---> �����ɂȂ����I�v�V�����w���L���ɂ���
				pEditWnd->SetDocumentTypeWhenCreate(
					fi.nCharCode,
					bViewMode,
					nType
				);
			}
			if (
				(0 <= fi.nViewTopLine && 0 <= fi.nViewLeftCol)
				&& fi.nViewTopLine < (int)editDoc.layoutMgr.GetLineCount()
			) {
				activeView.GetTextArea().SetViewTopLine(fi.nViewTopLine);
				activeView.GetTextArea().SetViewLeftCol(fi.nViewLeftCol);
			}

			// �I�v�V�����w�肪�Ȃ��Ƃ��̓J�[�\���ʒu�ݒ���s��Ȃ��悤�ɂ���
			// 0���ʒu�Ƃ��Ă͗L���Ȓl�Ȃ̂Ŕ���Ɋ܂߂Ȃ��Ă͂Ȃ�Ȃ�
			if (0 <= fi.ptCursor.x || 0 <= fi.ptCursor.y) {
				/*
				  �J�[�\���ʒu�ϊ�
				  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
				  ��
				  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
				*/
				Point ptPos = editDoc.layoutMgr.LogicToLayout(fi.ptCursor);
				// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
				const DocLine* pTmpDocLine = editDoc.docLineMgr.GetLine(fi.ptCursor.y);
				if (pTmpDocLine) {
					if ((int)pTmpDocLine->GetLengthWithoutEOL() < fi.ptCursor.x) {
						ptPos.x--;
					}
				}

				activeView.GetCaret().MoveCursor(ptPos, true);
				activeView.GetCaret().nCaretPosX_Prev =
					activeView.GetCaret().GetCaretLayoutPos().x;
			}
			activeView.RedrawAll();
		}else {
			editDoc.SetCurDirNotitle();	// (����)�E�B���h�E
			// �t�@�C�������^�����Ȃ��Ă�ReadOnly�ƃ^�C�v�w���L���ɂ���
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.nCharCode,
				bViewMode,	// �r���[���[�h��
				nType
			);
		}
		if (!editDoc.docFile.GetFilePathClass().IsValidPath()) {
			editDoc.SetCurDirNotitle();	// (����)�E�B���h�E
			AppNodeManager::getInstance().GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
		}
	}

	SetMainWindow(pEditWnd->GetHwnd());

	activeView.SetIMECompFormPos();

	// WM_SIZE���|�X�g
	{	// �t�@�C���ǂݍ��݂��Ȃ������ꍇ�ɂ͂��� WM_SIZE ���A�E�g���C����ʂ�z�u����
		HWND hEditWnd = pEditWnd->GetHwnd();
		if (!::IsIconic(hEditWnd)) {
			RECT rc;
			::GetClientRect(hEditWnd, &rc);
			::PostMessage(hEditWnd, WM_SIZE, ::IsZoomed(hEditWnd) ? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
		}
	}

	// �ĕ`��
	::InvalidateRect(pEditWnd->GetHwnd(), NULL, TRUE);

	if (hMutex) {
		::ReleaseMutex(hMutex);
		::CloseHandle(hMutex);
	}

	// �v���O�C���FEditorStart�C�x���g���s
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(
			PP_EDITOR_START,
			0,
			&plugs
		);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(activeView, params);
	}

	// �I�[�v���㎩�����s�}�N�������s����
	if (!(bDebugMode || bGrepMode)) {
		editDoc.RunAutoMacro(GetDllShareData().common.macro.nMacroOnOpened);
	}

	// �N�����}�N���I�v�V����
	LPCWSTR pszMacro = cmdLine.GetMacro();
	if (pEditWnd->GetHwnd() && pszMacro && pszMacro[0] != L'\0') {
		LPCWSTR pszMacroType = cmdLine.GetMacroType();
		if (!pszMacroType || pszMacroType[0] == L'\0' || wcsicmp(pszMacroType, L"file") == 0) {
			pszMacroType = NULL;
		}
		activeView.GetCommander().HandleCommand(F_EXECEXTMACRO, true, (LPARAM)pszMacro, (LPARAM)pszMacroType, 0, 0);
	}

	// �����t�@�C���ǂݍ���
	size_t fileNum = cmdLine.GetFileNum();
	if (fileNum > 0) {
		size_t nDropFileNumMax = GetDllShareData().common.file.nDropFileNumMax - 1;
		// �t�@�C���h���b�v���̏���ɍ��킹��
		if (fileNum > nDropFileNumMax) {
			fileNum = nDropFileNumMax;
		}
		EditInfo openFileInfo = fi;
		for (size_t i=0; i<fileNum; ++i) {
			// �t�@�C���������ւ�
			_tcscpy_s(openFileInfo.szPath, cmdLine.GetFileName(i));
			bool ret = ControlTray::OpenNewEditor2(GetProcessInstance(), pEditWnd->GetHwnd(), openFileInfo, bViewMode);
			if (!ret) {
				break;
			}
		}
		// �p�ς݂Ȃ̂ō폜
		cmdLine.ClearFile();
	}

	// �v���O�C���FDocumentOpen�C�x���g���s
	plugs.clear();
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(activeView, params);
	}

	return pEditWnd->GetHwnd() != NULL;
}

/*!
	@brief �G�f�B�^�v���Z�X�̃��b�Z�[�W���[�v
*/
bool NormalProcess::MainLoop()
{
	if (GetMainWindow()) {
		pEditApp->GetEditWindow()->MessageLoop();	// ���b�Z�[�W���[�v
		return true;
	}
	return false;
}

/*!
	@brief �G�f�B�^�v���Z�X���I������
	�����͂Ȃɂ����Ȃ��B��n����dtor�ŁB
*/
void NormalProcess::OnExitProcess()
{
	// �v���O�C�����
	PluginManager::getInstance().UnloadAllPlugin();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief Mutex(�v���Z�X�������̖ڈ�)���擾����

	���������ɋN������ƃE�B���h�E���\�ɏo�Ă��Ȃ����Ƃ�����B
*/
HANDLE NormalProcess::_GetInitializeMutex() const
{
	MY_RUNNINGTIMER(runningTimer, "NormalProcess::_GetInitializeMutex");
	HANDLE hMutex;
	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
	std::tstring strMutexInitName = GSTR_MUTEX_SAKURA_INIT;
	strMutexInitName += strProfileName;
	hMutex = ::CreateMutex( NULL, TRUE, strMutexInitName.c_str() );
	if (!hMutex) {
		ErrorBeep();
		TopErrorMessage(NULL, _T("CreateMutex()���s�B\n�I�����܂��B"));
		return NULL;
	}
	if (::GetLastError() == ERROR_ALREADY_EXISTS) {
		DWORD dwRet = ::WaitForSingleObject(hMutex, 15000);
		if (dwRet == WAIT_TIMEOUT) { // �ʂ̒N�����N����
			TopErrorMessage(NULL, _T("�G�f�B�^�܂��̓V�X�e�����r�W�[��Ԃł��B\n���΂炭�҂��ĊJ���Ȃ����Ă��������B"));
			::CloseHandle(hMutex);
			return NULL;
		}
	}
	return hMutex;
}

