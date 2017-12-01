#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "_main/ControlTray.h"
#include "uiparts/WaitCursor.h"
#include "dlg/DlgProperty.h"
#include "dlg/DlgCancel.h"
#include "dlg/DlgProfileMgr.h"
#include "doc/DocReader.h"	//  Command_Property_File for _DEBUG
#include "print/PrintPreview.h"
#include "io/BinaryStream.h"
#include "io/FileLoad.h"
#include "WriteManager.h"
#include "EditApp.h"
#include "recent/MRUFile.h"
#include "util/window.h"
#include "charset/CodeFactory.h"
#include "plugin/Plugin.h"
#include "plugin/JackManager.h"
#include "env/SakuraEnvironment.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h"

// ViewCommander�N���X�̃R�}���h(�t�@�C������n)�֐��Q

// �V�K�쐬
void ViewCommander::Command_FileNew(void)
{
	// �V���ȕҏW�E�B���h�E���N��
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_NONE;
	loadInfo.bViewMode = false;
	std::tstring curDir = SakuraEnvironment::GetDlgInitialDir();
	ControlTray::OpenNewEditor(
		G_AppInstance(),
		view.GetHwnd(),
		loadInfo,
		NULL,
		false,
		curDir.c_str(),
		false
	);
	return;
}


// �V�K�쐬�i�V�����E�B���h�E�ŊJ���j
void ViewCommander::Command_FileNew_NewWindow(void)
{
	// �V���ȕҏW�E�B���h�E���N��
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_DEFAULT;
	loadInfo.bViewMode = false;
	std::tstring curDir = SakuraEnvironment::GetDlgInitialDir();
	ControlTray::OpenNewEditor(
		G_AppInstance(),
		view.GetHwnd(),
		loadInfo,
		NULL,
		false,
		curDir.c_str(),
		true
	);
	return;
}


/*! @brief �t�@�C�����J�� */
void ViewCommander::Command_FileOpen(
	const wchar_t* filename,
	EncodingType nCharCode,
	bool bViewMode,
	const wchar_t* defaultName
	)
{
	if (!IsValidCodeType(nCharCode) && nCharCode != CODE_AUTODETECT) {
		nCharCode = CODE_AUTODETECT;
	}
	// ���[�h���
	LoadInfo loadInfo(filename ? to_tchar(filename) : _T(""), nCharCode, bViewMode);
	std::vector<std::tstring> files;
	std::tstring defName = (defaultName ? to_tchar(defaultName) : _T(""));

	// �K�v�ł���΁u�t�@�C�����J���v�_�C�A���O
	if (!loadInfo.filePath.IsValidPath()) {
		if (!defName.empty()) {
			TCHAR szPath[_MAX_PATH];
			TCHAR szDir[_MAX_DIR];
			TCHAR szName[_MAX_FNAME];
			TCHAR szExt  [_MAX_EXT];
			my_splitpath_t(defName.c_str(), szPath, szDir, szName, szExt);
			auto_strcat(szPath, szDir);
			if (auto_stricmp(defName.c_str(), szPath) == 0) {
				// defName�̓t�H���_��������
			}else {
				FilePath path = defName.c_str();
				if (auto_stricmp(path.GetDirPath().c_str(), szPath) == 0) {
					// �t�H���_���܂ł͎��݂��Ă���
					loadInfo.filePath = defName.c_str();
				}
			}
		}
		bool bDlgResult = GetDocument().docFileOperation.OpenFileDialog(
			EditWnd::getInstance().GetHwnd(),	// [in]  �I�[�i�[�E�B���h�E
			defName.length() == 0 ? NULL : defName.c_str(),	// [in]  �t�H���_
			&loadInfo,							// [out] ���[�h���󂯎��
			files								// [out] �t�@�C����
		);
		if (!bDlgResult) return;

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

	// �J��
	GetDocument().docFileOperation.FileLoad(&loadInfo);
}


/*! �㏑���ۑ�

	F_FILESAVEALL�Ƃ̑g�ݍ��킹�݂̂Ŏg����R�}���h�D
	@param warnbeep [in] true: �ۑ��s�v or �ۑ��֎~�̂Ƃ��Ɍx�����o��
	@param askname	[in] true: �t�@�C�������ݒ�̎��ɓ��͂𑣂�
*/
bool ViewCommander::Command_FileSave(bool warnbeep, bool askname)
{
	auto& doc = GetDocument();

	// �t�@�C�������w�肳��Ă��Ȃ��ꍇ�́u���O��t���ĕۑ��v�̃t���[�֑J��
	if (!GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		if (!askname) {
			return false;	// �ۑ����Ȃ�
		}
		return doc.docFileOperation.FileSaveAs();
	}

	// �Z�[�u���
	SaveInfo saveInfo;
	doc.GetSaveInfo(&saveInfo);
	saveInfo.eol = EolType::None; // ���s�R�[�h���ϊ�
	saveInfo.bOverwriteMode = true; // �㏑���v��

	// �㏑������
	auto& soundSet = EditApp::getInstance().soundSet;
	if (!warnbeep) soundSet.MuteOn();
	bool bRet = doc.docFileOperation.DoSaveFlow(&saveInfo);
	if (!warnbeep) soundSet.MuteOff();

	return bRet;
}


// ���O��t���ĕۑ��_�C�A���O
bool ViewCommander::Command_FileSaveAs_Dialog(
	const wchar_t* fileNameDef,
	EncodingType eCodeType,
	EolType eEolType
	)
{
	return GetDocument().docFileOperation.FileSaveAs(fileNameDef, eCodeType, eEolType, true);
}


/* ���O��t���ĕۑ�
	filename�ŕۑ��BNULL�͌��ցB
*/
bool ViewCommander::Command_FileSaveAs(
	const wchar_t* filename,
	EolType eEolType
	)
{
	return GetDocument().docFileOperation.FileSaveAs(filename, CODE_NONE, eEolType, false);
}


/*!	�S�ď㏑���ۑ�

	�ҏW���̑S�ẴE�B���h�E�ŏ㏑���ۑ����s���D
	�������C�㏑���ۑ��̎w�����o���݂̂Ŏ��s���ʂ̊m�F�͍s��Ȃ��D

	�㏑���֎~�y�уt�@�C�������ݒ�̃E�B���h�E�ł͉����s��Ȃ��D
*/
bool ViewCommander::Command_FileSaveAll(void)
{
	AppNodeGroupHandle(0).SendMessageToAllEditors(
		WM_COMMAND,
		MAKELONG(F_FILESAVE_QUIET, 0),
		0,
		NULL
	);
	return true;
}


// ����(����)
void ViewCommander::Command_FileClose(void)
{
	GetDocument().docFileOperation.FileClose();
}


/*! @brief ���ĊJ�� */
void ViewCommander::Command_FileClose_Open(
	LPCWSTR filename,
	EncodingType nCharCode,
	bool bViewMode
	)
{
	GetDocument().docFileOperation.FileCloseOpen(LoadInfo(to_tchar(filename), nCharCode, bViewMode));

	// �v���O�C���FDocumentOpen�C�x���g���s
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(GetEditWindow().GetActiveView(), params);
	}
}


// �t�@�C���̍ăI�[�v��
void ViewCommander::Command_File_Reopen(
	EncodingType	nCharCode,	// [in] �J�������ۂ̕����R�[�h
	bool			bNoConfirm	// [in] �t�@�C�����X�V���ꂽ�ꍇ�Ɋm�F���s��u�Ȃ��v���ǂ����Btrue:�m�F���Ȃ� false:�m�F����
	)
{
	auto& doc = GetDocument();
	if (!bNoConfirm && fexist(doc.docFile.GetFilePath()) && doc.docEditor.IsModified()) {
		int nDlgResult = MYMESSAGEBOX(
			view.GetHwnd(),
			MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			LS(STR_ERR_CEDITVIEW_CMD29),
			doc.docFile.GetFilePath()
		);
		if (nDlgResult == IDOK) {
			// �p���B���֐i��
		}else {
			return; // ���f
		}
	}

	// ����t�@�C���̍ăI�[�v��
	doc.docFileOperation.ReloadCurrentFile(nCharCode);
}


// ���
void ViewCommander::Command_Print(void)
{
	Command_Print_Preview();

	// ������s
	GetEditWindow().pPrintPreview->OnPrint();
}


// ���Preview
void ViewCommander::Command_Print_Preview(void)
{
	// ���Preview���[�h�̃I��/�I�t
	GetEditWindow().PrintPreviewModeONOFF();
	return;
}


// ����̃y�[�W���C�A�E�g�̐ݒ�
void ViewCommander::Command_Print_PageSetUp(void)
{
	// ����y�[�W�ݒ�
	GetEditWindow().OnPrintPageSetting();
	return;
}


// C/C++�w�b�_�t�@�C���܂��̓\�[�X�t�@�C�� �I�[�v���@�\
bool ViewCommander::Command_Open_HfromtoC(bool bCheckOnly)
{
	if (Command_Open_HHPP(bCheckOnly, false))	return true;
	if (Command_Open_CCPP(bCheckOnly, false))	return true;
	ErrorBeep();
	return false;
}


// C/C++�w�b�_�t�@�C�� �I�[�v���@�\
//BOOL ViewCommander::Command_OPENINCLUDEFILE(bool bCheckOnly)
bool ViewCommander::Command_Open_HHPP(bool bCheckOnly, bool bBeepWhenMiss)
{
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return view.OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, source_ext, header_ext,
		_countof(source_ext), _countof(header_ext),
		LS(STR_ERR_CEDITVIEW_CMD08));
}


// C/C++�\�[�X�t�@�C�� �I�[�v���@�\
bool ViewCommander::Command_Open_CCPP(bool bCheckOnly, bool bBeepWhenMiss)
{
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return view.OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, header_ext, source_ext,
		_countof(header_ext), _countof(source_ext),
		LS(STR_ERR_CEDITVIEW_CMD09));
}


// Oracle SQL*Plus���A�N�e�B�u�\��
void ViewCommander::Command_Activate_SQLPlus(void)
{
	HWND hwndSQLPLUS = ::FindWindow(_T("SqlplusWClass"), _T("Oracle SQL*Plus"));
	if (!hwndSQLPLUS) {
		ErrorMessage(view.GetHwnd(), LS(STR_SQLERR_ACTV_BUT_NOT_RUN));	// "Oracle SQL*Plus���A�N�e�B�u�\�����܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n"
		return;
	}
	// Oracle SQL*Plus���A�N�e�B�u�ɂ���
	// �A�N�e�B�u�ɂ���
	ActivateFrameWindow(hwndSQLPLUS);
	return;
}


// Oracle SQL*Plus�Ŏ��s
void ViewCommander::Command_PLSQL_Compile_On_SQLPlus(void)
{
	int			nRet;
	BOOL		nBool;
	TCHAR		szPath[MAX_PATH + 2];

	HWND hwndSQLPLUS = ::FindWindow(_T("SqlplusWClass"), _T("Oracle SQL*Plus"));
	if (!hwndSQLPLUS) {
		ErrorMessage(view.GetHwnd(), LS(STR_SQLERR_EXEC_BUT_NOT_RUN));	// "Oracle SQL*Plus�Ŏ��s���܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n"
		return;
	}
	// �e�L�X�g���ύX����Ă���ꍇ
	if (GetDocument().docEditor.IsModified()) {
		nRet = ::MYMESSAGEBOX(
			view.GetHwnd(),
			MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			GSTR_APPNAME,
			LS(STR_ERR_CEDITVIEW_CMD18),
			GetDocument().docFile.GetFilePathClass().IsValidPath() ? GetDocument().docFile.GetFilePath() : LS(STR_NO_TITLE1)
		);
		switch (nRet) {
		case IDYES:
			if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
				nBool = Command_FileSave();
			}else {
				nBool = Command_FileSaveAs_Dialog(NULL, CODE_NONE, EolType::None);
			}
			if (!nBool) {
				return;
			}
			break;
		case IDNO:
			return;
		case IDCANCEL:
		default:
			return;
		}
	}
	if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		// �t�@�C���p�X�ɋ󔒂��܂܂�Ă���ꍇ�̓_�u���N�H�[�e�[�V�����ň͂�
		if (_tcschr(GetDocument().docFile.GetFilePath(), TCODE::SPACE) != NULL) {
			auto_sprintf( szPath, _T("@\"%ts\"\r\n"), GetDocument().docFile.GetFilePath() );
		}else {
			auto_sprintf( szPath, _T("@%ts\r\n"), GetDocument().docFile.GetFilePath() );
		}
		// �N���b�v�{�[�h�Ƀf�[�^��ݒ�
		view.MySetClipboardData(szPath, _tcslen(szPath), false);

		// Oracle SQL*Plus���A�N�e�B�u�ɂ���
		// �A�N�e�B�u�ɂ���
		ActivateFrameWindow(hwndSQLPLUS);

		// Oracle SQL*Plus�Ƀy�[�X�g�̃R�}���h�𑗂�
		DWORD_PTR dwResult;
		LRESULT bResult = ::SendMessageTimeout(
			hwndSQLPLUS,
			WM_COMMAND,
			MAKELONG(201, 0),
			0,
			SMTO_ABORTIFHUNG | SMTO_NORMAL,
			3000,
			&dwResult
		);
		if (!bResult) {
			TopErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD20));
		}
	}else {
		ErrorBeep();
		ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD21));
		return;
	}
	return;
}


// �u���E�Y
void ViewCommander::Command_Browse(void)
{
	if (!GetDocument().docFile.GetFilePathClass().IsValidPath()) {
		ErrorBeep();
		return;
	}
	SHELLEXECUTEINFO info; 
	info.cbSize = sizeof(info);
	info.fMask = 0;
	info.hwnd = NULL;
	info.lpVerb = NULL;
	info.lpFile = GetDocument().docFile.GetFilePath();
	info.lpParameters = NULL;
	info.lpDirectory = NULL;
	info.nShow = SW_SHOWNORMAL;
	info.hInstApp = 0;
	info.lpIDList = NULL;
	info.lpClass = NULL;
	info.hkeyClass = 0; 
	info.dwHotKey = 0;
	info.hIcon =0;

	::ShellExecuteEx(&info);

	return;
}


// �r���[���[�h
void ViewCommander::Command_ViewMode(void)
{
	// �r���[���[�h�𔽓]
	AppMode::getInstance().SetViewMode(!AppMode::getInstance().IsViewMode());

	// �r������̐؂�ւ�
	// ���r���[���[�h ON ���͔r������ OFF�A�r���[���[�h OFF ���͔r������ ON �̎d�l�i>>data:5262�j�𑦎����f����
	auto& doc = GetDocument();
	doc.docFileOperation.DoFileUnlock();	// �t�@�C���̔r�����b�N����
	doc.docLocker.CheckWritable(!AppMode::getInstance().IsViewMode());	// �t�@�C�������\�̃`�F�b�N
	if (doc.docLocker.IsDocWritable()) {
		doc.docFileOperation.DoFileLock();	// �t�@�C���̔r�����b�N
	}

	// �e�E�B���h�E�̃^�C�g�����X�V
	GetEditWindow().UpdateCaption();
}


// �t�@�C���̃v���p�e�B
void ViewCommander::Command_Property_File(void)
{
#ifdef _DEBUG
	{
		// �S�s�f�[�^��Ԃ��e�X�g
		size_t nDataAllLen;
		RunningTimer runningTimer("ViewCommander::Command_Property_File �S�s�f�[�^��Ԃ��e�X�g");
		runningTimer.Reset();
		wchar_t* pDataAll = DocReader(GetDocument().docLineMgr).GetAllData(&nDataAllLen);
//		MYTRACE(_T("�S�f�[�^�擾             (%d�o�C�g) ���v����(�~���b) = %d\n"), nDataAllLen, runningTimer.Read());
		free(pDataAll);
		pDataAll = NULL;
//		MYTRACE(_T("�S�f�[�^�擾�̃������J�� (%d�o�C�g) ���v����(�~���b) = %d\n"), nDataAllLen, runningTimer.Read());
	}
#endif


	DlgProperty	dlgProperty;
//	cDlgProperty.Create(G_AppInstance(), view.GetHwnd(), GetDocument());
	dlgProperty.DoModal(G_AppInstance(), view.GetHwnd(), (LPARAM)&GetDocument());
	return;
}


void ViewCommander::Command_ProfileMgr( void )
{
	DlgProfileMgr profMgr;
	if (profMgr.DoModal( G_AppInstance(), view.GetHwnd(), 0 )) {
		TCHAR szOpt[MAX_PATH+10];
		auto_sprintf( szOpt, _T("-PROF=\"%ts\""), profMgr.strProfileName.c_str() );
		LoadInfo loadInfo;
		loadInfo.filePath = _T("");
		loadInfo.eCharCode = CODE_DEFAULT;
		loadInfo.bViewMode = false;
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			view.GetHwnd(),
			loadInfo,
			szOpt,
			false,
			NULL,
			false
		);
	}
}

// �ҏW�̑S�I��
void ViewCommander::Command_ExitAllEditors(void)
{
	ControlTray::CloseAllEditor(true, GetMainWindow(), true, 0);
	return;
}


// �T�N���G�f�B�^�̑S�I��
void ViewCommander::Command_ExitAll(void)
{
	ControlTray::TerminateApplication(GetMainWindow());
	return;
}


/*!	@brief �ҏW���̓��e��ʖ��ۑ�

	��ɕҏW���̈ꎞ�t�@�C���o�͂Ȃǂ̖ړI�Ɏg�p����D
	���݊J���Ă���t�@�C��(szFilePath)�ɂ͉e�����Ȃ��D

	@retval	true ����I��
	@retval	false �t�@�C���쐬�Ɏ��s
*/
bool ViewCommander::Command_PutFile(
	LPCWSTR			filename,	// [in] filename �o�̓t�@�C����
	EncodingType	nCharCode,	// [in] nCharCode �����R�[�h�w��
								//  @li CODE_xxxxxxxxxx:�e�핶���R�[�h
								//  @li CODE_AUTODETECT:���݂̕����R�[�h���ێ�
	int				nFlgOpt		// [in] nFlgOpt ����I�v�V����
								//  @li 0x01:�I��͈͂��o�� (��I����Ԃł���t�@�C�����o�͂���)
)
{
	bool bResult = true;
	EncodingType nSaveCharCode = nCharCode;
	if (filename[0] == L'\0') {
		return false;
	}

	if (nSaveCharCode == CODE_AUTODETECT) {
		nSaveCharCode = GetDocument().GetDocumentEncoding();
	}

	// EditDoc::FileWrite()�ɂȂ���č����v�J�[�\��
	WaitCursor waitCursor(view.GetHwnd());

	std::unique_ptr<CodeBase> pcSaveCode(CodeFactory::CreateCodeBase(nSaveCharCode, 0));

	bool bBom = false;
	if (CodeTypeName(nSaveCharCode).UseBom()) {
		bBom = GetDocument().GetDocumentBomExist();
	}

	if (nFlgOpt & 0x01) {	// �I��͈͂��o��
		try {
			BinaryOutputStream out(to_tchar(filename), true);

			// �I��͈͂̎擾 -> mem
			NativeW mem;
			view.GetSelectedDataSimple(mem);

			// BOM�ǉ�
			NativeW mem2;
			const NativeW* pConvBuffer;
			if (bBom) {
				NativeW memBom;
				std::unique_ptr<CodeBase> pcUtf16(CodeFactory::CreateCodeBase(CODE_UNICODE, 0));
				pcUtf16->GetBom(memBom._GetMemory());
				mem2.AppendNativeData(memBom);
				mem2.AppendNativeData(mem);
				mem.Clear();
				pConvBuffer = &mem2;
			}else {
				pConvBuffer = &mem;
			}

			// �������ݎ��̃R�[�h�ϊ� -> dst
			Memory dst;
			pcSaveCode->UnicodeToCode(*pConvBuffer, &dst);

			// ����
			if (0 < dst.GetRawLength())
				out.Write(dst.GetRawPtr(), dst.GetRawLength());
		}catch (Error_FileOpen) {
			WarningMessage(
				NULL,
				LS(STR_SAVEAGENT_OTHER_APP),
				to_tchar(filename)
			);
			bResult = false;
		}catch (Error_FileWrite) {
			WarningMessage(
				NULL,
				LS(STR_ERR_DLGEDITVWCMDNW11)
			);
			bResult = false;
		}
	}else {	// �t�@�C���S�̂��o��
		HWND	hwndProgress;
		auto&	editWnd = GetEditWindow();
		hwndProgress = editWnd.statusBar.GetProgressHwnd();
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_SHOW);
		}

		// �ꎞ�t�@�C���o��
		CodeConvertResult eRet = WriteManager().WriteFile_From_CDocLineMgr(
			GetDocument().docLineMgr,
			SaveInfo(
				to_tchar(filename),
				nSaveCharCode,
				EolType::None,
				bBom
			)
		);
		bResult = (eRet != CodeConvertResult::Failure);
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}
	return bResult;
}


/*!	@brief �J�[�\���ʒu�Ƀt�@�C����}��

	���݂̃J�[�\���ʒu�Ɏw��̃t�@�C����ǂݍ��ށD

	@param[in] filename ���̓t�@�C����
	@param[in] nCharCode �����R�[�h�w��
		@li	CODE_xxxxxxxxxx:�e�핶���R�[�h
		@li	CODE_AUTODETECT:�O�񕶎��R�[�h�������͎������ʂ̌��ʂɂ��
	@param[in] nFlgOpt ����I�v�V�����i���݂͖���`�D0���w��̂��Ɓj

	@retval	true ����I��
	@retval	false �t�@�C���I�[�v���Ɏ��s
*/
bool ViewCommander::Command_InsFile(
	LPCWSTR filename,
	EncodingType nCharCode,
	int nFlgOpt
	)
{
	FileLoad	fl;
	Eol			eol;
	int			nLineNum = 0;

	DlgCancel*	pDlgCancel = nullptr;
	HWND		hwndCancel = NULL;
	HWND		hwndProgress = NULL;
	int			nOldPercent = -1;
	bool		bResult = true;

	if (filename[0] == L'\0') {
		return false;
	}

	// EditDoc::FileLoad()�ɂȂ���č����v�J�[�\��
	WaitCursor waitCursor(view.GetHwnd());

	// �͈͑I�𒆂Ȃ�}������I����Ԃɂ��邽��
	bool bBeforeTextSelected = view.GetSelectionInfo().IsTextSelected();
	Point ptFrom;
	if (bBeforeTextSelected) {
		ptFrom = view.GetSelectionInfo().select.GetFrom();
	}


	EncodingType	nSaveCharCode = nCharCode;
	if (nSaveCharCode == CODE_AUTODETECT) {
		EditInfo    fi;
		const MruFile  mru;
		if (mru.GetEditInfo(to_tchar(filename), &fi)) {
				nSaveCharCode = fi.nCharCode;
		}else {
			nSaveCharCode = GetDocument().GetDocumentEncoding();
		}
	}

	// �����܂ł��ĕ����R�[�h�����肵�Ȃ��Ȃ�ǂ�����������
	if (!IsValidCodeType(nSaveCharCode)) nSaveCharCode = CODE_SJIS;

	try {
		bool bBigFile;
#ifdef _WIN64
		bBigFile = true;
#else
		bBigFile = false;
#endif
		// �t�@�C�����J��
		fl.FileOpen(view.pTypeData->encoding, to_tchar(filename), bBigFile, nSaveCharCode, 0 );

		// �t�@�C���T�C�Y��65KB���z������i���_�C�A���O�\��
		if (0x10000 < fl.GetFileSize()) {
			pDlgCancel = new DlgCancel;
			if ((hwndCancel = pDlgCancel->DoModeless(::GetModuleHandle(NULL), NULL, IDD_OPERATIONRUNNING))) {
				hwndProgress = ::GetDlgItem(hwndCancel, IDC_PROGRESS);
				Progress_SetRange(hwndProgress, 0, 101);
				Progress_SetPos(hwndProgress, 0);
			}
		}

		// ReadLine�̓t�@�C������ �����R�[�h�ϊ����ꂽ1�s��ǂݏo���܂�
		// �G���[����throw Error_FileRead �𓊂��܂�
		NativeW buf;
		while (CodeConvertResult::Failure != fl.ReadLine(&buf, &eol)) {

			const wchar_t* pLine = buf.GetStringPtr();
			size_t nLineLen = buf.GetStringLength();

			++nLineNum;
			Command_InsText(false, pLine, nLineLen, true);

			// �i���_�C�A���O�L��
			if (!pDlgCancel) {
				continue;
			}
			// �������̃��[�U�[������\�ɂ���
			if (!::BlockingHook(pDlgCancel->GetHwnd())) {
				break;
			}
			// ���f�{�^�������`�F�b�N
			if (pDlgCancel->IsCanceled()) {
				break;
			}
			if ((nLineNum & 0xFF) == 0) {
				if (nOldPercent != fl.GetPercent()) {
					Progress_SetPos(hwndProgress, fl.GetPercent() + 1);
					Progress_SetPos(hwndProgress, fl.GetPercent());
					nOldPercent = fl.GetPercent();
				}
				view.Redraw();
			}
		}
		// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
		fl.FileClose();
	}catch (Error_FileOpen) {
		WarningMessage(NULL, LS(STR_GREP_ERR_FILEOPEN), to_tchar(filename));
		bResult = false;
	}catch (Error_FileRead) {
		WarningMessage(NULL, LS(STR_ERR_DLGEDITVWCMDNW12));
		bResult = false;
	} // ��O�����I���

	delete pDlgCancel;

	if (bBeforeTextSelected) {	// �}�����ꂽ������I����Ԃ�
		view.GetSelectionInfo().SetSelectArea(
			Range(
				ptFrom,
				GetCaret().GetCaretLayoutPos()
				/*
				nCaretPosY, nCaretPosX
				*/
			)
		);
		view.GetSelectionInfo().DrawSelectArea();
	}
	view.Redraw();
	return bResult;
}

