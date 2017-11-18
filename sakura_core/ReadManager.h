#pragma once

#include "doc/DocListener.h" // CProgressSubject
#include "charset/CodeBase.h" // CodeConvertResult

class DocLineMgr;
struct FileInfo; // doc/DocFile.h

class ReadManager : public ProgressSubject {
public:
	//	Nov. 12, 2000 genta 引数追加
	//	Jul. 26, 2003 ryoji BOM引数追加
	CodeConvertResult ReadFile_To_CDocLineMgr(
		DocLineMgr&			docLineMgr,
		const LoadInfo&		loadInfo,
		FileInfo*			pFileInfo
	);
};

