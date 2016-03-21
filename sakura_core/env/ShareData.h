/*!	@file
	@brief プロセス間共有データへのアクセス

	@author Norio Nakatani
	@date 1998/05/26  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, aroka, Moca, MIK, YAZAKI, hor
	Copyright (C) 2003, Moca, aroka, MIK, genta
	Copyright (C) 2004, Moca, novice, genta
	Copyright (C) 2005, MIK, genta, ryoji, aroka, Moca
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, Uchi
	Copyright (C) 2011, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
// 2007.09.23 kobake m_nSEARCHKEYArrNum,      m_szSEARCHKEYArr      を searchKeys      にまとめました
// 2007.09.23 kobake m_nREPLACEKEYArrNum,     m_szREPLACEKEYArr     を replaceKeys     にまとめました
// 2007.09.23 kobake m_nGREPFILEArrNum,       m_szGREPFILEArr       を grepFiles       にまとめました
// 2007.09.23 kobake m_nGREPFOLDERArrNum,     m_szGREPFOLDERArr     を grepFolders     にまとめました
// 2007.09.23 kobake m_szCmdArr,              m_nCmdArrNum          を m_aCommands        にまとめました
// 2007.09.23 kobake m_nTagJumpKeywordArrNum, m_szTagJumpKeywordArr を aTagJumpKeywords にまとめました
// 2007.12.13 kobake DllSharedDataへの簡易アクセサを用意

#pragma once

#include "SelectLang.h"		// 2011.04.10 nasukoji

class ShareData;

// 2010.04.19 Moca DllSharedData関連はDllSharedData.h等最低限必要な場所へ移動
// ShareData.hは、自分のInterfaceしか提供しません。別にDllSharedData.hをincludeすること。
struct DllSharedData;
struct TypeConfig;
class Mutex;

/*!	@brief 共有データの管理

	ShareDataはProcessのメンバであるため，両者の寿命は同一です．
	本来はProcessオブジェクトを通じてアクセスするべきですが，
	Process内のデータ領域へのポインタをstatic変数に保存することで
	Singletonのようにどこからでもアクセスできる構造になっています．

	共有メモリへのポインタをm_pShareDataに保持します．このメンバは
	公開されていますが，ShareDataによってMap/Unmapされるために
	ChareDataの消滅によってポインタm_pShareDataも無効になることに
	注意してください．

	@date 2002.01.03 YAZAKI m_tbMyButtonなどをShareDataからCMenuDrawerへ移動したことによる修正。
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
	bool ActiveAlreadyOpenedWindow(const TCHAR* pszPath, HWND* phwndOwner, EncodingType nCharCode); // 指定ファイルが開かれているか調べつつ、多重オープン時の文字コード衝突も確認 // 2007.03.16

	// デバッグ  今は主にマクロ・外部コマンド実行用
	void TraceOut(LPCTSTR lpFmt, ...);	// アウトプットウィンドウに出力(printfフォーマット)
	void TraceOutString(const wchar_t* pszStr, int len = -1);	// アウトプットウィンドウに出力(未加工文字列)
	void SetTraceOutSource(HWND hwnd) { m_hwndTraceOutSource = hwnd; }	// TraceOut起動元ウィンドウの設定
	bool OpenDebugWindow(HWND hwnd, bool bAllwaysActive);	// デバッグウィンドウを開く

	bool IsPrivateSettings(void);

	// マクロ関連
	int	 GetMacroFilename(int idx, TCHAR* pszPath, int nBufLen); // idxで指定したマクロファイル名（フルパス）を取得する	// Jun. 14, 2003 genta 引数追加．書式変更
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

	// Jan. 30, 2005 genta 初期化関数の分割
	void InitKeyword(DllSharedData&);
	bool InitKeyAssign(DllSharedData&); // 2007.11.04 genta 起動中止のため値を返す
	void RefreshKeyAssignString(DllSharedData&);
	void InitToolButtons(DllSharedData&);
	void InitTypeConfigs(DllSharedData&, std::vector<TypeConfig*>&);
	void InitPopupMenu(DllSharedData&);

public:
	static void InitFileTree(FileTree*);

private:
	SelectLang		m_selectLang;			// メッセージリソースDLL読み込み用（プロセスに1個）		// 2011.04.10 nasukoji
	HANDLE			m_hFileMap;
	DllSharedData*	m_pShareData;
	std::vector<TypeConfig*>* 	m_pvTypeSettings;	// (コントロールプロセスのみ)
	HWND			m_hwndTraceOutSource;	// TraceOutA()起動元ウィンドウ（いちいち起動元を指定しなくてすむように）

};

