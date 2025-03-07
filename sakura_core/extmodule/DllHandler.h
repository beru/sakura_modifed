/*!	@file
	@brief DLLのロード、アンロード
*/
#pragma once

#include <Windows.h>
#include <string>
#include "_main/global.h"

/*! DllImp をラップ
	DllImp::DeinitDll を呼び忘れないためのヘルパ的クラス。
	今のところDeinitDllが使われている箇所が無いので、このクラスの出番はありませんが。
*/
template <class DLLIMP>
class DllHandler {
public:
	// コンストラクタ・デストラクタ
	DllHandler() {
		pDllImp = new DLLIMP();
		pDllImp->InitDll();
	}
	~DllHandler() {
		pDllImp->DeinitDll(true); // ※終了処理に失敗しても強制的にDLL解放
		delete pDllImp;
	}

	// アクセサ
	DLLIMP* operator -> () { return pDllImp; }

	// 利用状態のチェック（operator版）
	bool operator!() const { return pDllImp->IsAvailable(); }

private:
	DLLIMP*	pDllImp;
};


// 結果定数
enum class InitDllResultType {
	Success,		// 成功
	LoadFailure,	// DLLロード失敗
	InitFailure,	// 初期処理に失敗
};

// DLLの動的なLoad/Unloadを行うためのクラス
class DllImp {
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                            型                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*!
		アドレスとエントリ名の対応表。RegisterEntriesで使われる。
	*/
	struct ImportTable {
		void*		proc;
		const char*	name;
	};

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        生成と破棄                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// コンストラクタ・デストラクタ
	DllImp();
	virtual ~DllImp();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         DLLロード                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// DLLの関数を呼び出せるか状態どうか
	virtual bool IsAvailable() const { return hInstance != NULL; }

	// DLLロードと初期処理
	InitDllResultType InitDll(
		LPCTSTR pszSpecifiedDllName = NULL	// [in] クラスが定義しているDLL名以外のDLLを読み込みたいときに、そのDLL名を指定。
	);

	// 終了処理とDLLアンロード
	bool DeinitDll(
		bool force = false	// [in] 終了処理に失敗してもDLLを解放するかどうか
	);

	// インスタンスハンドルの取得
	HINSTANCE GetInstance() const { return hInstance; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           属性                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ロード済みDLLファイル名の取得。ロードされていない (またはロードに失敗した) 場合は NULL を返す。
	LPCTSTR GetLoadedDllName() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  オーバーロード可能実装                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	// DLLの初期化
	/*!
		DLLのロードに成功した直後に呼び出される．エントリポイントの
		確認などを行う．

		@retval true 正常終了
		@retval false 異常終了

		@note falseを返した場合は、読み込んだDLLを解放する．
	*/
	virtual bool InitDllImp() = 0;

	// 関数の初期化
	/*!
		DLLのアンロードを行う直前に呼び出される．メモリの解放などを
		行う．

		@retval true 正常終了
		@retval false 異常終了

		@note falseを返したときはDLLのUnloadは行われない．
		@par 注意
		デストラクタからDeinitDll及びDeinitDllImpが呼び出されたときは
		ポリモーフィズムが行われないためにサブクラスのDeinitDllImpが呼び出されない。
		そのため、サブクラスのデストラクタではDeinitDllImpを明示的に呼び出す必要がある。
		
		DeinitDllがデストラクタ以外から呼び出される場合はDeinitDllImpは仮想関数として
		サブクラスのものが呼び出され、デストラクタは当然呼び出されないので
		DeinitDllImpそのものは必要である。
		
		デストラクタからDeinitDllImpを呼ぶときは、初期化されているという保証がないので
		呼び出し前にIsAvailableによる確認を必ず行うこと。
	*/
	virtual bool DeinitDllImp();

	// DLLファイル名の取得(複数を順次)
	/*!
		DLLファイル名として複数の可能性があり，そのうちの一つでも
		見つかったものを使用する場合に対応する．
		
		番号に応じてそれぞれ異なるファイル名を返すことができる．
		LoadLibrary()からはcounterを0から1ずつ増加させて順に呼びだされる．
		それはDLLのロードに成功する(成功)か，戻り値としてNULLを返す(失敗)
		まで続けられる．

		@param[in] nIndex インデックス．(0〜)
		
		@return 引数に応じてDLL名(LoadLibraryに渡す文字列)，またはNULL．
	*/
	virtual LPCTSTR GetDllNameImp(int nIndex) = 0;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         実装補助                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	bool RegisterEntries(const ImportTable table[]);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	HINSTANCE		hInstance;
	std::tstring	strLoadedDllName;
};

