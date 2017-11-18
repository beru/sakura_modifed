#pragma once

#include "doc/DocListener.h"

class LoadAgent : public DocListenerEx {
public:
	CallbackResultType OnCheckLoad(LoadInfo* pLoadInfo);
	void OnBeforeLoad(LoadInfo* loadInfo);
	LoadResultType OnLoad(const LoadInfo& loadInfo);
	void OnAfterLoad(const LoadInfo& loadInfo);
	void OnFinalLoad(LoadResultType eLoadResult);
};

