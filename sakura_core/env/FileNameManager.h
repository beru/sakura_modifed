#pragma once

// �v��s��`
// #include "DllSharedData.h"

#include "util/design_template.h"
#include "config/maxdata.h"

struct EditInfo;

// ini�t�H���_�ݒ�
struct IniFolder {
	bool bInit;							// �������σt���O
	bool bReadPrivate;					// �}���`���[�U�pini����̓ǂݏo���t���O
	bool bWritePrivate;					// �}���`���[�U�pini�ւ̏������݃t���O
	TCHAR szIniFile[_MAX_PATH];			// EXE���ini�t�@�C���p�X
	TCHAR szPrivateIniFile[_MAX_PATH];	// �}���`���[�U�p��ini�t�@�C���p�X
};	// ini�t�H���_�ݒ�


// ���L���������\����
struct Share_FileNameManagement {
	IniFolder	iniFolder;	// **** ini�t�H���_�ݒ� ****
};


// �t�@�C�����Ǘ�
class FileNameManager : public TSingleton<FileNameManager> {
	friend class TSingleton<FileNameManager>;
	FileNameManager() {
		pShareData = &GetDllShareData();
		nTransformFileNameCount = -1;
	}

public:
	// �t�@�C�����֘A
	LPTSTR GetTransformFileNameFast(LPCTSTR, LPTSTR, size_t nDestLen, HDC hDC, bool bFitMode = true, int cchMaxWidth = 0);
	int TransformFileName_MakeCache(void);
	static LPCTSTR GetFilePathFormat(LPCTSTR, LPTSTR, size_t, LPCTSTR, LPCTSTR);
	static bool ExpandMetaToFolder(LPCTSTR, LPTSTR, int);

	// ���j���[�ނ̃t�@�C�����쐬
	bool GetMenuFullLabel_WinList(TCHAR* pszOutput, size_t nBuffSize, const EditInfo* editInfo, int id, size_t index, HDC hDC){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, false, index, false, hDC);
	}
	bool GetMenuFullLabel_MRU(TCHAR* pszOutput, size_t nBuffSize, const EditInfo* editInfo, int id, bool bFavorite, size_t index, HDC hDC){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, editInfo, id, bFavorite, index, true, hDC);
	}
	bool GetMenuFullLabel_WinListNoEscape(TCHAR* pszOutput, size_t nBuffSize, const EditInfo* editInfo, int id, size_t index, HDC hDC){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, editInfo, id, false, index, false, hDC);
	}
	bool GetMenuFullLabel_File(TCHAR* pszOutput, size_t nBuffSize, const TCHAR* pszFile, int id, HDC hDC, bool bModified = false, EncodingType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, true, pszFile, id, false, nCharCode, false, -1, false, hDC);
	}
	bool GetMenuFullLabel_FileNoEscape(TCHAR* pszOutput, size_t nBuffSize, const TCHAR* pszFile, int id, HDC hDC, bool bModified = false, EncodingType nCharCode = CODE_NONE){
		return GetMenuFullLabel(pszOutput, nBuffSize, false, pszFile, id, false, nCharCode, false, -1, false, hDC);
	}

	bool GetMenuFullLabel(TCHAR* pszOutput, size_t nBuffSize, bool bEspaceAmp, const EditInfo* editInfo, int id, bool bFavorite, size_t index, bool bAccKeyZeroOrigin, HDC hDC);
	bool GetMenuFullLabel(TCHAR* pszOutput, size_t nBuffSize, bool bEspaceAmp, const TCHAR* pszFile, int id, bool bModified, EncodingType nCharCode, bool bFavorite, size_t index, bool bAccKeyZeroOrigin, HDC hDC);
	
	static TCHAR GetAccessKeyByIndex(size_t index, bool bZeroOrigin);

	static void GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile, LPCTSTR pszProfName );	// �\���ݒ�t�@�C������ini�t�@�C�������擾����
	void GetIniFileName( LPTSTR pszIniFileName, LPCTSTR pszProfName, bool bRead = false );	// ini�t�@�C�����̎擾

private:
	DllSharedData* pShareData;

	// �t�@�C�����ȈՕ\���p�L���b�V��
	int		nTransformFileNameCount; // �L����
	TCHAR	szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int		nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];
};

