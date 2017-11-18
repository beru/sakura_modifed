#pragma once

#include "doc/DocListener.h"
#include "util/design_template.h"
class WaitCursor;

class VisualProgress :
	public DocListenerEx,
	public ProgressListener
{
public:
	// コンストラクタ・デストラクタ
	VisualProgress();
	virtual ~VisualProgress();

	// ロード前後
	void OnBeforeLoad(LoadInfo* pLoadInfo);
	void OnAfterLoad(const LoadInfo& loadInfo);

	// セーブ前後
	void OnBeforeSave(const SaveInfo& saveInfo);
	void OnFinalSave(SaveResultType eSaveResult);

	// プログレス受信
	void OnProgress(size_t nPer);

protected:
	// 実装補助
	void _Begin();
	void _Doing(int nPer);
	void _End();
private:
	WaitCursor* pWaitCursor;
	int	nOldValue;

private:
	DISALLOW_COPY_AND_ASSIGN(VisualProgress);
};

