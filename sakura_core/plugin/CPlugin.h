/*!	@file
	@brief プラグイン基本クラス

*/
/*
	Copyright (C) 2009, syat

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include <algorithm>
#include "macro/CWSHIfObj.h"
#include "CDataProfile.h"
#include "util/string_ex.h"

// プラグインの管理番号index
typedef int PluginId;
// プラグの管理番号 プラグインのコマンドプラグごとに一意。ほかは0
typedef int PlugId;

// プラグイン定義ファイル名
#define PII_FILENAME				_T("plugin.def")
#define PII_L10NDIR					_T("local")
#define PII_L10NFILEBASE			_T("plugin_")
#define PII_L10NFILEEXT				_T(".def")
// オプションファイル拡張子（オプションファイル＝個別フォルダ名＋拡張子）
#define PII_OPTFILEEXT				_T(".ini")

// プラグイン定義ファイル・キー文字列
#define	PII_PLUGIN					L"Plugin"		// 共通情報
#define	PII_PLUGIN_ID				L"Id"			// ID：プラグインID
#define	PII_PLUGIN_NAME				L"Name"			// 名前：プラグイン名
#define	PII_PLUGIN_DESCRIPTION		L"Description"	// 説明：簡潔な説明
#define	PII_PLUGIN_PLUGTYPE			L"Type"			// 種別：wsh / dll
#define	PII_PLUGIN_AUTHOR			L"Author"		// 作者：著作権者名
#define	PII_PLUGIN_VERSION			L"Version"		// バージョン：プラグインのバージョン
#define	PII_PLUGIN_URL				L"Url"			// 配布URL：配布元URL

#define PII_PLUG					L"Plug"			// プラグ情報
#define PII_STRING					L"String"		// 文字列情報

#define PII_COMMAND					L"Command"		// コマンド情報
#define PII_OPTION					L"Option"		// オプション定義情報	// 2010/3/24 Uchi


class Plugin;

// プラグ（プラグイン内の処理単位）クラス
class Plug {
	// 型定義
protected:
	typedef std::wstring wstring;
public:
	/*!
	  Plug::Arrayはstd::vectorなので、要素の追加削除（insert/erase）をすると
	  イテレータが無効になることがある。そのため変数に格納したイテレータを
	  insert/eraseの第一引数に指定すると、VC2005でビルドエラーが出る。
	  かわりにbegin/endからの相対位置指定や、インデックス指定を使うこと。
	*/
	typedef std::vector<Plug*> Array;			// プラグのリスト
	typedef Array::const_iterator ArrayIter;	// そのイテレータ

	// コンストラクタ
public:
	Plug(Plugin& plugin, PlugId id, const wstring& sJack, const wstring& sHandler, const wstring& sLabel)
		:
		m_id(id),
		m_sJack(sJack),
		m_sHandler(sHandler),
		m_sLabel(sLabel),
		m_plugin(plugin)
	{
	}
	// デストラクタ
public:
	virtual ~Plug() {}

	// 操作
public:
	bool Invoke(EditView* view, WSHIfObj::List& params);	// プラグを実行する

	// 属性
public:
	EFunctionCode GetFunctionCode() const;

	// 補助関数
public:
	// Plug Function番号の計算(クラス外でも使えるバージョン)
	// 2010/4/19 Uchi
	// 2011/8/20 syat 関数コードの割り当て直し
	static inline EFunctionCode GetPluginFunctionCode(PluginId nPluginId, PlugId nPlugId) {
		return static_cast<EFunctionCode>((nPluginId%20 * 100) + (nPluginId/20 * 50) + nPlugId + F_PLUGCOMMAND_FIRST);
	}

	// PluginId番号の計算(クラス外でも使えるバージョン)
	// 2010/4/19 Uchi
	// 2011/8/20 syat 関数コードの割り当て直し
	static inline PluginId GetPluginId(EFunctionCode nFunctionCode) {
		if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
			return PluginId((nFunctionCode - F_PLUGCOMMAND_FIRST)/100 + (nFunctionCode%100/50 * 20));
		}
		return PluginId(-1);
	}

	// PluginNo番号の計算(クラス外でも使えるバージョン)
	// 2010/6/24 Uchi
	// 2011/8/20 syat 関数コードの割り当て直し
	static inline PlugId GetPlugId(EFunctionCode nFunctionCode) {
		if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
			return PlugId(nFunctionCode%100 - (nFunctionCode%100/50 * 50));
		}
		return PlugId(-1);
	}

	/* PluginId, PlugId と 関数コードのマッピング *****************************
	 *   PluginId … プラグインの番号 0～39
	 *     PlugId … プラグイン内のプラグの番号 0～49
	 *
	 *   関数コード 20000～21999   ()内は(PluginId, PlugId)を表す
	 *   +------------+------------+----+------------+
	 *   |20000(0,0)  |20100(1,0)  |    |21900(19,0) |
	 *   |  :         |  :         | … |  :         |
	 *   |20049(0,49) |20149(1,49) |    |21949(19,49)| 
	 *   +------------+------------+----+------------+
	 *   |20050(20,0) |20150(21,0) |    |21950(39,0) |
	 *   |  :         |  :         | … |  :         |
	 *   |20099(20,49)|20199(21,49)|    |21999(39,49)| 
	 *   +------------+------------+----+------------+
	 *   もし足りなければ、22000～23999を払い出して食いつぶす
	 *************************************************************************/
	static EOutlineType GetOutlineType(EFunctionCode nFunctionCode) {
		return static_cast<EOutlineType>(nFunctionCode);
	}

	static SmartIndentType GetSmartIndentType(EFunctionCode nFunctionCode) {
		return static_cast<SmartIndentType>(nFunctionCode);
	}

	// メンバ変数
public:
	const PlugId m_id;					// プラグID
	const wstring m_sJack;				// 関連付けるジャック名
	const wstring m_sHandler;			// ハンドラ文字列（関数名）
	const wstring m_sLabel;				// ラベル文字列
	wstring m_sIcon;					// アイコンのファイルパス
	Plugin& m_plugin;					// 親プラグイン
};

// オプション定義	// 2010/3/24 Uchi
std::vector<std::wstring> wstring_split(std::wstring, wchar_t);

class PluginOption {
	// 型定義
protected:
	typedef std::wstring wstring;
public:
	typedef std::vector<PluginOption*> Array;	// オプションのリスト
	typedef Array::const_iterator ArrayIter;	// そのイテレータ

	// コンストラクタ
public:
	PluginOption(
		Plugin* parent,
		const wstring& sLabel,
		const wstring& sSection,
		const wstring& sKey,
		const wstring& sType,
		const wstring& sSelects,
		const wstring& sDefaultVal,
		int index
		)
	{
		m_parent		= parent;
		m_sLabel		= sLabel;
		m_sSection	= sSection;
		m_sKey		= sKey;
		// 小文字変換
		m_sType		= sType;
		std::transform(m_sType.begin(), m_sType.end(), m_sType.begin(), tolower);
		m_sSelects	= sSelects;
		m_sDefaultVal = sDefaultVal;
		m_index		= index;
	}

	// デストラクタ
public:
	~PluginOption() {}

	// 操作
public:
	wstring	GetLabel(void)	{ return m_sLabel; }
	void	GetKey(wstring* sectin, wstring* key)	{ 
		*sectin = m_sSection; 
		*key = m_sKey;
	}
	wstring	GetType(void)	{ return m_sType; }
	int 	GetIndex(void)	{ return m_index; }
	std::vector<wstring> GetSelects() {
		return (wstring_split(m_sSelects, L'|'));
	}
	wstring	GetDefaultVal() { return m_sDefaultVal; }

protected:
	Plugin*	m_parent;
	wstring		m_sLabel;
	wstring		m_sSection;
	wstring		m_sKey;
	wstring		m_sType;
	wstring		m_sSelects;		// 選択候補
	wstring		m_sDefaultVal;
	int 		m_index; 
};


// プラグインクラス

class Plugin {
	// 型定義
protected:
	typedef std::wstring wstring;
	typedef std::string string;

public:
	typedef std::list<Plugin*> List;		// プラグインのリスト
	typedef List::const_iterator ListIter;	// そのイテレータ

	// コンストラクタ
public:
	Plugin(const tstring& sBaseDir);

	// デストラクタ
public:
	virtual ~Plugin(void);

	// 操作
public:
	virtual int AddCommand(const WCHAR* handler, const WCHAR* label, const WCHAR* icon, bool doRegister);	// コマンドを追加する
	int 	GetCommandCount()	{ return m_nCommandCount; }			// コマンド数を返す	2010/7/4 Uchi

protected:
	bool ReadPluginDefCommon(DataProfile *cProfile, DataProfile *cProfileMlang);	// プラグイン定義ファイルのCommonセクションを読み込む
	bool ReadPluginDefPlug(DataProfile *cProfile, DataProfile *cProfileMlang);	// プラグイン定義ファイルのPlugセクションを読み込む
	bool ReadPluginDefCommand(DataProfile *cProfile, DataProfile *cProfileMlang);	// プラグイン定義ファイルのCommandセクションを読み込む
	bool ReadPluginDefOption(DataProfile *cProfile, DataProfile *cProfileMlang);	// プラグイン定義ファイルのOptionセクションを読み込む	// 2010/3/24 Uchi
	bool ReadPluginDefString(DataProfile *cProfile, DataProfile *cProfileMlang);	// プラグイン定義ファイルのStringセクションを読み込む

	// Plugインスタンスの作成。ReadPluginDefPlug/Command から呼ばれる。
	virtual Plug* CreatePlug(Plugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel) {
		return new Plug(plugin, id, sJack, sHandler, sLabel);
	}

//	void NormalizeExtList(const wstring& sExtList, wstring& sOut);	// カンマ区切り拡張子リストを正規化する

	// 属性
public:
	tstring GetFilePath(const tstring& sFileName) const;					// プラグインフォルダ基準の相対パスをフルパスに変換
	tstring GetPluginDefPath() const { return GetFilePath(PII_FILENAME); }	// プラグイン定義ファイルのパス
	tstring GetOptionPath() const { return m_sOptionDir + PII_OPTFILEEXT; }	// オプションファイルのパス
	tstring GetFolderName() const;	// プラグインのフォルダ名を取得
	virtual Plug::Array GetPlugs() const = 0;								// プラグの一覧

	// メンバ変数
public:
	PluginId m_id;				// プラグイン番号（エディタがふる0～MAX_PLUGIN-1の番号）
	wstring m_sId;				// プラグインID
	wstring m_sName;			// プラグイン和名
	wstring m_sDescription;		// プラグインについての簡単な記述
	wstring m_sAuthor;			// 作者
	wstring m_sVersion;			// バージョン
	wstring m_sUrl;				// 配布URL
	tstring m_sBaseDir;
	tstring m_sOptionDir;
	tstring m_sLangName;		// 言語名
	PluginOption::Array m_options;		// オプション	// 2010/3/24 Uchi
	std::vector<std::wstring> m_aStrings;	// 文字列
private:
	bool m_bLoaded;
protected:
	Plug::Array m_plugs;
	int m_nCommandCount;

	// 非実装提供
public:
	virtual bool InvokePlug(EditView* view, Plug& plug, WSHIfObj::List& param) = 0;			// プラグを実行する
	virtual bool ReadPluginDef(DataProfile* cProfile, DataProfile* cProfileMlang) = 0;		// プラグイン定義ファイルを読み込む
	virtual bool ReadPluginOption(DataProfile* cProfile) = 0;									// オプションファイルを読み込む
};

