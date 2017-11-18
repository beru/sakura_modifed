#pragma once

#include "doc/DocListener.h"

class BackupAgent : public DocListenerEx {
public:
	CallbackResultType OnPreBeforeSave(SaveInfo* pSaveInfo);

protected:
	int MakeBackUp(const TCHAR* target_file);				// �o�b�N�A�b�v�̍쐬
	bool FormatBackUpPath(TCHAR*, size_t, const TCHAR*);	// �o�b�N�A�b�v�p�X�̍쐬 2005.11.21 aroka
};

