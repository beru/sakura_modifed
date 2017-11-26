/*!	@file
	@brief 表示用文字列等の取得

	機能名，機能分類，機能番号などの変換．設定画面での表示用文字列を用意する．
*/

#pragma once

#include <Windows.h>
#include "_main/global.h"
#include "config/maxdata.h"
#include "func/Funccode.h"

struct MacroRec;
struct CommonSetting;

// マクロ情報
struct MacroRec {
	TCHAR	szName[MACRONAME_MAX];	// 表示名
	TCHAR	szFile[_MAX_PATH + 1];	// ファイル名(ディレクトリを含まない)
	bool	bReloadWhenExecute;		// 実行時に読み込みなおすか（デフォルトon）
	
	bool IsEnabled() const { return szFile[0] != _T('\0'); }
	const TCHAR* GetTitle() const { return szName[0] == _T('\0') ? szFile: szName; }
};

/*!
	@brief 表示用文字列等の取得

	機能，機能分類と位置，機能番号，文字列などの対応を集約する．
*/
class FuncLookup {

public:
	FuncLookup() : pMacroRec(nullptr) {}

	void Init(MacroRec* pMacroRec, CommonSetting* pCom) {
		this->pMacroRec = pMacroRec;
		pCommon = pCom;
	}

	EFunctionCode Pos2FuncCode(int category, int position, bool bGetUnavailable = true) const;
	bool Pos2FuncName(int category, int position, wchar_t* ptr, int bufsize) const;
	bool Funccode2Name(int funccode, wchar_t* ptr, int bufsize) const ;
	const TCHAR* Category2Name(int category) const;
	const wchar_t* Custmenu2Name(int index, wchar_t buf[], int bufSize) const;

	void SetCategory2Combo(HWND hComboBox) const ;
	void SetListItem(HWND hListBox, size_t category) const;
	
	size_t GetCategoryCount(void) const {
		return nsFuncCode::nFuncKindNum + 3;	// 分類＋外部マクロ＋カスタムメニュー＋プラグイン
	}
	
	size_t GetItemCount(size_t category) const;


private:
	MacroRec* pMacroRec;	// マクロ情報
	CommonSetting* pCommon;	// 共通設定データ領域へのポインタ

};

