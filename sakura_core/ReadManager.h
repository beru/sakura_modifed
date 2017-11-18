#pragma once

#include "doc/DocListener.h" // CProgressSubject
#include "charset/CodeBase.h" // CodeConvertResult

class DocLineMgr;
struct FileInfo; // doc/DocFile.h

class ReadManager : public ProgressSubject {
public:
	//	Nov. 12, 2000 genta �����ǉ�
	//	Jul. 26, 2003 ryoji BOM�����ǉ�
	CodeConvertResult ReadFile_To_CDocLineMgr(
		DocLineMgr&			docLineMgr,
		const LoadInfo&		loadInfo,
		FileInfo*			pFileInfo
	);
};

