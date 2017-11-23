/*!	@file
	@brief マクロエンジン
*/

#pragma once

#include <Windows.h>
class EditView;

class MacroBeforeAfter {
public:
	MacroBeforeAfter() : nOpeBlkCount(0), bDrawSwitchOld(true) {};
	virtual ~MacroBeforeAfter() {};
	virtual void ExecKeyMacroBefore(class EditView& editView, int flags);
	virtual void ExecKeyMacroAfter(class EditView& editView, int flags, bool bRet);
private:
	int nOpeBlkCount;
	bool bDrawSwitchOld;
};

/*!
	@brief マクロを処理するエンジン部分の基底クラス
*/
class MacroManagerBase : MacroBeforeAfter {
public:

	/*! キーボードマクロの実行
	
		@param[in] pcEditView マクロ実行対象の編集ウィンドウ
		@param[in] flags マクロ実行属性．
	*/
	virtual bool ExecKeyMacro(class EditView& editView, int flags) const = 0;
	virtual void ExecKeyMacro2(class EditView& editView, int flags);
	
	/*! キーボードマクロをファイルから読み込む

		@param hInstance [in]
		@param pszPath [in] ファイル名
	*/
	virtual bool LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath) = 0;

	/*! キーボードマクロを文字列から読み込む

		@param hInstance [in]
		@param pszCode [in] マクロコード
	*/
	virtual bool LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode) = 0;

	// static MacroManagerBase* Creator(const char* str);
	// 純粋仮想クラスは実体化できないのでFactoryは不要。
	// 継承先クラスでは必要。
	
	// デストラクタのvirtualを忘れずに
	virtual ~MacroManagerBase();
	

protected:
	// Load済みかどうかを表すフラグ true...Load済み、false...未Load。
	bool nReady;

public:
	/*!	Load済みかどうか

		@retval true Load済み
		@retval false 未Load
	*/
	bool IsReady() { return nReady; }

	// Constructor
	MacroManagerBase();

};

