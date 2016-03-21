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
#pragma once

#include "Eol.h"
#include "env/CommonSetting.h"
#include "doc/DocTypeSetting.h"
#include "doc/LineComment.h"
#include "doc/BlockComment.h"
#include "charset/charset.h"	// EncodingType
#include "RegexKeyword.h"		// RegexKeywordInfo
#include "env/CommonSetting.h"

enum class DockSideType;

// タブ表示方法
enum class TabArrowType {
	String,		// 文字指定
	Short,		// 短い矢印
	Long,		// 長い矢印
};

// アウトライン解析の種類
enum class OutlineType {
	C,
	CPP,
	PLSQL,
	Text,
	Java,
	Cobol,
	Asm,
	Perl,				// Sep. 8, 2000 genta
	VisualBasic,		// June 23, 2001 N.Nakatani
	WZText,				// 2003.05.20 zenryaku 階層付テキストアウトライン解析
	HTML,				// 2003.05.20 zenryaku HTMLアウトライン解析
	TeX,				// 2003.07.20 naoh TeXアウトライン解析
	RuleFile,			// 2002.04.01 YAZAKI ルールファイル用
	Python,				// 2007.02.08 genta Pythonアウトライン解析
	Erlang,				// 2009.08.10 genta Erlangアウトライン解析
	//	新しいアウトライン解析は必ずこの直前へ挿入
	CodeMax,
	BookMark,			// 2001.12.03 hor
	PlugIn,				// 2009.10.29 syat プラグインによるアウトライン解析
	FileTree,			//	2012.06.20 Moca ファイルツリー
	Default = -1,		// 2001.12.03 hor
	UnknownOutlineType	= 99,
	Tree = 100,			// 汎用ツリー 2010.03.28 syat
	TreeTagJump = 101,	// 汎用ツリー(タグジャンプ付き) 2013.05.01 Moca
	ClassTree = 200,	// 汎用ツリー(クラス) 2010.03.28 syat
	List = 300,			// 汎用リスト 2010.03.28 syat
};

// スマートインデント種別
enum class SmartIndentType {
	None,		// なし
	Cpp			// C/C++
};

// ヒアドキュメント種別
enum class HereDocType {
	PHP,			// PHP
	Ruby,			// Ruby
	Perl			// Perl
};

// 背景画像表示位置
enum class BackgroundImagePosType {
	TopLeft,		// 左上
	TopRight,		// 右上
	BottomLeft,		// 左下
	BottomRight,	// 右下
	Center,			// 中央
	TopCenter,		// 中央上
	BottomCenter,	// 中央下
	CenterLeft,		// 中央左
	CenterRight	,	// 中央右
};

// エンコードオプション
struct EncodingConfig {
	bool			bPriorCesu8;					// 自動判別時に CESU-8 を優先するかどうか
	EncodingType	eDefaultCodetype;				// デフォルト文字コード
	EolType			eDefaultEoltype;				// デフォルト改行コード	// 2011.01.24 ryoji
	bool			bDefaultBom;					// デフォルトBOM			// 2011.01.24 ryoji
};

// 文字列区切り記号エスケープ方法
enum class StringLiteralType {
	CPP,	// C/C++言語風
	PLSQL,	// PL/SQL風
	HTML,	// HTML/XML風
	CSharp,	// C#風
	Python,	// Python風
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       タイプ別設定                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// タイプ別設定
struct TypeConfig {
	// 2007.09.07 変数名変更: m_nMaxLineSize→nMaxLineKetas
	int					nIdx;
	int					id;
	TCHAR				szTypeName[64];				// タイプ属性：名称
	TCHAR				szTypeExts[MAX_TYPES_EXTS];	// タイプ属性：拡張子リスト
	TextWrappingMethod	nTextWrapMethod;			// テキストの折り返し方法		// 2008.05.30 nasukoji
	LayoutInt			nMaxLineKetas;				// 折り返し桁数
	int					nColumnSpace;				// 文字と文字の隙間
	int					nLineSpace;					// 行間のすきま
	LayoutInt			nTabSpace;					// TABの文字数
	TabArrowType		bTabArrow;					// タブ矢印表示		//@@@ 2003.03.26 MIK
	EDIT_CHAR			szTabViewString[8 + 1];		// TAB表示文字列	// 2003.1.26 aroka サイズ拡張	// 2009.02.11 ryoji サイズ戻し(17->8+1)
	bool				bInsSpace;					// スペースの挿入	// 2001.12.03 hor
	// 2005.01.13 MIK 配列化
	int					nKeywordSetIdx[MAX_KEYWORDSET_PER_TYPE];	// キーワードセット

	LineComment			lineComment;					// 行コメントデリミタ				//@@@ 2002.09.22 YAZAKI
	BlockComment		blockComments[2];				// ブロックコメントデリミタ		//@@@ 2002.09.22 YAZAKI

	StringLiteralType	stringType;						// 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	bool				bStringLineOnly;				// 文字列は行内のみ
	bool				bStringEndLine;					// (終了文字列がない場合)行末まで色分け
	HereDocType			nHeredocType;
	wchar_t				szIndentChars[64];				// その他のインデント対象文字

	int					nColorInfoArrNum;				// 色設定配列の有効数
	ColorInfo			colorInfoArr[64];				// 色設定配列

	SFilePath			szBackImgPath;					// 背景画像
	BackgroundImagePosType backImgPos;					// 背景画像表示位置
	bool				backImgRepeatX;					// 背景画像表示横方向繰り返し
	bool				backImgRepeatY;					// 背景画像表示縦方向繰り返し
	bool				backImgScrollX;					// 背景画像表示横方向スクロール
	bool				backImgScrollY;					// 背景画像表示縦方向スクロール
	POINT				backImgPosOffset;				// 背景画像表示オフセット

	bool				bLineNumIsCRLF;					// 行番号の表示 false=折り返し単位／true=改行単位
	int					nLineTermType;					// 行番号区切り  0=なし 1=縦線 2=任意
	wchar_t				cLineTermChar;					// 行番号区切り文字
	LayoutInt			nVertLineIdx[MAX_VERTLINES];	// 指定桁縦線
	int 				nNoteLineOffset;				// ノート線のオフセット

	bool				bWordWrap;						// 英文ワードラップをする
	bool				bKinsokuHead;					// 行頭禁則をする		//@@@ 2002.04.08 MIK
	bool				bKinsokuTail;					// 行末禁則をする		//@@@ 2002.04.08 MIK
	bool				bKinsokuRet;					// 改行文字のぶら下げ	//@@@ 2002.04.13 MIK
	bool				bKinsokuKuto;					// 句読点のぶらさげ	//@@@ 2002.04.17 MIK
	bool				bKinsokuHide;					// ぶら下げを隠す		// 2011/11/30 Uchi
	wchar_t				szKinsokuHead[200];				// 行頭禁則文字		//@@@ 2002.04.08 MIK
	wchar_t				szKinsokuTail[200];				// 行頭禁則文字 		//@@@ 2002.04.08 MIK
	wchar_t				szKinsokuKuto[200];				// 句読点ぶらさげ文字	// 2009.08.07 ryoji

	int					nCurrentPrintSetting;			// 現在選択している印刷設定

	bool				bOutlineDockDisp;				// アウトライン解析表示の有無
	DockSideType		eOutlineDockSide;				// アウトライン解析ドッキング配置
	int					cxOutlineDockLeft;				// アウトラインの左ドッキング幅
	int					cyOutlineDockTop;				// アウトラインの上ドッキング高
	int					cxOutlineDockRight;				// アウトラインの右ドッキング幅
	int					cyOutlineDockBottom;			// アウトラインの下ドッキング高
	OutlineType			nDockOutline;					// ドッキング時のアウトライン/ブックマーク
	OutlineType			eDefaultOutline;				// アウトライン解析方法
	SFilePath			szOutlineRuleFilename;			// アウトライン解析ルールファイル
	int					nOutlineSortCol;				// アウトライン解析ソート列番号
	bool				bOutlineSortDesc;				// アウトライン解析ソート降順
	int					nOutlineSortType;				// アウトライン解析ソート基準
	FileTree			fileTree;						// ファイルツリー設定

	SmartIndentType		eSmartIndent;					// スマートインデント種別
	int					nImeState;						// 初期IME状態	Nov. 20, 2000 genta

	//	2001/06/14 asa-o 補完のタイプ別設定
	SFilePath			szHokanFile;					// 入力補完 単語ファイル
	int					nHokanType;						// 入力補完 種別(プラグイン)
	//	2003.06.23 Moca ファイル内からの入力補完機能
	bool				bUseHokanByFile;				// 入力補完 開いているファイル内から候補を探す
	bool				bUseHokanByKeyword;				// 強調キーワードから入力補完
	
	//	2001/06/19 asa-o
	bool				bHokanLoHiCase;					// 入力補完機能：英大文字小文字を同一視する

	SFilePath			szExtHelp;						// 外部ヘルプ１
	SFilePath			szExtHtmlHelp;					// 外部HTMLヘルプ
	bool				bHtmlHelpIsSingle;				// HtmlHelpビューアはひとつ

	bool				bChkEnterAtEnd;					// 保存時に改行コードの混在を警告する	2013/4/14 Uchi

	EncodingConfig		encoding;						// エンコードオプション


//@@@ 2001.11.17 add start MIK
	bool				bUseRegexKeyword;								// 正規表現キーワードを使うか
	DWORD				nRegexKeyMagicNumber;							// 正規表現キーワード更新マジックナンバー
	RegexKeywordInfo	regexKeywordArr[MAX_REGEX_KEYWORD];				// 正規表現キーワード
	wchar_t				regexKeywordList[MAX_REGEX_KEYWORDLISTLEN];		// 正規表現キーワード
//@@@ 2001.11.17 add end MIK

//@@@ 2006.04.10 fon ADD-start
	bool				bUseKeywordHelp;				// キーワード辞書セレクト機能を使うか
	int					nKeyHelpNum;					// キーワード辞書の冊数
	KeyHelpInfo			keyHelpArr[MAX_KEYHELP_FILE];	// キーワード辞書ファイル
	bool				bUseKeyHelpAllSearch;			// ヒットした次の辞書も検索(&A)
	bool				bUseKeyHelpKeyDisp;				// 1行目にキーワードも表示する(&W)
	bool				bUseKeyHelpPrefix;				// 選択範囲で前方一致検索(&P)
//@@@ 2006.04.10 fon ADD-end

	//	2002/04/30 YAZAKI Commonから移動。
	bool				bAutoIndent;					// オートインデント
	bool				bAutoIndent_ZENSPACE;			// 日本語空白もインデント
	bool				bRTrimPrevLine;					// 2005.10.11 ryoji 改行時に末尾の空白を削除
	int					nIndentLayout;					// 折り返しは2行目以降を字下げ表示

	//	Sep. 10, 2002 genta
	bool				bUseDocumentIcon;				// ファイルに関連づけられたアイコンを使う

	bool				bUseTypeFont;					// タイプ別フォントの使用
	LOGFONT				lf;								// フォント // 2013.03.18 aroka
	INT					nPointSize;						// フォントサイズ（1/10ポイント単位）

	TypeConfig()
		:
		nMaxLineKetas(10) //	画面折り返し幅がTAB幅以下にならないことを初期値でも保証する	//	2004.04.03 Moca
	{
	}

	int			nLineNumWidth;					// 行番号の最小桁数 2014.08.02 katze
}; // TypeConfig

// タイプ別設定(mini)
struct TypeConfigMini {
	int			id;
	TCHAR		szTypeName[64];				// タイプ属性：名称
	TCHAR		szTypeExts[MAX_TYPES_EXTS];	// タイプ属性：拡張子リスト
	EncodingConfig		encoding;			// エンコードオプション
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   タイプ別設定アクセサ                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// ドキュメント種類。共有データ内 TypeConfig へのアクセサも兼ねる。
// 2007.12.13 kobake 作成
class TypeConfigNum {
public:
	TypeConfigNum()
	{
#ifdef _DEBUG
		// 元がintだったので、未初期化で使うと問題が発生するように、あえて、変な値を入れておく。
		type = 1234;
#else
		// リリース時は、未初期化でも問題が起こりにくいように、ゼロクリアしておく
		type = 0;
#endif
	}
	
	explicit TypeConfigNum(int n) {
		type = n;
	}
	bool IsValidType() const { return type >= 0 && type < MAX_TYPES; }
	int GetIndex() const { /*assert(IsValid());*/ return type; }

	// 共有データへの簡易アクセサ
//	TypeConfig* operator->() { return GetTypeConfig(); }
//	TypeConfig* GetTypeConfig();
private:
	int type;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        タイプ設定                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class Type {
public:
	virtual ~Type() { }
	void InitTypeConfig(int nIdx, TypeConfig&);
protected:
	virtual void InitTypeConfigImp(TypeConfig& type) = 0;
};

#define GEN_CTYPE(CLASS_NAME) \
class CLASS_NAME : public Type { \
protected: \
	void InitTypeConfigImp(TypeConfig& type); \
};

GEN_CTYPE(CType_Asm)
GEN_CTYPE(CType_Awk)
GEN_CTYPE(CType_Basis)
GEN_CTYPE(CType_Cobol)
GEN_CTYPE(CType_Cpp)
GEN_CTYPE(CType_Dos)
GEN_CTYPE(CType_Html)
GEN_CTYPE(CType_Ini)
GEN_CTYPE(CType_Java)
GEN_CTYPE(CType_Pascal)
GEN_CTYPE(CType_Perl)
GEN_CTYPE(CType_Rich)
GEN_CTYPE(CType_Sql)
GEN_CTYPE(CType_Tex)
GEN_CTYPE(CType_Text)
GEN_CTYPE(CType_Vb)
GEN_CTYPE(CType_Other)


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	スペースの判定
*/
inline
bool C_IsSpace( wchar_t c, bool bExtEol )
{
	return (
		L'\t' == c ||
		L' ' == c ||
		WCODE::IsLineDelimiter(c, bExtEol)
	);
}

