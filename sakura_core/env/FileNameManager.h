#pragma once

// 要先行定義
// #include "DllSharedData.h"

#include "util/design_template.h"
#include "config/maxdata.h"

struct EditInfo;

// iniフォルダ設定
struct IniFolder {
	bool bInit;							// 初期化済フラグ
	bool bReadPrivate;					// マルチユーザ用iniからの読み出しフラグ
	bool bWritePrivate;					// マルチユーザ用iniへの書き込みフラグ
	TCHAR szIniFile[_MAX_PATH];			// EXE基準のiniファイルパス
	TCHAR szPrivateIniFile[_MAX_PATH];	// マルチユーザ用のiniファイルパス
};	// iniフォルダ設定


// 共有メモリ内構造体
struct Share_FileNameManagement {
	IniFolder	iniFolder;	// **** iniフォルダ設定 ****
};


// ファイル名管理
class FileNameManager : public TSingleton<FileNameManager> {
	friend class TSingleton<FileNameManager>;
	FileNameManager() {
		pShareData = &GetDllShareData();
		nTransformFileNameCount = -1;
	}

public:
	// ファイル名関連
	LPTSTR GetTransformFileNameFast(LPCTSTR, LPTSTR, size_t nDestLen, HDC hDC, bool bFitMode = true, int cchMaxWidth = 0);
	int TransformFileName_MakeCache(void);
	static LPCTSTR GetFilePathFormat(LPCTSTR, LPTSTR, size_t, LPCTSTR, LPCTSTR);
	static bool ExpandMetaToFolder(LPCTSTR, LPTSTR, int);

	// メニュー類のファイル名作成
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

	static void GetIniFileNameDirect( LPTSTR pszPrivateIniFile, LPTSTR pszIniFile, LPCTSTR pszProfName );	// 構成設定ファイルからiniファイル名を取得する
	void GetIniFileName( LPTSTR pszIniFileName, LPCTSTR pszProfName, bool bRead = false );	// iniファイル名の取得

private:
	DllSharedData* pShareData;

	// ファイル名簡易表示用キャッシュ
	int		nTransformFileNameCount; // 有効数
	TCHAR	szTransformFileNameFromExp[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	int		nTransformFileNameOrgId[MAX_TRANSFORM_FILENAME];
};

