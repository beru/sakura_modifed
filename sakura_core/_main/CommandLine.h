/*!	@file
	@brief コマンドラインパーサ ヘッダファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CControlTrayより分離
	Copyright (C) 2002, genta
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "global.h"
#include "EditInfo.h"
#include "util/design_template.h"

class Memory;

/*!	検索オプション
	20020118 aroka
*/
struct GrepInfo {
	NativeW			mGrepKey;				// 検索キー
	NativeW			mGrepRep;				// 置換キー
	NativeT			mGrepFile;				// 検索対象ファイル
	NativeT			mGrepFolder;			// 検索対象フォルダ
	SearchOption	grepSearchOption;		// 検索オプション
	bool			bGrepCurFolder;			// カレントディレクトリを維持
	bool			bGrepStdout;			// 標準出力モード
	bool			bGrepHeader;			// ヘッダ情報表示
	bool			bGrepSubFolder;			// サブフォルダを検索する
	EncodingType	charEncoding;			// 文字コードセット
	int				nGrepOutputStyle;		// 結果出力形式
	int				nGrepOutputLineType;	// 結果出力：行を出力/該当部分/否マッチ行
	bool			bGrepOutputFileOnly;	// ファイル毎最初のみ検索
	bool			bGrepOutputBaseFolder;	// ベースフォルダ表示
	bool			bGrepSeparateFolder;	// フォルダ毎に表示
	bool			bGrepReplace;			// Grep置換
	bool			bGrepPaste;				// クリップボードから貼り付け
	bool			bGrepBackup;			// 置換でバックアップを保存
};


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

/*!
	@brief コマンドラインパーサ クラス
*/
class CommandLine : public TSingleton<CommandLine> {
	friend class TSingleton<CommandLine>;
	CommandLine();

	static int CheckCommandLine(
		LPTSTR	str,		// [in] 検証する文字列（先頭の-は含まない）
		TCHAR**	arg,		// [out] 引数がある場合はその先頭へのポインタ
		int*	arglen		// [out] 引数の長さ
	);

	/*!
		引用符で囲まれている数値を認識するようにする
		@date 2002.12.05 genta
	*/
	static int AtoiOptionInt(const TCHAR* arg) {
		return (arg[0] == _T('"') || arg[0] == _T('\'')) ?
			_ttoi(arg + 1) : _ttoi(arg);
	}

// member accessor method
public:
	bool IsNoWindow() const {return bNoWindow;}
	bool IsWriteQuit() const {return bWriteQuit;}	// 2007.05.19 ryoji sakuext用に追加
	bool IsGrepMode() const {return bGrepMode;}
	bool IsGrepDlg() const {return bGrepDlg;}
	bool IsDebugMode() const {return bDebugMode;}
	bool IsViewMode() const {return bViewMode;}
	const EditInfo& GetEditInfo() const { return fi; }
	const GrepInfo& GetGrepInfo() const { return gi; }
	int GetGroupId() const {return nGroup;}	// 2007.06.26 ryoji
	LPCWSTR GetMacro() const { return mMacro.GetStringPtr(); }
	LPCWSTR GetMacroType() const { return mMacroType.GetStringPtr(); }
	LPCWSTR GetProfileName() const{ return mProfile.GetStringPtr(); }
	bool IsSetProfile() const{ return bSetProfile; }
	void SetProfileName(LPCWSTR s){
		bSetProfile = true;
		mProfile.SetString(s);
	}
	bool IsProfileMgr() { return bProfileMgr; }
	int GetFileNum(void) { return fileNames.size(); }
	const TCHAR* GetFileName(int i) { return i < GetFileNum() ? fileNames[i].c_str() : NULL; }
	void ClearFile(void) { fileNames.clear(); }
	void ParseCommandLine(LPCTSTR pszCmdLineSrc, bool bResponse = true);

// member valiables
private:
	bool		bGrepMode;		// [out] true: Grep Mode
	bool		bGrepDlg;		// Grepダイアログ
	bool		bDebugMode;		
	bool		bNoWindow;		// [out] true: 編集Windowを開かない
	bool		bWriteQuit;		// [out] true: 設定を保存して終了	// 2007.05.19 ryoji sakuext用に追加
	bool		bProfileMgr;
	bool		bSetProfile;
	EditInfo	fi;				//
	GrepInfo	gi;				//
	bool		bViewMode;		// [out] true: Read Only
	int			nGroup;			// グループID	// 2007.06.26 ryoji
	NativeW	mMacro;				// [out] マクロファイル名／マクロ文
	NativeW	mMacroType;			// [out] マクロ種別
	NativeW	mProfile;			// プロファイル名
	std::vector<std::tstring> fileNames;	// ファイル名(複数)
};

