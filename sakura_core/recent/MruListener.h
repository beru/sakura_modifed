#pragma once

#include "doc/DocListener.h"

class MruListener : public DocListenerEx {
public:
	// ロード前後
//	CallbackResultType OnCheckLoad(SLoadInfo* pLoadInfo);
	void OnBeforeLoad(LoadInfo* pLoadInfo);
	void OnAfterLoad(const LoadInfo& loadInfo);

	// セーブ前後
	void OnAfterSave(const SaveInfo& saveInfo);

	// クローズ前後
	CallbackResultType OnBeforeClose();

protected:
	// ヘルパ
	void _HoldBookmarks_And_AddToMRU(); // Mar. 30, 2003 genta
};

