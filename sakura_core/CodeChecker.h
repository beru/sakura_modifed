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
	// �Z�[�u���`�F�b�N
	CallbackResultType OnCheckSave(SaveInfo* pSaveInfo);
	void OnFinalSave(SaveResultType eSaveResult);

	// ���[�h���`�F�b�N
	void OnFinalLoad(LoadResultType eLoadResult);
};

