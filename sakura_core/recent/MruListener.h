#pragma once

#include "doc/DocListener.h"

class MruListener : public DocListenerEx {
public:
	// ���[�h�O��
//	CallbackResultType OnCheckLoad(SLoadInfo* pLoadInfo);
	void OnBeforeLoad(LoadInfo* pLoadInfo);
	void OnAfterLoad(const LoadInfo& loadInfo);

	// �Z�[�u�O��
	void OnAfterSave(const SaveInfo& saveInfo);

	// �N���[�Y�O��
	CallbackResultType OnBeforeClose();

protected:
	// �w���p
	void _HoldBookmarks_And_AddToMRU(); // Mar. 30, 2003 genta
};

