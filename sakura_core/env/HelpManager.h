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

