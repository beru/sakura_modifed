/*!	@file
	@brief マクロ種別管理
*/

#pragma once

#include <map>
#include <list>
#include <string>
#include "util/design_template.h"

class MacroManagerBase;

/*!
	@brief マクロHandler生成クラス
	
	@par 初期化
	MacroManagerBase::Declare() により，MacroEngineのCreaterの登録
	RegisterEngine() 及び 対応拡張子の登録 RegisterExt() が呼び出される．
	
	@par 呼び出し
	MacroFactory::Create()を拡張子を引数にして呼び出すと対応する
	マクロエンジンが返される．得られたEngineに対してLoadKeyMacro()及び
	ExecKeyMacro() を呼び出すことでマクロの読み込み・実行が行われる．

	Singleton
*/
class MacroFactory : public TSingleton<MacroFactory> {
	friend class TSingleton<MacroFactory>;
	MacroFactory();

public:
	typedef MacroManagerBase* (*Creator)(class EditView& view, const TCHAR*);

	bool RegisterCreator(Creator);
	// Jan. 31, 2004 genta
	// バイナリサイズ削減のためmMacroExtsを削除
	//bool RegisterExt(const char*, Creator);
	bool Unregister(Creator);

	MacroManagerBase* Create(class EditView& view, const TCHAR* ext);

private:
	std::tstring Ext2Key(const TCHAR* ext);

	// Jan. 31, 2004 genta
	// バイナリサイズ削減のため拡張子保持用mapを削除
	// typedef std::map<std::string, Creator> MacroTypeRep;
	typedef std::list<Creator> MacroEngineRep;

	// Jan. 31, 2004 genta
	// バイナリサイズ削減のため
	//MacroTypeRep mMacroExts;	// 拡張子対応表
	/*!
		Creatorリスト
		@date 2002.08.25 genta 追加
	*/
	MacroEngineRep mMacroCreators;

};

