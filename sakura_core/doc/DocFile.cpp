#include "StdAfx.h"
#include "DocFile.h"

/*
	保存時のファイルのパス（マクロ用）の取得

	2017/5/17 File.hから移動
*/
const TCHAR* DocFile::GetSaveFilePath(void) const
{
	if (szSaveFilePath.IsValidPath()) {
		return szSaveFilePath;
	}else {
		return GetFilePath();
	}
}

