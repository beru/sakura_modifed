/*!	@file
	@brief �G�f�B�^�v���Z�X�N���X

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka Process��蕪��
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, genta, Moca, MIK
	Copyright (C) 2004, Moca, naoh
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, Uchi
	Copyright (C) 2009, syat, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "NormalProcess.h"
#include "CommandLine.h"
#include "ControlTray.h"
#include "window/EditWnd.h" // 2002/2/3 aroka
#include "GrepAgent.h"
#include "doc/EditDoc.h"
#include "doc/logic/DocLine.h" // 2003/03/28 MIK
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
	m_pEditApp(nullptr)
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
	
	@author aroka
	@date 2002/01/07

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
	@date 2004.05.13 Moca EditWnd::Create()�Ɏ��s�����ꍇ��false��Ԃ��悤�ɁD
	@date 2007.06.26 ryoji �O���[�vID���w�肵�ĕҏW�E�B���h�E���쐬����
	@date 2012.02.25 novice �����t�@�C���ǂݍ���
*/
bool NormalProcess::InitializeProcess()
{
	MY_RUNNINGTIMER(runningTimer, "NormalProcess::Init");

	// �v���Z�X�������̖ڈ�
	HANDLE	hMutex = _GetInitializeMutex();	// 2002/2/8 aroka ���ݓ����Ă����̂ŕ���
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
	bool		bViewMode = false;
	bool		bDebugMode;
	bool		bGrepMode;
	bool		bGrepDlg;
	
	auto& cmdLine = CommandLine::getInstance();
	// �R�}���h���C���Ŏ󂯎�����t�@�C�����J����Ă���ꍇ��
	// ���̕ҏW�E�B���h�E���A�N�e�B�u�ɂ���
	EditInfo fi = cmdLine.GetEditInfo(); // 2002/2/8 aroka �����Ɉړ�
	if (fi.szPath[0] != _T('\0')) {
		// Oct. 27, 2000 genta
		// MRU����J�[�\���ʒu�𕜌����鑀���CEditDoc::FileLoad��
		// �s����̂ł����ł͕K�v�Ȃ��D

		// �w��t�@�C�����J����Ă��邩���ׂ�
		// 2007.03.13 maru �����R�[�h���قȂ�Ƃ��̓��[�j���O���o���悤��
		HWND hwndOwner;
		if (GetShareData().ActiveAlreadyOpenedWindow(fi.szPath, &hwndOwner, fi.nCharCode)) {
			// From Here Oct. 19, 2001 genta
			// �J�[�\���ʒu�������Ɏw�肳��Ă�����w��ʒu�ɃW�����v
			if (fi.ptCursor.y >= 0) {	// �s�̎w�肪���邩
				LogicPoint& pt = GetDllShareData().workBuffer.logicPoint;
				if (fi.ptCursor.x < 0) {
					// ���̎w�肪�����ꍇ
					::SendMessage(hwndOwner, MYWM_GETCARETPOS, 0, 0);
				}else {
					pt.x = fi.ptCursor.x;
				}
				pt.y = fi.ptCursor.y;
				::SendMessage(hwndOwner, MYWM_SETCARETPOS, 0, 0);
			}
			// To Here Oct. 19, 2001 genta
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

	// �G�f�B�^�A�v���P�[�V�������쐬�B2007.10.23 kobake
	// �O���[�vID���擾
	int nGroupId = cmdLine.GetGroupId();
	if (GetDllShareData().common.tabBar.bNewWindow && nGroupId == -1) {
		nGroupId = AppNodeManager::getInstance().GetFreeGroupId();
	}
	// CEditApp���쐬
	m_pEditApp = &EditApp::getInstance();
	m_pEditApp->Create(GetProcessInstance(), nGroupId);
	EditWnd* pEditWnd = m_pEditApp->GetEditWindow();
	auto& activeView = pEditWnd->GetActiveView();
	if (!pEditWnd->GetHwnd()) {
		::ReleaseMutex(hMutex);
		::CloseHandle(hMutex);
		return false;	// 2009.06.23 ryoji EditWnd::Create()���s�̂��ߏI��
	}

	// �R�}���h���C���̉��		2002/2/8 aroka �����Ɉړ�
	bDebugMode = cmdLine.IsDebugMode();
	bGrepMode  = cmdLine.IsGrepMode();
	bGrepDlg   = cmdLine.IsGrepDlg();

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
		// 2004.09.20 naoh �A�E�g�v�b�g�p�^�C�v�ʐݒ�
		// �����R�[�h��L���Ƃ��� Uchi 2008/6/8
		// 2010.06.16 Moca �A�E�g�v�b�g�� CCommnadLine�� -TYPE=output �����Ƃ���
		pEditWnd->SetDocumentTypeWhenCreate(fi.nCharCode, false, nType);
		pEditWnd->m_dlgFuncList.Refresh();	// �A�E�g���C����\������
	}else if (bGrepMode) {
		// GREP
		// 2010.06.16 Moca Grep�ł��I�v�V�����w���K�p
		pEditWnd->SetDocumentTypeWhenCreate(fi.nCharCode, false, nType);
		pEditWnd->m_dlgFuncList.Refresh();	// �A�E�g���C����\�ߕ\�����Ă���
		HWND hEditWnd = pEditWnd->GetHwnd();
		if (!::IsIconic(hEditWnd) && pEditWnd->m_dlgFuncList.GetHwnd()) {
			RECT rc;
			::GetClientRect(hEditWnd, &rc);
			::SendMessage(hEditWnd, WM_SIZE, ::IsZoomed(hEditWnd) ? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
		}
		GrepInfo gi = cmdLine.GetGrepInfo(); // 2002/2/8 aroka �����Ɉړ�
		if (!bGrepDlg) {
			// Grep�ł͑Ώۃp�X��͂Ɍ��݂̃J�����g�f�B���N�g����K�v�Ƃ���
			// editDoc->SetCurDirNotitle();
			// 2003.06.23 Moca GREP���s�O��Mutex���J��
			// �������Ȃ���Grep���I���܂ŐV�����E�B���h�E���J���Ȃ�
			SetMainWindow(pEditWnd->GetHwnd());
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			this->m_pEditApp->m_pGrepAgent->DoGrep(
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
				gi.charEncoding,	// 2002/09/21 Moca
				gi.nGrepOutputLineType,
				gi.nGrepOutputStyle,
				gi.bGrepOutputFileOnly,
				gi.bGrepOutputBaseFolder,
				gi.bGrepSeparateFolder,
				gi.bGrepPaste,
				gi.bGrepBackup
			);
			pEditWnd->m_dlgFuncList.Refresh();	// �A�E�g���C�����ĉ�͂���
			//return true; // 2003.06.23 Moca
		}else {
			AppNodeManager::getInstance().GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
			
			//-GREPDLG�Ń_�C�A���O���o���B�@���������f�i2002/03/24 YAZAKI�j
			if (gi.mGrepKey.GetStringLength() < _MAX_PATH) {
				SearchKeywordManager().AddToSearchKeys(gi.mGrepKey.GetStringPtr());
			}
			if (gi.mGrepFile.GetStringLength() < _MAX_PATH) {
				SearchKeywordManager().AddToGrepFiles(gi.mGrepFile.GetStringPtr());
			}
			NativeT memGrepFolder = gi.mGrepFolder;
			if (gi.mGrepFolder.GetStringLength() < _MAX_PATH) {
				SearchKeywordManager().AddToGrepFolders(gi.mGrepFolder.GetStringPtr());
				// 2013.05.21 �w��Ȃ��̏ꍇ�̓J�����g�t�H���_�ɂ���
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
			// 2003.06.23 Moca GREP�_�C�A���O�\���O��Mutex���J��
			// �������Ȃ���Grep���I���܂ŐV�����E�B���h�E���J���Ȃ�
			SetMainWindow(pEditWnd->GetHwnd());
			::ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			hMutex = NULL;
			
			// Oct. 9, 2003 genta �R�}���h���C������GERP�_�C�A���O��\���������ꍇ��
			// �����̐ݒ肪BOX�ɔ��f����Ȃ�
			pEditWnd->m_dlgGrep.m_strText = gi.mGrepKey.GetStringPtr();		// ����������
			pEditWnd->m_dlgGrep.m_bSetText = true;
			int nSize = _countof2(pEditWnd->m_dlgGrep.m_szFile);
			_tcsncpy(pEditWnd->m_dlgGrep.m_szFile, gi.mGrepFile.GetStringPtr(), nSize);	// �����t�@�C��
			pEditWnd->m_dlgGrep.m_szFile[nSize - 1] = _T('\0');
			nSize = _countof2(pEditWnd->m_dlgGrep.m_szFolder);
			_tcsncpy(pEditWnd->m_dlgGrep.m_szFolder, memGrepFolder.GetStringPtr(), nSize);	// �����t�H���_
			pEditWnd->m_dlgGrep.m_szFolder[nSize - 1] = _T('\0');

			
			// Feb. 23, 2003 Moca Owner window���������w�肳��Ă��Ȃ�����
			int nRet = pEditWnd->m_dlgGrep.DoModal(GetProcessInstance(), pEditWnd->GetHwnd(), NULL);
			if (nRet != FALSE) {
				activeView.GetCommander().HandleCommand(F_GREP, true, 0, 0, 0, 0);
			}else {
				// ������Grep�łȂ�
				editDoc.SetCurDirNotitle();
			}
			pEditWnd->m_dlgFuncList.Refresh();	// �A�E�g���C�����ĉ�͂���
			//return true; // 2003.06.23 Moca
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

		return true; // 2003.06.23 Moca
	}else {
		// 2004.05.13 Moca �����if���̒�����O�Ɉړ�
		// �t�@�C�������^�����Ȃ��Ă�ReadOnly�w���L���ɂ��邽�߁D
		bViewMode = cmdLine.IsViewMode(); // 2002/2/8 aroka �����Ɉړ�
		if (fi.szPath[0] != _T('\0')) {
			// Mar. 9, 2002 genta �����^�C�v�w��
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
			if (!editDoc.m_docFile.GetFilePathClass().IsValidPath()) {
				// �ǂݍ��ݒ��f���āu(����)�v�ɂȂ���
				// ---> �����ɂȂ����I�v�V�����w���L���ɂ���
				pEditWnd->SetDocumentTypeWhenCreate(
					fi.nCharCode,
					bViewMode,
					nType
				);
			}
			// Nov. 6, 2000 genta
			// �L�����b�g�ʒu�̕����̂���
			// �I�v�V�����w�肪�Ȃ��Ƃ��͉�ʈړ����s��Ȃ��悤�ɂ���
			// Oct. 19, 2001 genta
			// ���ݒ聁-1�ɂȂ�悤�ɂ����̂ŁC���S�̂��ߗ��҂��w�肳�ꂽ�Ƃ�����
			// �ړ�����悤�ɂ���D || �� &&
			if (
				(LayoutInt(0) <= fi.nViewTopLine && LayoutInt(0) <= fi.nViewLeftCol)
				&& fi.nViewTopLine < editDoc.m_layoutMgr.GetLineCount()
			) {
				activeView.GetTextArea().SetViewTopLine(fi.nViewTopLine);
				activeView.GetTextArea().SetViewLeftCol(fi.nViewLeftCol);
			}

			// �I�v�V�����w�肪�Ȃ��Ƃ��̓J�[�\���ʒu�ݒ���s��Ȃ��悤�ɂ���
			// Oct. 19, 2001 genta
			// 0���ʒu�Ƃ��Ă͗L���Ȓl�Ȃ̂Ŕ���Ɋ܂߂Ȃ��Ă͂Ȃ�Ȃ�
			if (0 <= fi.ptCursor.x || 0 <= fi.ptCursor.y) {
				/*
				  �J�[�\���ʒu�ϊ�
				  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
				  ��
				  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
				*/
				LayoutPoint ptPos;
				editDoc.m_layoutMgr.LogicToLayout(
					fi.ptCursor,
					&ptPos
				);

				// From Here Mar. 28, 2003 MIK
				// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
				// 2008.08.20 ryoji ���s�P�ʂ̍s�ԍ���n���悤�ɏC��
				const DocLine* pTmpDocLine = editDoc.m_docLineMgr.GetLine(fi.ptCursor.GetY2());
				if (pTmpDocLine) {
					if (pTmpDocLine->GetLengthWithoutEOL() < fi.ptCursor.x) {
						ptPos.x--;
					}
				}
				// To Here Mar. 28, 2003 MIK

				activeView.GetCaret().MoveCursor(ptPos, true);
				activeView.GetCaret().m_nCaretPosX_Prev =
					activeView.GetCaret().GetCaretLayoutPos().GetX2();
			}
			activeView.RedrawAll();
		}else {
			editDoc.SetCurDirNotitle();	// (����)�E�B���h�E
			// 2004.05.13 Moca �t�@�C�������^�����Ȃ��Ă�ReadOnly�ƃ^�C�v�w���L���ɂ���
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.nCharCode,
				bViewMode,	// �r���[���[�h��
				nType
			);
		}
		if (!editDoc.m_docFile.GetFilePathClass().IsValidPath()) {
			editDoc.SetCurDirNotitle();	// (����)�E�B���h�E
			AppNodeManager::getInstance().GetNoNameNumber(pEditWnd->GetHwnd());
			pEditWnd->UpdateCaption();
		}
	}

	SetMainWindow(pEditWnd->GetHwnd());

	// YAZAKI 2002/05/30 IME�E�B���h�E�̈ʒu�����������̂��C���B
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

	// 2006.09.03 ryoji �I�[�v���㎩�����s�}�N�������s����
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
	int fileNum = cmdLine.GetFileNum();
	if (fileNum > 0) {
		int nDropFileNumMax = GetDllShareData().common.file.nDropFileNumMax - 1;
		// �t�@�C���h���b�v���̏���ɍ��킹��
		if (fileNum > nDropFileNumMax) {
			fileNum = nDropFileNumMax;
		}
		EditInfo openFileInfo = fi;
		for (int i=0; i<fileNum; ++i) {
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
	
	@author aroka
	@date 2002/01/07
*/
bool NormalProcess::MainLoop()
{
	if (GetMainWindow()) {
		m_pEditApp->GetEditWindow()->MessageLoop();	// ���b�Z�[�W���[�v
		return true;
	}
	return false;
}

/*!
	@brief �G�f�B�^�v���Z�X���I������
	
	@author aroka
	@date 2002/01/07
	�����͂Ȃɂ����Ȃ��B��n����dtor�ŁB
*/
void NormalProcess::OnExitProcess()
{
	// �v���O�C�����
	PluginManager::getInstance().UnloadAllPlugin();		// Mpve here	2010/7/11 Uchi
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief Mutex(�v���Z�X�������̖ڈ�)���擾����

	���������ɋN������ƃE�B���h�E���\�ɏo�Ă��Ȃ����Ƃ�����B
	
	@date 2002/2/8 aroka InitializeProcess����ړ�
	@retval Mutex �̃n���h����Ԃ�
	@retval ���s�������̓����[�X���Ă��� NULL ��Ԃ�
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
		DWORD dwRet = ::WaitForSingleObject(hMutex, 15000);	// 2002/2/8 aroka ������������
		if (dwRet == WAIT_TIMEOUT) { // �ʂ̒N�����N����
			TopErrorMessage(NULL, _T("�G�f�B�^�܂��̓V�X�e�����r�W�[��Ԃł��B\n���΂炭�҂��ĊJ���Ȃ����Ă��������B"));
			::CloseHandle(hMutex);
			return NULL;
		}
	}
	return hMutex;
}

