/*!	@file
	@brief マクロ種別管理
*/
#include "StdAfx.h"
#include <algorithm>
#include <CType.h>
#include "MacroFactory.h"

static const TCHAR NULSTR[] = _T("");

MacroFactory::MacroFactory()
{}

/*!
	与えられた拡張子をmapのkeyに変換する
	
	@param ext [in] 拡張子
	
	@par Rule
	@li NULLは""にする。
	@li アルファベットは小文字に統一
*/
std::tstring MacroFactory::Ext2Key(const TCHAR* ext)
{
	if (!ext) {
		ext = NULSTR;
	}
	
	std::tstring key = ext;
	std::transform(key.begin(), key.end(), key.begin(), _totlower);

	return key;
}

/*!
	Creatorの登録
	
	拡張子の対応を初期に登録しないCreatorを登録する．
	ただし，一旦対応がわかったら次回以降は対応表が使われる．
	
	@param f [in] 登録するFactory関数
	
	@sa MacroFactory::RegisterExts
*/
bool MacroFactory::RegisterCreator(Creator f)
{
	if (!f) {
		return false;
	}

	mMacroCreators.push_back(f);
	return true;
}

/*!
	Creatorの登録解除
	
	@param f [in] 登録解除するCreator
*/
bool MacroFactory::Unregister(Creator f)
{
	// Creator Listからの削除
	auto c_it = mMacroCreators.begin();
	while (c_it != mMacroCreators.end()) {
		if (*c_it == f) {
			// いきなり削除するとiteratorが無効になるので，
			// iteratorを1つ進めてから現在位置を削除する．
			auto tmp_it = c_it++;
			mMacroCreators.erase(tmp_it);
			// 重複登録されている場合を考慮して，
			// 1つ見つかっても最後までチェックする
		}else {
			++ c_it;
		}
	}
	
	return true;
}

/*
	Object Factory
	
	登録されたFactory Objectを順に呼び出して、
	Objectが得られたらそれを返す。

	@pararm ext [in] 拡張子
	@return Macroオブジェクト。適切なものが見つからなければNULL。
*/
MacroManagerBase* MacroFactory::Create(EditView& view, const TCHAR* ext)
{
	std::tstring key = Ext2Key(ext);

	// Creatorを順に試す
	for (auto c_it=mMacroCreators.begin(); c_it!=mMacroCreators.end(); ++c_it) {
		MacroManagerBase* pobj = (*c_it)(view, key.c_str());
		if (pobj) {
			DEBUG_TRACE(_T("MacroFactory::Create/ Answered for (%ts)\n"), key.c_str());
			return pobj;
		}
	}
	
	return nullptr;
}

