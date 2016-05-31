/*
	2008.05.18 kobake CShareData ���番��
*/
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
#include <ShlObj.h> // CSIDL_PROFILE��

#include "DllSharedData.h"
#include "FileNameManager.h"
#include "charset/CodePage.h"
#include "util/module.h"
#include "util/os.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/fileUtil.h"
#include "util/window.h"
#include "_main/CommandLine.h"
#include "_os/OsVersionInfo.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �t�@�C�����Ǘ�                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!	���L�f�[�^�̐ݒ�ɏ]���ăp�X���k���\�L�ɕϊ�����
	@param pszSrc   [in]  �t�@�C����
	@param pszDest  [out] �ϊ���̃t�@�C�����̊i�[��
	@param nDestLen [in]  �I�[��NULL���܂�pszDest��TCHAR�P�ʂ̒��� _MAX_PATH �܂�
	@date 2003.01.27 Moca �V�K�쐬
	@note �A�����ČĂяo���ꍇ�̂��߁A�W�J�ς݃��^��������L���b�V�����č��������Ă���B
*/
LPTSTR FileNameManager::GetTransformFileNameFast( LPCTSTR pszSrc, LPTSTR pszDest, size_t nDestLen, HDC hDC, bool bFitMode, int cchMaxWidth )
{
	TCHAR szBuf[_MAX_PATH + 1];

	if (nTransformFileNameCount == -1) {
		TransformFileName_MakeCache();
	}

	int nPxWidth = -1;
	auto& csFileName = pShareData->common.fileName;
	if (csFileName.bTransformShortPath && cchMaxWidth != -1) {
		if (cchMaxWidth == 0) {
			cchMaxWidth = csFileName.nTransformShortMaxWidth;
		}
		TextWidthCalc calc(hDC);
		nPxWidth = calc.GetTextWidth(_T("x")) * cchMaxWidth;
	}

	if (0 < nTransformFileNameCount) {
		GetFilePathFormat(pszSrc, pszDest, nDestLen,
			szTransformFileNameFromExp[0],
			csFileName.szTransformFileNameTo[nTransformFileNameOrgId[0]]
		);
		for (int i=1; i<nTransformFileNameCount; ++i) {
			_tcscpy(szBuf, pszDest);
			GetFilePathFormat(szBuf, pszDest, nDestLen,
				szTransformFileNameFromExp[i],
				csFileName.szTransformFileNameTo[nTransformFileNameOrgId[i]]);
		}
		if (nPxWidth != -1) {
			_tcscpy( szBuf, pszDest );
			GetShortViewPath( pszDest, nDestLen, szBuf, hDC, nPxWidth, bFitMode );
		}
	}else if (nPxWidth != -1) {
		GetShortViewPath( pszDest, nDestLen, pszSrc, hDC, nPxWidth, bFitMode );
	}else {
		// �ϊ�����K�v���Ȃ� �R�s�[��������
		_tcsncpy(pszDest, pszSrc, nDestLen - 1);
		pszDest[nDestLen - 1] = '\0';
	}
	return pszDest;
}

/*!	�W�J�ς݃��^������̃L���b�V�����쐬�E�X�V����
	@retval �L���ȓW�J�ςݒu���O������̐�
	@date 2003.01.27 Moca �V�K�쐬
	@date 2003.06.23 Moca �֐����ύX
*/
int FileNameManager::TransformFileName_MakeCache(void) {
	int nCount = 0;
	auto& csFileName = pShareData->common.fileName;
	for (int i=0; i<csFileName.nTransformFileNameArrNum; ++i) {
		if (csFileName.szTransformFileNameFrom[i][0] != L'\0') {
			if (ExpandMetaToFolder(
				csFileName.szTransformFileNameFrom[i],
				szTransformFileNameFromExp[nCount],
				_MAX_PATH
				)
			) {
				// szTransformFileNameTo��szTransformFileNameFromExp�̔ԍ�������邱�Ƃ�����̂ŋL�^���Ă���
				nTransformFileNameOrgId[nCount] = i;
				++nCount;
			}
		}
	}
	nTransformFileNameCount = nCount;
	return nCount;
}


/*!	�t�@�C���E�t�H���_����u�����āA�ȈՕ\�������擾����
	@date 2002.11.27 Moca �V�K�쐬
	@note �召��������ʂ��Ȃ��BnDestLen�ɒB�����Ƃ��͌���؂�̂Ă���
*/
LPCTSTR FileNameManager::GetFilePathFormat(LPCTSTR pszSrc, LPTSTR pszDest, size_t nDestLen, LPCTSTR pszFrom, LPCTSTR pszTo)
{
	size_t nSrcLen  = _tcslen(pszSrc);
	size_t nFromLen = _tcslen(pszFrom);
	size_t nToLen   = _tcslen(pszTo);

	assert(nDestLen > 0);
	--nDestLen;

	size_t j = 0;
	for (size_t i=0; i<nSrcLen && j<nDestLen; ++i) {
		if (_tcsncicmp(&pszSrc[i], pszFrom, nFromLen) == 0)
		{
			size_t nCopy = t_min(nToLen, nDestLen - j);
			memcpy(&pszDest[j], pszTo, nCopy * sizeof(TCHAR));
			j += nCopy;
			i += nFromLen - 1;
		}else {
			pszDest[j] = pszSrc[i];
			++j;
		}
	}
// end_of_func:;
	pszDest[j] = '\0';
	return pszDest;
}


/*!	%MYDOC%�Ȃǂ̃p�����[�^�w������ۂ̃p�X���ɕϊ�����

	@param pszSrc  [in]  �ϊ��O������
	@param pszDes  [out] �ϊ��㕶����
	@param nDesLen [in]  pszDes��NULL���܂�TCHAR�P�ʂ̒���
	@retval true  ����ɕϊ��ł���
	@retval false �o�b�t�@������Ȃ������C�܂��̓G���[�BpszDes�͕s��
	@date 2002.11.27 Moca �쐬�J�n
*/
bool FileNameManager::ExpandMetaToFolder(LPCTSTR pszSrc, LPTSTR pszDes, int nDesLen)
{
#define _USE_META_ALIAS
#ifdef _USE_META_ALIAS
	struct MetaAlias {
		LPCTSTR szAlias;
		int nLenth;
		LPCTSTR szOrig;
	};
	static const MetaAlias AliasList[] = {
		{  _T("COMDESKTOP"), 10, _T("Common Desktop") },
		{  _T("COMMUSIC"), 8, _T("CommonMusic") },
		{  _T("COMVIDEO"), 8, _T("CommonVideo") },
		{  _T("MYMUSIC"),  7, _T("My Music") },
		{  _T("MYVIDEO"),  7, _T("Video") },
		{  _T("COMPICT"),  7, _T("CommonPictures") },
		{  _T("MYPICT"),   6, _T("My Pictures") },
		{  _T("COMDOC"),   6, _T("Common Documents") },
		{  _T("MYDOC"),    5, _T("Personal") },
		{ NULL, 0 , NULL }
	};
#endif

	LPCTSTR ps;
	LPTSTR pd;
	LPTSTR pd_end = pszDes + (nDesLen - 1);
	for (ps=pszSrc, pd=pszDes; *ps!=_T('\0'); ++ps) {
		if (pd_end <= pd) {
			if (pd_end == pd) {
				*pd = _T('\0');
			}
			return false;
		}

		if (*ps != _T('%')) {
			*pd = *ps;
			++pd;
			continue;
		}

		// %% �� %
		if (ps[1] == _T('%')) {
			*pd = _T('%');
			++pd;
			++ps;
			continue;
		}

		if (ps[1] != _T('\0')) {
			TCHAR szMeta[_MAX_PATH];
			TCHAR szPath[_MAX_PATH + 1];
			ptrdiff_t nMetaLen;
			size_t   nPathLen;
			bool  bFolderPath;
			LPCTSTR  pStr;
			++ps;
			// %SAKURA%
			if (auto_strnicmp(_T("SAKURA%"), ps, 7) == 0) {
				// exe�̂���t�H���_
				GetExedir(szPath);
				nMetaLen = 6;
			// %SAKURADATA%	// 2007.06.06 ryoji
			}else if (auto_strnicmp(_T("SAKURADATA%"), ps, 11) == 0) {
				// ini�̂���t�H���_
				GetInidir(szPath);
				nMetaLen = 10;
			// ���^��������ۂ�
			}else if (pStr = _tcschr(ps, _T('%'))) {
				nMetaLen = pStr - ps;
				if (nMetaLen < _MAX_PATH) {
					auto_memcpy(szMeta, ps, nMetaLen);
					szMeta[nMetaLen] = _T('\0');
				}else {
					*pd = _T('\0');
					return false;
				}
#ifdef _USE_META_ALIAS
				// ���^�����񂪃G�C���A�X���Ȃ珑��������
				const MetaAlias* pAlias;
				for (pAlias=&AliasList[0]; nMetaLen<pAlias->nLenth; ++pAlias)
					; // �ǂݔ�΂�
				for (; nMetaLen==pAlias->nLenth; ++pAlias) {
					if (auto_stricmp(pAlias->szAlias, szMeta) == 0) {
						_tcscpy(szMeta, pAlias->szOrig);
						break;
					}
				}
#endif
				// ���ڃ��W�X�g���Œ��ׂ�
				szPath[0] = _T('\0');
				bFolderPath = ReadRegistry(HKEY_CURRENT_USER,
					_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
					szMeta, szPath, _countof(szPath));
				if (!bFolderPath || _T('\0') == szPath[0]) {
					bFolderPath = ReadRegistry(HKEY_LOCAL_MACHINE,
						_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
						szMeta, szPath, _countof(szPath));
				}
				if (!bFolderPath || szPath[0] == _T('\0')) {
					pStr = _tgetenv(szMeta);
					// ���ϐ�
					if (pStr) {
						nPathLen = _tcslen(pStr);
						if (nPathLen < _MAX_PATH) {
							_tcscpy(szPath, pStr);
						}else {
							*pd = _T('\0');
							return false;
						}
					// ����`�̃��^������� ���͂��ꂽ%...%���C���̂܂ܕ����Ƃ��ď�������
					}else if (pd + (nMetaLen + 2) < pd_end) {
						*pd = _T('%');
						auto_memcpy(&pd[1], ps, nMetaLen);
						pd[nMetaLen + 1] = _T('%');
						pd += nMetaLen + 2;
						ps += nMetaLen;
						continue;
					}else {
						*pd = _T('\0');
						return false;
					}
				}
			}else {
				// %...%�̏I����%���Ȃ� �Ƃ肠�����C%���R�s�[
				*pd = _T('%');
				++pd;
				--ps; // ���++ps���Ă��܂����̂Ŗ߂�
				continue;
			}

			// �����O�t�@�C�����ɂ���
			nPathLen = _tcslen(szPath);
			LPTSTR pStr2 = szPath;
			if (nPathLen < _MAX_PATH && nPathLen != 0) {
				if (GetLongFileName(szPath, szMeta) != FALSE) {
					pStr2 = szMeta;
				}
			}

			// �Ō�̃t�H���_��؂�L�����폜����
			// [A:\]�Ȃǂ̃��[�g�ł����Ă��폜
			for (nPathLen=0; pStr2[nPathLen]!=_T('\0'); ++nPathLen) {
				if (pStr2[nPathLen] == _T('\\') && pStr2[nPathLen + 1] == _T('\0')) {
					pStr2[nPathLen] = _T('\0');
					break;
				}
			}

			if (pd + nPathLen < pd_end && nPathLen != 0) {
				auto_memcpy(pd, pStr2, nPathLen);
				pd += nPathLen;
				ps += nMetaLen;
			}else {
				*pd = _T('\0');
				return false;
			}
		}else {
			// �Ō�̕�����%������
			*pd = *ps;
			++pd;
		}
	}
	*pd = _T('\0');
	return true;
}


// static
TCHAR FileNameManager::GetAccessKeyByIndex(int index, bool bZeroOrigin)
{
	if (index < 0) {
		return 0;
	}
	int accKeyIndex = ((bZeroOrigin? index: index + 1) % 36);
	TCHAR c = (TCHAR)((accKeyIndex < 10) ? (_T('0') + accKeyIndex) : (_T('A') + accKeyIndex - 10));
	return c;
}

static void GetAccessKeyLabelByIndex(TCHAR* pszLabel, bool bEspaceAmp, int index, bool bZeroOrigin)
{
	TCHAR c = FileNameManager::GetAccessKeyByIndex(index, bZeroOrigin);
	if (c) {
		if (bEspaceAmp) {
			pszLabel[0] = _T('&');
			pszLabel[1] = c;
			pszLabel[2] = _T(' ');
			pszLabel[3] = _T('\0');
		}else {
			pszLabel[0] = c;
			pszLabel[1] = _T(' ');
			pszLabel[2] = _T('\0');
		}
	}else {
		pszLabel[0] = _T('\0');
	}
}

/*
	@param editInfo      �E�B���h�E���BNUL�ŕs������
	@param index         ����0origin�Ŏw��B -1�Ŕ�\��
	@param bZeroOrigin   �A�N�Z�X�L�[��0����U��
*/
bool FileNameManager::GetMenuFullLabel(
	TCHAR* pszOutput, size_t nBuffSize, bool bEspaceAmp,
	const EditInfo* editInfo, int nId, bool bFavorite,
	int index, bool bAccKeyZeroOrigin, HDC hDC
) {
	const EditInfo* pfi = editInfo;
	TCHAR szAccKey[4];
	int ret = 0;
	if (!pfi) {
		GetAccessKeyLabelByIndex(szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin);
		ret = auto_snprintf_s(pszOutput, nBuffSize, LS(STR_MENU_UNKOWN), szAccKey);
		return 0 < ret;
	}else if (pfi->bIsGrep) {
		
		GetAccessKeyLabelByIndex(szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin);
		//pfi->szGrepKeyShort �� memDes
		NativeW memDes;
		size_t nGrepKeyLen = wcslen(pfi->szGrepKey);
		const int GREPKEY_LIMIT_LEN = 64;
		// CSakuraEnvironment::ExpandParameter �ł� 32��������
		// ���j���[�� 64��������
		LimitStringLength(pfi->szGrepKey, nGrepKeyLen, GREPKEY_LIMIT_LEN, memDes);
		
		const TCHAR* pszKey;
		TCHAR szMenu2[GREPKEY_LIMIT_LEN * 2 * 2 + 1]; // WCHAR=>ACHAR��2�{�A&��2�{
		if (bEspaceAmp) {
			dupamp(memDes.GetStringT(), szMenu2);
			pszKey = szMenu2;
		}else {
			pszKey = memDes.GetStringT();
		}

		// szMenu�����
		// Jan. 19, 2002 genta
		// &�̏d��������ǉ��������ߌp��������኱�ύX
		// 20100729 ExpandParameter�ɂ��킹�āA�E�E�E��...�ɕύX
		ret = auto_snprintf_s(pszOutput, nBuffSize, LS(STR_MENU_GREP),
			szAccKey, pszKey,
			(nGrepKeyLen > memDes.GetStringLength()) ? _T("..."):_T("")
		);
	}else if (pfi->bIsDebug) {
		GetAccessKeyLabelByIndex(szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin);
		ret = auto_snprintf_s(pszOutput, nBuffSize, LS(STR_MENU_OUTPUT), szAccKey);
	}else {
		return GetMenuFullLabel(pszOutput, nBuffSize, bEspaceAmp, pfi->szPath, nId, pfi->bIsModified, pfi->nCharCode, bFavorite,
			 index, bAccKeyZeroOrigin, hDC);
	}
	return 0 < ret;
}

bool FileNameManager::GetMenuFullLabel(
	TCHAR* pszOutput, size_t nBuffSize, bool bEspaceAmp,
	const TCHAR* pszFile, int nId, bool bModified, EncodingType nCharCode, bool bFavorite,
	int index, bool bAccKeyZeroOrigin, HDC hDC
) {
	TCHAR szAccKey[4];
	TCHAR szFileName[_MAX_PATH];
	TCHAR szMenu2[_MAX_PATH * 2];
	const TCHAR* pszName;

	GetAccessKeyLabelByIndex(szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin);
	if (pszFile[0]) {
		this->GetTransformFileNameFast( pszFile, szFileName, _MAX_PATH, hDC );

		// szFileName �� szMenu2
		// Jan. 19, 2002 genta
		// ���j���[�������&���l��
		if (bEspaceAmp) {
			dupamp(szFileName, szMenu2);
			pszName = szMenu2;
		}else {
			pszName = szFileName;
		}
	}else {
		if (nId == -1) {
			wsprintf(szFileName, LS(STR_NO_TITLE1));
		}else {
			wsprintf(szFileName, _T("%s%d"), LS(STR_NO_TITLE1), nId);
		}
		pszName = szFileName;
	}
	const TCHAR* pszCharset = _T("");
	TCHAR szCodePageName[100];
	if (IsValidCodeTypeExceptSJIS(nCharCode)) {
		pszCharset = CodeTypeName(nCharCode).Bracket();
	}else if (IsValidCodeOrCPTypeExceptSJIS(nCharCode)) {
		CodePage::GetNameBracket(szCodePageName, nCharCode);
		pszCharset = szCodePageName;
	}
	
	int ret = auto_snprintf_s(pszOutput, nBuffSize, _T("%ts%ts%ts %ts%ts"),
		szAccKey, (bFavorite ? _T("�� ") : _T("")), pszName,
		(bModified ? _T("*"):_T(" ")), pszCharset
	);
	return 0 < ret;
}


/**
	�\���ݒ�t�@�C������ini�t�@�C�������擾����

	sakura.exe.ini����sakura.ini�̊i�[�t�H���_���擾���A�t���p�X����Ԃ�

	@param[out] pszPrivateIniFile �}���`���[�U�p��ini�t�@�C���p�X
	@param[out] pszIniFile EXE���ini�t�@�C���p�X

	@author ryoji
	@date 2007.09.04 ryoji �V�K�쐬
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
void FileNameManager::GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile, LPCTSTR pszProfName )
{
	TCHAR szPath[_MAX_PATH];
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFname[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];

	::GetModuleFileName(
		NULL,
		szPath, _countof(szPath)
	);
	_tsplitpath(szPath, szDrive, szDir, szFname, szExt);

	if (pszProfName[0] == '\0') {
		auto_snprintf_s(pszIniFile, _MAX_PATH - 1, _T("%ts%ts%ts%ts"), szDrive, szDir, szFname, _T(".ini"));
	}else {
		auto_snprintf_s( pszIniFile, _MAX_PATH - 1, _T("%ts%ts%ts\\%ts%ts"), szDrive, szDir, pszProfName, szFname, _T(".ini") );
	}

	// �}���`���[�U�p��ini�t�@�C���p�X
	//		exe�Ɠ����t�H���_�ɒu���ꂽ�}���`���[�U�\���ݒ�t�@�C���isakura.exe.ini�j�̓��e
	//		�ɏ]���ă}���`���[�U�p��ini�t�@�C���p�X�����߂�
	pszPrivateIniFile[0] = _T('\0');
	if (IsWin2000_or_later()) {
		auto_snprintf_s(szPath, _MAX_PATH - 1, _T("%ts%ts%ts%ts"), szDrive, szDir, szFname, _T(".exe.ini"));
		int nEnable = ::GetPrivateProfileInt(_T("Settings"), _T("MultiUser"), 0, szPath);
		if (nEnable) {
			int nFolder = ::GetPrivateProfileInt(_T("Settings"), _T("UserRootFolder"), 0, szPath);
			switch (nFolder) {
			case 1:
				nFolder = CSIDL_PROFILE;			// ���[�U�̃��[�g�t�H���_
				break;
			case 2:
				nFolder = CSIDL_PERSONAL;			// ���[�U�̃h�L�������g�t�H���_
				break;
			case 3:
				nFolder = CSIDL_DESKTOPDIRECTORY;	// ���[�U�̃f�X�N�g�b�v�t�H���_
				break;
			default:
				nFolder = CSIDL_APPDATA;			// ���[�U�̃A�v���P�[�V�����f�[�^�t�H���_
				break;
			}
			::GetPrivateProfileString(_T("Settings"), _T("UserSubFolder"), _T("sakura"), szDir, _MAX_DIR, szPath);
			if (szDir[0] == _T('\0'))
				::lstrcpy(szDir, _T("sakura"));
			if (GetSpecialFolderPath(nFolder, szPath)) {
				if (pszProfName[0] == '\0') {
					auto_snprintf_s(pszPrivateIniFile, _MAX_PATH - 1, _T("%ts\\%ts\\%ts%ts"), szPath, szDir, szFname, _T(".ini"));
				}else {
					auto_snprintf_s( pszPrivateIniFile, _MAX_PATH - 1, _T("%ts\\%ts\\%ts\\%ts%ts"), szPath, szDir, szFname, pszProfName, _T(".ini") );
				}
			}
		}
	}
}

/**
	ini�t�@�C�����̎擾

	���L�f�[�^����sakura.ini�̊i�[�t�H���_���擾���A�t���p�X����Ԃ�
	�i���L�f�[�^���ݒ�̂Ƃ��͋��L�f�[�^�ݒ���s���j

	@param[out] pszIniFileName ini�t�@�C�����i�t���p�X�j
	@param[in] bRead true: �ǂݍ��� / false: ��������

	@author ryoji
	@date 2007.05.19 ryoji �V�K�쐬
*/
void FileNameManager::GetIniFileName( LPTSTR pszIniFileName, LPCTSTR pszProfName, BOOL bRead/*=FALSE*/ )
{
	auto& iniFolder = pShareData->fileNameManagement.iniFolder;
	if (!iniFolder.bInit) {
		iniFolder.bInit = true;			// �������σt���O
		iniFolder.bReadPrivate = false;	// �}���`���[�U�pini����̓ǂݏo���t���O
		iniFolder.bWritePrivate = false;	// �}���`���[�U�pini�ւ̏������݃t���O

		GetIniFileNameDirect(iniFolder.szPrivateIniFile, iniFolder.szIniFile, pszProfName);
		if (iniFolder.szPrivateIniFile[0] != _T('\0')) {
			iniFolder.bReadPrivate = true;
			iniFolder.bWritePrivate = true;
			if (CommandLine::getInstance().IsNoWindow() && CommandLine::getInstance().IsWriteQuit())
				iniFolder.bWritePrivate = false;

			// �}���`���[�U�p��ini�t�H���_���쐬���Ă���
			if (iniFolder.bWritePrivate) {
				TCHAR szPath[_MAX_PATH];
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				_tsplitpath( iniFolder.szPrivateIniFile, szDrive, szDir, NULL, NULL );
				auto_snprintf_s( szPath, _MAX_PATH - 1, _T("%ts\\%ts"), szDrive, szDir );
				MakeSureDirectoryPathExistsT( szPath );
			}
		}else {
			if (pszProfName[0] != _T('\0')) {
				TCHAR szPath[_MAX_PATH];
				TCHAR szDrive[_MAX_DRIVE];
				TCHAR szDir[_MAX_DIR];
				_tsplitpath( iniFolder.szIniFile, szDrive, szDir, NULL, NULL );
				auto_snprintf_s(szPath, _MAX_PATH - 1, _T("%ts\\%ts"), szDrive, szDir);
				MakeSureDirectoryPathExistsT(szPath);
			}
		}
	}

	bool bPrivate = bRead ? iniFolder.bReadPrivate : iniFolder.bWritePrivate;
	::lstrcpy( pszIniFileName, bPrivate? iniFolder.szPrivateIniFile: iniFolder.szIniFile );
}

