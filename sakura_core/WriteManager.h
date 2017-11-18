#pragma once

#include "doc/DocListener.h"
#include "charset/CodeBase.h"

class DocLineMgr;
struct SaveInfo;

class WriteManager : public ProgressSubject {
public:
	CodeConvertResult WriteFile_From_CDocLineMgr(
		const DocLineMgr& pcDocLineMgr,	// [in]
		const SaveInfo&	saveInfo		// [in]
	);
};

