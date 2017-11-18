#pragma once

#include "doc/DocListener.h"

class DocLocker : public DocListenerEx {
public:
	DocLocker();

	// クリア
	void Clear(void) { bIsDocWritable = true; }

	// ロード前後
	void OnAfterLoad(const LoadInfo& loadInfo);
	
	// セーブ前後
	void OnBeforeSave(const SaveInfo& saveInfo);
	void OnAfterSave(const SaveInfo& saveInfo);

	// 状態
	bool IsDocWritable() const { return bIsDocWritable; }

	// チェック
	void CheckWritable(bool bMsg);

private:
	bool bIsDocWritable;
};

