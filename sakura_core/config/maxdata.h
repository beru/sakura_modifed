#pragma once

enum maxdata{
	MAX_EDITWINDOWS				= 256,
	MAX_SEARCHKEY				=  30,
	MAX_REPLACEKEY				=  30,
	MAX_GREPFILE				=  30,
	MAX_GREPFOLDER				=  30,
	MAX_TYPES					=  60,
	MAX_TYPES_EXTS				=  64,
	MAX_PrintSettingARR			=   8,

	MACRONAME_MAX				= 64,
	MAX_EXTCMDLEN				= 1024,
	MAX_EXTCMDMRUNUM			= 32,

	MAX_CMDLEN					= 1024,
	MAX_CMDARR					= 32,
	MAX_REGEX_KEYWORD			= 100,
	MAX_REGEX_KEYWORDLEN		= 1000,
	MAX_REGEX_KEYWORDLISTLEN	= MAX_REGEX_KEYWORD * 100 + 1,

	MAX_KEYHELP_FILE			= 20,

	MAX_MARKLINES_LEN			= 1023,
	MAX_DOCTYPE_LEN				= 7,
	MAX_TRANSFORM_FILENAME		= 16,

	MAX_CUSTMACRO				= 50,
	MAX_CUSTMACRO_ICO			= 50,	// アイコンに専用位置を割り当てている数

	MAX_TAGJUMPNUM				= 100,	// タブジャンプ情報最大値
	MAX_TAGJUMP_KEYWORD			= 30,	// タグジャンプ用キーワード最大登録数
	MAX_KEYWORDSET_PER_TYPE		= 10,	// タイプ別設定毎のキーワードセット数
	MAX_VERTLINES = 10,	// 指定桁縦線

	// MRUリストに関係するmaxdata
	MAX_MRU						=  36,	//  A-Z で36個になるので
	MAX_OPENFOLDER				=  36,

	MAX_PLUGIN					= 40,	// 登録できるプラグインの数
	MAX_PLUG_CMD				= 50,	// 登録できるプラグイン コマンドの数+1(1 origin分)
	MAX_PLUG_OPTION				= 100,	// 登録できるプラグインオプションの数
	MAX_PLUGIN_ID				= 63+1,	// プラグインIDの最大長さ
	MAX_PLUGIN_NAME				= 63+1,	// プラグイン名の最大長さ
	MAX_PLUG_STRING				= 100,	// 登録できるプラグイン文字列の数

	// MainMenu
	MAX_MAINMENU				= 500,	// 登録できるメインメニューの数
	MAX_MAINMENU_TOP			= 20,	// 登録できるメインメニューの数
	MAX_MAIN_MENU_NAME_LEN		= 40,	// メインメニュー名文字列長
};

