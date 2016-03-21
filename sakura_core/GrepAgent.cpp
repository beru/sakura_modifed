#include "StdAfx.h"
#include "GrepAgent.h"
#include "GrepEnumKeys.h"
#include "GrepEnumFilterFiles.h"
#include "GrepEnumFilterFolders.h"
#include "SearchAgent.h"
#include "dlg/DlgCancel.h"
#include "_main/AppMode.h"
#include "OpeBlk.h"
#include "window/EditWnd.h"
#include "charset/CodeMediator.h"
#include "view/colors/ColorStrategy.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "charset/CodePage.h"
#include "io/FileLoad.h"
#include "io/BinaryStream.h"
#include "util/window.h"
#include "util/module.h"
#include "debug/RunningTimer.h"
#include <deque>
#include <memory>

#include "sakura_rc.h"

GrepAgent::GrepAgent()
	:
	m_bGrepMode(false),		// Grep���[�h��
	m_bGrepRunning(false)	// Grep������
{
}

CallbackResultType GrepAgent::OnBeforeClose()
{
	// GREP�������͏I���ł��Ȃ�
	if (m_bGrepRunning) {
		// �A�N�e�B�u�ɂ���
		ActivateFrameWindow(EditWnd::getInstance().GetHwnd());	//@@@ 2003.06.25 MIK
		TopInfoMessage(
			EditWnd::getInstance().GetHwnd(),
			LS(STR_GREP_RUNNINNG)
		);
		return CallbackResultType::Interrupt;
	}
	return CallbackResultType::Continue;
}

void GrepAgent::OnAfterSave(const SaveInfo& saveInfo)
{
	// ���O��t���ĕۑ�����ă��[�h���������ꂽ���̕s��������ǉ��iANSI�łƂ̍��فj	// 2009.08.12 ryoji
	m_bGrepMode = false;	// grep�E�B���h�E�͒ʏ�E�B���h�E��
	AppMode::getInstance().m_szGrepKey[0] = 0;
}

/*!
	@date 2014.03.09 novice �Ō��\\����菜���̂���߂�(d:\\ -> d:�ɂȂ�)
*/
void GrepAgent::CreateFolders(const TCHAR* pszPath, std::vector<std::tstring>& vPaths)
{
	const int nPathLen = auto_strlen(pszPath);
	auto szPath = std::make_unique<TCHAR[]>(nPathLen + 1);
	auto szTmp = std::make_unique<TCHAR[]>(nPathLen + 1);
	auto_strcpy(&szPath[0], pszPath);
	TCHAR* token;
	int nPathPos = 0;
	while (token = my_strtok<TCHAR>(&szPath[0], nPathLen, &nPathPos, _T(";"))) {
		auto_strcpy(&szTmp[0], token);
		TCHAR* p;
		TCHAR* q;
		p = q = &szTmp[0];
		while (*p) {
			if (*p != _T('"')) {
				if (p != q) {
					*q = *p;
				}
				++q;
			}
			++p;
		}
		*q = _T('\0');
#if 0
		// 2011.12.25 �d�l�ύX�B�Ō��\\�͎�菜��
		int	nFolderLen = q - &szTmp[0];
		if (0 < nFolderLen) {
			int nCharChars = &szTmp[nFolderLen] - CNativeT::GetCharPrev(&szTmp[0], nFolderLen, &szTmp[nFolderLen]);
			if (nCharChars == 1 && (_T('\\') == szTmp[nFolderLen - 1] || _T('/') == szTmp[nFolderLen - 1])) {
				szTmp[nFolderLen - 1] = _T('\0');
			}
		}
#endif
		// �����O�t�@�C�������擾����
		TCHAR szTmp2[_MAX_PATH];
		if (::GetLongFileName(&szTmp[0], szTmp2)) {
			vPaths.push_back(szTmp2);
		}else {
			vPaths.push_back(&szTmp[0]);
		}
	}
}

/*! �Ō��\\����菜��
	@date 2014.03.09 novice �V�K�쐬
*/
std::tstring GrepAgent::ChopYen( const std::tstring& str )
{
	std::tstring dst = str;
	size_t nPathLen = dst.length();

	// �Ō�̃t�H���_��؂�L�����폜����
	// [A:\]�Ȃǂ̃��[�g�ł����Ă��폜
	for (size_t i=0; i<nPathLen; ++i) {
		if (1
			&& dst[i] == _T('\\')
			&& i == nPathLen - 1
		) {
			dst.resize( nPathLen - 1 );
			break;
		}
	}

	return dst;
}

void GrepAgent::AddTail( EditView& editView, const NativeW& mem, bool bAddStdout )
{
	if (bAddStdout) {
		HANDLE out = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (out && out != INVALID_HANDLE_VALUE) {
			Memory memOut;
			std::unique_ptr<CodeBase> pCodeBase( CodeFactory::CreateCodeBase(
					editView.GetDocument().GetDocumentEncoding(), 0) );
			pCodeBase->UnicodeToCode( mem, &memOut );
			DWORD dwWrite = 0;
			::WriteFile(out, memOut.GetRawPtr(), memOut.GetRawLength(), &dwWrite, NULL);
		}
	}else {
		editView.GetCommander().Command_ADDTAIL( mem.GetStringPtr(), mem.GetStringLength() );
	}
}

/*! Grep���s

  @param[in] pmGrepKey �����p�^�[��
  @param[in] pmGrepFile �����Ώۃt�@�C���p�^�[��(!�ŏ��O�w��))
  @param[in] pmGrepFolder �����Ώۃt�H���_

  @date 2008.12.07 nasukoji	�t�@�C�����p�^�[���̃o�b�t�@�I�[�o�����΍�
  @date 2008.12.13 genta �����p�^�[���̃o�b�t�@�I�[�o�����΍�
  @date 2012.10.13 novice �����I�v�V�������N���X���Ƒ��
*/
DWORD GrepAgent::DoGrep(
	EditView&				viewDst,
	bool					bGrepReplace,
	const NativeW*			pmGrepKey,
	const NativeW*			pmGrepReplace,
	const NativeT*			pmGrepFile,
	const NativeT*			pmGrepFolder,
	bool					bGrepCurFolder,
	bool					bGrepSubFolder,
	bool					bGrepStdout,
	bool					bGrepHeader,
	const SearchOption&		searchOption,
	EncodingType			nGrepCharSet,	// 2002/09/21 Moca �����R�[�h�Z�b�g�I��
	int						nGrepOutputLineType,
	int						nGrepOutputStyle,
	bool					bGrepOutputFileOnly,
	bool					bGrepOutputBaseFolder,
	bool					bGrepSeparateFolder,
	bool					bGrepPaste,
	bool					bGrepBackup
	)
{
#ifdef _DEBUG
	RunningTimer runningTimer("EditView::DoGrep");
#endif

	// �ē��s��
	if (this->m_bGrepRunning) {
		assert_warning(!this->m_bGrepRunning);
		return 0xffffffff;
	}

	this->m_bGrepRunning = true;

	int			nHitCount = 0;
	DlgCancel	dlgCancel;
	HWND		hwndCancel;
	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	Bregexp		regexp;
	NativeW		memMessage;
	int			nWork;
	GrepOption	grepOption;

	/*
	|| �o�b�t�@�T�C�Y�̒���
	*/
	memMessage.AllocStringBuffer(4000);

	viewDst.m_bDoing_UndoRedo		= true;


	// �A���h�D�o�b�t�@�̏���
	if (viewDst.GetDocument().m_docEditor.m_pOpeBlk) {	// ����u���b�N
//@@@2002.2.2 YAZAKI NULL����Ȃ��Ɛi�܂Ȃ��̂ŁA�Ƃ肠�����R�����g�B��NULL�̂Ƃ��́Anew OpeBlk����B
//		while (m_pOpeBlk) {}
//		delete m_pOpeBlk;
//		m_pOpeBlk = NULL;
	}else {
		viewDst.GetDocument().m_docEditor.m_pOpeBlk = new OpeBlk;
		viewDst.GetDocument().m_docEditor.m_nOpeBlkRedawCount = 0;
	}
	viewDst.GetDocument().m_docEditor.m_pOpeBlk->AddRef();

	viewDst.m_bCurSrchKeyMark = true;								// ����������̃}�[�N
	viewDst.m_strCurSearchKey = pmGrepKey->GetStringPtr();		// ����������
	viewDst.m_curSearchOption = searchOption;						// �����I�v�V����
	viewDst.m_nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;

	// �u���㕶����̏���
	NativeW memReplace;
	if (bGrepReplace) {
		if (bGrepPaste) {
			// ��`�E���C�����[�h�\��t���͖��T�|�[�g
			bool bColmnSelect;
			bool bLineSelect;
			if (!viewDst.MyGetClipboardData(memReplace, &bColmnSelect, GetDllShareData().common.edit.bEnableLineModePaste? &bLineSelect: NULL)) {
				this->m_bGrepRunning = false;
				viewDst.m_bDoing_UndoRedo = false;
				ErrorMessage( viewDst.m_hwndParent, LS(STR_DLGREPLC_CLIPBOARD) );
				return 0;
			}
			if (bLineSelect) {
				int len = memReplace.GetStringLength();
				if (memReplace[len - 1] != WCODE::CR && memReplace[len - 1] != WCODE::LF) {
					memReplace.AppendString(viewDst.GetDocument().m_docEditor.GetNewLineCode().GetValue2());
				}
			}
			if (GetDllShareData().common.edit.bConvertEOLPaste) {
				LogicInt len = memReplace.GetStringLength();
				std::vector<wchar_t> convertedText(len * 2); // �S����\n��\r\n�ϊ��ōő�̂Q�{�ɂȂ�
				wchar_t* pszConvertedText = &convertedText[0];
				LogicInt nConvertedTextLen = viewDst.m_commander.ConvertEol(memReplace.GetStringPtr(), len, pszConvertedText);
				memReplace.SetString(pszConvertedText, nConvertedTextLen);
			}
		}else {
			memReplace = *pmGrepReplace;
		}
	}	// ���K�\��

	//	From Here Jun. 27 genta
	/*
		Grep���s���ɓ������Č����E��ʐF�����p���K�\���o�b�t�@��
		����������D�����Grep�������ʂ̐F�������s�����߁D

		Note: �����ŋ�������͍̂Ō�̌���������ł�����
		Grep�Ώۃp�^�[���ł͂Ȃ����Ƃɒ���
	*/
	if (!viewDst.m_searchPattern.SetPattern(viewDst.GetHwnd(),
												viewDst.m_strCurSearchKey.c_str(),
												viewDst.m_strCurSearchKey.size(),
												viewDst.m_curSearchOption,
												&viewDst.m_curRegexp)
	) {
		this->m_bGrepRunning = false;
		viewDst.m_bDoing_UndoRedo = false;
		viewDst.SetUndoBuffer();
		return 0;
	}

	// 2014.06.13 �ʃE�B���h�E�Ō��������Ƃ��p��Grep�_�C�A���O�̌����L�[��ݒ�
	viewDst.m_editWnd.m_dlgGrep.m_strText = pmGrepKey->GetStringPtr();
	viewDst.m_editWnd.m_dlgGrep.m_bSetText = true;
	viewDst.m_editWnd.m_dlgGrepReplace.m_strText = pmGrepKey->GetStringPtr();
	if (bGrepReplace) {
		viewDst.m_editWnd.m_dlgGrepReplace.m_strText2 = pmGrepReplace->GetStringPtr();
	}
	viewDst.m_editWnd.m_dlgGrepReplace.m_bSetText = true;
	hwndCancel = dlgCancel.DoModeless(G_AppInstance(), viewDst.m_hwndParent, IDD_GREPRUNNING);

	::SetDlgItemInt(hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE);
	::DlgItem_SetText(hwndCancel, IDC_STATIC_CURFILE, _T(" "));	// 2002/09/09 Moca add
	::CheckDlgButton(hwndCancel, IDC_CHECK_REALTIMEVIEW, GetDllShareData().common.search.bGrepRealTimeView);	// 2003.06.23 Moca

	//	2008.12.13 genta �p�^�[������������ꍇ�͓o�^���Ȃ�
	//	(���K�\�����r���œr�؂��ƍ���̂�)
	//	2011.12.10 Moca �\���̍ۂ�...�ɐ؂�̂Ă���̂œo�^����悤��
	wcsncpy_s(AppMode::getInstance().m_szGrepKey, _countof(AppMode::getInstance().m_szGrepKey), pmGrepKey->GetStringPtr(), _TRUNCATE);
	this->m_bGrepMode = true;

	//	2007.07.22 genta
	//	�o�[�W�����ԍ��擾�̂��߁C������O�̕��ֈړ�����
	SearchStringPattern pattern;
	{
		// �����p�^�[���̃R���p�C��
		bool bError;
		if (bGrepReplace && !bGrepPaste) {
			// Grep�u��
			bError = !pattern.SetPattern(viewDst.GetHwnd(),
										pmGrepKey->GetStringPtr(),
										pmGrepKey->GetStringLength(),
										memReplace.GetStringPtr(),
										searchOption,
										&regexp);
		}else {
			bError = !pattern.SetPattern(viewDst.GetHwnd(),
										pmGrepKey->GetStringPtr(),
										pmGrepKey->GetStringLength(),
										searchOption,
										&regexp);
		}
		if (bError) {
			this->m_bGrepRunning = false;
			viewDst.m_bDoing_UndoRedo = false;
			viewDst.SetUndoBuffer();
			return 0;
		}
	}
	
	// Grep�I�v�V�����܂Ƃ�
	grepOption.bGrepSubFolder = bGrepSubFolder;
	grepOption.bGrepStdout = bGrepStdout;
	grepOption.bGrepHeader = bGrepHeader;
	grepOption.nGrepCharSet = nGrepCharSet;
	grepOption.nGrepOutputLineType = nGrepOutputLineType;
	grepOption.nGrepOutputStyle = nGrepOutputStyle;
	grepOption.bGrepOutputFileOnly = bGrepOutputFileOnly;
	grepOption.bGrepOutputBaseFolder = bGrepOutputBaseFolder;
	grepOption.bGrepSeparateFolder = bGrepSeparateFolder;
	grepOption.bGrepReplace = bGrepReplace;
	grepOption.bGrepPaste = bGrepPaste;
	grepOption.bGrepBackup = bGrepBackup;
	if (grepOption.bGrepReplace) {
		// Grep�ے�s��Grep�u���ł͖���
		if (grepOption.nGrepOutputLineType == 2) {
			grepOption.nGrepOutputLineType = 1; // �s�P��
		}
	}

	// 2002.02.08 Grep�A�C�R�����傫���A�C�R���Ə������A�C�R����ʁX�ɂ���B
	HICON	hIconBig, hIconSmall;
	// Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	hIconBig   = GetAppIcon(G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, false);
	hIconSmall = GetAppIcon(G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, true);

	// Sep. 10, 2002 genta
	// EditWnd�ɐV�݂����֐����g���悤��
	auto& editWnd = EditWnd::getInstance();	//	Sep. 10, 2002 genta
	editWnd.SetWindowIcon(hIconSmall, ICON_SMALL);
	editWnd.SetWindowIcon(hIconBig, ICON_BIG);

	GrepEnumKeys grepEnumKeys;
	{
		int nErrorNo = grepEnumKeys.SetFileKeys(pmGrepFile->GetStringPtr());
		if (nErrorNo != 0) {
			this->m_bGrepRunning = false;
			viewDst.m_bDoing_UndoRedo = false;
			viewDst.SetUndoBuffer();

			const TCHAR* pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS0);
			if (nErrorNo == 1) {
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS1);
			}else if (nErrorNo == 2) {
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS2);
			}
			ErrorMessage(viewDst.m_hwndParent, _T("%ts"), pszErrorMessage);
			return 0;
		}
	}

	std::vector<std::tstring> vPaths;
	CreateFolders(pmGrepFolder->GetStringPtr(), vPaths);

	nWork = pmGrepKey->GetStringLength(); // 2003.06.10 Moca ���炩���ߒ������v�Z���Ă���

	// �Ō�Ƀe�L�X�g��ǉ�
	NativeW memWork;
	memMessage.AppendString(LSW(STR_GREP_SEARCH_CONDITION));	//L"\r\n����������  "
	if (0 < nWork) {
		NativeW memWork2;
		memWork2.SetNativeData(*pmGrepKey);
		const TypeConfig& type = viewDst.m_pEditDoc->m_docType.GetDocumentAttribute();
		if (!type.colorInfoArr[COLORIDX_WSTRING].bDisp) {
			// 2011.11.28 �F�w�肪�����Ȃ�G�X�P�[�v���Ȃ�
		}else
		if (type.stringType == StringLiteralType::CPP
			|| type.stringType == StringLiteralType::CSharp
			|| type.stringType == StringLiteralType::Python
		) {	// �������؂�L���G�X�P�[�v���@
			memWork2.Replace(L"\\", L"\\\\");
			memWork2.Replace(L"\'", L"\\\'");
			memWork2.Replace(L"\"", L"\\\"");
		}else if (type.stringType == StringLiteralType::PLSQL) {
			memWork2.Replace(L"\'", L"\'\'");
			memWork2.Replace(L"\"", L"\"\"");
		}
		memWork.AppendString(L"\"");
		memWork.AppendNativeData(memWork2);
		memWork.AppendString(L"\"\r\n");
	}else {
		memWork.AppendString(LSW(STR_GREP_SEARCH_FILE));	// L"�u�t�@�C�������v\r\n"
	}
	memMessage += memWork;

	if (bGrepReplace) {
		memMessage.AppendString( LSW(STR_GREP_REPLACE_TO) );
		if (bGrepPaste) {
			memMessage.AppendString( LSW(STR_GREP_PASTE_CLIPBOAD) );
		}else {
			NativeW memWork2;
			memWork2.SetNativeData( memReplace );
			const TypeConfig& type = viewDst.m_pEditDoc->m_docType.GetDocumentAttribute();
			if (!type.colorInfoArr[COLORIDX_WSTRING].bDisp) {
				// 2011.11.28 �F�w�肪�����Ȃ�G�X�P�[�v���Ȃ�
			}else
			if (0
				|| type.stringType == StringLiteralType::CPP
				|| type.stringType == StringLiteralType::CSharp
				|| type.stringType == StringLiteralType::Python
			) {	// �������؂�L���G�X�P�[�v���@
				memWork2.Replace( L"\\", L"\\\\" );
				memWork2.Replace( L"\'", L"\\\'" );
				memWork2.Replace( L"\"", L"\\\"" );
			}else if (type.stringType == StringLiteralType::PLSQL) {
				memWork2.Replace( L"\'", L"\'\'" );
				memWork2.Replace( L"\"", L"\"\"" );
			}
			memMessage.AppendString( L"\"" );
			memMessage.AppendNativeData( memWork2 );
			memMessage.AppendString( L"\"\r\n" );
		}
	}


	memMessage.AppendString(LSW(STR_GREP_SEARCH_TARGET));	// L"�����Ώ�   "
	if (viewDst.m_pEditDoc->m_docType.GetDocumentAttribute().stringType == StringLiteralType::CPP) {	// �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""]['']
	}else {
	}
	memWork.SetStringT(pmGrepFile->GetStringPtr());
	memMessage += memWork;

	memMessage.AppendString(L"\r\n");
	memMessage.AppendString(LSW(STR_GREP_SEARCH_FOLDER));	// L"�t�H���_   "
	{
		std::tstring grepFolder;
		for (int i=0; i<(int)vPaths.size(); ++i) {
			if (i) {
				grepFolder += _T(';');
			}
			std::tstring sPath = ChopYen( vPaths[i] );
			if (auto_strchr( sPath.c_str(), _T(';') )) {
				grepFolder += _T('"');
				grepFolder += sPath;
				grepFolder += _T('"');
			}else {
				grepFolder += sPath;
			}
		}
		memWork.SetStringT(grepFolder.c_str());
	}
	if (viewDst.m_pEditDoc->m_docType.GetDocumentAttribute().stringType == StringLiteralType::CPP) {	// �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""]['']
	}else {
	}
	memMessage += memWork;
	memMessage.AppendString(L"\r\n");

	const wchar_t*	pszWork;
	if (grepOption.bGrepSubFolder) {
		pszWork = LSW(STR_GREP_SUBFOLDER_YES);	// L"    (�T�u�t�H���_������)\r\n"
	}else {
		pszWork = LSW(STR_GREP_SUBFOLDER_NO);	// L"    (�T�u�t�H���_���������Ȃ�)\r\n"
	}
	memMessage.AppendString(pszWork);

	if (0 < nWork) { // 2003.06.10 Moca �t�@�C�������̏ꍇ�͕\�����Ȃ� // 2004.09.26 �������C��
		if (searchOption.bWordOnly) {
		// �P��P�ʂŒT��
			memMessage.AppendString(LSW(STR_GREP_COMPLETE_WORD));	// L"    (�P��P�ʂŒT��)\r\n"
		}

		if (searchOption.bLoHiCase) {
			pszWork = LSW(STR_GREP_CASE_SENSITIVE);	// L"    (�p�啶������������ʂ���)\r\n"
		}else {
			pszWork = LSW(STR_GREP_IGNORE_CASE);	// L"    (�p�啶������������ʂ��Ȃ�)\r\n"
		}
		memMessage.AppendString(pszWork);

		if (searchOption.bRegularExp) {
			//	2007.07.22 genta : ���K�\�����C�u�����̃o�[�W�������o�͂���
			memMessage.AppendString(LSW(STR_GREP_REGEX_DLL));	// L"    (���K�\��:"
			memMessage.AppendStringT(regexp.GetVersionT());
			memMessage.AppendString(L")\r\n");
		}
	}

	if (grepOption.nGrepCharSet == CODE_AUTODETECT) {
		memMessage.AppendString(LSW(STR_GREP_CHARSET_AUTODETECT));	// L"    (�����R�[�h�Z�b�g�̎�������)\r\n"
	}else if (IsValidCodeOrCPType(grepOption.nGrepCharSet)) {
		memMessage.AppendString(LSW(STR_GREP_CHARSET));	// L"    (�����R�[�h�Z�b�g�F"
		TCHAR szCpName[100];
		CodePage::GetNameNormal(szCpName, grepOption.nGrepCharSet);
		memMessage.AppendStringT( szCpName );
		memMessage.AppendString(L")\r\n");
	}

	if (0 < nWork) { // 2003.06.10 Moca �t�@�C�������̏ꍇ�͕\�����Ȃ� // 2004.09.26 �������C��
		if (grepOption.nGrepOutputLineType == 1) {
			// �Y���s
			pszWork = LSW(STR_GREP_SHOW_MATCH_LINE);	// L"    (��v�����s���o��)\r\n"
		}else if (grepOption.nGrepOutputLineType == 2) {
			// �ۊY���s
			pszWork = LSW( STR_GREP_SHOW_MATCH_NOHITLINE );	//L"    (��v���Ȃ������s���o��)\r\n"
		}else {
			if (bGrepReplace && searchOption.bRegularExp && !bGrepPaste) {
				pszWork = LSW(STR_GREP_SHOW_FIRST_LINE);
			}else {
				pszWork = LSW( STR_GREP_SHOW_MATCH_AREA );
			}
		}
		memMessage.AppendString(pszWork);

		if (grepOption.bGrepOutputFileOnly) {
			pszWork = LSW(STR_GREP_SHOW_FIRST_MATCH);	// L"    (�t�@�C�����ŏ��̂݌���)\r\n"
			memMessage.AppendString(pszWork);
		}
	}

	memMessage.AppendString(L"\r\n\r\n");
	pszWork = memMessage.GetStringPtr(&nWork);
//@@@ 2002.01.03 YAZAKI Grep����̓J�[�\����Grep���O�̈ʒu�ɓ�����
	LayoutInt tmp_PosY_Layout = viewDst.m_pEditDoc->m_layoutMgr.GetLineCount();
	if (0 < nWork && grepOption.bGrepHeader) {
		AddTail( viewDst, memMessage, grepOption.bGrepStdout );
	}
	memMessage.Clear(); // ��������Ȃ�
	pszWork = NULL;
	
	//	2007.07.22 genta �o�[�W�������擾���邽�߂ɁC
	//	���K�\���̏���������ֈړ�

	// �\������ON/OFF
	// 2003.06.23 Moca ���ʐݒ�ŕύX�ł���悤��
	// 2008.06.08 ryoji �S�r���[�̕\��ON/OFF�𓯊�������
//	SetDrawSwitch(false);
	if (!EditWnd::getInstance().UpdateTextWrap()) {		// �܂�Ԃ����@�֘A�̍X�V
		EditWnd::getInstance().RedrawAllViews(&viewDst);	// ���̃y�C���̕\�����X�V
	}
	const bool bDrawSwitchOld = viewDst.SetDrawSwitch(GetDllShareData().common.search.bGrepRealTimeView != 0);

	GrepEnumOptions grepEnumOptions;
	GrepEnumFiles grepExceptAbsFiles;
	grepExceptAbsFiles.Enumerates(_T(""), grepEnumKeys.m_vecExceptAbsFileKeys, grepEnumOptions);
	GrepEnumFolders grepExceptAbsFolders;
	grepExceptAbsFolders.Enumerates(_T(""), grepEnumKeys.m_vecExceptAbsFolderKeys, grepEnumOptions);

	int nGrepTreeResult = 0;

	for (int nPath=0; nPath<(int)vPaths.size(); ++nPath) {
		bool bOutputBaseFolder = false;
		std::tstring sPath = ChopYen( vPaths[nPath] );
		int nTreeRet = DoGrepTree(
			viewDst,
			dlgCancel,
			pmGrepKey->GetStringPtr(),
			memReplace,
			grepEnumKeys,
			grepExceptAbsFiles,
			grepExceptAbsFolders,
			sPath.c_str(),
			sPath.c_str(),
			searchOption,
			grepOption,
			pattern,
			regexp,
			0,
			bOutputBaseFolder,
			&nHitCount
		);
		if (nTreeRet == -1) {
			nGrepTreeResult = -1;
			break;
		}
		nGrepTreeResult += nTreeRet;
	}
	if (nGrepTreeResult == -1 && grepOption.bGrepHeader) {
		const wchar_t* p = LSW(STR_GREP_SUSPENDED);	// L"���f���܂����B\r\n"
		NativeW memSuspend;
		memSuspend.SetString( p );
		AddTail( viewDst, memSuspend, grepOption.bGrepStdout );
	}
	if (grepOption.bGrepHeader) {
		WCHAR szBuffer[128];
		if (bGrepReplace) {
			auto_sprintf( szBuffer, LSW(STR_GREP_REPLACE_COUNT), nHitCount );
		}else {
			auto_sprintf( szBuffer, LSW( STR_GREP_MATCH_COUNT ), nHitCount );
		}
		NativeW memOutput;
		memOutput.SetString( szBuffer );
		AddTail( viewDst, memOutput, grepOption.bGrepStdout );
#ifdef _DEBUG
		auto_sprintf( szBuffer, LSW(STR_GREP_TIMER), runningTimer.Read() );
		memOutput.SetString( szBuffer );
		AddTail( viewDst, memOutput, grepOption.bGrepStdout );
#endif
	}
	viewDst.GetCaret().MoveCursor(LayoutPoint(LayoutInt(0), tmp_PosY_Layout), true);	// �J�[�\����Grep���O�̈ʒu�ɖ߂��B

	dlgCancel.CloseDialog(0);

	// �A�N�e�B�u�ɂ���
	ActivateFrameWindow(EditWnd::getInstance().GetHwnd());

	// �A���h�D�o�b�t�@�̏���
	viewDst.SetUndoBuffer();

	// Apr. 13, 2001 genta
	// Grep���s��̓t�@�C����ύX�����̏�Ԃɂ���D
	viewDst.m_pEditDoc->m_docEditor.SetModified(false, false);

	this->m_bGrepRunning = false;
	viewDst.m_bDoing_UndoRedo = false;

	// �\������ON/OFF
	editWnd.SetDrawSwitchOfAllViews(bDrawSwitchOld);

	// �ĕ`��
	if (!editWnd.UpdateTextWrap()) {	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
		editWnd.RedrawAllViews(NULL);
	}

	if (!bGrepCurFolder) {
		// ���s�t�H���_�����������t�H���_�ɕύX
		if (0 < vPaths.size()) {
			::SetCurrentDirectory(vPaths[0].c_str());
		}
	}

	return nHitCount;
}


/*! @brief Grep���s

	@date 2001.06.27 genta	���K�\�����C�u�����̍����ւ�
	@date 2003.06.23 Moca   �T�u�t�H���_���t�@�C���������̂��t�@�C�����T�u�t�H���_�̏��ɕύX
	@date 2003.06.23 Moca   �t�@�C��������""����菜���悤��
	@date 2003.03.27 �݂�   ���O�t�@�C���w��̓����Əd�������h�~�̒ǉ��D
		�啔�����ύX���ꂽ���߁C�ʂ̕ύX�_�L���͖����D
*/
int GrepAgent::DoGrepTree(
	EditView&				viewDst,
	DlgCancel&				dlgCancel,				// [in] Cancel�_�C�A���O�ւ̃|�C���^
	const wchar_t*			pszKey,					// [in] �����L�[
	const NativeW&			mGrepReplace,
	const GrepEnumKeys&		grepEnumKeys,			// [in] �����Ώۃt�@�C���p�^�[��
	GrepEnumFiles&			grepExceptAbsFiles,		// [in] ���O�t�@�C����΃p�X
	GrepEnumFolders&		grepExceptAbsFolders,	// [in] ���O�t�H���_��΃p�X
	const TCHAR*			pszPath,				// [in] �����Ώۃp�X
	const TCHAR*			pszBasePath,			// [in] �����Ώۃp�X(�x�[�X�t�H���_)
	const SearchOption&		searchOption,			// [in] �����I�v�V����
	const GrepOption&		grepOption,				// [in] Grep�I�v�V����
	const SearchStringPattern& pattern,				// [in] �����p�^�[��
	Bregexp&				regexp,					// [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
	int						nNest,					// [in] �l�X�g���x��
	bool&					bOutputBaseFolder,		// [i/o] �x�[�X�t�H���_���o��
	int*					pnHitCount				// [i/o] �q�b�g���̍��v
	)
{
	dlgCancel.SetItemText(IDC_STATIC_CURPATH, pszPath);

	NativeW	memMessage;
	int			nWork = 0;
	int			nHitCountOld = -100;
	bool		bOutputFolderName = false;
	int			nBasePathLen = auto_strlen(pszBasePath);
	GrepEnumOptions grepEnumOptions;
	GrepEnumFilterFiles grepEnumFilterFiles;
	grepEnumFilterFiles.Enumerates( pszPath, grepEnumKeys, grepEnumOptions, grepExceptAbsFiles );

	/*
	 * �J�����g�t�H���_�̃t�@�C����T������B
	 */
	int count = grepEnumFilterFiles.GetCount();
	for (int i=0; i<count; ++i) {
		LPCTSTR lpFileName = grepEnumFilterFiles.GetFileName(i);

		// �������̃��[�U�[������\�ɂ���
		if (!::BlockingHook(dlgCancel.GetHwnd())) {
			goto cancel_return;
		}
		// ���f�{�^�������`�F�b�N
		if (dlgCancel.IsCanceled()) {
			goto cancel_return;
		}

		// �\���ݒ���`�F�b�N
		EditWnd::getInstance().SetDrawSwitchOfAllViews(
			dlgCancel.IsButtonChecked(IDC_CHECK_REALTIMEVIEW)
		);

		// GREP���s�I
		dlgCancel.SetItemText(IDC_STATIC_CURFILE, lpFileName);

		std::tstring currentFile = pszPath;
		currentFile += _T("\\");
		currentFile += lpFileName;
		int nBasePathLen2 = nBasePathLen + 1;
		if ((int)auto_strlen(pszPath) < nBasePathLen2) {
			nBasePathLen2 = nBasePathLen;
		}

		// �t�@�C�����̌���
		int nRet;
		if (grepOption.bGrepReplace) {
			nRet = DoGrepReplaceFile(
				viewDst,
				dlgCancel,
				pszKey,
				mGrepReplace,
				lpFileName,
				searchOption,
				grepOption,
				pattern,
				regexp,
				pnHitCount,
				currentFile.c_str(),
				pszBasePath,
				(grepOption.bGrepSeparateFolder && grepOption.bGrepOutputBaseFolder ? pszPath + nBasePathLen2 : pszPath),
				(grepOption.bGrepSeparateFolder ? lpFileName : currentFile.c_str() + nBasePathLen + 1),
				bOutputBaseFolder,
				bOutputFolderName,
				memMessage
			);
		}else {
			nRet = DoGrepFile(
				viewDst,
				dlgCancel,
				pszKey,
				lpFileName,
				searchOption,
				grepOption,
				pattern,
				regexp,
				pnHitCount,
				currentFile.c_str(),
				pszBasePath,
				(grepOption.bGrepSeparateFolder && grepOption.bGrepOutputBaseFolder ? pszPath + nBasePathLen2 : pszPath),
				(grepOption.bGrepSeparateFolder ? lpFileName : currentFile.c_str() + nBasePathLen + 1),
				bOutputBaseFolder,
				bOutputFolderName,
				memMessage
			);
		}

		// 2003.06.23 Moca ���A���^�C���\���̂Ƃ��͑��߂ɕ\��
		if (viewDst.GetDrawSwitch()) {
			if (pszKey[0] != LTEXT('\0')) {
				// �f�[�^�����̂Ƃ��t�@�C���̍��v���ő�10MB�𒴂�����\��
				nWork += (grepEnumFilterFiles.GetFileSizeLow(i) + 1023) / 1024;
			}
			if (*pnHitCount - nHitCountOld && 
				(*pnHitCount < 20 || 10000 < nWork)
			) {
				nHitCountOld = -100; // ���\��
			}
		}
		if (*pnHitCount - nHitCountOld  >= 10) {
			// ���ʏo��
			if (0 < memMessage.GetStringLength()) {
				AddTail( viewDst, memMessage, grepOption.bGrepStdout );
				viewDst.GetCommander().Command_GOFILEEND(false);
				if (!EditWnd::getInstance().UpdateTextWrap()) {		// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
					EditWnd::getInstance().RedrawAllViews(&viewDst);	//	���̃y�C���̕\�����X�V
				}
				memMessage.Clear();
			}
			nWork = 0;
			nHitCountOld = *pnHitCount;
		}
		if (nRet == -1) {
			goto cancel_return;
		}
	}

	// 2010.08.25 �t�H���_�ړ��O�Ɏc����ɏo��
	if (0 < memMessage.GetStringLength()) {
		AddTail( viewDst, memMessage, grepOption.bGrepStdout );
		viewDst.GetCommander().Command_GOFILEEND(false);
		if (!EditWnd::getInstance().UpdateTextWrap()) {		// �܂�Ԃ����@�֘A�̍X�V
			EditWnd::getInstance().RedrawAllViews(&viewDst);	//	���̃y�C���̕\�����X�V
		}
		memMessage.Clear();
	}

	/*
	 * �T�u�t�H���_����������B
	 */
	if (grepOption.bGrepSubFolder) {
		GrepEnumOptions grepEnumOptionsDir;
		GrepEnumFilterFolders grepEnumFilterFolders;
		grepEnumFilterFolders.Enumerates( pszPath, grepEnumKeys, grepEnumOptionsDir, grepExceptAbsFolders );

		int count = grepEnumFilterFolders.GetCount();
		for (int i=0; i<count; ++i) {
			LPCTSTR lpFileName = grepEnumFilterFolders.GetFileName(i);

			// �T�u�t�H���_�̒T�����ċA�Ăяo���B
			// �������̃��[�U�[������\�ɂ���
			if (!::BlockingHook(dlgCancel.GetHwnd())) {
				goto cancel_return;
			}
			// ���f�{�^�������`�F�b�N
			if (dlgCancel.IsCanceled()) {
				goto cancel_return;
			}
			// �\���ݒ���`�F�b�N
			EditWnd::getInstance().SetDrawSwitchOfAllViews(
				dlgCancel.IsButtonChecked(IDC_CHECK_REALTIMEVIEW)
			);

			// �t�H���_�����쐬����B
			// 2010.08.01 �L�����Z���Ń������[���[�N���Ă܂���
			std::tstring currentPath  = pszPath;
			currentPath += _T("\\");
			currentPath += lpFileName;

			int nGrepTreeResult = DoGrepTree(
				viewDst,
				dlgCancel,
				pszKey,
				mGrepReplace,
				grepEnumKeys,
				grepExceptAbsFiles,
				grepExceptAbsFolders,
				currentPath.c_str(),
				pszBasePath,
				searchOption,
				grepOption,
				pattern,
				regexp,
				nNest + 1,
				bOutputBaseFolder,
				pnHitCount
			);
			if (nGrepTreeResult == -1) {
				goto cancel_return;
			}
			dlgCancel.SetItemText(IDC_STATIC_CURPATH, pszPath);	//@@@ 2002.01.10 add �T�u�t�H���_����߂��Ă�����...
		}
	}

	dlgCancel.SetItemText(IDC_STATIC_CURFILE, LTEXT(" "));	// 2002/09/09 Moca add
	return 0;

cancel_return:;
	// ���ʏo��
	if (0 < memMessage.GetStringLength()) {
		AddTail( viewDst, memMessage, grepOption.bGrepStdout );
		viewDst.GetCommander().Command_GOFILEEND(false);
		if (!EditWnd::getInstance().UpdateTextWrap()) {		// �܂�Ԃ����@�֘A�̍X�V
			EditWnd::getInstance().RedrawAllViews(&viewDst);	//	���̃y�C���̕\�����X�V
		}
		memMessage.Clear();
	}

	return -1;
}


/*!	@brief Grep���ʂ��\�z����


	pWork�͏[���ȃ������̈�������Ă���R�g
	@date 2002/08/29 Moca �o�C�i���[�f�[�^�ɑΉ� pnWorkLen �ǉ�
	@date 2013.11.05 Moca memMessage�ɒ��ڒǉ�����悤��
*/
void GrepAgent::SetGrepResult(
	// �f�[�^�i�[��
	NativeW& memMessage,
	// �}�b�`�����t�@�C���̏��
	const TCHAR*	pszFilePath,		// [in] �t���p�X or ���΃p�X
	const TCHAR*	pszCodeName,		// [in] �����R�[�h���D" [SJIS]"�Ƃ�
	// �}�b�`�����s�̏��
	LONGLONG	nLine,					// [in] �}�b�`�����s�ԍ�(1�`)
	int			nColumn,				// [in] �}�b�`�������ԍ�(1�`)
	const wchar_t*	pCompareData,		// [in] �s�̕�����
	int			nLineLen,				// [in] �s�̕�����̒���
	int			nEolCodeLen,			// [in] EOL�̒���
	// �}�b�`����������̏��
	const wchar_t*	pMatchData,			// [in] �}�b�`����������
	int			nMatchLen,				// [in] �}�b�`����������̒���
	// �I�v�V����
	const GrepOption&	grepOption
	)
{

	NativeW memBuf(L"");
	wchar_t strWork[64];
	const wchar_t* pDispData;
	int k;
	bool bEOL = true;
	int nMaxOutStr = 0;

	switch (grepOption.nGrepOutputStyle) {
	// �m�[�}��
	case 1:
		if (grepOption.bGrepOutputBaseFolder || grepOption.bGrepSeparateFolder) {
			memBuf.AppendString(L"�E");
		}
		memBuf.AppendStringT(pszFilePath);
		::auto_sprintf( strWork, L"(%I64d,%d)", nLine, nColumn );
		memBuf.AppendString(strWork);
		memBuf.AppendStringT(pszCodeName);
		memBuf.AppendString(L": ");
		nMaxOutStr = 2000; // 2003.06.10 Moca �ő咷�ύX
		break;
	// WZ��
	case 2:
		::auto_sprintf( strWork, L"�E(%6I64d,%-5d): ", nLine, nColumn );
		memBuf.AppendString(strWork);
		nMaxOutStr = 2500; // 2003.06.10 Moca �ő咷�ύX
		break;
	// ���ʂ̂�
	case 3:
		nMaxOutStr = 2500;
		break;
	}

	// �Y���s
	if (grepOption.nGrepOutputLineType != 0) {
		pDispData = pCompareData;
		k = nLineLen - nEolCodeLen;
		if (nMaxOutStr < k) {
			k = nMaxOutStr; // 2003.06.10 Moca �ő咷�ύX
		}
	// �Y������
	}else {
		pDispData = pMatchData;
		k = nMatchLen;
		if (nMaxOutStr < k) {
			k = nMaxOutStr; // 2003.06.10 Moca �ő咷�ύX
		}
		// �Y�������ɉ��s���܂ޏꍇ�͂��̉��s�R�[�h�����̂܂ܗ��p����(���̍s�ɋ�s�����Ȃ�)
		// 2003.06.10 Moca k==0�̂Ƃ��Ƀo�b�t�@�A���_�[�������Ȃ��悤��
		if (0 < k && WCODE::IsLineDelimiter(pMatchData[ k - 1 ], GetDllShareData().common.edit.bEnableExtEol)) {
			bEOL = false;
		}
	}

	memMessage.AllocStringBuffer(memMessage.GetStringLength() + memBuf.GetStringLength() + 2);
	memMessage.AppendNativeData(memBuf);
	memMessage.AppendString(pDispData, k);
	if (bEOL) {
		memMessage.AppendString(L"\r\n", 2);
	}
}

static void OutputPathInfo(
	NativeW&		memMessage,
	GrepOption		grepOption,
	const TCHAR*	pszFullPath,
	const TCHAR*	pszBaseFolder,
	const TCHAR*	pszFolder,
	const TCHAR*	pszRelPath,
	const TCHAR*	pszCodeName,
	bool&			bOutputBaseFolder,
	bool&			bOutputFolderName,
	BOOL&			bOutFileName
	)
{
	{
		// �o�b�t�@��2^n ���m�ۂ���
		int n = 1024;
		int size = memMessage.GetStringLength() + 300;
		while (n < size) {
			n *= 2;
		}
		memMessage.AllocStringBuffer(n);
	}
	if (grepOption.nGrepOutputStyle == 3) {
		return;
	}

	if (!bOutputBaseFolder && grepOption.bGrepOutputBaseFolder) {
		if (!grepOption.bGrepSeparateFolder && grepOption.nGrepOutputStyle == 1) {
			memMessage.AppendString(L"��\"");
		}else {
			memMessage.AppendString(L"��\"");
		}
		memMessage.AppendStringT(pszBaseFolder);
		memMessage.AppendString(L"\"\r\n");
		bOutputBaseFolder = true;
	}
	if (!bOutputFolderName && grepOption.bGrepSeparateFolder) {
		if (pszFolder[0]) {
			memMessage.AppendString(L"��\"");
			memMessage.AppendStringT(pszFolder);
			memMessage.AppendString(L"\"\r\n");
		}else {
			memMessage.AppendString(L"��\r\n");
		}
		bOutputFolderName = true;
	}
	if (grepOption.nGrepOutputStyle == 2) {
		if (!bOutFileName) {
			const TCHAR* pszDispFilePath = (grepOption.bGrepSeparateFolder || grepOption.bGrepOutputBaseFolder) ? pszRelPath : pszFullPath;
			if (grepOption.bGrepSeparateFolder) {
				memMessage.AppendString(L"��\"");
			}else {
				memMessage.AppendString(L"��\"");
			}
			memMessage.AppendStringT(pszDispFilePath);
			memMessage.AppendString(L"\"");
			memMessage.AppendStringT(pszCodeName);
			memMessage.AppendString(L"\r\n");
			bOutFileName = TRUE;
		}
	}
}

/*!
	Grep���s (FileLoad���g�����e�X�g��)

	@retval -1 GREP�̃L�����Z��
	@retval ����ȊO �q�b�g��(�t�@�C���������̓t�@�C����)

	@date 2001/06/27 genta	���K�\�����C�u�����̍����ւ�
	@date 2002/08/30 Moca FileLoad���g�����e�X�g��
	@date 2004/03/28 genta �s�v�Ȉ���nNest, bGrepSubFolder, pszPath���폜
*/
int GrepAgent::DoGrepFile(
	EditView&				viewDst,			// 
	DlgCancel&				dlgCancel,			// [in] Cancel�_�C�A���O�ւ̃|�C���^
	const wchar_t*			pszKey,				// [in] �����p�^�[��
	const TCHAR*			pszFile,			// [in] �����Ώۃt�@�C����(�\���p)
	const SearchOption&		searchOption,		// [in] �����I�v�V����
	const GrepOption&		grepOption,			// [in] Grep�I�v�V����
	const SearchStringPattern& pattern,			// [in] �����p�^�[��
	Bregexp&				regexp,			// [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
	int*					pnHitCount,			// [i/o] �q�b�g���̍��v�D���X�̒l�Ɍ��������������Z���ĕԂ��D
	const TCHAR*			pszFullPath,		// [in] �����Ώۃt�@�C���p�X C:\Folder\SubFolder\File.ext
	const TCHAR*			pszBaseFolder,		// [in] �����t�H���_ C:\Folder
	const TCHAR*			pszFolder,			// [in] �T�u�t�H���_ SubFolder (!bGrepSeparateFolder) �܂��� C:\Folder\SubFolder (!bGrepSeparateFolder)
	const TCHAR*			pszRelPath,			// [in] ���΃p�X File.ext(bGrepSeparateFolder) �܂���  SubFolder\File.ext(!bGrepSeparateFolder)
	bool&					bOutputBaseFolder,	// 
	bool&					bOutputFolderName,	// 
	NativeW&				memMessage			// 
	)
{
	int		nHitCount;
	LONGLONG	nLine;
	const wchar_t*	pszRes; // 2002/08/29 const�t��
	EncodingType	nCharCode;
	const wchar_t*	pCompareData; // 2002/08/29 const�t��
	int		nColumn;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
	Eol	eol;
	int		nEolCodeLen;
	const TypeConfigMini* type;
	DocTypeManager().GetTypeConfigMini(DocTypeManager().GetDocumentTypeOfPath(pszFile), &type);
	FileLoad	fl(type->encoding);	// 2012/12/18 Uchi ��������t�@�C���̃f�t�H���g�̕����R�[�h���擾����l��
	int		nOldPercent = 0;

	int	nKeyLen = wcslen(pszKey);
	// �t�@�C�����\��
	const TCHAR* pszDispFilePath = (grepOption.bGrepSeparateFolder || grepOption.bGrepOutputBaseFolder) ? pszRelPath : pszFullPath;

	//	�����ł͐��K�\���R���p�C���f�[�^�̏������͕s�v
	const TCHAR*	pszCodeName; // 2002/08/29 const�t��
	pszCodeName = _T("");
	nHitCount = 0;
	nLine = 0;

	// ���������������[���̏ꍇ�̓t�@�C���������Ԃ�
	// 2002/08/29 �s���[�v�̑O���炱���Ɉړ�
	if (nKeyLen == 0) {
		TCHAR szCpName[100];
		if (grepOption.nGrepCharSet == CODE_AUTODETECT) {
			// 2003.06.10 Moca �R�[�h���ʏ����������Ɉړ��D
			// ���ʃG���[�ł��t�@�C�����ɃJ�E���g���邽��
			// �t�@�C���̓��{��R�[�h�Z�b�g����
			// 2014.06.19 Moca �t�@�C�����̃^�C�v�ʂ�m_encoding�ɕύX
			CodeMediator mediator( type->encoding );
			nCharCode = mediator.CheckKanjiCodeOfFile(pszFullPath);
			if (!IsValidCodeOrCPType(nCharCode)) {
				pszCodeName = _T("  [(DetectError)]");
			}else if (IsValidCodeType(nCharCode)) {
				pszCodeName = CodeTypeName(nCharCode).Bracket();
			}else {
				CodePage::GetNameBracket(szCpName, nCharCode);
				pszCodeName = szCpName;
			}
		}
		{
			const wchar_t* pszFormatFullPath = L"";
			const wchar_t* pszFormatBasePath2 = L"";
			const wchar_t* pszFormatFilePath = L"";
			const wchar_t* pszFormatFilePath2 = L"";
			if (grepOption.nGrepOutputStyle == 1) {
				// �m�[�}��
				pszFormatFullPath   = L"%ts%ts\r\n";
				pszFormatBasePath2  = L"��\"%ts\"\r\n";
				pszFormatFilePath   = L"�E\"%ts\"%ts\r\n";
				pszFormatFilePath2  = L"�E\"%ts\"%ts\r\n";
			}else if (grepOption.nGrepOutputStyle == 2) {
				// WZ��
				pszFormatFullPath   = L"��\"%ts\"%ts\r\n";
				pszFormatBasePath2  = L"��\"%ts\"\r\n";
				pszFormatFilePath   = L"��\"%ts\"%ts\r\n";
				pszFormatFilePath2  = L"��\"%ts\"%ts\r\n";
			}else if (grepOption.nGrepOutputStyle == 3) {
				// ���ʂ̂�
				pszFormatFullPath   = L"%ts%ts\r\n";
				pszFormatBasePath2  = L"��\"%ts\"\r\n";
				pszFormatFilePath   = L"%ts\r\n";
				pszFormatFilePath2  = L"%ts\r\n";
			}
/*
			Base/Sep
			O / O  : (A)BaseFolder -> (C)Folder(Rel) -> (E)RelPath(File)
			O / X  : (B)BaseFolder ->                   (F)RelPath(RelFolder/File)
			X / O  :                  (D)Folder(Abs) -> (G)RelPath(File)
			X / X  : (H)FullPath
*/
			auto pszWork = std::make_unique<wchar_t[]>(auto_strlen(pszFullPath) + auto_strlen(pszCodeName) + 10);
			wchar_t* szWork0 = &pszWork[0];
			if (grepOption.bGrepOutputBaseFolder || grepOption.bGrepSeparateFolder) {
				if (!bOutputBaseFolder && grepOption.bGrepOutputBaseFolder) {
					const wchar_t* pszFormatBasePath = L"";
					if (grepOption.bGrepSeparateFolder) {
						pszFormatBasePath = L"��\"%ts\"\r\n";	// (A)
					}else {
						pszFormatBasePath = pszFormatBasePath2;	// (B)
					}
					auto_sprintf(szWork0, pszFormatBasePath, pszBaseFolder);
					memMessage.AppendString(szWork0);
					bOutputBaseFolder = true;
				}
				if (!bOutputFolderName && grepOption.bGrepSeparateFolder) {
					if (pszFolder[0]) {
						auto_sprintf(szWork0, L"��\"%ts\"\r\n", pszFolder);	// (C), (D)
					}else {
						auto_strcpy(szWork0, L"��\r\n");
					}
					memMessage.AppendString(szWork0);
					bOutputFolderName = true;
				}
				auto_sprintf(szWork0,
					(grepOption.bGrepSeparateFolder ? pszFormatFilePath // (E)
						: pszFormatFilePath2),	// (F), (G)
					pszDispFilePath, pszCodeName);
				memMessage.AppendString(szWork0);
			}else {
				auto_sprintf(szWork0, pszFormatFullPath, pszFullPath, pszCodeName);	// (H)
				memMessage.AppendString(szWork0);
			}
		}
		++(*pnHitCount);
		dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE);
		return 1;
	}

	try {
		// �t�@�C�����J��
		// FileClose�Ŗ����I�ɕ��邪�A���Ă��Ȃ��Ƃ��̓f�X�g���N�^�ŕ���
		// 2003.06.10 Moca �����R�[�h���菈����FileOpen�ōs��
		nCharCode = fl.FileOpen( pszFullPath, true, grepOption.nGrepCharSet, GetDllShareData().common.file.GetAutoMIMEdecode() );
		TCHAR szCpName[100];
		{
			if (grepOption.nGrepCharSet == CODE_AUTODETECT) {
				if (IsValidCodeType(nCharCode)) {
					auto_strcpy( szCpName, CodeTypeName(nCharCode).Bracket() );
					pszCodeName = szCpName;
				}else {
					CodePage::GetNameBracket(szCpName, nCharCode);
					pszCodeName = szCpName;
				}
			}
		}

	//	// �������̃��[�U�[������\�ɂ���
		if (!::BlockingHook(dlgCancel.GetHwnd())) {
			return -1;
		}
		// ���f�{�^�������`�F�b�N
		if (dlgCancel.IsCanceled()) {
			return -1;
		}
		int nOutputHitCount = 0;

		// ���������������[���̏ꍇ�̓t�@�C���������Ԃ�
		// 2002/08/29 �t�@�C���I�[�v���̎�O�ֈړ�
		
		std::vector<std::pair<const wchar_t*, LogicInt>> searchWords;
		if (searchOption.bWordOnly) {
			SearchAgent::CreateWordList(searchWords, pszKey, nKeyLen);
		}

		// ���� : fl.ReadLine �� throw ����\��������
		NativeW unicodeBuffer;
		while (CodeConvertResult::Failure != fl.ReadLine(&unicodeBuffer, &eol)) {
			const wchar_t*	pLine = unicodeBuffer.GetStringPtr();
			int		nLineLen = unicodeBuffer.GetStringLength();

			nEolCodeLen = eol.GetLen();
			++nLine;
			pCompareData = pLine;

			// �������̃��[�U�[������\�ɂ���
			// 2010.08.31 �Ԋu��1/32�ɂ���
			if (((nLine%32 == 0) || nLineLen > 10000) && !::BlockingHook(dlgCancel.GetHwnd())) {
				return -1;
			}
			if (nLine%64 == 0) {
				// ���f�{�^�������`�F�b�N
				if (dlgCancel.IsCanceled()) {
					return -1;
				}
				//	2003.06.23 Moca �\���ݒ���`�F�b�N
				EditWnd::getInstance().SetDrawSwitchOfAllViews(
					dlgCancel.IsButtonChecked(IDC_CHECK_REALTIMEVIEW)
				);
				// 2002/08/30 Moca �i�s��Ԃ�\������(5MB�ȏ�)
				if (5000000 < fl.GetFileSize()) {
					int nPercent = fl.GetPercent();
					if (5 <= nPercent - nOldPercent) {
						nOldPercent = nPercent;
						TCHAR szWork[10];
						::auto_sprintf( szWork, _T(" (%3d%%)"), nPercent );
						std::tstring str;
						str = str + pszFile + szWork;
						dlgCancel.SetItemText(IDC_STATIC_CURFILE, str.c_str());
					}
				}
			}
			int nHitOldLine = nHitCount;
			int nHitCountOldLine = *pnHitCount;

			// ���K�\������
			if (searchOption.bRegularExp) {
				int nIndex = 0;
	#ifdef _DEBUG
				int nIndexPrev = -1;
	#endif

				//	Jun. 21, 2003 genta ���[�v����������
				//	�}�b�`�ӏ���1�s���畡�����o����P�[�X��W���ɁC
				//	�}�b�`�ӏ���1�s����1�������o����ꍇ���O�P�[�X�ƂƂ炦�C
				//	���[�v�p���E�ł��؂����(nGrepOutputLineType)���t�ɂ����D
				//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
				// From Here 2005.03.19 ����� ���͂�BREGEXP�\���̂ɒ��ڃA�N�Z�X���Ȃ�
				// 2010.08.25 �s���ȊO��^�Ƀ}�b�`����s��̏C��
				while (nIndex <= nLineLen && regexp.Match(pLine, nLineLen, nIndex)) {

					// �p�^�[������
					nIndex = regexp.GetIndex();
					int matchlen = regexp.GetMatchLen();
#ifdef _DEBUG
					if (nIndex <= nIndexPrev) {
						MYTRACE(_T("ERROR: EditView::DoGrepFile() nIndex <= nIndexPrev break \n"));
						break;
					}
					nIndexPrev = nIndex;
#endif
					++nHitCount;
					++(*pnHitCount);
					if (grepOption.nGrepOutputLineType != 2) {

						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						SetGrepResult(
							memMessage,
							pszDispFilePath,
							pszCodeName,
							nLine,
							nIndex + 1,
							pLine,
							nLineLen,
							nEolCodeLen,
							pLine + nIndex,
							matchlen,
							grepOption
						);
						if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
							dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE);
						}
					}
					// To Here 2005.03.19 ����� ���͂�BREGEXP�\���̂ɒ��ڃA�N�Z�X���Ȃ�
					//	Jun. 21, 2003 genta �s�P�ʂŏo�͂���ꍇ��1������Ώ\��
					if ( grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly ) {
						break;
					}
					//	�T���n�߂�ʒu��␳
					//	2003.06.10 Moca �}�b�`����������̌�납�玟�̌������J�n����
					if (matchlen <= 0) {
						matchlen = NativeW::GetSizeOfChar(pLine, nLineLen, nIndex);
						if (matchlen <= 0) {
							matchlen = 1;
						}
					}
					nIndex += matchlen;
				}
			// �P��̂݌���
			}else if (searchOption.bWordOnly) {
				/*
					2002/02/23 Norio Nakatani
					�P��P�ʂ�Grep�������I�Ɏ����B�P���WhereCurrentWord()�Ŕ��ʂ��Ă܂��̂ŁA
					�p�P���C/C++���ʎq�Ȃǂ̌��������Ȃ�q�b�g���܂��B

					2002/03/06 YAZAKI
					Grep�ɂ����������B
					WhereCurrentWord�ŒP��𒊏o���āA���̒P�ꂪ������Ƃ����Ă��邩��r����B
				*/
				int nMatchLen;
				int nIdx = 0;
				// Jun. 26, 2003 genta ���ʂ�while�͍폜
				while ((pszRes = SearchAgent::SearchStringWord(pLine, nLineLen, nIdx, searchWords, searchOption.bLoHiCase, &nMatchLen))) {
					nIdx = pszRes - pLine + nMatchLen;
					++nHitCount;
					++(*pnHitCount);
					if (grepOption.nGrepOutputLineType != 2) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						SetGrepResult(
							memMessage,
							pszDispFilePath,
							pszCodeName,
							//	Jun. 25, 2002 genta
							//	���ʒu��1�n�܂�Ȃ̂�1�𑫂��K�v������
							nLine,
							pszRes - pLine + 1,
							pLine,
							nLineLen,
							nEolCodeLen,
							pszRes,
							nMatchLen,
							grepOption
						);
						//	May 22, 2000 genta
						if (((*pnHitCount)%128)==0 || *pnHitCount<128) {
							dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE);
						}
					}
	
					// 2010.10.31 ryoji �s�P�ʂŏo�͂���ꍇ��1������Ώ\��
					if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
						break;
					}
				}
			}else {
				// �����񌟍�
				int nColumnPrev = 0;
				//	Jun. 21, 2003 genta ���[�v����������
				//	�}�b�`�ӏ���1�s���畡�����o����P�[�X��W���ɁC
				//	�}�b�`�ӏ���1�s����1�������o����ꍇ���O�P�[�X�ƂƂ炦�C
				//	���[�v�p���E�ł��؂����(nGrepOutputLineType)���t�ɂ����D
				for (;;) {
					pszRes = SearchAgent::SearchString(
						pCompareData,
						nLineLen,
						0,
						pattern
					);
					if (!pszRes) {
						break;
					}

					nColumn = pszRes - pCompareData + 1;
	
					++nHitCount;
					++(*pnHitCount);
					if (grepOption.nGrepOutputLineType != 2) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						SetGrepResult(
							memMessage,
							pszDispFilePath,
							pszCodeName,
							nLine,
							nColumn + nColumnPrev,
							pCompareData,
							nLineLen,
							nEolCodeLen,
							pszRes,
							nKeyLen,
							grepOption
						);
						//	May 22, 2000 genta
						if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
							dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE);
						}
					}
						
					//	Jun. 21, 2003 genta �s�P�ʂŏo�͂���ꍇ��1������Ώ\��
					if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
						break;
					}
					//	�T���n�߂�ʒu��␳
					//	2003.06.10 Moca �}�b�`����������̌�납�玟�̌������J�n����
					//	nClom : �}�b�`�ʒu
					//	matchlen : �}�b�`����������̒���
					int nPosDiff = nColumn += nKeyLen - 1;
					pCompareData += nPosDiff;
					nLineLen -= nPosDiff;
					nColumnPrev += nPosDiff;
				}
			}
			// 2014.09.23 �ۃq�b�g�s���o��
			if (grepOption.nGrepOutputLineType == 2) {
				bool bNoHit = nHitOldLine == nHitCount;
				// �q�b�g����߂�
				nHitCount = nHitOldLine;
				*pnHitCount = nHitCountOldLine;
				// �ۃq�b�g�s������
				if (bNoHit) {
					++nHitCount;
					(*pnHitCount)++;
					OutputPathInfo(
						memMessage, grepOption,
						pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
						bOutputBaseFolder, bOutputFolderName, bOutFileName
					);
					SetGrepResult(
						memMessage, pszDispFilePath, pszCodeName,
						nLine, 1, pLine, nLineLen, nEolCodeLen,
						pLine, nLineLen, grepOption
					);
					if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
						dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
				}
			}
			// 2014.09.23 �f�[�^���������̓o�b�t�@�o��
			if (0 < memMessage.GetStringLength() && 2800 < nHitCount - nOutputHitCount) {
				nOutputHitCount = nHitCount;
				AddTail( viewDst, memMessage, grepOption.bGrepStdout );
				viewDst.GetCommander().Command_GOFILEEND( FALSE );
				if (!EditWnd::getInstance().UpdateTextWrap()) {	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
					EditWnd::getInstance().RedrawAllViews( &viewDst );	//	���̃y�C���̕\�����X�V
				}
				memMessage._SetStringLength(0);
			}

			// �t�@�C�������̏ꍇ�́A1����������I��
			if (grepOption.bGrepOutputFileOnly && 1 <= nHitCount) {
				break;
			}
		}

		// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
		fl.FileClose();
	} // try
	catch (Error_FileOpen) {
		NativeW str(LSW(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		memMessage.AppendNativeData( str );
		return 0;
	}
	catch (Error_FileRead) {
		NativeW str(LSW(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		memMessage.AppendNativeData( str );
	} // ��O�����I���

	return nHitCount;
}

class Error_WriteFileOpen
{
public:
	virtual ~Error_WriteFileOpen(){}
};

class WriteData {
public:
	WriteData(int& hit,
				LPCTSTR name,
				EncodingType code_,
				bool bBom_,
				bool bOldSave_,
				NativeW& message)
		:
		nHitCount(hit),
		fileName(name),
		code(code_),
		bBom(bBom_),
		bOldSave(bOldSave_),
		bufferSize(0),
		out(NULL),
		pCodeBase(CodeFactory::CreateCodeBase(code_,0)),
		memMessage(message)
	{
	}
	void AppendBuffer(const NativeW& strLine)
	{
		if (!out) {
			bufferSize += strLine.GetStringLength();
			buffer.push_back(strLine);
			// 10MB �ȏゾ������o�͂��Ă��܂�
			if (0xa00000 <= bufferSize) {
				OutputHead();
			}
		}else {
			Output(strLine);
		}
	}
	void OutputHead()
	{
		if (!out) {
			std::tstring name = fileName;
			name += _T(".skrnew");
			try {
				out = new BinaryOutputStream(name.c_str(), true);
			}catch (Error_FileOpen) {
				throw Error_WriteFileOpen();
			}
			if (bBom) {
				Memory bom;
				pCodeBase->GetBom(&bom);
				out->Write(bom.GetRawPtr(), bom.GetRawLength());
			}
			for (size_t i=0; i<buffer.size(); ++i) {
				Output(buffer[i]);
			}
			buffer.clear();
			std::deque<NativeW>().swap(buffer);
		}
	}
	void Output(const NativeW& strLine)
	{
		Memory dest;
		pCodeBase->UnicodeToCode(strLine, &dest);
		// �ꍇ�ɂ���Ă͉��s���Ƃł͂Ȃ��̂ŁAJIS/UTF-7�ł̏o�͂����łȂ��\������
		out->Write(dest.GetRawPtr(), dest.GetRawLength());
	}
	void Close()
	{
		if (nHitCount && out) {
			out->Close();
			delete out;
			out = NULL;
			if (bOldSave) {
				std::tstring oldFile = fileName;
				oldFile += _T(".skrold");
				if (fexist(oldFile.c_str())) {
					if (::DeleteFile(oldFile.c_str()) == FALSE) {
						std::wstring msg = LSW(STR_GREP_REP_ERR_DELETE);
						msg += L"[";
						msg += to_wchar(oldFile.c_str());
						msg += L"]\r\n";
						memMessage.AppendString( msg.c_str() );
						return;
					}
				}
				if (::MoveFile(fileName, oldFile.c_str()) == FALSE) {
					std::wstring msg = LSW(STR_GREP_REP_ERR_REPLACE);
					msg += L"[";
					msg += to_wchar(oldFile.c_str());
					msg += L"]\r\n";
					memMessage.AppendString( msg.c_str() );
					return;
				}
			}else {
				if (::DeleteFile(fileName) == FALSE) {
					std::wstring msg = LSW(STR_GREP_REP_ERR_DELETE);
					msg += L"[";
					msg += to_wchar(fileName);
					msg += L"]\r\n";
					memMessage.AppendString( msg.c_str() );
					return;
				}
			}
			std::tstring name = std::tstring(fileName);
			name += _T(".skrnew");
			if (::MoveFile(name.c_str(), fileName) == FALSE) {
				std::wstring msg = LSW(STR_GREP_REP_ERR_REPLACE);
				msg += L"[";
				msg += to_wchar(fileName);
				msg += L"]\r\n";
				memMessage.AppendString( msg.c_str()  );
				return;
			}
		}
		return;
	}
	~WriteData()
	{
		if (out) {
			out->Close();
			delete out;
			out = NULL;
			std::tstring name = std::tstring(fileName);
			name += _T(".skrnew");
			::DeleteFile( name.c_str() );
		}
	}
private:
	int& nHitCount;
	LPCTSTR fileName;
	EncodingType code;
	bool bBom;
	bool bOldSave;
	size_t bufferSize;
	std::deque<NativeW> buffer;
	BinaryOutputStream* out;
	std::unique_ptr<CodeBase> pCodeBase;
	NativeW&	memMessage;
};

/*!
	Grep�u�����s
	@date 2013.06.12 Moca �V�K�쐬
*/
int GrepAgent::DoGrepReplaceFile(
	EditView&				viewDst,
	DlgCancel&				dlgCancel,
	const wchar_t*			pszKey,
	const NativeW&			mGrepReplace,
	const TCHAR*			pszFile,
	const SearchOption&		searchOption,
	const GrepOption&		grepOption,
	const SearchStringPattern& pattern,
	Bregexp&				regexp,		//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	int*					pnHitCount,
	const TCHAR*			pszFullPath,
	const TCHAR*			pszBaseFolder,
	const TCHAR*			pszFolder,
	const TCHAR*			pszRelPath,
	bool&					bOutputBaseFolder,
	bool&					bOutputFolderName,
	NativeW&				memMessage
	)
{
	LONGLONG	nLine = 0;
	int			nHitCount = 0;
	EncodingType	nCharCode;
	BOOL		bOutFileName = FALSE;
	Eol		cEol;
	int		nEolCodeLen;
	int		nOldPercent = 0;
	int		nKeyLen = wcslen( pszKey );
	const TCHAR*	pszCodeName = _T("");

	const TypeConfigMini* type;
	DocTypeManager().GetTypeConfigMini( DocTypeManager().GetDocumentTypeOfPath( pszFile ), &type );
	FileLoad fl( type->encoding );	// 2012/12/18 Uchi ��������t�@�C���̃f�t�H���g�̕����R�[�h���擾����l��
	bool bBom;
	// �t�@�C�����\��
	const TCHAR* pszDispFilePath = ( grepOption.bGrepSeparateFolder || grepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;


	try {
		// �t�@�C�����J��
		// FileClose�Ŗ����I�ɕ��邪�A���Ă��Ȃ��Ƃ��̓f�X�g���N�^�ŕ���
		// 2003.06.10 Moca �����R�[�h���菈����FileOpen�ōs��
		nCharCode = fl.FileOpen( pszFullPath, true, grepOption.nGrepCharSet, GetDllShareData().common.file.GetAutoMIMEdecode(), &bBom );
		WriteData output(nHitCount, pszFullPath, nCharCode, bBom, grepOption.bGrepBackup, memMessage );
		TCHAR szCpName[100];
		{
			if (grepOption.nGrepCharSet == CODE_AUTODETECT) {
				if (IsValidCodeType(nCharCode)) {
					auto_strcpy( szCpName, CodeTypeName(nCharCode).Bracket() );
					pszCodeName = szCpName;
				}else {
					CodePage::GetNameBracket(szCpName, nCharCode);
					pszCodeName = szCpName;
				}
			}
		}
		// �������̃��[�U�[������\�ɂ���
		if (!::BlockingHook( dlgCancel.GetHwnd() )) {
			return -1;
		}
		// ���f�{�^�������`�F�b�N
		if (dlgCancel.IsCanceled()) {
			return -1;
		}
		int nOutputHitCount = 0;
	
		std::vector<std::pair<const wchar_t*, LogicInt>> searchWords;
		if (searchOption.bWordOnly) {
			SearchAgent::CreateWordList( searchWords, pszKey, nKeyLen );
		}
	
		NativeW outBuffer;
		// ���� : fl.ReadLine �� throw ����\��������
		NativeW unicodeBuffer;
		while (fl.ReadLine(&unicodeBuffer, &cEol) != CodeConvertResult::Failure) {
			const wchar_t*	pLine = unicodeBuffer.GetStringPtr();
			int nLineLen = unicodeBuffer.GetStringLength();
	
			nEolCodeLen = cEol.GetLen();
			++nLine;
	
			// �������̃��[�U�[������\�ɂ���
			// 2010.08.31 �Ԋu��1/32�ɂ���
			if (((nLine%32 == 0)|| 10000 < nLineLen ) && !::BlockingHook( dlgCancel.GetHwnd() )) {
				return -1;
			}
			if (nLine%64 == 0) {
				// ���f�{�^�������`�F�b�N
				if (dlgCancel.IsCanceled()) {
					return -1;
				}
				//	2003.06.23 Moca �\���ݒ���`�F�b�N
				EditWnd::getInstance().SetDrawSwitchOfAllViews(
					dlgCancel.IsButtonChecked(IDC_CHECK_REALTIMEVIEW)
				);
				// 2002/08/30 Moca �i�s��Ԃ�\������(5MB�ȏ�)
				if (5000000 < fl.GetFileSize()) {
					int nPercent = fl.GetPercent();
					if (5 <= nPercent - nOldPercent) {
						nOldPercent = nPercent;
						TCHAR szWork[10];
						::auto_sprintf( szWork, _T(" (%3d%%)"), nPercent );
						std::tstring str;
						str = str + pszFile + szWork;
						dlgCancel.SetItemText(IDC_STATIC_CURFILE, str.c_str() );
					}
				}
			}
			outBuffer.SetString( L"", 0 );
			bool bOutput = true;
			if (grepOption.bGrepOutputFileOnly && 1 <= nHitCount) {
				bOutput = false;
			}
	
			// ���K�\������
			if (searchOption.bRegularExp) {
				int nIndex = 0;
				int nIndexOld = nIndex;
				int nMatchNum = 0;
				//	Jun. 21, 2003 genta ���[�v����������
				//	�}�b�`�ӏ���1�s���畡�����o����P�[�X��W���ɁC
				//	�}�b�`�ӏ���1�s����1�������o����ꍇ���O�P�[�X�ƂƂ炦�C
				//	���[�v�p���E�ł��؂����(bGrepOutputLine)���t�ɂ����D
				//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
				// From Here 2005.03.19 ����� ���͂�BREGEXP�\���̂ɒ��ڃA�N�Z�X���Ȃ�
				// 2010.08.25 �s���ȊO��^�Ƀ}�b�`����s��̏C��
				while (
					nIndex <= nLineLen
					&& (  (!grepOption.bGrepPaste && (nMatchNum = regexp.Replace(pLine, nLineLen, nIndex)))
						|| (grepOption.bGrepPaste && regexp.Match(pLine, nLineLen, nIndex))
					)
				) {
					//	�p�^�[������
					nIndex = regexp.GetIndex();
					int matchlen = regexp.GetMatchLen();
					if (bOutput) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						// Grep���ʂ��AmemMessage�Ɋi�[����
						SetGrepResult(
							memMessage, pszDispFilePath, pszCodeName,
							nLine, nIndex + 1,
							pLine, nLineLen, nEolCodeLen,
							pLine + nIndex, matchlen,
							grepOption
						);
						// To Here 2005.03.19 ����� ���͂�BREGEXP�\���̂ɒ��ڃA�N�Z�X���Ȃ�
						if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
							bOutput = false;
						}
					}
					output.OutputHead();
					++nHitCount;
					++(*pnHitCount);
					if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
						dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					if (!grepOption.bGrepPaste) {
						// g�I�v�V�����ł͍s���܂ň�x�ɒu���ς�
						nHitCount += nMatchNum - 1;
						*pnHitCount += nMatchNum - 1;
						outBuffer.AppendString( regexp.GetString(), regexp.GetStringLen() );
						nIndexOld = nLineLen;
						break;
					}
					if (0 < nIndex - nIndexOld) {
						outBuffer.AppendString( &pLine[nIndexOld], nIndex - nIndexOld );
					}
					outBuffer.AppendNativeData( mGrepReplace );
					//	�T���n�߂�ʒu��␳
					//	2003.06.10 Moca �}�b�`����������̌�납�玟�̌������J�n����
					if (matchlen <= 0) {
						matchlen = NativeW::GetSizeOfChar( pLine, nLineLen, nIndex );
						if (matchlen <= 0) {
							matchlen = 1;
						}
					}
					nIndex += matchlen;
					nIndexOld = nIndex;
				}
				if (0 < nLineLen - nIndexOld) {
					outBuffer.AppendString( &pLine[nIndexOld], nLineLen - nIndexOld );
				}
			}
			// �P��̂݌���
			else if (searchOption.bWordOnly) {
				/*
					2002/02/23 Norio Nakatani
					�P��P�ʂ�Grep�������I�Ɏ����B�P���WhereCurrentWord()�Ŕ��ʂ��Ă܂��̂ŁA
					�p�P���C/C++���ʎq�Ȃǂ̌��������Ȃ�q�b�g���܂��B
	
					2002/03/06 YAZAKI
					Grep�ɂ����������B
					WhereCurrentWord�ŒP��𒊏o���āA���̒P�ꂪ������Ƃ����Ă��邩��r����B
				*/
				const wchar_t* pszRes;
				int nMatchLen;
				int nIdx = 0;
				int nOutputPos = 0;
				// Jun. 26, 2003 genta ���ʂ�while�͍폜
				while (pszRes = SearchAgent::SearchStringWord(pLine, nLineLen, nIdx, searchWords, searchOption.bLoHiCase, &nMatchLen)) {
					nIdx = pszRes - pLine + nMatchLen;
					if (bOutput) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						// Grep���ʂ��AmemMessage�Ɋi�[����
						SetGrepResult(
							memMessage, pszDispFilePath, pszCodeName,
							//	Jun. 25, 2002 genta
							//	���ʒu��1�n�܂�Ȃ̂�1�𑫂��K�v������
							nLine, pszRes - pLine + 1, pLine, nLineLen, nEolCodeLen,
							pszRes, nMatchLen,
							grepOption
						);
						if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
							bOutput = false;
						}
					}
					output.OutputHead();
					++nHitCount;
					++(*pnHitCount);
					//	May 22, 2000 genta
					if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
						dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					if (0 < pszRes - pLine - nOutputPos) {
						outBuffer.AppendString( &pLine[nOutputPos], pszRes - pLine - nOutputPos );
					}
					outBuffer.AppendNativeData( mGrepReplace );
					nOutputPos = pszRes - pLine + nMatchLen;
				}
				outBuffer.AppendString( &pLine[nOutputPos], nLineLen - nOutputPos );
			}else {
				// �����񌟍�
				int nColumnPrev = 0;
				const wchar_t*	pCompareData = pLine;
				int nCompareLen = nLineLen;
				//	Jun. 21, 2003 genta ���[�v����������
				//	�}�b�`�ӏ���1�s���畡�����o����P�[�X��W���ɁC
				//	�}�b�`�ӏ���1�s����1�������o����ꍇ���O�P�[�X�ƂƂ炦�C
				//	���[�v�p���E�ł��؂����(bGrepOutputLine)���t�ɂ����D
				for (;;) {
					const wchar_t* pszRes = SearchAgent::SearchString( pCompareData, nCompareLen, 0, pattern );
					if (!pszRes) {
						break;
					}
	
					int	nColumn = pszRes - pCompareData;
					if (bOutput) {
						OutputPathInfo(
							memMessage, grepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						// Grep���ʂ��AmemMessage�Ɋi�[����
						SetGrepResult(
							memMessage, pszDispFilePath, pszCodeName,
							nLine, nColumn + nColumnPrev + 1, pLine, nLineLen, nEolCodeLen,
							pszRes, nKeyLen,
							grepOption
						);
						if (grepOption.nGrepOutputLineType != 0 || grepOption.bGrepOutputFileOnly) {
							bOutput = false;
						}
					}
					output.OutputHead();
					++nHitCount;
					++(*pnHitCount);
					//	May 22, 2000 genta
					if (((*pnHitCount)%128) == 0 || *pnHitCount < 128) {
						dlgCancel.SetItemInt(IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					if (nColumn) {
						outBuffer.AppendString( pCompareData, nColumn );
					}
					outBuffer.AppendNativeData( mGrepReplace );
					//	�T���n�߂�ʒu��␳
					//	2003.06.10 Moca �}�b�`����������̌�납�玟�̌������J�n����
					//	nClom : �}�b�`�ʒu
					//	matchlen : �}�b�`����������̒���
					int nPosDiff = nColumn + nKeyLen;
					pCompareData += nPosDiff;
					nCompareLen -= nPosDiff;
					nColumnPrev += nPosDiff;
				}
				outBuffer.AppendString( &pLine[nColumnPrev], nLineLen - nColumnPrev );
			}
			output.AppendBuffer(outBuffer);
	
			// 2014.09.23 �f�[�^���������̓o�b�t�@�o��
			if (0 < memMessage.GetStringLength() && 2800 < nHitCount - nOutputHitCount) {
				nOutputHitCount = nHitCount;
				AddTail( viewDst, memMessage, grepOption.bGrepStdout );
				viewDst.GetCommander().Command_GOFILEEND( false );
				if (!EditWnd::getInstance().UpdateTextWrap())	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
					EditWnd::getInstance().RedrawAllViews( &viewDst );	//	���̃y�C���̕\�����X�V
				memMessage._SetStringLength(0);
			}
		}
	
		// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
		fl.FileClose();
		output.Close();
	} // try
	catch( Error_FileOpen ){
		NativeW str(LSW(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		memMessage.AppendNativeData(str);
		return 0;
	}catch (Error_FileRead) {
		NativeW str(LSW(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		memMessage.AppendNativeData(str);
	}catch (Error_WriteFileOpen) {
		std::tstring file = pszFullPath;
		file += _T(".skrnew");
		NativeW str(LSW(STR_GREP_ERR_FILEWRITE));
		str.Replace(L"%ts", to_wchar(file.c_str()));
		memMessage.AppendNativeData( str );
	} // ��O�����I���

	return nHitCount;
}


