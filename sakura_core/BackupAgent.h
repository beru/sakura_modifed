#pragma once

#include "doc/DocListener.h"

class BackupAgent : public DocListenerEx {
public:
	CallbackResultType OnPreBeforeSave(SaveInfo* pSaveInfo);

protected:
	int MakeBackUp(const TCHAR* target_file);				// バックアップの作成
	bool FormatBackUpPath(TCHAR*, size_t, const TCHAR*);	// バックアップパスの作成 2005.11.21 aroka
};

