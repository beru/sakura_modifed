#pragma once

#include "util/design_template.h"
#include "doc/DocListener.h"

class AppMode
	:
	public TSingleton<AppMode>,
	public DocListenerEx
{ // ###仮
	friend class TSingleton<AppMode>;
	AppMode()
		:
		bViewMode(false),	// ビューモード
		bDebugMode(false)	// デバッグモニタモード
	{
		szGrepKey[0] = 0;
	}

public:
	// インターフェース
	bool	IsViewMode() const				{ return bViewMode; }				// ビューモードを取得
	void	SetViewMode(bool bViewMode)		{ this->bViewMode = bViewMode; }	// ビューモードを設定
	bool	IsDebugMode() const				{ return bDebugMode; }
	void	SetDebugModeON();	// デバッグモニタモード設定
	void	SetDebugModeOFF();	// デバッグモニタモード解除

	// イベント
	void OnAfterSave(const SaveInfo& saveInfo);

protected:
	void _SetDebugMode(bool bDebugMode) { this->bDebugMode = bDebugMode; }

private:
	bool	bViewMode;				// ビューモード
	bool	bDebugMode;				// デバッグモニタモード
public:
	wchar_t	szGrepKey[1024];		// Grepモードの場合、その検索キー
};

