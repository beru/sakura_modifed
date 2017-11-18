#pragma once

#include "doc/DocListener.h"

// ファイルが更新された場合に再読込を行うかどうかのフラグ
enum class WatchUpdateType {
	Query,		// 再読込を行うかどうかダイアログボックスで問い合わせる
	Notify,		// 更新されたことをステータスバーで通知
	None,		// 更新監視を行わない
	AutoLoad,	// 更新され未編集の場合に再ロード
};

class AutoReloadAgent : public DocListenerEx {
public:
	AutoReloadAgent();
	void OnBeforeSave(const SaveInfo& saveInfo);
	void OnAfterSave(const SaveInfo& saveInfo);
	void OnAfterLoad(const LoadInfo& loadInfo);
	
	// 監視の一時停止
	void PauseWatching() { ++nPauseCount; }
	void ResumeWatching() { --nPauseCount; assert(nPauseCount >= 0); }
	bool IsPausing() const { return nPauseCount >= 1; }
	
public://#####仮
	bool _ToDoChecking() const;
	bool _IsFileUpdatedByOther(FILETIME* pNewFileTime) const;
	void CheckFileTimeStamp();	// ファイルのタイムスタンプのチェック処理
	
public:
	WatchUpdateType	watchUpdateType;	// 更新監視方法
	
private:
	int nPauseCount;	// これが1以上の場合は監視をしない
	int nDelayCount;	// 未編集で再ロード時の遅延カウンタ
};

