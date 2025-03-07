#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         アクセサ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// どこからでもアクセスできる、共有データアクセサ
struct DllSharedData;

// DllSharedDataへの簡易アクセサ
inline DllSharedData& GetDllShareData()
{
	extern DllSharedData* g_theDLLSHAREDATA;

	assert(g_theDLLSHAREDATA);
	return *g_theDLLSHAREDATA;
}

inline DllSharedData& GetDllShareData(bool bNullCheck)
{
	extern DllSharedData* g_theDLLSHAREDATA;

	if (bNullCheck) {
		assert(g_theDLLSHAREDATA);
	}
	return *g_theDLLSHAREDATA;
}

// DllSharedDataを確保したら、まずこれを呼ぶ。破棄する前にも呼ぶ。
inline
void SetDllShareData(DllSharedData* pShareData)
{
	extern DllSharedData* g_theDLLSHAREDATA;

	g_theDLLSHAREDATA = pShareData;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    共有メモリ構成要素                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include "config/maxdata.h"

#include "env/AppNodeManager.h"	// Share_Nodes
#include "env/CommonSetting.h"
#include "env/SearchKeywordManager.h"	// Share_SearchKeywords
#include "env/TagJumpManager.h"		// Share_TagJump
#include "env/FileNameManager.h"		// Share_FileNameManagement

#include "EditInfo.h"
#include "types/Type.h" // TypeConfig
#include "print/Print.h" // PrintSetting
#include "recent/Share_History.h"	// Share_History


// 共有フラグ
struct Share_Flags {
	bool	bEditWndChanging;		// 編集ウィンドウ切替中
	/*
		キーボードマクロは、記録終了した時点でファイル「szKeyMacroFileName」に書き出すことにする。
		bRecordingKeyMacroがtrueのときは、キーボードマクロの記録中なので、szKeyMacroFileNameにアクセスしてはならない。
	*/
	bool	bRecordingKeyMacro;		// キーボードマクロの記録中
	HWND	hwndRecordingKeyMacro;	// キーボードマクロを記録中のウィンドウ
};

// 共有ワークバッファ
struct Share_WorkBuffer {
	//           UNICODE版では、余分に領域を使うことが予想されるため、ANSI版の2倍確保。
private:
	BYTE pWork[32000 * sizeof(TCHAR)];
public:
	template <class T>
	T* GetWorkBuffer() { return reinterpret_cast<T*>(pWork); }

	template <class T>
	size_t GetWorkBufferCount() { return sizeof(pWork)/sizeof(T); }

public:
	EditInfo editInfo_MYWM_GETFILEINFO;	// MYWM_GETFILEINFOデータ受け渡し用	####美しくない
	Point logicPoint;
	TypeConfig typeConfig;
};

// 共有ハンドル
struct Share_Handles {
	HWND	hwndTray;
	HWND	hwndDebug;
	HACCEL	hAccel;
};

// EXE情報
struct Share_Version {
	DWORD	dwProductVersionMS;
	DWORD	dwProductVersionLS;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   共有メモリ構造体本体                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DllSharedData {
	// -- -- バージョン -- -- //
	/*!
		データ構造 Version
		データ構造の異なるバージョンの同時起動を防ぐため
		必ず先頭になくてはならない．
	*/
	unsigned int				vStructureVersion;
	unsigned int				nSize;

	// -- -- 非保存対象 -- -- //
	Share_Version				version;		// ※読込は行わないが、書込は行う
	Share_WorkBuffer			workBuffer;
	Share_Flags					flags;
	Share_Nodes					nodes;
	Share_Handles				handles;

	CharWidthCache				charWidth;								// 文字半角全角キャッシュ
	DWORD						dwCustColors[16];						// フォントDialogカスタムパレット

	// プラグイン
	uint16_t					plugCmdIcons[MAX_PLUGIN * MAX_PLUG_CMD];	// プラグイン コマンド ICON 番号
	size_t						maxToolBarButtonNum;					// ツールバーボタン 最大値

	// -- -- 保存対象 -- -- //
	// 設定
	CommonSetting				common;									// 共通設定
	int							nTypesCount;							// タイプ別設定数
	TypeConfig					typeBasis;								// タイプ別設定: 共通
	TypeConfigMini				typesMini[MAX_TYPES];					// タイプ別設定(mini)
	PrintSetting				printSettingArr[MAX_PrintSettingARR];	// 印刷ページ設定
	int							nLockCount;								// ロックカウント
	
	// その他
	Share_SearchKeywords		searchKeywords;
	Share_TagJump				tagJump;
	Share_FileNameManagement	fileNameManagement;
	Share_History				history;

	// 外部コマンド実行ダイアログのオプション
	int							nExecFlgOpt;				// 外部コマンド実行オプション
	// DIFF差分表示ダイアログのオプション
	int							nDiffFlgOpt;				// DIFF差分表示
	// タグファイルの作成ダイアログのオプション
	TCHAR						szTagsCmdLine[_MAX_PATH];	// TAGSコマンドラインオプション
	int							nTagsOpt;					// TAGSオプション(チェック)

	// -- -- テンポラリ -- -- //
	// 指定行へジャンプダイアログのオプション
	bool						bLineNumIsCRLF_ForJump;		// 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か
};

class ShareDataLockCounter {
public:
	ShareDataLockCounter();
	~ShareDataLockCounter();

	static int GetLockCounter();
	static void WaitLock(HWND, ShareDataLockCounter** = NULL);
private:
};

