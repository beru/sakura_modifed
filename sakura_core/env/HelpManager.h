/*
	2008.05.18 kobake CShareData から分離
*/
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

// 要先行定義
// #include "DllSharedData.h"


// ヘルプ管理
class HelpManager {
public:
	HelpManager() {
		pShareData = &GetDllShareData();
	}
	// ヘルプ関連	//@@@ 2002.2.3 YAZAKI
	bool			ExtWinHelpIsSet(const TypeConfig* pType = nullptr);		// タイプがnTypeのときに、外部ヘルプが設定されているか。
	const TCHAR*	GetExtWinHelp(const TypeConfig* pType = nullptr);		// タイプがnTypeのときの、外部ヘルプファイル名を取得。
	bool			ExtHTMLHelpIsSet(const TypeConfig* pType = nullptr);	// タイプがnTypeのときに、外部HTMLヘルプが設定されているか。
	const TCHAR*	GetExtHTMLHelp(const TypeConfig* pType = nullptr);		// タイプがnTypeのときの、外部HTMLヘルプファイル名を取得。
	bool			HTMLHelpIsSingle(const TypeConfig* pType = nullptr);	// タイプがnTypeのときの、外部HTMLヘルプ「ビューアを複数起動しない」がONかを取得。
private:
	DllSharedData* pShareData;
};

