#pragma once

// オープンダイアログ用ファイル拡張子管理

#include "_main/global.h"
#include "config/maxdata.h"
#include "util/design_template.h"

class FileExt {
public:
	FileExt();
	~FileExt();

	bool AppendExt(const TCHAR* pszName, const TCHAR* pszExt);
	bool AppendExtRaw(const TCHAR* pszName, const TCHAR* pszExt);
	const TCHAR* GetName(int nIndex);
	const TCHAR* GetExt(int nIndex);

	// ダイアログに渡す拡張子フィルタを取得する。(lpstrFilterに直接指定可能)
	//2回呼び出すと古いバッファが無効になることがあるのに注意
	const TCHAR* GetExtFilter(void);

	int GetCount(void) { return nCount; }

protected:
	// 2014.10.30 syat ConvertTypesExtToDlgExtをCDocTypeManagerに移動
	//bool ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, TCHAR *pszDstExt );

private:
	struct FileExtInfoTag {
		TCHAR	szName[64];						// 名前(64文字以下のはず→szTypeName)
		TCHAR	szExt[MAX_TYPES_EXTS * 3 + 1];	// 拡張子(64文字以下のはず→szTypeExts) なお "*." を追加するのでそれなりに必要
	};

	int nCount;
	FileExtInfoTag*	puFileExtInfo;
	std::vector<TCHAR>	vstrFilter;

private:
	DISALLOW_COPY_AND_ASSIGN(FileExt);
};

