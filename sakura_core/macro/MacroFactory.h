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
	bool Unregister(Creator);

	MacroManagerBase* Create(class EditView& view, const TCHAR* ext);

private:
	std::tstring Ext2Key(const TCHAR* ext);

	typedef std::list<Creator> MacroEngineRep;

	MacroEngineRep mMacroCreators;

};

