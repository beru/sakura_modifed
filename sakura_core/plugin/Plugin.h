/*!	@file
	@brief プラグイン基本クラス
*/
#pragma once

#include <algorithm>
#include "macro/WSHIfObj.h"
#include "DataProfile.h"
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
#define PII_OPTION					L"Option"		// オプション定義情報


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
		id(id),
		sJack(sJack),
		sHandler(sHandler),
		sLabel(sLabel),
		plugin(plugin)
	{
	}
	// デストラクタ
public:
	virtual ~Plug() {}

	// 操作
public:
	bool Invoke(EditView& view, WSHIfObj::List& params);	// プラグを実行する

	// 属性
public:
	EFunctionCode GetFunctionCode() const;

	// 補助関数
public:
	// Plug Function番号の計算(クラス外でも使えるバージョン)
	static inline EFunctionCode GetPluginFunctionCode(PluginId nPluginId, PlugId nPlugId) {
		return static_cast<EFunctionCode>((nPluginId%20 * 100) + (nPluginId/20 * 50) + nPlugId + F_PLUGCOMMAND_FIRST);
	}

	// PluginId番号の計算(クラス外でも使えるバージョン)
	static inline PluginId GetPluginId(EFunctionCode nFunctionCode) {
		if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
			return PluginId((nFunctionCode - F_PLUGCOMMAND_FIRST)/100 + (nFunctionCode%100/50 * 20));
		}
		return PluginId(-1);
	}

	// PluginNo番号の計算(クラス外でも使えるバージョン)
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
	static OutlineType GetOutlineType(EFunctionCode nFunctionCode) {
		return static_cast<OutlineType>(nFunctionCode);
	}

	static SmartIndentType GetSmartIndentType(EFunctionCode nFunctionCode) {
		return static_cast<SmartIndentType>(nFunctionCode);
	}

	// メンバ変数
public:
	const PlugId id;				// プラグID
	const wstring sJack;			// 関連付けるジャック名
	const wstring sHandler;			// ハンドラ文字列（関数名）
	const wstring sLabel;			// ラベル文字列
	wstring sIcon;					// アイコンのファイルパス
	Plugin& plugin;					// 親プラグイン
};

// オプション定義
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
		Plugin& parent,
		const wstring& sLabel,
		const wstring& sSection,
		const wstring& sKey,
		const wstring& sType,
		const wstring& sSelects,
		const wstring& sDefaultVal,
		int index
		)
		:
		parent(parent),
		sLabel(sLabel),
		sSection(sSection),
		sKey(sKey),
		sType(sType),
		sSelects(sSelects),
		sDefaultVal(sDefaultVal),
		index(index)
	{
		// 小文字変換
		std::transform(this->sType.begin(), this->sType.end(), this->sType.begin(), tolower);
	}

	// デストラクタ
public:
	~PluginOption() {}

	// 操作
public:
	wstring	GetLabel(void)	{ return sLabel; }
	void	GetKey(wstring* sectin, wstring* key)	{ 
		*sectin = sSection; 
		*key = sKey;
	}
	wstring	GetType(void)	{ return sType; }
	int 	GetIndex(void)	{ return index; }
	std::vector<wstring> GetSelects() {
		return (wstring_split(sSelects, L'|'));
	}
	wstring	GetDefaultVal() { return sDefaultVal; }

protected:
	Plugin&		parent;
	wstring		sLabel;
	wstring		sSection;
	wstring		sKey;
	wstring		sType;
	wstring		sSelects;		// 選択候補
	wstring		sDefaultVal;
	int 		index; 
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
	virtual int AddCommand(const wchar_t* handler, const wchar_t* label, const wchar_t* icon, bool doRegister);	// コマンドを追加する
	int 	GetCommandCount()	{ return nCommandCount; }			// コマンド数を返す

protected:
	bool ReadPluginDefCommon(DataProfile& profile, DataProfile* profileMlang);	// プラグイン定義ファイルのCommonセクションを読み込む
	bool ReadPluginDefPlug(DataProfile& profile, DataProfile* profileMlang);	// プラグイン定義ファイルのPlugセクションを読み込む
	bool ReadPluginDefCommand(DataProfile& profile, DataProfile* profileMlang);	// プラグイン定義ファイルのCommandセクションを読み込む
	bool ReadPluginDefOption(DataProfile& profile, DataProfile* profileMlang);	// プラグイン定義ファイルのOptionセクションを読み込む
	bool ReadPluginDefString(DataProfile& profile, DataProfile* profileMlang);	// プラグイン定義ファイルのStringセクションを読み込む

	// Plugインスタンスの作成。ReadPluginDefPlug/Command から呼ばれる。
	virtual Plug* CreatePlug(
		Plugin& plugin,
		PlugId id,
		const wstring& sJack,
		const wstring& sHandler,
		const wstring& sLabel
	) {
		return new Plug(plugin, id, sJack, sHandler, sLabel);
	}

//	void NormalizeExtList(const wstring& sExtList, wstring& sOut);	// カンマ区切り拡張子リストを正規化する

	// 属性
public:
	tstring GetFilePath(const tstring& sFileName) const;					// プラグインフォルダ基準の相対パスをフルパスに変換
	tstring GetPluginDefPath() const { return GetFilePath(PII_FILENAME); }	// プラグイン定義ファイルのパス
	tstring GetOptionPath() const { return sOptionDir + PII_OPTFILEEXT; }	// オプションファイルのパス
	tstring GetFolderName() const;	// プラグインのフォルダ名を取得
	virtual Plug::Array GetPlugs() const = 0;								// プラグの一覧

	// メンバ変数
public:
	PluginId id;				// プラグイン番号（エディタがふる0～MAX_PLUGIN-1の番号）
	wstring sId;				// プラグインID
	wstring sName;				// プラグイン和名
	wstring sDescription;		// プラグインについての簡単な記述
	wstring sAuthor;			// 作者
	wstring sVersion;			// バージョン
	wstring sUrl;				// 配布URL
	tstring sBaseDir;
	tstring sOptionDir;
	tstring sLangName;		// 言語名
	PluginOption::Array options;		// オプション
	std::vector<std::wstring> aStrings;	// 文字列
private:
	bool bLoaded;
protected:
	Plug::Array plugs;
	int nCommandCount;

	// 非実装提供
public:
	virtual bool InvokePlug(EditView& view, Plug& plug, WSHIfObj::List& param) = 0;			// プラグを実行する
	virtual bool ReadPluginDef(DataProfile& profile, DataProfile* profileMlang) = 0;		// プラグイン定義ファイルを読み込む
	virtual bool ReadPluginOption(DataProfile& profile) = 0;									// オプションファイルを読み込む
};

