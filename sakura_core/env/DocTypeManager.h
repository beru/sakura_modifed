#pragma once

#include "DllSharedData.h"

// ドキュメントタイプ管理
class DocTypeManager {
public:
	DocTypeManager() {
		pShareData = &GetDllShareData();
	}
	TypeConfigNum GetDocumentTypeOfPath(const TCHAR* pszFilePath);	// ファイルパスを渡して、ドキュメントタイプ（数値）を取得する
	TypeConfigNum GetDocumentTypeOfExt(const TCHAR* pszExt);		// 拡張子を渡して、ドキュメントタイプ（数値）を取得する
	TypeConfigNum GetDocumentTypeOfId(int id);

	bool GetTypeConfig(TypeConfigNum documentType, TypeConfig& type);
	bool SetTypeConfig(TypeConfigNum documentType, const TypeConfig& type);
	bool GetTypeConfigMini(TypeConfigNum documentType, const TypeConfigMini** type);
	bool AddTypeConfig(TypeConfigNum documentType);
	bool DelTypeConfig(TypeConfigNum documentType);

	static bool IsFileNameMatch(const TCHAR* pszTypeExts, const TCHAR* pszFileName);	// タイプ別拡張子にファイル名がマッチするか
	static void GetFirstExt(const TCHAR* pszTypeExts, TCHAR szFirstExt[], int nBuffSize);	// タイプ別拡張子の先頭拡張子を取得する
	static bool ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, const TCHAR* szExt, TCHAR *pszDstExt );	// タイプ別設定の拡張子リストをダイアログ用リストに変換する

	static const TCHAR* typeExtSeps;			// タイプ別拡張子の区切り文字
	static const TCHAR* typeExtWildcards;		// タイプ別拡張子のワイルドカード

private:
	DllSharedData* pShareData;
};

