/*!	@file
	@brief ジャック管理クラス
*/
#pragma once

#include "plugin/Plugin.h"
#include <list>

#define PP_COMMAND_STR	L"Command"

// ジャック（＝プラグイン可能箇所）
enum EJack {
	PP_NONE			= -1,
	PP_COMMAND		= 0,
//	PP_INSTALL,
//	PP_UNINSTALL,
//	PP_APP_START,	// 現状エディタごとにプラグイン管理しているため
//	PP_APP_END,		// アプリレベルのイベントは扱いにくい
	PP_EDITOR_START,
	PP_EDITOR_END,
	PP_DOCUMENT_OPEN,
	PP_DOCUMENT_CLOSE,
	PP_DOCUMENT_BEFORE_SAVE,
	PP_DOCUMENT_AFTER_SAVE,
	PP_OUTLINE,
	PP_SMARTINDENT,
	PP_COMPLEMENT,
	PP_COMPLEMENTGLOBAL,
	PP_MACRO,

	// ↑ジャックを追加するときはこの行の上に。
	PP_BUILTIN_JACK_COUNT	// 組み込みジャック数
};

// ジャック定義構造体
struct JackDef {
	EJack			ppId;
	const wchar_t*	szName;
	Plug::Array	plugs;	// ジャックに関連付けられたプラグ
};

// プラグ登録結果
enum ERegisterPlugResult {
	PPMGR_REG_OK,				// プラグイン登録成功
	PPMGR_INVALID_NAME,			// ジャック名が不正
	PPMGR_CONFLICT				// 指定したジャックは別のプラグインが接続している
};

// ジャック管理クラス
class JackManager : public TSingleton<JackManager> {
	friend class TSingleton<JackManager>;
	JackManager();

	typedef std::wstring wstring;

	// 操作
public:
	ERegisterPlugResult RegisterPlug(wstring pszJack, Plug* plug);	// プラグをジャックに関連付ける
	bool UnRegisterPlug(wstring pszJack, Plug* plug);	// プラグの関連付けを解除する
	bool GetUsablePlug(EJack jack, PlugId plugId, Plug::Array* plugs);	// 利用可能なプラグを検索する
private:
	EJack GetJackFromName(wstring sName);	// ジャック名をジャック番号に変換する

	// 属性
public:
	std::vector<JackDef> GetJackDef() const;	// ジャック定義一覧を返す
	EFunctionCode GetCommandCode(int index) const;		// プラグインコマンドの機能コードを返す
	int GetCommandName(int funccode, wchar_t* buf, int size) const;	// プラグインコマンドの名前を返す
	size_t GetCommandCount() const;	// プラグインコマンドの数を返す
	Plug* GetCommandById(int id) const;	// IDに合致するコマンドプラグを返す
	const Plug::Array& GetPlugs(EJack jack) const;	// プラグを返す

	// メンバ変数
private:
	DllSharedData* pShareData;
	std::vector<JackDef> jacks;	// ジャック定義の一覧
};

