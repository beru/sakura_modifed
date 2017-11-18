#pragma once

#include "doc/DocListener.h"
#include "util/design_template.h"

class CodeChecker :
	public DocListenerEx,
	public TSingleton<CodeChecker>
{
	friend class TSingleton<CodeChecker>;
	CodeChecker() {}
	
public:
	// セーブ時チェック
	CallbackResultType OnCheckSave(SaveInfo* pSaveInfo);
	void OnFinalSave(SaveResultType eSaveResult);

	// ロード時チェック
	void OnFinalLoad(LoadResultType eLoadResult);
};

