/*
	Copyright (C) 2008, kobake

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

#include "StdAfx.h"
#include "Type.h"
#include "view/Colors/EColorIndexType.h"
#include "env/DocTypeManager.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"

void _DefaultConfig(TypeConfig* pType);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          Type                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void Type::InitTypeConfig(int nIdx, TypeConfig& type)
{
	// 規定値をコピー
	static TypeConfig sDefault;
	static bool bLoadedDefault = false;
	if (!bLoadedDefault) {
		_DefaultConfig(&sDefault);
		bLoadedDefault=true;
	}
	type = sDefault;
	
	// インデックスを設定
	type.nIdx = nIdx;
	type.id = nIdx;
	
	// 個別設定
	InitTypeConfigImp(type);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ShareData                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief 共有メモリ初期化/タイプ別設定

	タイプ別設定の初期化処理

	@date 2005.01.30 genta ShareData::Init()から分離．
*/
void ShareData::InitTypeConfigs(
	DllSharedData* pShareData,
	std::vector<TypeConfig*>& types
	)
{
	Type* table[] = {
		new CType_Basis(),	// 基本
		new CType_Text(),	// テキスト
		new CType_Cpp(),	// C/C++
		new CType_Html(),	// HTML
		new CType_Sql(),	// PL/SQL
		new CType_Cobol(),	// COBOL
		new CType_Java(),	// Java
		new CType_Asm(),	// アセンブラ
		new CType_Awk(),	// awk
		new CType_Dos(),	// MS-DOSバッチファイル
		new CType_Pascal(),	// Pascal
		new CType_Tex(),	// TeX
		new CType_Perl(),	// Perl
		new CType_Vb(),		// Visual Basic
		new CType_Rich(),	// リッチテキスト
		new CType_Ini(),	// 設定ファイル
	};
	types.clear();
	assert(_countof(table) <= MAX_TYPES);
	for (int i=0; i<_countof(table) && i<MAX_TYPES; ++i) {
		TypeConfig* type = new TypeConfig;
		types.push_back(type);
		table[i]->InitTypeConfig(i, *type);
		auto& typeMini = pShareData->typesMini[i];
		auto_strcpy(typeMini.szTypeExts, type->szTypeExts);
		auto_strcpy(typeMini.szTypeName, type->szTypeName);
		typeMini.encoding = type->encoding;
		typeMini.id = type->id;
		SAFE_DELETE(table[i]);
	}
	pShareData->typeBasis = *types[0];
	pShareData->nTypesCount = (int)types.size();
}

/*!	@brief 共有メモリ初期化/強調キーワード

	強調キーワード関連の初期化処理

	@date 2005.01.30 genta ShareData::Init()から分離．
		キーワード定義を関数の外に出し，登録をマクロ化して簡潔に．
*/
void ShareData::InitKeyword(DllSharedData* pShareData)
{
	// 強調キーワードのテストデータ
	pShareData->common.specialKeyword.keywordSetMgr.m_nCurrentKeywordSetIdx = 0;

	int nSetCount = -1;

#define PopulateKeyword(name, case_sensitive, aryname) \
	extern const wchar_t* g_ppszKeywords##aryname[]; \
	extern int g_nKeywords##aryname; \
	pShareData->common.specialKeyword.keywordSetMgr.AddKeywordSet((name), (case_sensitive));	\
	pShareData->common.specialKeyword.keywordSetMgr.SetKeywordArr(++nSetCount, g_nKeywords##aryname, g_ppszKeywords##aryname);
	
	PopulateKeyword(L"C/C++",			true,	CPP);			// セット 0の追加
	PopulateKeyword(L"HTML",			false,	HTML);			// セット 1の追加
	PopulateKeyword(L"PL/SQL",			false,	PLSQL);			// セット 2の追加
	PopulateKeyword(L"COBOL",			true,	COBOL);			// セット 3の追加
	PopulateKeyword(L"Java",			true,	JAVA);			// セット 4の追加
	PopulateKeyword(L"CORBA IDL",		true,	CORBA_IDL);		// セット 5の追加
	PopulateKeyword(L"AWK"	,			true,	AWK);			// セット 6の追加
	PopulateKeyword(L"MS-DOS batch",	false,	BAT);			// セット 7の追加	// Oct. 31, 2000 JEPRO 'バッチファイル'→'batch' に短縮
	PopulateKeyword(L"Pascal",			false,	PASCAL);		// セット 8の追加	// Nov. 5, 2000 JEPRO 大・小文字の区別を'しない'に変更
	PopulateKeyword(L"TeX",				true,	TEX);			// セット 9の追加	// Sept. 2, 2000 jepro Tex →TeX に修正 Bool値は大・小文字の区別
	PopulateKeyword(L"TeX2",			true,	TEX2);			// セット10の追加	// Jan. 19, 2001 JEPRO 追加
	PopulateKeyword(L"Perl",			true,	PERL);			// セット11の追加
	PopulateKeyword(L"Perl2",			true,	PERL2);			// セット12の追加	// Jul. 10, 2001 JEPRO Perlから変数を分離・独立
	PopulateKeyword(L"Visual Basic",	false,	VB);			// セット13の追加	// Jul. 10, 2001 JEPRO
	PopulateKeyword(L"Visual Basic2",	false,	VB2);			// セット14の追加	// Jul. 10, 2001 JEPRO
	PopulateKeyword(L"Rich Text",		true,	RTF);			// セット15の追加	// Jul. 10, 2001 JEPRO

#undef PopulateKeyword
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        デフォルト                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void _DefaultConfig(TypeConfig* pType)
{
// キーワード：デフォルトカラー設定
/************************/
/* タイプ別設定の規定値 */
/************************/

	pType->nTextWrapMethod = TextWrappingMethod::SettingWidth;		// テキストの折り返し方法		// 2008.05.30 nasukoji
	pType->nMaxLineKetas = LayoutInt(MAXLINEKETAS);	// 折り返し桁数
	pType->nColumnSpace = 0;							// 文字と文字の隙間
	pType->nLineSpace = 1;							// 行間のすきま
	pType->nTabSpace = LayoutInt(4);					// TABの文字数
	for (int i=0; i<MAX_KEYWORDSET_PER_TYPE; ++i) {
		pType->nKeywordSetIdx[i] = -1;
	}
	wcscpy_s(pType->szTabViewString, _EDITL("^       "));	// TAB表示文字列
	pType->bTabArrow = TabArrowType::String;	// タブ矢印表示	// 2001.12.03 hor	// default on 2013/4/11 Uchi
	pType->bInsSpace = false;				// スペースの挿入	// 2001.12.03 hor
	
	//@@@ 2002.09.22 YAZAKI 以下、lineCommentとblockCommentsを使うように修正
	pType->lineComment.CopyTo(0, L"", -1);	// 行コメントデリミタ
	pType->lineComment.CopyTo(1, L"", -1);	// 行コメントデリミタ2
	pType->lineComment.CopyTo(2, L"", -1);	// 行コメントデリミタ3	//Jun. 01, 2001 JEPRO 追加
	pType->blockComments[0].SetBlockCommentRule(L"", L"");	// ブロックコメントデリミタ
	pType->blockComments[1].SetBlockCommentRule(L"", L"");	// ブロックコメントデリミタ2

	pType->stringType = StringLiteralType::CPP;					// 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""]['']
	pType->bStringLineOnly = false;
	pType->bStringEndLine  = false;
	pType->nHeredocType = HereDocType::PHP;
	pType->szIndentChars[0] = 0;		// その他のインデント対象文字

	pType->nColorInfoArrNum = COLORIDX_LAST;

	// 2001/06/14 Start by asa-o
	pType->szHokanFile[0] = 0;		// 入力補完 単語ファイル
	// 2001/06/14 End

	pType->nHokanType = 0;

	// 2001/06/19 asa-o
	pType->bHokanLoHiCase = false;			// 入力補完機能：英大文字小文字を同一視する

	//	2003.06.23 Moca ファイル内からの入力補完機能
	pType->bUseHokanByFile = true;			//! 入力補完 開いているファイル内から候補を探す
	pType->bUseHokanByKeyword = false;		// 強調キーワードから入力補完

	// 文字コード設定
	auto& encoding = pType->encoding;
	encoding.bPriorCesu8 = false;
	encoding.eDefaultCodetype = CODE_SJIS;
	encoding.eDefaultEoltype = EolType::CRLF;
	encoding.bDefaultBom = false;

	//@@@2002.2.4 YAZAKI
	pType->szExtHelp[0] = L'\0';
	pType->szExtHtmlHelp[0] = L'\0';
	pType->bHtmlHelpIsSingle = true;

	pType->bAutoIndent = true;			// オートインデント
	pType->bAutoIndent_ZENSPACE = true;	// 日本語空白もインデント
	pType->bRTrimPrevLine = false;		// 2005.10.11 ryoji 改行時に末尾の空白を削除

	pType->nIndentLayout = 0;	// 折り返しは2行目以降を字下げ表示

	assert(COLORIDX_LAST <= _countof(pType->colorInfoArr));
	for (int i=0; i<COLORIDX_LAST; ++i) {
		GetDefaultColorInfo(&pType->colorInfoArr[i], i);
	}
	pType->szBackImgPath[0] = '\0';
	pType->backImgPos = BackgroundImagePosType::TopLeft;
	pType->backImgRepeatX = true;
	pType->backImgRepeatY = true;
	pType->backImgScrollX = true;
	pType->backImgScrollY = true;
	{
		POINT pt ={0, 0};
		pType->backImgPosOffset = pt;
	}
	pType->bLineNumIsCRLF = true;					// 行番号の表示 false=折り返し単位／true=改行単位
	pType->nLineTermType = 1;						// 行番号区切り 0=なし 1=縦線 2=任意
	pType->cLineTermChar = L':';					// 行番号区切り文字
	pType->bWordWrap = false;						// 英文ワードラップをする
	pType->nCurrentPrintSetting = 0;				// 現在選択している印刷設定
	pType->bOutlineDockDisp = false;				// アウトライン解析表示の有無
	pType->eOutlineDockSide = DockSideType::Float;	// アウトライン解析ドッキング配置
	pType->cxOutlineDockLeft = 0;					// アウトラインの左ドッキング幅
	pType->cyOutlineDockTop = 0;					// アウトラインの上ドッキング高
	pType->cxOutlineDockRight = 0;					// アウトラインの右ドッキング幅
	pType->cyOutlineDockBottom = 0;					// アウトラインの下ドッキング高
	pType->eDefaultOutline = OutlineType::Text;		// アウトライン解析方法
	pType->nOutlineSortCol = 0;						// アウトライン解析ソート列番号
	pType->bOutlineSortDesc = false;				// アウトライン解析ソート降順
	pType->nOutlineSortType = 0;					// アウトライン解析ソート基準
	pType->eSmartIndent = SmartIndentType::None;		// スマートインデント種別
	pType->nImeState = IME_CMODE_NOCONVERSION;	// IME入力

	pType->szOutlineRuleFilename[0] = L'\0';		//Dec. 4, 2000 MIK
	pType->bKinsokuHead = false;					// 行頭禁則				//@@@ 2002.04.08 MIK
	pType->bKinsokuTail = false;					// 行末禁則				//@@@ 2002.04.08 MIK
	pType->bKinsokuRet  = false;					// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	pType->bKinsokuKuto = false;					// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	pType->szKinsokuHead[0] = 0;					// 行頭禁則				//@@@ 2002.04.08 MIK
	pType->szKinsokuTail[0] = 0;					// 行末禁則				//@@@ 2002.04.08 MIK
	wcscpy_s(pType->szKinsokuKuto, L"、。，．､｡,.");	// 句読点ぶら下げ文字	// 2009.08.07 ryoji

	pType->bUseDocumentIcon = false;				// 文書に関連づけられたアイコンを使う

//@@@ 2001.11.17 add start MIK
	for (int i=0; i<_countof(pType->regexKeywordArr); ++i) {
		pType->regexKeywordArr[i].nColorIndex = COLORIDX_REGEX1;
	}
	pType->regexKeywordList[0] = L'\0';
	pType->bUseRegexKeyword = false;
//@@@ 2001.11.17 add end MIK
	pType->nRegexKeyMagicNumber = 0;

//@@@ 2006.04.10 fon ADD-start
	for (int i=0; i<MAX_KEYHELP_FILE; ++i) {
		auto& attr = pType->keyHelpArr[i];
		attr.bUse = false;
		attr.szAbout[0] = _T('\0');
		attr.szPath[0] = _T('\0');
	}
	pType->bUseKeywordHelp = false;		// 辞書選択機能の使用可否
	pType->nKeyHelpNum = 0;				// 登録辞書数
	pType->bUseKeyHelpAllSearch = false;	// ヒットした次の辞書も検索(&A)
	pType->bUseKeyHelpKeyDisp = false;	// 1行目にキーワードも表示する(&W)
	pType->bUseKeyHelpPrefix = false;		// 選択範囲で前方一致検索(&P)
//@@@ 2006.04.10 fon ADD-end

	// 2005.11.08 Moca 指定位置縦線の設定
	for (int i=0; i<MAX_VERTLINES; ++i) {
		pType->nVertLineIdx[i] = LayoutInt(0);
	}
	pType->nNoteLineOffset = 0;

	//  保存時に改行コードの混在を警告する	2013/4/14 Uchi
	pType->bChkEnterAtEnd = true;

	pType->bUseTypeFont = false;				// タイプ別フォントの使用

	pType->nLineNumWidth = LINENUMWIDTH_MIN;	// 行番号最小桁数 2014.08.02 katze
}

