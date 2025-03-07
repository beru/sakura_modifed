/*!	@file
	@brief DLLのロード、アンロード
*/
#include "StdAfx.h"
#include "DllHandler.h"
#include "util/module.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

DllImp::DllImp()
	:
	hInstance(NULL)
{
}

/*!
	オブジェクト消滅前にDLLが読み込まれた状態であればDLLの解放を行う．
*/
DllImp::~DllImp()
{
	if (IsAvailable()) {
		DeinitDll(true);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         DLLロード                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

InitDllResultType DllImp::InitDll(LPCTSTR pszSpecifiedDllName)
{
	if (IsAvailable()) {
		// 既に利用可能で有れば何もしない．
		return InitDllResultType::Success;
	}

	// 名前候補を順次検証し、有効なものを採用する
	LPCTSTR pszLastName  = NULL;
	bool bInitImpFailure = false;
	for (int i=-1; ; ++i) {
		// 名前候補
		LPCTSTR pszName = NULL;
		if (i == -1) { // まずは引数で指定された名前から。
			pszName = pszSpecifiedDllName;
		}else { // クラス定義のDLL名
			pszName = GetDllNameImp(i);
			// GetDllNameImpから取得した名前が無効ならループを抜ける
			if (!pszName || !pszName[0]) {
				break;
			}
			// GetDllNameImpから取得した名前が前回候補と同じならループを抜ける
			if (pszLastName && _tcsicmp(pszLastName, pszName) == 0) {
				break;
			}
		}
		pszLastName = pszName;

		// 名前が無効の場合は、次の名前候補を試す。
		if (!pszName || !pszName[0]) continue;

		// DLLロード。ロードできなかったら次の名前候補を試す。
		hInstance = LoadLibraryExedir(pszName);
		if (!hInstance) continue;

		// 初期処理
		bool ret = InitDllImp();

		// 初期処理に失敗した場合はDLLを解放し、次の名前候補を試す。
		if (!ret) {
			bInitImpFailure = true;
			::FreeLibrary(hInstance);
			hInstance = NULL;
			continue;
		}

		// 初期処理に成功した場合は、DLL名を保存し、ループを抜ける
		if (ret) {
			strLoadedDllName = pszName;
			break;
		}
	}

	// ロードと初期処理に成功なら
	if (IsAvailable()) {
		return InitDllResultType::Success;
	// 初期処理に失敗したことがあったら
	}else if (bInitImpFailure) {
		return InitDllResultType::InitFailure; // DLLロードはできたけど、その初期処理に失敗
	// それ以外
	}else {
		return InitDllResultType::LoadFailure; // DLLロード自体に失敗
	}
}

bool DllImp::DeinitDll(bool force)
{
	if (!hInstance || (!IsAvailable())) {
		// DLLが読み込まれていなければ何もしない
		return true;
	}

	// 終了処理
	bool ret = DeinitDllImp();
	
	// DLL解放
	if (ret || force) {
		// DLL名を解放
		strLoadedDllName = _T("");

		// DLL解放
		::FreeLibrary(hInstance);
		hInstance = NULL;

		return true;
	}else {
		return false;
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           属性                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LPCTSTR DllImp::GetLoadedDllName() const
{
	return strLoadedDllName.c_str();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  オーバーロード可能実装                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	実装を省略できるようにするため、空の関数を用意しておく
*/
bool DllImp::DeinitDllImp()
{
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	テーブルで与えられたエントリポインタアドレスを入れる場所に
	対応する文字列から調べたエントリポインタを設定する。
	
	@param table [in] 名前とアドレスの対応表。最後は{NULL,0}で終わること。
	@retval true 全てのアドレスが設定された。
	@retval false アドレスの取得に失敗した関数があった。
*/
bool DllImp::RegisterEntries(const ImportTable table[])
{
	if (!IsAvailable()) {
		return false;
	}
	for (int i=0; table[i].proc; ++i) {
		FARPROC proc;
		if (!(proc = ::GetProcAddress(GetInstance(), table[i].name))) {
			return false;
		}
		*((FARPROC*)table[i].proc) = proc;
	}
	return true;
}

