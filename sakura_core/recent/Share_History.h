#pragma once

#include "config/maxdata.h"

// 共有メモリ内構造体
struct Share_History {
	//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUを経由してください。
	size_t				nMRUArrNum;
	EditInfo			fiMRUArr[MAX_MRU];
	bool				bMRUArrFavorite[MAX_MRU];	//お気に入り	//@@@ 2003.04.08 MIK

	//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUFolderを経由してください。
	size_t							nOPENFOLDERArrNum;
	StaticString<TCHAR,_MAX_PATH>	szOPENFOLDERArr[MAX_OPENFOLDER];
	bool							bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	// お気に入り	//@@@ 2003.04.08 MIK

	// MRU除外リスト一覧
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_MRU,  const TCHAR* >	aExceptMRU;

	// MRU以外の情報
	SFilePath													szIMPORTFOLDER;	// インポートディレクトリの履歴
	StaticVector< StaticString<TCHAR, MAX_CMDLEN>, MAX_CMDARR >	aCommands;		// 外部コマンド実行履歴
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_CMDARR >	aCurDirs;		// カレントディレクトリ履歴
};

