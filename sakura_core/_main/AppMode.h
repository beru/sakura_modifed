/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
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
	bool	IsViewMode() const				{ return bViewMode; }			// ビューモードを取得
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
	wchar_t	szGrepKey[1024];			// Grepモードの場合、その検索キー
};

