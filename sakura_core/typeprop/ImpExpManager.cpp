// �C���|�[�g�A�G�N�X�|�[�g�}�l�[�W��

#include "StdAfx.h"

#include <memory>

#include "ImpExpManager.h"
#include "typeprop/DlgTypeAscertain.h"

#include "dlg/DlgOpenFile.h"
#include "io/TextStream.h"
#include "env/ShareData_IO.h"
#include "plugin/Plugin.h"
#include "view/EditView.h"
#include "view/colors/ColorStrategy.h"

/*-----------------------------------------------------------------------
�萔
-----------------------------------------------------------------------*/
static const wchar_t	szSecInfo[]				= L"Info";

// �^�C�v�ʐݒ�
static const wchar_t	WSTR_TYPE_HEAD[]		= L" �^�C�v�ʐݒ� Ver1";

static const wchar_t	szSecTypeEx[]			= L"TypeEx";
static const wchar_t	szSecTypes[]			= L"Types";

static const wchar_t	szKeyKeywordTemp[]				= L"szKeyword[%d]";
static const wchar_t	szKeyKeywordFileTemp[]			= L"szKeywordFile[%d]";
static const wchar_t	szKeyKeywordCaseTemp[]			= L"szKeywordCase[%d]";
static const wchar_t	szKeyPluginOutlineName[]		= L"szPluginOutlineName";
static const wchar_t	szKeyPluginOutlineId[]			= L"szPluginOutlineId";
static const wchar_t	szKeyPluginSmartIndentName[]	= L"szPluginSmartIndentName";
static const wchar_t	szKeyPluginSmartIndentId[]		= L"szPluginSmartIndentId";
static const wchar_t	szKeyVersion[]					= L"szVersion";
static const wchar_t	szKeyStructureVersion[]			= L"vStructureVersion";

static const wchar_t	WSTR_COLORDATA_HEAD3[]	=  L" �e�L�X�g�G�f�B�^�F�ݒ� Ver3";
static const wchar_t	szSecColor[]			=  L"SakuraColor";

// ���K�\���L�[���[�h
static const wchar_t	WSTR_REGEXKW_HEAD[]		= L"// ���K�\���L�[���[�h Ver1\n";

// �L�[���[�h�w���v
static const wchar_t	WSTR_KEYHELP_HEAD[]		= L"// �L�[���[�h�����ݒ� Ver1\n";

// �L�[���蓖��
static const wchar_t	WSTR_KEYBIND_HEAD4[]	= L"SakuraKeyBind_Ver4";
static const wchar_t	WSTR_KEYBIND_HEAD3[]	= L"SakuraKeyBind_Ver3";
static const wchar_t	WSTR_KEYBIND_HEAD2[]	= L"// �e�L�X�g�G�f�B�^�L�[�ݒ� Ver2";

// �J�X�^�����j���[�t�@�C��
static       wchar_t	WSTR_CUSTMENU_HEAD_V2[]	= L"SakuraEditorMenu_Ver2";

// �L�[���[�h��`�t�@�C��
static const wchar_t	WSTR_KEYWORD_HEAD[]		= L" �L�[���[�h��`�t�@�C��\n";
static const wchar_t	WSTR_KEYWORD_CASE[]		= L"// CASE=";
static const wchar_t	WSTR_CASE_TRUE[]		= L"// CASE=True";
static const wchar_t	WSTR_CASE_FALSE[]		= L"// CASE=False";

// ���C�����j���[�t�@�C��
static       wchar_t	WSTR_MAINMENU_HEAD_V1[]	= L"SakuraEditorMainMenu Ver1";

static       wchar_t	WSTR_FILETREE_HEAD_V1[]	= L"SakuraEditorFileTree_Ver1";

// Export�t�@�C�����̍쐬
//	  �^�C�v���Ȃǃt�@�C���Ƃ��Ĉ������Ƃ��l���Ă��Ȃ������������
static wchar_t* MakeExportFileName(wchar_t* res, const wchar_t* trg, const wchar_t* ext)
{
	wchar_t		conv[_MAX_PATH + 1];
	wchar_t*	p;

	auto_strcpy(conv, trg);

	p = conv;
	while ((p = wcspbrk(p, L"\t\\:*?\"<>|"))) {
		// �t�@�C�����Ɏg���Ȃ������� _ �ɒu��������
		*p++ = L'_';
	}
	p = conv;
	while ((p = wcspbrk(p, L"/"))) {
		// �t�@�C�����Ɏg���Ȃ������� �^ �ɒu��������
		*p++ = L'�^';
	}
	auto_sprintf_s(res, _MAX_PATH, L"%ls.%ls", conv, ext);

	return res;
}

// �C���|�[�g �t�@�C���w��t��
bool ImpExpManager::ImportUI(HINSTANCE hInstance, HWND hwndParent)
{
	// �t�@�C���I�[�v���_�C�A���O�̏�����
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		hInstance,
		hwndParent,
		GetDefaultExtension(),
		GetDllShareData().history.szIMPORTFOLDER // �C���|�[�g�p�t�H���_
	);
	TCHAR szPath[_MAX_PATH + 1];
	szPath[0] = _T('\0');
	if (!GetFileName().empty()) {
		auto_strcpy(szPath, to_tchar(GetFullPath().c_str()));
	}
	if (!dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
		return false;
	}

	const wstring sPath = to_wchar(szPath);
	wstring	sErrMsg;

	// �m�F
	if (!ImportAscertain(hInstance, hwndParent, sPath, sErrMsg)) {
		if (sErrMsg.length() > 0) {
			ErrorMessage(hwndParent, _T("%ls"), sErrMsg.c_str());
		}
		return false;
	}

	// Import Folder�̐ݒ�
	SetImportFolder(szPath);

	// Import
	if (!Import(sPath, sErrMsg)) {
		ErrorMessage(hwndParent, _T("%ls"), sErrMsg.c_str());
		return false;
	}

	if (sErrMsg.length() > 0) {
		InfoMessage(hwndParent, _T("%ls"), sErrMsg.c_str());
	}

	return true;
}

// �G�N�X�|�[�g �t�@�C���w��t��
bool ImpExpManager::ExportUI(HINSTANCE hInstance, HWND hwndParent)
{
	// �t�@�C���I�[�v���_�C�A���O�̏�����
	DlgOpenFile dlgOpenFile;
	dlgOpenFile.Create(
		hInstance,
		hwndParent,
		GetDefaultExtension(),
		GetDllShareData().history.szIMPORTFOLDER // �C���|�[�g�p�t�H���_
	);
	TCHAR szPath[_MAX_PATH + 1];
	szPath[0] = _T('\0');
	if (!GetFileName().empty()) {
		auto_strcpy(szPath, to_tchar(GetFullPath().c_str()));
	}
	if (!dlgOpenFile.DoModal_GetSaveFileName(szPath)) {
		return false;
	}

	// Import Folder�̐ݒ�
	SetImportFolder(szPath);

	// Export
	const wstring sPath = to_wchar(szPath);
	wstring	sErrMsg;
	if (!Export(sPath, sErrMsg)) {
		ErrorMessage(hwndParent, _T("%ls"), sErrMsg.c_str());
		return false;
	}

	if (sErrMsg.length() == 0) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_OK_EXPORT)) + to_wchar(szPath);
	}
	InfoMessage(hwndParent, _T("%ls"), sErrMsg.c_str());

	return true;
}

// �C���|�[�g�m�F
bool ImpExpManager::ImportAscertain(HINSTANCE hInstance, HWND hwndParent, const wstring& sFileName, wstring& sErrMsg)
{
	return true;
}

// �f�t�H���g�g���q�̎擾
const TCHAR* ImpExpManager::GetDefaultExtension()
{
	return _T("");
}

const wchar_t* ImpExpManager::GetOriginExtension()
{
	return L"";
}

// �t�@�C�����̏����l��ݒ�
void ImpExpManager::SetBaseName(const wstring& sBase)
{
	wchar_t wbuff[_MAX_PATH + 1];
	this->sBase = sBase;
	sOriginName = MakeExportFileName(wbuff, sBase.c_str(), GetOriginExtension());
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �^�C�v�ʐݒ�                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g�m�F
bool ImpExpType::ImportAscertain(HINSTANCE hInstance, HWND hwndParent, const wstring& sFileName, wstring& sErrMsg)
{
	const tstring sPath = to_tchar(sFileName.c_str());
	profile.SetReadingMode();

	if (!profile.ReadProfile(sPath.c_str())) {
		// �ݒ�t�@�C�������݂��Ȃ�
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	// Check Version
	int nStructureVersion = 0;
	wchar_t	szKeyVersion[64];
	if (!profile.IOProfileData(szSecInfo, szKeyStructureVersion, nStructureVersion)) {
		sErrMsg = LSW(STR_IMPEXP_ERR_TYPE);
		return false;
	}
	if ((unsigned int)nStructureVersion != pShareData->vStructureVersion) {
		auto_strcpy(szKeyVersion, L"?");
		profile.IOProfileData(szSecInfo, szKeyVersion, MakeStringBufferW(szKeyVersion));
		int nRet = ConfirmMessage(hwndParent,
			LS(STR_IMPEXP_VER), 
			_APP_NAME_(LTEXT), szKeyVersion, nStructureVersion);
		if (nRet != IDYES) {
			return false;
		}
	}

	// �m�F���F�w��
	DlgTypeAscertain::AscertainInfo sAscertainInfo;
	DlgTypeAscertain dlgTypeAscertain;
	wchar_t wszLabel[1024];
	TypeConfig TmpType;

	// �p�����[�^�̐ݒ�
	sAscertainInfo.sImportFile = sPath;
	List_GetText(hwndList, nIdx, wszLabel);
	sAscertainInfo.sTypeNameTo = wszLabel;
	wszLabel[0] = L'\0';
	profile.IOProfileData(szSecTypes, L"szTypeName", MakeStringBufferW(wszLabel));
	sAscertainInfo.sTypeNameFile = wszLabel;

	// �m�F
	if (!dlgTypeAscertain.DoModal(hInstance, hwndParent, &sAscertainInfo)) {
		return false;
	}

	nColorType = sAscertainInfo.nColorType;
	sColorFile = sAscertainInfo.sColorFile;
	bAddType   = sAscertainInfo.bAddType;

	return true;
}

// �C���|�[�g
bool ImpExpType::Import(const wstring& sFileName, wstring& sErrMsg)
{
	wstring	files = L"";
	wstring TmpMsg;
	ColorInfo colorInfoArr[_countof(types.colorInfoArr)];				// �F�ݒ�z��(�o�b�N�A�b�v)
	
	// �F�̕ύX
	if (nColorType >= MAX_TYPES) {
		// �F�ݒ�C���|�[�g
		ImpExpColors	cImpExpColors(colorInfoArr);
		if (cImpExpColors.Import(cImpExpColors.MakeFullPath(sColorFile), TmpMsg)) {
			files += wstring(L"\n") + sColorFile;
		}else {
			// ���s�������{���R�s�[(���b�Z�[�W�͏o���Ȃ�)
			memcpy(&colorInfoArr, GetDllShareData().typeBasis.colorInfoArr, sizeof(colorInfoArr));
			files += wstring(L"\n�~ ") + sColorFile;	// ���s
		}
	}else if (nColorType >= 0) {
		// �F�w��(����)
		TypeConfig type;
		DocTypeManager().GetTypeConfig(TypeConfigNum(nColorType), type);
		memcpy(&colorInfoArr, type.colorInfoArr, sizeof(colorInfoArr));
	}

	// �ǂݍ���
	ShareData_IO::ShareData_IO_Type_One(profile, types, szSecTypes);

	types.nIdx = nIdx;
	if (nIdx == 0) {
		// ��{�̏ꍇ�̖��O�Ɗg���q��������
		_tcscpy_s(types.szTypeName, LS(STR_TYPE_NAME_BASIS));
		types.szTypeExts[0] = 0;
		types.id = 0;
	}else {
		types.id = (::GetTickCount() & 0x3fffffff) + nIdx * 0x10000;
	}

	// �F�̐ݒ�
	if (nColorType >= 0) {
		// �F�w�肠��
		for (size_t i=0; i<_countof(colorInfoArr); ++i) {
			bool bDisp = types.colorInfoArr[i].bDisp;
			types.colorInfoArr[i] = colorInfoArr[i];
			types.colorInfoArr[i].bDisp = bDisp;		// �\���t���O�̓t�@�C���̂��̂��g�p����
		}
	}

	// ���ʐݒ�Ƃ̘A����
	wchar_t	szKeyName[64];
	wchar_t	szKeyData[1024];
	int		nPlug = 0;
	size_t	nDataLen;
	wchar_t* pSlashPos;
	wchar_t	szFileName[_MAX_PATH + 1];
	bool	bCase;
	wstring	sErrMag;
	CommonSetting& common = pShareData->common;

	// �����L�[���[�h
	KeywordSetMgr&	keywordSetMgr = common.specialKeyword.keywordSetMgr;
	for (size_t i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		//types.nKeywordSetIdx[i] = -1;
		auto_sprintf_s(szKeyName, szKeyKeywordTemp, i + 1);
		if (profile.IOProfileData(szSecTypeEx, szKeyName, MakeStringBufferW(szKeyData))) {
			int nIdx = keywordSetMgr.SearchKeywordSet(szKeyData);
			if (nIdx < 0) {
				// �G���g���쐬
				keywordSetMgr.AddKeywordSet(szKeyData, false);
				nIdx = keywordSetMgr.SearchKeywordSet(szKeyData);
			}
			if (nIdx >= 0) {
				auto_sprintf_s(szKeyName, szKeyKeywordCaseTemp, i + 1);
				bCase = false;		// �啶����������ʂ��Ȃ� (Defaule)
				profile.IOProfileData(szSecTypeEx, szKeyName, bCase);

				// �L�[���[�h��`�t�@�C������
				ImpExpKeyword	impExpKeyword(common, nIdx, bCase);

				auto_sprintf_s(szKeyName, szKeyKeywordFileTemp, i + 1);
				szFileName[0] = L'\0';
				if (profile.IOProfileData(szSecTypeEx, szKeyName, MakeStringBufferW(szFileName))) {
					if (impExpKeyword.Import(impExpKeyword.MakeFullPath(szFileName), TmpMsg)) {
						files += wstring(L"\n") + szFileName;
					}else {
						files += wstring(L"\n�~ ") + szFileName;	// ���s
					}
				}
			}
			types.nKeywordSetIdx[i] = nIdx;
		}
	}

	// Plugin
	//  �A�E�g���C����͕��@
	CommonSetting_Plugin& plugin = common.plugin;
	if (profile.IOProfileData(szSecTypeEx, szKeyPluginOutlineId, MakeStringBufferW(szKeyData))) {
		nDataLen = wcslen(szKeyData);
		pSlashPos = wcschr(szKeyData, L'/');
		int nIdx = -1;
		for (int i=0; i<MAX_PLUGIN; ++i) {
			if (auto_strncmp(szKeyData, plugin.pluginTable[i].szId, pSlashPos ? pSlashPos-szKeyData : nDataLen) == 0) {
				nIdx = i;
				if (pSlashPos) {	// �X���b�V���̌��̃v���OID���擾
					nPlug = _wtoi(pSlashPos + 1);
				}else {
					nPlug = 0;
				}
				break;
			}
		}
		if (nIdx >= 0) {
			types.eDefaultOutline = Plug::GetOutlineType(Plug::GetPluginFunctionCode(nIdx, nPlug));
		}
	}
	// �X�}�[�g�C���f���g
	if (profile.IOProfileData(szSecTypeEx, szKeyPluginSmartIndentId, MakeStringBufferW(szKeyData))) {
		nDataLen = wcslen(szKeyData);
		pSlashPos = wcschr(szKeyData, L'/');
		int nIdx = -1;
		for (int i=0; i<MAX_PLUGIN; ++i) {
			if (auto_strncmp(szKeyData, plugin.pluginTable[i].szId, pSlashPos ? pSlashPos-szKeyData : nDataLen) == 0) {
				nIdx = i;
				if (pSlashPos) {	// �X���b�V���̌��̃v���OID���擾
					nPlug = _wtoi(pSlashPos + 1);
				}else {
					nPlug = 0;
				}
				break;
			}
		}
		if (nIdx >= 0) {
			types.eSmartIndent = Plug::GetSmartIndentType(Plug::GetPluginFunctionCode(nIdx, nPlug));
		}
	}

	sErrMsg = std::wstring(LSW(STR_IMPEXP_OK_IMPORT)) + sFileName + files;

	return true;
}


// �G�N�X�|�[�g
bool ImpExpType::Export(const wstring& sFileName, wstring& sErrMsg)
{
	DataProfile profile;

	profile.SetWritingMode();

	ShareData_IO::ShareData_IO_Type_One(profile , types, szSecTypes);

	// ���ʐݒ�Ƃ̘A����
	wchar_t	szKeyName[64];
	wchar_t buff[64];
	wchar_t	szFileName[_MAX_PATH + 1];
	wstring	files = L"";
	CommonSetting& common = pShareData->common;

	// �����L�[���[�h
	auto& keywordSetMgr = common.specialKeyword.keywordSetMgr;
	for (size_t i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		if (types.nKeywordSetIdx[i] >= 0) {
			int nIdx = types.nKeywordSetIdx[i];
			auto_sprintf_s(szKeyName, szKeyKeywordTemp, i + 1);
			auto_strcpy(buff, keywordSetMgr.GetTypeName(nIdx));
			profile.IOProfileData(szSecTypeEx, szKeyName, MakeStringBufferW(buff));

			// �啶�����������
			bool bCase = keywordSetMgr.GetKeywordCase(nIdx);

			// �L�[���[�h��`�t�@�C���o��
			ImpExpKeyword impExpKeyword(common, types.nKeywordSetIdx[i], bCase);
			impExpKeyword.SetBaseName(keywordSetMgr.GetTypeName(nIdx));

			wstring	sTmpMsg;
			if (impExpKeyword.Export(impExpKeyword.GetFullPath(), sTmpMsg)) {
				auto_strcpy(szFileName, impExpKeyword.GetFileName().c_str());
				auto_sprintf_s(szKeyName, szKeyKeywordFileTemp, i + 1);
				if (profile.IOProfileData(szSecTypeEx, szKeyName, MakeStringBufferW(szFileName))) {
					files += wstring(L"\n") + impExpKeyword.GetFileName();
				}
			}

			auto_sprintf_s(szKeyName, szKeyKeywordCaseTemp, i + 1);
			profile.IOProfileData(szSecTypeEx, szKeyName, bCase);
		}
	}

	// Plugin
	//  �A�E�g���C����͕��@
	CommonSetting_Plugin& plugin = common.plugin;
	int nPIdx;
	int nPlug;
	wchar_t szId[MAX_PLUGIN_ID + 1 + 2];
	if ((nPIdx = Plug::GetPluginId(static_cast<EFunctionCode>(types.eDefaultOutline))) >= 0) {
		profile.IOProfileData(szSecTypeEx, szKeyPluginOutlineName, MakeStringBufferW(plugin.pluginTable[nPIdx].szName));
		wcscpyn(szId, plugin.pluginTable[nPIdx].szId, _countof(szId));
		if ((nPlug = Plug::GetPlugId(static_cast<EFunctionCode>(types.eDefaultOutline))) != 0) {
			wchar_t szPlug[8];
			swprintf(szPlug, L"/%d", nPlug);
			wcscat(szId, szPlug);
		}
		profile.IOProfileData(szSecTypeEx, szKeyPluginOutlineId,   MakeStringBufferW(szId));
	}
	// �X�}�[�g�C���f���g
	if ((nPIdx = Plug::GetPluginId(static_cast<EFunctionCode>(types.eSmartIndent))) >= 0) {
		profile.IOProfileData(szSecTypeEx, szKeyPluginSmartIndentName, MakeStringBufferW(plugin.pluginTable[nPIdx].szName));
		wcscpyn(szId, plugin.pluginTable[nPIdx].szId, _countof(szId));
		if ((nPlug = Plug::GetPlugId(static_cast<EFunctionCode>(types.eSmartIndent))) != 0) {
			wchar_t szPlug[8];
			swprintf(szPlug, L"/%d", nPlug);
			wcscat(szId, szPlug);
		}
		profile.IOProfileData(szSecTypeEx, szKeyPluginSmartIndentId,   MakeStringBufferW(szId));
	}

	// Version
	DllSharedData* pShare = &GetDllShareData();
	int nStructureVersion;
	wchar_t	wbuff[_MAX_PATH + 1];
	auto_sprintf_s(wbuff, L"%d.%d.%d.%d", 
				HIWORD(pShare->version.dwProductVersionMS),
				LOWORD(pShare->version.dwProductVersionMS),
				HIWORD(pShare->version.dwProductVersionLS),
				LOWORD(pShare->version.dwProductVersionLS));
	profile.IOProfileData(szSecInfo, szKeyVersion, MakeStringBufferW(wbuff));
	nStructureVersion = int(pShare->vStructureVersion);
	profile.IOProfileData(szSecInfo, szKeyStructureVersion, nStructureVersion);

	// ��������
	if (!profile.WriteProfile(to_tchar(sFileName.c_str()), WSTR_TYPE_HEAD)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	sErrMsg = std::wstring(LSW(STR_IMPEXP_OK_EXPORT)) + sFileName + files;

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �J���[                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool ImpExpColors::Import(const wstring& sFileName, wstring& sErrMsg)
{
	const tstring strPath = to_tchar(sFileName.c_str());

	// �J���邩
	TextInputStream in(strPath.c_str());
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	// �t�@�C���擪
	// �w�b�_�ǎ�
	wstring szHeader = in.ReadLineW();
	if (szHeader.length() >= 2) {
		// �R�����g�𔲂�
		szHeader = &szHeader.c_str()[szHeader.c_str()[0] == _T(';') ? 1 : 2];
	}
	// ��r
	if (szHeader != WSTR_COLORDATA_HEAD3) {
		in.Close();
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_COLOR_OLD))	// ���o�[�W�����̐����̍폜
			+ sFileName;
		return false;
	}
	in.Close();

	DataProfile profile;
	profile.SetReadingMode();

	// �F�ݒ�Ver3
	if (!profile.ReadProfile(strPath.c_str())) {
		return false;
	}

	// �F�ݒ� I/O
	ShareData_IO::IO_ColorSet(&profile, szSecColor, colorInfoArr);

	return true;
}

// �G�N�X�|�[�g
bool ImpExpColors::Export(const wstring& sFileName, wstring& sErrMsg)
{
	// �F�ݒ� I/O
	DataProfile profile;
	profile.SetWritingMode();
	ShareData_IO::IO_ColorSet(&profile, szSecColor, colorInfoArr);
	if (!profile.WriteProfile(to_tchar(sFileName.c_str()), WSTR_COLORDATA_HEAD3)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���K�\���L�[���[�h                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool ImpExpRegex::Import(const wstring& sFileName, wstring& sErrMsg)
{
	TextInputStream in(to_tchar(sFileName.c_str()));
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	RegexKeywordInfo regexKeyArr[MAX_REGEX_KEYWORD];
	auto szKeywordList = std::make_unique<wchar_t[]>(MAX_REGEX_KEYWORDLISTLEN);
	wchar_t* pKeyword = &szKeywordList[0];
	size_t keywordPos = 0;
	TCHAR buff[MAX_REGEX_KEYWORDLEN + 20];
	size_t count = 0;
	while (in) {
		// 1�s�ǂݍ���
		wstring line = in.ReadLineW();
		_wcstotcs(buff, line.c_str(), _countof(buff));

		if (count >= MAX_REGEX_KEYWORD) {
			sErrMsg = LSW(STR_IMPEXP_REGEX1);
			break;
		}

		//RxKey[999]=ColorName,RegexKeyword
		if (auto_strlen(buff) < 12) {
			continue;
		}
		if (auto_memcmp(buff, _T("RxKey["), 6) != 0) {
			continue;
		}
		if (auto_memcmp(&buff[9], _T("]="), 2) != 0) {
			continue;
		}
		TCHAR* p = auto_strstr(&buff[11], _T(","));
		if (p) {
			*p = _T('\0');
			++p;
			if (p[0] && RegexKeyword::RegexKeyCheckSyntax(to_wchar(p))) {	// �݂͂�����
				// �F�w�薼�ɑΉ�����ԍ���T��
				int k = GetColorIndexByName(&buff[11]);
				if (k == -1) {
					// ���{�ꖼ����C���f�b�N�X�ԍ��ɕϊ�����
					for (int m=0; m<COLORIDX_LAST; ++m) {
						if (auto_strcmp(types.colorInfoArr[m].szName, &buff[11]) == 0) {
							k = m;
							break;
						}
					}
				}
				if (k != -1) {	// 3�����J���[������C���f�b�N�X�ԍ��ɕϊ�
					if (0 < MAX_REGEX_KEYWORDLISTLEN - keywordPos - 1) {
						regexKeyArr[count].nColorIndex = k;
						_tcstowcs(&pKeyword[keywordPos], p, t_min<size_t>(MAX_REGEX_KEYWORDLEN, MAX_REGEX_KEYWORDLISTLEN - keywordPos - 1));
						++count;
						keywordPos += auto_strlen(&pKeyword[keywordPos]) + 1;
					}else {
						sErrMsg = LSW(STR_IMPEXP_REGEX2);
					}
				}
			}else {
				sErrMsg = LSW(STR_IMPEXP_REGEX3);
			}
		}
	}
	pKeyword[keywordPos] = L'\0';

	in.Close();

	for (size_t i=0; i<count; ++i) {
		types.regexKeywordArr[i] = regexKeyArr[i];
	}
	for (size_t i=0; i<=keywordPos; ++i) {
		types.regexKeywordList[i] = pKeyword[i];
	}

	return true;
}

// �G�N�X�|�[�g
bool ImpExpRegex::Export(const wstring& sFileName, wstring& sErrMsg)
{
	TextOutputStream out(to_tchar(sFileName.c_str()));
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.WriteF(WSTR_REGEXKW_HEAD);

	const wchar_t* regex = types.regexKeywordList;
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (regex[0] == L'\0') {
			break;
		}
		
		const TCHAR* name  = GetColorNameByIndex(types.regexKeywordArr[i].nColorIndex);
		out.WriteF(L"RxKey[%03d]=%ts,%ls\n", i, name, regex);

		for (; *regex!='\0'; ++regex) {}
		++regex;
	}

	out.Close();

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�w���v                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*! �C���|�[�g */
bool ImpExpKeyHelp::Import(const wstring& sFileName, wstring& sErrMsg)
{
	wchar_t msgBuff[_MAX_PATH + 1];
	TextInputStream in(to_tchar(sFileName.c_str()));
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	// �f�[�^�擾
	int invalid_record = 0; // �s���ȍs
	int i = 0;
	while (in && i<MAX_KEYHELP_FILE) {
		wstring buff = in.ReadLineW();

		if (buff[0] == LTEXT('\0') ||
			buff[0] == LTEXT('\n') ||
			buff[0] == LTEXT('#') ||
			buff[0] == LTEXT(';') ||
			(buff[0] == LTEXT('/') && buff[1] == LTEXT('/'))
		) {
			continue;
		}

		//KDct[99]=ON/OFF,DictAbout,KeyHelpPath
		if (buff.length() < 10 ||
			auto_memcmp(buff.c_str(), LTEXT("KDct["), 5) != 0 ||
			auto_memcmp(&buff[7], LTEXT("]="), 2) != 0
		) {
			// �������p��
			++invalid_record;
			continue;
		}

		wchar_t *p1, *p2, *p3;
		p1 = &buff[9];
		p3 = p1;					// ���ʊm�F�p�ɏ�����
		if ((p2 = wcsstr(p1, LTEXT(",")))) {
			*p2 = LTEXT('\0');
			p2 += 1;				// �J���}�̎����A���̗v�f
			if ((p3=wcsstr(p2, LTEXT(",")))) {
				*p3 = LTEXT('\0');
				p3 += 1;			// �J���}�̎����A���̗v�f
			}
		}// ���ʂ̊m�F
		if (!p3 ||			// �J���}��1����Ȃ�
			(p3 == p1) //||			// �J���}��2����Ȃ�
		) {
			// �������p��
			++invalid_record;
			continue;
		}
		// value�̃`�F�b�N
		// ON/OFF
		// 1�łȂ����1�ɂ���
		unsigned int b_enable_flag = (unsigned int)_wtoi(p1);
		if (b_enable_flag > 1) {
			b_enable_flag = 1;
		}
		// Path
		FILE* fp2;
		const wchar_t* p4 = p2;
		if (!(fp2=_tfopen_absini(to_tchar(p3), _T("r")))) {	// ���΃p�X��sakura.exe��ŊJ��
			// ������������Ȃ��ꍇ�̑[�u�D�x�����o������荞��
			p4 = LSW(STR_IMPEXP_DIC_NOTFOUND);
			b_enable_flag = 0;
		}else {
			fclose(fp2);
		}

		// About
		if (wcslen(p2) > DICT_ABOUT_LEN) {
			auto_sprintf_s(msgBuff, LSW(STR_IMPEXP_DIC_LENGTH), DICT_ABOUT_LEN);
			sErrMsg = msgBuff;
			++invalid_record;
			continue;
		}

		// �ǂ������Ȃ�
		types.keyHelpArr[i].bUse = (b_enable_flag != 0);
		_tcscpy_s(types.keyHelpArr[i].szAbout, to_tchar(p4));
		_tcscpy(types.keyHelpArr[i].szPath,  to_tchar(p3));
		++i;
	}
	in.Close();

	// �󂫂�����Ȃ�ԕ���ݒ�
	if (i < _countof(types.keyHelpArr)) {
		types.keyHelpArr[i].bUse = false;
		types.keyHelpArr[i].szAbout[0] = _T('\0');
		types.keyHelpArr[i].szPath[0]  = _T('\0');
	}
	types.nKeyHelpNum = i;

	// ���s������x������
	if (invalid_record > 0) {
		auto_sprintf_s(msgBuff, LSW(STR_IMPEXP_DIC_RECORD), invalid_record);
		sErrMsg = msgBuff;
	}

	return true;
}

/*! �G�N�X�|�[�g */
bool ImpExpKeyHelp::Export(const wstring& sFileName, wstring& sErrMsg)
{
	TextOutputStream out(to_tchar(sFileName.c_str()));
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.WriteF(WSTR_KEYHELP_HEAD);
	for (int i=0; i<types.nKeyHelpNum; ++i) {
		out.WriteF(
			L"KDct[%02d]=%d,%ts,%ts\n",
			i,
			types.keyHelpArr[i].bUse ? 1 : 0,
			types.keyHelpArr[i].szAbout,
			types.keyHelpArr[i].szPath.c_str()
		);
	}
	out.Close();

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���蓖��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool ImpExpKeybind::Import(const wstring& sFileName, wstring& sErrMsg)
{
	const tstring strPath = to_tchar(sFileName.c_str());
	const size_t keyNameSize = _countof(common.keyBind.pKeyNameArr) - 1; // �Ō�̂P�v�f�̓_�~�[�p�ɗ\��
	CommonSetting_KeyBind sKeyBind = common.keyBind;

	// �I�[�v��
	DataProfile in;
	in.SetReadingMode();
	if (!in.ReadProfile(to_tchar(sFileName.c_str()))) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	// �o�[�W�����m�F
	bool bVer4;			// �V�o�[�W�����i������Ή��j�̃t�@�C��
	bool bVer3;			// �V�o�[�W�����̃t�@�C��
	bool bVer2;
	wchar_t szHeader[256];
	bVer4 = false;
	bVer3 = false;
	bVer2 = false;
	in.IOProfileData(szSecInfo, L"KEYBIND_VERSION", MakeStringBufferW(szHeader));
	if (wcscmp(szHeader, WSTR_KEYBIND_HEAD4) == 0) {
		bVer4 = true;
	}else if (wcscmp(szHeader, WSTR_KEYBIND_HEAD3) == 0) {
		bVer3 = true;
	}

	//int	nKeyNameArrNum;			// �L�[���蓖�ĕ\�̗L���f�[�^��
	if (bVer3 || bVer4) {
		// Count�擾 -> nKeyNameArrNum
		in.IOProfileData(szSecInfo, L"KEYBIND_COUNT", sKeyBind.nKeyNameArrNum);
		if (sKeyBind.nKeyNameArrNum < 0 || sKeyBind.nKeyNameArrNum > keyNameSize) {	bVer3=false; bVer4=false; } // �͈̓`�F�b�N

		ShareData_IO::IO_KeyBind(in, sKeyBind, true);
	}

	if (!bVer3 && !bVer4) {
		// �V�o�[�W�����łȂ�
		TextInputStream in(strPath.c_str());
		if (!in) {
			sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
			return false;
		}
		// �w�b�_�`�F�b�N
		wstring	szLine = in.ReadLineW();
		bVer2 = true;
		if (wcscmp(szLine.c_str(), WSTR_KEYBIND_HEAD2) != 0) {
			bVer2 = false;
		}
		// �J�E���g�`�F�b�N
		int	cnt;
		if (bVer2) {
			int	an;
			szLine = in.ReadLineW();
			cnt = swscanf(szLine.c_str(), L"Count=%d", &an);
			if (cnt != 1 || an < 0 || an > keyNameSize) {
				bVer2 = false;
			}else {
				sKeyBind.nKeyNameArrNum = an;
			}
		}
		if (bVer2) {
			// �e�v�f�擾
			for (int i=0; i<keyNameSize; ++i) {
				int n, kc, nc;
				// �l -> szData
				wchar_t szData[1024];
				auto_strcpy(szData, in.ReadLineW().c_str());

				// ��͊J�n
				cnt = swscanf(szData, L"KeyBind[%03d]=%04x,%n",
												&n,   &kc, &nc);
				if (cnt !=2 && cnt !=3)	{
					bVer2= false;
					break;
				}
				if (i != n) {
					break;
				}
				sKeyBind.pKeyNameArr[i].nKeyCode = (short)kc;
				wchar_t* p = szData + nc;

				// ��ɑ����g�[�N��
				for (int j=0; j<8; ++j) {
					wchar_t* q = auto_strchr(p, L',');
					if (!q) {
						bVer2 = false;
						break;
					}
					*q = L'\0';

					// �@�\���𐔒l�ɒu��������B(���l�̋@�\�������邩��)
					EFunctionCode n = SMacroMgr::GetFuncInfoByName(G_AppInstance(), p, NULL);
					if (n == F_INVALID) {
						if (WCODE::Is09(*p)) {
							n = (EFunctionCode)auto_atol(p);
						}else {
							n = F_DEFAULT;
						}
					}
					sKeyBind.pKeyNameArr[i].nFuncCodeArr[j] = n;
					p = q + 1;
				}

				auto_strncpy(sKeyBind.pKeyNameArr[i].szKeyName, to_tchar(p), _countof(sKeyBind.pKeyNameArr[i].szKeyName) - 1);
				sKeyBind.pKeyNameArr[i].szKeyName[_countof(sKeyBind.pKeyNameArr[i].szKeyName) - 1] = '\0';
			}
		}
	}
	if (!bVer4 && !bVer3 && !bVer2) {
		sErrMsg = wstring(LSW(STR_IMPEXP_KEY_FORMAT)) + sFileName;
		return false;
	}

	int nKeyNameArrUsed = common.keyBind.nKeyNameArrNum; // �g�p�ςݗ̈�
	for (int j=sKeyBind.nKeyNameArrNum-1; j>=0; --j) {
		if ((bVer2 || bVer3) && sKeyBind.pKeyNameArr[j].nKeyCode <= 0) { // �}�E�X�R�[�h�͐擪�ɌŒ肳��Ă��� KeyCode�������Ȃ̂�KeyName�Ŕ���
			for (int im=0; im<(int)MouseFunctionType::KeyBegin; ++im) {
				if (_tcscmp(sKeyBind.pKeyNameArr[j].szKeyName, common.keyBind.pKeyNameArr[im].szKeyName) == 0) {
					common.keyBind.pKeyNameArr[im] = sKeyBind.pKeyNameArr[j];
				}
			}
		}else {
			// ���蓖�čς݃L�[�R�[�h�͏㏑��
			int idx = sKeyBind.keyToKeyNameArr[sKeyBind.pKeyNameArr[j].nKeyCode];
			if (idx != keyNameSize) {
				common.keyBind.pKeyNameArr[idx] = sKeyBind.pKeyNameArr[j];
			}
		}
	}
	// �����蓖�ẴL�[�R�[�h�͋󂫗̈悪��t�ɂȂ�܂Œǉ�
	for (int j2=0; j2<sKeyBind.nKeyNameArrNum; ++j2) {
		int idx = sKeyBind.keyToKeyNameArr[sKeyBind.pKeyNameArr[j2].nKeyCode];
		if (idx == keyNameSize) { // not assigned
			if (nKeyNameArrUsed >= keyNameSize) {
				continue;
			}
			common.keyBind.pKeyNameArr[nKeyNameArrUsed] = sKeyBind.pKeyNameArr[j2];
			sKeyBind.keyToKeyNameArr[sKeyBind.pKeyNameArr[j2].nKeyCode] = (BYTE)nKeyNameArrUsed++;
		}
	}
	common.keyBind.nKeyNameArrNum = nKeyNameArrUsed;
	memcpy_raw(common.keyBind.keyToKeyNameArr, sKeyBind.keyToKeyNameArr, sizeof_raw(sKeyBind.keyToKeyNameArr));

	return true;
}

// �G�N�X�|�[�g
bool ImpExpKeybind::Export(const wstring& sFileName, wstring& sErrMsg)
{
	const tstring strPath = to_tchar(sFileName.c_str());

	TextOutputStream out(strPath.c_str());
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.Close();

	// �L�[���蓖�ď��
	DataProfile profile;

	// �������݃��[�h�ݒ�
	profile.SetWritingMode();

	// �w�b�_
	StaticString<wchar_t, 256> szKeydataHead = WSTR_KEYBIND_HEAD4;
	profile.IOProfileData(szSecInfo, L"KEYBIND_VERSION", szKeydataHead);
	profile.IOProfileData_WrapInt(szSecInfo, L"KEYBIND_COUNT", common.keyBind.nKeyNameArrNum);

	// ���e
	ShareData_IO::IO_KeyBind(profile, common.keyBind, true);

	// ��������
	if (!profile.WriteProfile(strPath.c_str(), WSTR_KEYBIND_HEAD4)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �J�X�^�����j���[                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool ImpExpCustMenu::Import(const wstring& sFileName, wstring& sErrMsg)
{
	const tstring strPath = to_tchar(sFileName.c_str());

	// �w�b�_�m�F
	TextInputStream in(strPath.c_str());
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	DataProfile profile;
	profile.SetReadingMode();
	profile.ReadProfile(strPath.c_str());

	// �o�[�W�����m�F
	wchar_t szHeader[256];
	profile.IOProfileData(szSecInfo, L"MENU_VERSION", MakeStringBufferW(szHeader));
	if (wcscmp(szHeader, WSTR_CUSTMENU_HEAD_V2) != 0) {
		sErrMsg = wstring(LSW(STR_IMPEXP_CUSTMENU_FORMAT)) + sFileName;
		return false;
	}

	ShareData_IO::IO_CustMenu(profile, common.customMenu, true);

	return true;
}

// �G�N�X�|�[�g
bool ImpExpCustMenu::Export(const wstring& sFileName, wstring& sErrMsg)
{
	const tstring strPath = to_tchar(sFileName.c_str());

	// �I�[�v��
	TextOutputStream out(strPath.c_str());
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.Close();

	// �J�X�^�����j���[���
	// �w�b�_
	DataProfile	profile;
	CommonSetting_CustomMenu* menu=&common.customMenu;

	// �������݃��[�h�ݒ�
	profile.SetWritingMode();

	// �w�b�_
	profile.IOProfileData(szSecInfo, L"MENU_VERSION", MakeStringBufferW(WSTR_CUSTMENU_HEAD_V2));
	int iWork = MAX_CUSTOM_MENU;
	profile.IOProfileData_WrapInt(szSecInfo, L"MAX_CUSTOM_MENU", iWork);
	
	// ���e
	ShareData_IO::IO_CustMenu(profile, *menu, true);

	// ��������
	if (!profile.WriteProfile(strPath.c_str(), WSTR_CUSTMENU_HEAD_V2)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �����L�[���[�h                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool ImpExpKeyword::Import(const wstring& sFileName, wstring& sErrMsg)
{
	bool bAddError = false;

	TextInputStream in(to_tchar(sFileName.c_str()));
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}
	while (in) {
		wstring szLine = in.ReadLineW();

		// �R�����g����
		if (szLine.length() == 0) {
			continue;
		}
		if (2 <= szLine.length() && auto_memcmp(szLine.c_str(), L"//", 2) == 0) {
			if (szLine == WSTR_CASE_TRUE) {
				bCase = true;
			}else if (szLine == WSTR_CASE_FALSE) {
				bCase = false;
			}
			continue;
		}
		
		// ���
		if (0 < szLine.length()) {
			// ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ�
			size_t nRetValue = common.specialKeyword.keywordSetMgr.AddKeyword(nIdx, szLine.c_str());
			if (nRetValue == 2) {
				bAddError = true;
				break;
			}
		}
	}
	in.Close();

	// �啶�����������
	common.specialKeyword.keywordSetMgr.SetKeywordCase(nIdx, bCase);

	if (bAddError) {
		sErrMsg = LSW(STR_IMPEXP_KEYWORD);
	}

	return true;
}

// �G�N�X�|�[�g
bool ImpExpKeyword::Export(const wstring& sFileName, wstring& sErrMsg)
{
	TextOutputStream out(to_tchar(sFileName.c_str()));
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}
	out.WriteF(L"// ");
	out.WriteString(common.specialKeyword.keywordSetMgr.GetTypeName(nIdx));
	out.WriteF(WSTR_KEYWORD_HEAD);

	out.WriteF(WSTR_KEYWORD_CASE);
	out.WriteF(bCase ? L"True" : L"False");
	out.WriteF(L"\n\n");

	common.specialKeyword.keywordSetMgr.SortKeyword(nIdx);

	// ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ�
	size_t nKeywordNum = common.specialKeyword.keywordSetMgr.GetKeywordNum(nIdx);
	for (size_t i=0; i<nKeywordNum; ++i) {
		// ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ�
		out.WriteString(common.specialKeyword.keywordSetMgr.GetKeyword(nIdx, i));
		out.WriteF(L"\n");
	}
	out.Close();

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ���C�����j���[                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool ImpExpMainMenu::Import(const wstring& sFileName, wstring& sErrMsg)
{
	const tstring strPath = to_tchar(sFileName.c_str());

	// �w�b�_�m�F
	TextInputStream in(strPath.c_str());
	if (!in) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	DataProfile profile;
	profile.SetReadingMode();
	profile.ReadProfile(strPath.c_str());

	// �o�[�W�����m�F
	wchar_t szHeader[256];
	profile.IOProfileData(szSecInfo, L"MENU_VERSION", MakeStringBufferW(szHeader));
	if (wcscmp(szHeader, WSTR_MAINMENU_HEAD_V1) != 0) {
		sErrMsg = wstring(LSW(STR_IMPEXP_MEINMENU)) + sFileName;
		return false;
	}

	ShareData_IO::IO_MainMenu(profile, common.mainMenu, true);

	return true;
}

// �G�N�X�|�[�g
bool ImpExpMainMenu::Export(const wstring& sFileName, wstring& sErrMsg)
{
	const tstring strPath = to_tchar(sFileName.c_str());

	// �I�[�v��
	TextOutputStream out(strPath.c_str());
	if (!out) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_FILEOPEN)) + sFileName;
		return false;
	}

	out.Close();

	// �w�b�_
	DataProfile	profile;
	CommonSetting_MainMenu* menu=&common.mainMenu;

	// �������݃��[�h�ݒ�
	profile.SetWritingMode();

	// �w�b�_
	profile.IOProfileData(szSecInfo, L"MENU_VERSION", MakeStringBufferW(WSTR_MAINMENU_HEAD_V1));
	
	// ���e
	ShareData_IO::IO_MainMenu(profile, *menu, true);

	// ��������
	if (!profile.WriteProfile(strPath.c_str(), WSTR_MAINMENU_HEAD_V1)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �t�@�C���c���[                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �C���|�[�g
bool ImpExpFileTree::Import(const wstring& sFileName, wstring& sErrMsg)
{
	const tstring strPath = to_tchar(sFileName.c_str());

	DataProfile profile;
	profile.SetReadingMode();
	profile.ReadProfile(strPath.c_str());

	IO_FileTreeIni(profile, fileTreeItems);

	return true;
}

// �G�N�X�|�[�g
bool ImpExpFileTree::Export(const wstring& sFileName, wstring& sErrMsg)
{
	const tstring strPath = to_tchar(sFileName.c_str());

	DataProfile profile;

	// �������݃��[�h�ݒ�
	profile.SetWritingMode();

	IO_FileTreeIni(profile, fileTreeItems);

	// ��������
	if (!profile.WriteProfile(strPath.c_str(), WSTR_FILETREE_HEAD_V1)) {
		sErrMsg = std::wstring(LSW(STR_IMPEXP_ERR_EXPORT)) + sFileName;
		return false;
	}

	return true;
}

void ImpExpFileTree::IO_FileTreeIni( DataProfile& profile, std::vector<FileTreeItem>& data )
{
	const wchar_t* pszSecName = L"FileTree";
	int nItemCount = (int)data.size();
	profile.IOProfileData(pszSecName, L"nFileTreeItemCount", nItemCount);
	if (nItemCount < 0) {
		nItemCount = 0;
	}
	if (profile.IsReadingMode()) {
		data.resize(nItemCount);
	}
	for (int i=0; i<nItemCount; ++i) {
		ShareData_IO::ShareData_IO_FileTreeItem(profile, data[i], pszSecName, i);
	}
}

