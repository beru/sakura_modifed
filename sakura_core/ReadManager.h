#pragma once

#include "doc/DocListener.h" // CProgressSubject
#include "charset/CodeBase.h" // CodeConvertResult

class DocLineMgr;
struct FileInfo; // doc/DocFile.h

class ReadManager : public ProgressSubject {
public:
	CodeConvertResult ReadFile_To_CDocLineMgr(
		DocLineMgr&			docLineMgr,
		const LoadInfo&		loadInfo,
		FileInfo*			pFileInfo
	);
};

