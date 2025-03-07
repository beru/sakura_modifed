#pragma once

#include "SelectLang.h"

class ShareData;

// ShareData.hは、自分のInterfaceしか提供しません。別にDllSharedData.hをincludeすること。
struct DllSharedData;
struct TypeConfig;
class Mutex;

/*!	@brief 共有データの管理

	ShareDataはProcessのメンバであるため，両者の寿命は同一です．
	本来はProcessオブジェクトを通じてアクセスするべきですが，
	Process内のデータ領域へのポインタをstatic変数に保存することで
	Singletonのようにどこからでもアクセスできる構造になっています．

	共有メモリへのポインタをpShareDataに保持します．このメンバは
	公開されていますが，ShareDataによってMap/Unmapされるために
	ChareDataの消滅によってポインタpShareDataも無効になることに
	注意してください．
*/
class ShareData : public TSingleton<ShareData> {
	friend class TSingleton<ShareData>;
	ShareData();
	~ShareData();

public:
	/*
	||  Attributes & Operations
	*/
	bool InitShareData();	// ShareDataクラスの初期化処理
	void RefreshString();	// 言語選択後に共有メモリ内の文字列を更新する
	
	// MRU系
	bool IsPathOpened(const TCHAR* pszPath, HWND* phwndOwner); // 指定ファイルが開かれているか調べる
	bool ActiveAlreadyOpenedWindow(const TCHAR* pszPath, HWND* phwndOwner, EncodingType nCharCode); // 指定ファイルが開かれているか調べつつ、多重オープン時の文字コード衝突も確認

	// デバッグ  今は主にマクロ・外部コマンド実行用
	void TraceOut(LPCTSTR lpFmt, ...);	// アウトプットウィンドウに出力(printfフォーマット)
	void TraceOutString(const wchar_t* pszStr, int len = -1);	// アウトプットウィンドウに出力(未加工文字列)
	void SetTraceOutSource(HWND hwnd) { hwndTraceOutSource = hwnd; }	// TraceOut起動元ウィンドウの設定
	bool OpenDebugWindow(HWND hwnd, bool bAllwaysActive);	// デバッグウィンドウを開く

	bool IsPrivateSettings(void);

	// マクロ関連
	int	 GetMacroFilename(int idx, TCHAR* pszPath, size_t nBufLen); // idxで指定したマクロファイル名（フルパス）を取得する
	bool BeReloadWhenExecuteMacro(int idx);	// idxで指定したマクロは、実行するたびにファイルを読み込む設定か？

	// タイプ別設定(コントロールプロセス専用)
	void CreateTypeSettings();
	std::vector<TypeConfig*>& GetTypeSettings();

	// 国際化対応のための文字列を変更する(コントロールプロセス専用)
	void ConvertLangValues(std::vector<std::wstring>& values, bool bSetValues);

	static Mutex& GetMutexShareWork();

protected:
	/*
	||  実装ヘルパ関数
	*/

	void InitKeyword(DllSharedData&);
	bool InitKeyAssign(DllSharedData&);
	void RefreshKeyAssignString(DllSharedData&);
	void InitToolButtons(DllSharedData&);
	void InitTypeConfigs(DllSharedData&, std::vector<TypeConfig*>&);
	void InitPopupMenu(DllSharedData&);

public:
	static void InitFileTree(FileTree*);

private:
	SelectLang		selectLang;			// メッセージリソースDLL読み込み用（プロセスに1個）
	HANDLE			hFileMap;
	DllSharedData*	pShareData;
	std::vector<TypeConfig*>* 	pvTypeSettings;	// (コントロールプロセスのみ)
	HWND			hwndTraceOutSource;	// TraceOutA()起動元ウィンドウ（いちいち起動元を指定しなくてすむように）

};

