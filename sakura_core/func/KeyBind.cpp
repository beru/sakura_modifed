/*!	@file
	@brief キー割り当てに関するクラス
*/
#include "StdAfx.h"
#include "func/KeyBind.h"
#include "env/ShareData.h"
#include "macro/SMacroMgr.h"

// KeyDataとほぼ同じ
struct KEYDATAINIT {
	short nKeyCode;			// Key Code (0 for non-keybord button)
	union {
		const TCHAR* pszKeyName;	// Key Name (for display)
		UINT nKeyNameId;			// String Resource Id (0x0000 - 0xFFFF)
	};
	EFunctionCode	nFuncCodeArr[8];	// Key Function Number
//					nFuncCodeArr[0]	//                      Key
//					nFuncCodeArr[1]	// Shift +              Key
//					nFuncCodeArr[2]	//         Ctrl +       Key
//					nFuncCodeArr[3]	// Shift + Ctrl +       Key
//					nFuncCodeArr[4]	//                Alt + Key
//					nFuncCodeArr[5]	// Shift +        Alt + Key
//					nFuncCodeArr[6]	//         Ctrl + Alt + Key
//					nFuncCodeArr[7]	// Shift + Ctrl + Alt + Key
};

// 実装補助
// KeyData配列にデータをセット
static void SetKeyNameArrVal(
	DllSharedData&		shareData,
	int					nIdx,
	const KEYDATAINIT*	pKeydata
);

KeyBind::KeyBind()
{
}

KeyBind::~KeyBind()
{
}

/*! Windows アクセラレータの作成 */
HACCEL KeyBind::CreateAccerelator(
	int			nKeyNameArrNum,
	KeyData*	pKeyNameArr
	)
{
	// 機能が割り当てられているキーの数をカウント -> nAccelArrNum
	int nAccelArrNum = 0;
	for (int i=0; i<nKeyNameArrNum; ++i) {
		auto& keyName = pKeyNameArr[i];
		if (keyName.nKeyCode != 0) {
			for (int j=0; j<8; ++j) {
				if (GetFuncCodeAt(keyName, j) != 0) {
					++nAccelArrNum;
				}
			}
		}
	}

	if (nAccelArrNum <= 0) {
		// 機能割り当てがゼロ
		return NULL;
	}
	std::vector<ACCEL> accels(nAccelArrNum);
	ACCEL* pAccelArr = &accels[0];
	int k = 0;
	for (int i=0; i<nKeyNameArrNum; ++i) {
		auto& keyName = pKeyNameArr[i];
		if (keyName.nKeyCode != 0) {
			for (int j=0; j<8; ++j) {
				if (GetFuncCodeAt(keyName, j) != 0) {
					auto& accel = pAccelArr[k];
					accel.fVirt = FNOINVERT | FVIRTKEY;
					accel.fVirt |= (j & _SHIFT) ? FSHIFT   : 0;
					accel.fVirt |= (j & _CTRL ) ? FCONTROL : 0;
					accel.fVirt |= (j & _ALT  ) ? FALT     : 0;
					accel.key = keyName.nKeyCode;
					accel.cmd = keyName.nKeyCode | (((WORD)j)<<8) ;

					++k;
				}
			}
		}
	}
	HACCEL hAccel = ::CreateAcceleratorTable(pAccelArr, nAccelArrNum);
	return hAccel;
}


/*! アクラセレータ識別子に対応するコマンド識別子を返す．
	対応するアクラセレータ識別子がない場合または機能未割り当ての場合は0を返す．
*/
EFunctionCode KeyBind::GetFuncCode(
	WORD		nAccelCmd,
	int			nKeyNameArrNum,
	KeyData*	pKeyNameArr,
	bool		bGetDefFuncCode // = true
	)
{
	int nCmd = (int)LOBYTE(nAccelCmd);
	int nSts = (int)HIBYTE(nAccelCmd);
	if (nCmd == 0) { // mouse command
		for (int i=0; i<nKeyNameArrNum; ++i) {
			auto& keyName = pKeyNameArr[i];
			if (nCmd == keyName.nKeyCode) {
				return GetFuncCodeAt(keyName, nSts, bGetDefFuncCode);
			}
		}
	}else {
		DllSharedData& shareData = GetDllShareData();
		return GetFuncCodeAt(pKeyNameArr[shareData.common.keyBind.keyToKeyNameArr[nCmd]], nSts, bGetDefFuncCode);
	}
	return F_DEFAULT;
}


/*!
	@return 機能が割り当てられているキーストロークの数
*/
int KeyBind::CreateKeyBindList(
	HINSTANCE		hInstance,		// [in] インスタンスハンドル
	int				nKeyNameArrNum,	// [in]
	KeyData*		pKeyNameArr,	// [out]
	NativeW&		memList,		//
	FuncLookup*		pFuncLookup,	// [in] 機能番号→名前の対応を取る
	bool			bGetDefFuncCode // [in] ON:デフォルト機能割り当てを使う/OFF:使わない デフォルト:true
	)
{
	wchar_t	szStr[256];
	wchar_t	szFuncName[256];
	wchar_t	szFuncNameJapanese[256];

	int nValidKeys = 0;
	memList.SetString(LTEXT(""));
	const wchar_t* pszSHIFT = LTEXT("Shift+");
	const wchar_t* pszCTRL  = LTEXT("Ctrl+");
	const wchar_t* pszALT   = LTEXT("Alt+");
	const wchar_t* pszTAB   = LTEXT("\t");
	const wchar_t* pszCR    = LTEXT("\r\n");	// \r=0x0d=CRを追加

	memList.AppendString(LSW(STR_ERR_DLGKEYBIND1));
	memList.AppendString(pszCR);
	memList.AppendStringLiteral(LTEXT("-----\t-----\t-----\t-----\t-----"));
	memList.AppendString(pszCR);

	for (int j=0; j<8; ++j) {
		for (int i=0; i<nKeyNameArrNum; ++i) {
			int iFunc = GetFuncCodeAt(pKeyNameArr[i], j, bGetDefFuncCode);

			if (iFunc != 0) {
				++nValidKeys;
				if (j & _SHIFT) {
					memList.AppendString(pszSHIFT);
				}
				if (j & _CTRL) {
					memList.AppendString(pszCTRL);
				}
				if (j & _ALT) {
					memList.AppendString(pszALT);
				}
				memList.AppendString(to_wchar(pKeyNameArr[i].szKeyName));
				// Oct. 31, 2001 genta 
				if (!pFuncLookup->Funccode2Name(
					iFunc,
					szFuncNameJapanese, 255
					)
				) {
					auto_strcpy(szFuncNameJapanese, LSW(STR_ERR_DLGKEYBIND2));
				}
				szFuncName[0] = LTEXT('\0'); /*"---unknown()--"*/

//				// 機能名日本語
//				::LoadString(
//					hInstance,
//					pKeyNameArr[i].nFuncCodeArr[j],
//					 szFuncNameJapanese, 255
//				);
				memList.AppendString(pszTAB);
				memList.AppendString(szFuncNameJapanese);

				// 機能ID→関数名，機能名日本語
				SMacroMgr::GetFuncInfoByID(
					hInstance,
					iFunc,
					szFuncName,
					szFuncNameJapanese
				);

				// 関数名
				memList.AppendString(pszTAB);
				memList.AppendString(szFuncName);

				// 機能番号
				memList.AppendString(pszTAB);
				auto_sprintf(szStr, LTEXT("%d"), iFunc);
				memList.AppendString(szStr);

				// キーマクロに記録可能な機能かどうかを調べる
				memList.AppendString(pszTAB);
				if (SMacroMgr::CanFuncIsKeyMacro(iFunc)) {
					memList.AppendStringLiteral(LTEXT("○"));
				}else {
					memList.AppendStringLiteral(LTEXT("×"));
				}

				memList.AppendString(pszCR);
			}
		}
	}
	return nValidKeys;
}

/** 機能に対応するキー名のサーチ(補助関数)

	与えられたシフト状態に対して，指定された範囲のキーエリアから
	当該機能に対応するキーがあるかを調べ，見つかったら
	対応するキー文字列をセットする．
	
	関数から出るときには検索開始位置(nKeyNameArrBegin)に
	次に処理するindexを設定する．

	@param[in,out] nKeyNameArrBegin 調査開始INDEX (終了時には次回の開始INDEXに書き換えられる)
	@param[in] nKeyNameArrBegin 調査終了INDEX + 1
	@param[in] pKeyNameArr キー配列
	@param[in] nShiftState シフト状態
	@param[out] memList キー文字列設定先
	@param[in]	nFuncId 検索対象機能ID
	@param[in]	bGetDefFuncCode 標準機能を取得するかどうか
*/
bool KeyBind::GetKeyStrSub(
	int&		nKeyNameArrBegin,
	int			nKeyNameArrEnd,
	KeyData*	pKeyNameArr,
	int			nShiftState,
	NativeT&	memList,
	int			nFuncId,
	bool		bGetDefFuncCode // = true
	)
{
	static const TCHAR*	pszSHIFT = _T("Shift+");
	static const TCHAR*	pszCTRL  = _T("Ctrl+");
	static const TCHAR*	pszALT   = _T("Alt+");

	int i;
	for (i=nKeyNameArrBegin; i<nKeyNameArrEnd; ++i) {
		auto& keyName = pKeyNameArr[i];
		if (nFuncId == GetFuncCodeAt(keyName, nShiftState, bGetDefFuncCode)) {
			if (nShiftState & _SHIFT) {
				memList.AppendString(pszSHIFT);
			}
			if (nShiftState & _CTRL) {
				memList.AppendString(pszCTRL);
			}
			if (nShiftState & _ALT) {
				memList.AppendString(pszALT);
			}
			memList.AppendString(keyName.szKeyName);
			nKeyNameArrBegin = i + 1;
			return true;
		}
	}
	nKeyNameArrBegin = i;
	return false;
}


/** 機能に対応するキー名の取得 */
int KeyBind::GetKeyStr(
	HINSTANCE	hInstance,
	int			nKeyNameArrNum,
	KeyData*	pKeyNameArr,
	NativeT&	memList,
	int			nFuncId,
	bool		bGetDefFuncCode // = true
	)
{
	memList.SetString(_T(""));

	// 先にキー部分を調査する
	for (int j=0; j<8; ++j) {
		for (int i=(int)MouseFunctionType::KeyBegin; i<nKeyNameArrNum; /* 1を加えてはいけない */) {
			if (GetKeyStrSub(i, nKeyNameArrNum, pKeyNameArr, j, memList, nFuncId, bGetDefFuncCode)) {
				return 1;
			}
		}
	}

	// 後にマウス部分を調査する
	for (int j=0; j<8; ++j) {
		for (int i=0; i<(int)MouseFunctionType::KeyBegin; /* 1を加えてはいけない */) {
			if (GetKeyStrSub(i, nKeyNameArrNum, pKeyNameArr, j, memList, nFuncId, bGetDefFuncCode)) {
				return 1;
			}
		}
	}
	return 0;
}


/** 機能に対応するキー名の取得(複数) */
int KeyBind::GetKeyStrList(
	HINSTANCE	hInstance,
	int			nKeyNameArrNum,
	KeyData*	pKeyNameArr,
	NativeT***	pppMemList,
	int			nFuncId,
	bool		bGetDefFuncCode // = true
	)
{
	int nAssignedKeysNum = 0;
	if (nFuncId == 0) {
		return 0;
	}
	for (int j=0; j<8; ++j) {
		for (int i=0; i<nKeyNameArrNum; ++i) {
			if (nFuncId == GetFuncCodeAt(pKeyNameArr[i], j, bGetDefFuncCode)) {
				++nAssignedKeysNum;
			}
		}
	}
	if (nAssignedKeysNum == 0) {
		return 0;
	}
	(*pppMemList) = new NativeT*[nAssignedKeysNum + 1];
	int i;
	for (i=0; i<nAssignedKeysNum; ++i) {
		(*pppMemList)[i] = new NativeT;
	}
	(*pppMemList)[i] = NULL;
	
	nAssignedKeysNum = 0;
	for (int j=0; j<8; ++j) {
		for (int i=0; i<nKeyNameArrNum; /* 1を加えてはいけない */) {
			if (GetKeyStrSub(
				i,
				nKeyNameArrNum,
				pKeyNameArr,
				j,
				*((*pppMemList)[nAssignedKeysNum]),
				nFuncId,
				bGetDefFuncCode
				)
			) {
				++nAssignedKeysNum;
			}
		}
	}
	return nAssignedKeysNum;
}


/*! アクセスキー付きの文字列の作成
	@param name ラベル
	@param sKey アクセスキー
	@return アクセスキー付きの文字列
*/
TCHAR* KeyBind::MakeMenuLabel(const TCHAR* sName, const TCHAR* sKey)
{
	const int MAX_LABEL_CCH = _MAX_PATH * 2 + 30;
	static TCHAR sLabel[MAX_LABEL_CCH];
	const TCHAR* p;

	if (!sKey || sKey[0] == L'\0') {
		return const_cast<TCHAR*>(sName);
	}else {
		if (!GetDllShareData().common.mainMenu.bMainMenuKeyParentheses
			&& (
				((p = auto_strchr(sName, sKey[0])))
				|| ((p = auto_strchr(sName, _totlower(sKey[0]))))
			)
		) {
			// 欧文風、使用している文字をアクセスキーに
			auto_strcpy_s(sLabel, _countof(sLabel), sName);
			sLabel[p-sName] = _T('&');
			auto_strcpy_s(sLabel + (p-sName) + 1, _countof(sLabel), p);
		}else if (
			(p = auto_strchr(sName, _T('(')))
			&& (p = auto_strchr(p, sKey[0]))
		) {
			// (付その後にアクセスキー
			auto_strcpy_s(sLabel, _countof(sLabel), sName);
			sLabel[p-sName] = _T('&');
			auto_strcpy_s(sLabel + (p-sName) + 1, _countof(sLabel), p);
		}else if (_tcscmp(sName + _tcslen(sName) - 3, _T("...")) == 0) {
			// 末尾...
			auto_strcpy_s(sLabel, _countof(sLabel), sName);
			sLabel[_tcslen(sName) - 3] = '\0';						// 末尾の...を取る
			auto_strcat_s(sLabel, _countof(sLabel), _T("(&"));
			auto_strcat_s(sLabel, _countof(sLabel), sKey);
			auto_strcat_s(sLabel, _countof(sLabel), _T(")..."));
		}else {
			auto_sprintf_s(sLabel, _countof(sLabel), _T("%ts(&%ts)"), sName, sKey);
		}
		return sLabel;
	}
}

/*! メニューラベルの作成 */
TCHAR* KeyBind::GetMenuLabel(
	HINSTANCE		hInstance,
	int				nKeyNameArrNum,
	KeyData*		pKeyNameArr,
	int				nFuncId,
	TCHAR*      	pszLabel,   // [in,out] バッファは256以上と仮定
	const TCHAR*	pszKey,
	bool			bKeyStr,
	int				nLabelSize,
	bool			bGetDefFuncCode // = true
	)
{
	const unsigned int LABEL_MAX = nLabelSize;
	if (pszLabel[0] == _T('\0')) {
		_tcsncpy(pszLabel, LS(nFuncId), LABEL_MAX - 1);
		pszLabel[LABEL_MAX - 1] = _T('\0');
	}
	if (pszLabel[0] == _T('\0')) {
		_tcscpy(pszLabel, _T("-- undefined name --"));
	}
	// アクセスキーの追加	2010/5/17 Uchi
	_tcsncpy_s(pszLabel, LABEL_MAX, MakeMenuLabel(pszLabel, pszKey), _TRUNCATE);

	// 機能に対応するキー名を追加するか
	if (bKeyStr) {
		NativeT memAccessKey;
		// [ファイル/フォルダ/ウィンドウ一覧以外]から[アクセスキーがあるときのみ]に付加するように変更
		// 機能に対応するキー名の取得
		if (GetKeyStr(hInstance, nKeyNameArrNum, pKeyNameArr, memAccessKey, nFuncId, bGetDefFuncCode)) {
			// バッファが足りないときは入れない
			if (_tcslen(pszLabel) + memAccessKey.GetStringLength() + 1 < LABEL_MAX) {
				_tcscat(pszLabel, _T("\t"));
				_tcscat(pszLabel, memAccessKey.GetStringPtr());
			}
		}
	}
	return pszLabel;
}


/*! キーのデフォルト機能を取得する

	@param nKeyCode [in] キーコード
	@param nState [in] Shift,Ctrl,Altキー状態

	@return 機能番号
*/
EFunctionCode KeyBind::GetDefFuncCode(int nKeyCode, int nState)
{
	auto& csTabBar = GetDllShareData().common.tabBar;
	EFunctionCode nDefFuncCode = F_DEFAULT;
	if (nKeyCode == VK_F4) {
		if (nState == _CTRL) {
			nDefFuncCode = F_FILECLOSE;	// 閉じて(無題)
			if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
				nDefFuncCode = F_WINCLOSE;	// 閉じる
			}
		}else if (nState == _ALT) {
			nDefFuncCode = F_WINCLOSE;	// 閉じる
			if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
				if (!csTabBar.bTab_CloseOneWin) {
					nDefFuncCode = F_GROUPCLOSE;	// グループを閉じる
				}
			}
		}
	}
	return nDefFuncCode;
}


/*! 特定のキー情報から機能コードを取得する

	@param keyData [in] キー情報
	@param nState [in] Shift,Ctrl,Altキー状態
	@param bGetDefFuncCode [in] デフォルト機能を取得するかどうか

	@return 機能番号
*/
EFunctionCode KeyBind::GetFuncCodeAt(KeyData& keyData, int nState, bool bGetDefFuncCode)
{
	if (keyData.nFuncCodeArr[nState] != 0) {
		return keyData.nFuncCodeArr[nState];
	}
	if (bGetDefFuncCode) {
		return GetDefFuncCode(keyData.nKeyCode, nState);
	}
	return F_DEFAULT;
}


#define _SQL_RUN	F_PLSQL_COMPILE_ON_SQLPLUS
#define _COPYWITHLINENUM	F_COPYLINESWITHLINENUMBER
	static const KEYDATAINIT	KeyDataInit[] = {
	// note: key binding
	// note 2: 順番は2進で下位3ビット[Alt][Ctrl][Shift]の組合せの順(それに2を加えた値)
	//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)

		// マウスボタン
		//keycode,			keyname,							なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VKEX_DBL_CLICK,	(LPCTSTR)STR_KEY_BIND_DBL_CLICK,	{ F_SELECTWORD,		F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD }, }, //Feb. 19, 2001 JEPRO Altと右クリックの組合せは効かないので右クリックメニューのキー割り当てをはずした
		{ VKEX_R_CLICK,		(LPCTSTR)STR_KEY_BIND_R_CLICK,		{ F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_0,					F_0,				F_0,					F_0 }, },
		{ VKEX_MDL_CLICK,	(LPCTSTR)STR_KEY_BIND_MID_CLICK,	{ F_AUTOSCROLL,		F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, }, // novice 2004/10/11 マウス中ボタン対応
		{ VKEX_LSD_CLICK,	(LPCTSTR)STR_KEY_BIND_LSD_CLICK,	{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, }, // novice 2004/10/10 マウスサイドボタン対応
		{ VKEX_RSD_CLICK,	(LPCTSTR)STR_KEY_BIND_RSD_CLICK,	{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VKEX_TRI_CLICK,	(LPCTSTR)STR_KEY_BIND_TRI_CLICK,	{ F_SELECTLINE,		F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE }, },
		{ VKEX_QUA_CLICK,	(LPCTSTR)STR_KEY_BIND_QUA_CLICK,	{ F_SELECTALL,		F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL }, },
		{ VKEX_WHEEL_UP,	(LPCTSTR)STR_KEY_BIND_WHEEL_UP,		{ F_WHEELUP,		F_WHEELUP,			F_SETFONTSIZEUP,		F_WHEELUP,			F_WHEELUP,				F_WHEELUP,			F_WHEELUP,				F_WHEELUP }, },
		{ VKEX_WHEEL_DOWN,	(LPCTSTR)STR_KEY_BIND_WHEEL_DOWN,	{ F_WHEELDOWN,		F_WHEELDOWN,		F_SETFONTSIZEDOWN,		F_WHEELDOWN,		F_WHEELDOWN,			F_WHEELDOWN,		F_WHEELDOWN,			F_WHEELDOWN }, },
		{ VKEX_WHEEL_LEFT,	(LPCTSTR)STR_KEY_BIND_WHEEL_LEFT,	{ F_WHEELLEFT,		F_WHEELLEFT,		F_WHEELLEFT,			F_WHEELLEFT,		F_WHEELLEFT,			F_WHEELLEFT,		F_WHEELLEFT,			F_WHEELLEFT }, },
		{ VKEX_WHEEL_RIGHT,	(LPCTSTR)STR_KEY_BIND_WHEEL_RIGHT,	{ F_WHEELRIGHT,		F_WHEELRIGHT,		F_WHEELRIGHT,			F_WHEELRIGHT,		F_WHEELRIGHT,			F_WHEELRIGHT,		F_WHEELRIGHT,			F_WHEELRIGHT }, },

		// ファンクションキー
		//keycode,	keyname,			なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_F1,	_T("F1"),			{ F_EXTHTMLHELP,	F_MENU_ALLFUNC,		F_EXTHELP1,				F_ABOUT,			F_HELP_CONTENTS,		F_HELP_SEARCH,		F_0,					F_0 }, },
		{ VK_F2,	_T("F2"),			{ F_BOOKMARK_NEXT,	F_BOOKMARK_PREV,	F_BOOKMARK_SET,			F_BOOKMARK_RESET,	F_BOOKMARK_VIEW,		F_0,				F_0,					F_0 }, },
		{ VK_F3,	_T("F3"),			{ F_SEARCH_NEXT,	F_SEARCH_PREV,		F_SEARCH_CLEARMARK,		F_JUMP_SRCHSTARTPOS,F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F4,	_T("F4"),			{ F_SPLIT_V,		F_SPLIT_H,			F_0,					F_FILECLOSE_OPEN,	F_0,					F_EXITALLEDITORS,	F_EXITALL,				F_0 }, },
		{ VK_F5,	_T("F5"),			{ F_REDRAW,			F_0,				F_EXECMD_DIALOG,		F_0,				F_UUDECODE,				F_0,				F_TABTOSPACE,			F_SPACETOTAB }, },
		{ VK_F6,	_T("F6"),			{ F_BEGIN_SEL,		F_BEGIN_BOX,		F_TOLOWER,				F_0,				F_BASE64DECODE,			F_0,				F_0,					F_0 }, },
		{ VK_F7,	_T("F7"),			{ F_CUT,			F_0,				F_TOUPPER,				F_0,				F_CODECNV_UTF72SJIS,	F_CODECNV_SJIS2UTF7,F_FILE_REOPEN_UTF7,		F_0 }, },
		{ VK_F8,	_T("F8"),			{ F_COPY,			F_COPY_CRLF,		F_TOHANKAKU,			F_0,				F_CODECNV_UTF82SJIS,	F_CODECNV_SJIS2UTF8,F_FILE_REOPEN_UTF8,		F_0 }, },
		{ VK_F9,	_T("F9"),			{ F_PASTE,			F_PASTEBOX,			F_TOZENKAKUKATA,		F_0,				F_CODECNV_UNICODE2SJIS,	F_0,				F_FILE_REOPEN_UNICODE,	F_0 }, },
		{ VK_F10,	_T("F10"),			{ _SQL_RUN,			F_DUPLICATELINE,	F_TOZENKAKUHIRA,		F_0,				F_CODECNV_EUC2SJIS,		F_CODECNV_SJIS2EUC,	F_FILE_REOPEN_EUC,		F_0 }, },
		{ VK_F11,	_T("F11"),			{ F_OUTLINE,		F_ACTIVATE_SQLPLUS,	F_HANKATATOZENKATA,		F_0,				F_CODECNV_EMAIL,		F_CODECNV_SJIS2JIS,	F_FILE_REOPEN_JIS,		F_0 }, },
		{ VK_F12,	_T("F12"),			{ F_TAGJUMP,		F_TAGJUMPBACK,		F_HANKATATOZENHIRA,		F_0,				F_CODECNV_AUTO2SJIS,	F_0,				F_FILE_REOPEN_SJIS,		F_0 }, },
		{ VK_F13,	_T("F13"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F14,	_T("F14"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F15,	_T("F15"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F16,	_T("F16"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F17,	_T("F17"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F18,	_T("F18"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F19,	_T("F19"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F20,	_T("F20"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F21,	_T("F21"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F22,	_T("F22"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F23,	_T("F23"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F24,	_T("F24"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },

		// 特殊キー
		//keycode,	keyname,			なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_TAB,	_T("Tab"),			{ F_INDENT_TAB,		F_UNINDENT_TAB,		F_NEXTWINDOW,			F_PREVWINDOW,		F_0,					F_0,				F_0,					F_0 }, },
		{ VK_RETURN,_T("Enter"),		{ F_0,				F_0,				F_COMPARE,				F_0,				F_PROPERTY_FILE,		F_0,				F_0,					F_0 }, },
		{ VK_ESCAPE,_T("Esc"),			{ F_CANCEL_MODE,	F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_BACK,	_T("BkSp"),			{ F_DELETE_BACK,	F_0,				F_WordDeleteToStart,	F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_INSERT,_T("Ins"),			{ F_CHGMOD_INS,		F_PASTE,			F_COPY,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_DELETE,_T("Del"),			{ F_DELETE,			F_CUT,				F_WordDeleteToEnd,		F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_HOME,	_T("Home"),			{ F_GOLINETOP,		F_GOLINETOP_SEL,	F_GOFILETOP,			F_GOFILETOP_SEL,	F_GOLINETOP_BOX,		F_0,				F_GOFILETOP_BOX,		F_0 }, },
		{ VK_END,	_T("End(Help)"),	{ F_GOLINEEND,		F_GOLINEEND_SEL,	F_GOFILEEND,			F_GOFILEEND_SEL,	F_GOLINEEND_BOX,		F_0,				F_GOFILEEND_BOX,		F_0 }, },
		{ VK_LEFT,	_T("←"),			{ F_LEFT,			F_LEFT_SEL,			F_WORDLEFT,				F_WORDLEFT_SEL,		F_LEFT_BOX,				F_0,				F_WORDLEFT_BOX,			F_0 }, },
		{ VK_UP,	_T("↑"),			{ F_UP,				F_UP_SEL,			F_WndScrollDown,		F_UP2_SEL,			F_UP_BOX,				F_0,				F_UP2_BOX,				F_MAXIMIZE_V },}, 
		{ VK_RIGHT,	_T("→"),			{ F_RIGHT,			F_RIGHT_SEL,		F_WORDRIGHT,			F_WORDRIGHT_SEL,	F_RIGHT_BOX,			F_0,				F_WORDRIGHT_BOX,		F_MAXIMIZE_H },}, 
		{ VK_DOWN,	_T("↓"),			{ F_DOWN,			F_DOWN_SEL,			F_WndScrollUp,			F_DOWN2_SEL,		F_DOWN_BOX,				F_0,				F_DOWN2_BOX,			F_MINIMIZE_ALL },}, 
		{ VK_NEXT,	_T("PgDn(RollUp)"),	{ F_1PageDown,		F_1PageDown_Sel,	F_HalfPageDown,			F_HalfPageDown_Sel,	F_1PageDown_BOX,		F_0,				F_HalfPageDown_BOX,		F_0 }, },
		{ VK_PRIOR,	_T("PgUp(RollDn)"),	{ F_1PageUp,		F_1PageUp_Sel,		F_HalfPageUp,			F_HalfPageUp_Sel,	F_1PageUp_BOX,			F_0,				F_HalfPageUp_BOX,		F_0 }, },
		{ VK_SPACE,	_T("Space"),		{ F_INDENT_SPACE,	F_UNINDENT_SPACE,	F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 }, },

		// 数字
		//keycode,	keyname,			なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ '0',		_T("0"),			{ F_0,				F_0,				F_0,					F_0,				F_CUSTMENU_10,			F_CUSTMENU_20,		F_0,					F_0 }, },
		{ '1',		_T("1"),			{ F_0,				F_0,				F_SHOWTOOLBAR,			F_CUSTMENU_21,		F_CUSTMENU_1,			F_CUSTMENU_11,		F_0,					F_0 }, },
		{ '2',		_T("2"),			{ F_0,				F_0,				F_SHOWFUNCKEY,			F_CUSTMENU_22,		F_CUSTMENU_2,			F_CUSTMENU_12,		F_0,					F_0 }, },
		{ '3',		_T("3"),			{ F_0,				F_0,				F_SHOWSTATUSBAR,		F_CUSTMENU_23,		F_CUSTMENU_3,			F_CUSTMENU_13,		F_0,					F_0 }, },
		{ '4',		_T("4"),			{ F_0,				F_0,				F_TYPE_LIST,			F_CUSTMENU_24,		F_CUSTMENU_4,			F_CUSTMENU_14,		F_0,					F_0 }, },
		{ '5',		_T("5"),			{ F_0,				F_0,				F_OPTION_TYPE,			F_0,				F_CUSTMENU_5,			F_CUSTMENU_15,		F_0,					F_0 }, },
		{ '6',		_T("6"),			{ F_0,				F_0,				F_OPTION,				F_0,				F_CUSTMENU_6,			F_CUSTMENU_16,		F_0,					F_0 }, },
		{ '7',		_T("7"),			{ F_0,				F_0,				F_FONT,					F_0,				F_CUSTMENU_7,			F_CUSTMENU_17,		F_0,					F_0 }, },
		{ '8',		_T("8"),			{ F_0,				F_0,				F_0,					F_0,				F_CUSTMENU_8,			F_CUSTMENU_18,		F_0,					F_0 }, },
		{ '9',		_T("9"),			{ F_0,				F_0,				F_0,					F_0,				F_CUSTMENU_9,			F_CUSTMENU_19,		F_0,					F_0 }, },

		// アルファベット
		//keycode,	keyname,			なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 'A',		_T("A"),			{ F_0,				F_0,				F_SELECTALL,			F_0,				F_SORT_ASC,				F_0,				F_0,					F_0 }, },
		{ 'B',		_T("B"),			{ F_0,				F_0,				F_BROWSE,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'C',		_T("C"),			{ F_0,				F_0,				F_COPY,					F_OPEN_HfromtoC,	F_0,					F_0,				F_0,					F_0 }, },
		{ 'D',		_T("D"),			{ F_0,				F_0,				F_WordCut,				F_WordDelete,		F_SORT_DESC,			F_0,				F_0,					F_0 }, },
		{ 'E',		_T("E"),			{ F_0,				F_0,				F_CUT_LINE,				F_DELETE_LINE,		F_0,					F_0,				F_CASCADE,				F_0 }, },
		{ 'F',		_T("F"),			{ F_0,				F_0,				F_SEARCH_DIALOG,		F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'G',		_T("G"),			{ F_0,				F_0,				F_GREP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'H',		_T("H"),			{ F_0,				F_0,				F_CURLINECENTER,		F_OPEN_HfromtoC,	F_0,					F_0,				F_TILE_V,				F_0 }, },
		{ 'I',		_T("I"),			{ F_0,				F_0,				F_DUPLICATELINE,		F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'J',		_T("J"),			{ F_0,				F_0,				F_JUMP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'K',		_T("K"),			{ F_0,				F_0,				F_LineCutToEnd,			F_LineDeleteToEnd,	F_0,					F_0,				F_0,					F_0 }, },
		{ 'L',		_T("L"),			{ F_0,				F_0,				F_LOADKEYMACRO,			F_EXECKEYMACRO,		F_LTRIM,				F_0,				F_TOLOWER,				F_TOUPPER }, },
		{ 'M',		_T("M"),			{ F_0,				F_0,				F_SAVEKEYMACRO,			F_RECKEYMACRO,		F_MERGE,				F_0,				F_0,					F_0 }, },
		{ 'N',		_T("N"),			{ F_0,				F_0,				F_FILENEW,				F_0,				F_JUMPHIST_NEXT,		F_0,				F_0,					F_0 }, },
		{ 'O',		_T("O"),			{ F_0,				F_0,				F_FILEOPEN,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'P',		_T("P"),			{ F_0,				F_0,				F_PRINT,				F_PRINT_PREVIEW,	F_JUMPHIST_PREV,		F_0,				F_PRINT_PAGESETUP,		F_0 }, },
		{ 'Q',		_T("Q"),			{ F_0,				F_0,				F_CREATEKEYBINDLIST,	F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'R',		_T("R"),			{ F_0,				F_0,				F_REPLACE_DIALOG,		F_0,				F_RTRIM,				F_0,				F_0,					F_0 }, },
		{ 'S',		_T("S"),			{ F_0,				F_0,				F_FILESAVE,				F_FILESAVEAS_DIALOG,F_0,					F_0,				F_TMPWRAPSETTING,		F_0 }, },
		{ 'T',		_T("T"),			{ F_0,				F_0,				F_TAGJUMP,				F_TAGJUMPBACK,		F_0,					F_0,				F_TILE_H,				F_0 }, },
		{ 'U',		_T("U"),			{ F_0,				F_0,				F_LineCutToStart,		F_LineDeleteToStart,F_0,					F_0,				F_WRAPWINDOWWIDTH,		F_0 }, },
		{ 'V',		_T("V"),			{ F_0,				F_0,				F_PASTE,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'W',		_T("W"),			{ F_0,				F_0,				F_SELECTWORD,			F_0,				F_0,					F_0,				F_TMPWRAPWINDOW,		F_0 }, },
		{ 'X',		_T("X"),			{ F_0,				F_0,				F_CUT,					F_0,				F_0,					F_0,				F_TMPWRAPNOWRAP,		F_0 }, },
		{ 'Y',		_T("Y"),			{ F_0,				F_0,				F_REDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'Z',		_T("Z"),			{ F_0,				F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },

		// 記号
		//keycode,	keyname,			なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 0x00bd,	_T("-"),			{ F_0,				F_0,				F_COPYFNAME,			F_SPLIT_V,			F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00de,	(LPCTSTR)STR_KEY_BIND_HAT_ENG_QT,		{ F_0,				F_0,				F_COPYTAG,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00dc,	_T("\\"),			{ F_0,				F_0,				F_COPYPATH,				F_SPLIT_H,			F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00c0,	(LPCTSTR)STR_KEY_BIND_AT_ENG_BQ,		{ F_0,				F_0,				F_COPYLINES,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00db,	_T("["),			{ F_0,				F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00bb,	_T(";"),			{ F_0,				F_0,				F_0,					F_SPLIT_VH,			F_INS_DATE,				F_0,				F_0,					F_0 }, },
		{ 0x00ba,	_T(":"),			{ F_0,				F_0,				_COPYWITHLINENUM,		F_0,				F_INS_TIME,				F_0,				F_0,					F_0 }, },
		{ 0x00dd,	_T("]"),			{ F_0,				F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00bc,	_T(","),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00be,	_T("."),			{ F_0,				F_0,				F_COPYLINESASPASSAGE,	F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00bf,	_T("/"),			{ F_0,				F_0,				F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00e2,	_T("_"),			{ F_0,				F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00df,	_T("_(PC-98)"),		{ F_0,				F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_APPS,	(LPCTSTR)STR_KEY_BIND_APLI,	{ F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON }, }
	};

const TCHAR* jpVKEXNames[] = {
	_T("ダブルクリック"),
	_T("右クリック"),
	_T("中クリック"),
	_T("左サイドクリック"),
	_T("右サイドクリック"),
	_T("トリプルクリック"),
	_T("クアドラプルクリック"),
	_T("ホイールアップ"),
	_T("ホイールダウン"),
	_T("ホイール左"),
	_T("ホイール右")
};
const int jpVKEXNamesLen = _countof( jpVKEXNames );

/*!	@brief 共有メモリ初期化/キー割り当て

	デフォルトキー割り当て関連の初期化処理
*/
bool ShareData::InitKeyAssign(DllSharedData& shareData)
{
	/********************/
	/* 共通設定の規定値 */
	/********************/
	const int nKeyDataInitNum = _countof(KeyDataInit);
	const int KEYNAME_SIZE = _countof(shareData.common.keyBind.pKeyNameArr) -1;// 最後の１要素はダミー用に予約
	assert(!(nKeyDataInitNum > KEYNAME_SIZE));
//	if (nKeyDataInitNum > KEYNAME_SIZE) {
//		PleaseReportToAuthor(NULL, _T("キー設定数に対してDLLSHARE::nKeyNameArr[]のサイズが不足しています"));
//		return false;
//	}

	// マウスコードの固定と重複排除
	static const KEYDATAINIT dummy[] = {
		{ 0,		_T(""),				{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 } }
	};

	// インデックス用ダミー作成
	SetKeyNameArrVal(shareData, KEYNAME_SIZE, &dummy[0]);
	// インデックス作成 重複した場合は先頭にあるものを優先
	for (size_t ii=0; ii<_countof(shareData.common.keyBind.keyToKeyNameArr); ++ii) {
		shareData.common.keyBind.keyToKeyNameArr[ii] = KEYNAME_SIZE;
	}
	for (int i=nKeyDataInitNum-1; i>=0; --i) {
		shareData.common.keyBind.keyToKeyNameArr[KeyDataInit[i].nKeyCode] = (BYTE)i;
	}
	for (int i=0; i<nKeyDataInitNum; ++i) {
		SetKeyNameArrVal(shareData, i, &KeyDataInit[i]);
	}
	shareData.common.keyBind.nKeyNameArrNum = nKeyDataInitNum;
	return true;
}

//	@brief 言語選択後の文字列更新処理
void ShareData::RefreshKeyAssignString(DllSharedData& shareData)
{
	const int nKeyDataInitNum = _countof(KeyDataInit);
	for (int i=0; i<nKeyDataInitNum; ++i) {
		KeyData* pKeydata = &shareData.common.keyBind.pKeyNameArr[i];
		if (KeyDataInit[i].nKeyNameId <= 0xFFFF) {
			_tcscpy(pKeydata->szKeyName, LS(KeyDataInit[i].nKeyNameId));
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// KeyData配列にデータをセット
static void SetKeyNameArrVal(
	DllSharedData&		shareData,
	int					nIdx,
	const KEYDATAINIT*	pKeydataInit
	)
{
	KeyData* pKeydata = &shareData.common.keyBind.pKeyNameArr[nIdx];
	pKeydata->nKeyCode = pKeydataInit->nKeyCode;
	if (0xFFFF < pKeydataInit->nKeyNameId) {
		_tcscpy(pKeydata->szKeyName, pKeydataInit->pszKeyName);
	}
	assert( sizeof(pKeydata->nFuncCodeArr) == sizeof(pKeydataInit->nFuncCodeArr) );
	memcpy_raw(pKeydata->nFuncCodeArr, pKeydataInit->nFuncCodeArr, sizeof(pKeydataInit->nFuncCodeArr));
}

