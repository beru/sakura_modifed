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
	Perl,				// 
	VisualBasic,
	WZText,				// 階層付テキストアウトライン解析
	HTML,				// HTMLアウトライン解析
	TeX,				// TeXアウトライン解析
	RuleFile,			// ルールファイル用
	Python,				// Pythonアウトライン解析
	Erlang,				// Erlangアウトライン解析
	//	新しいアウトライン解析は必ずこの直前へ挿入
	CodeMax,
	BookMark,			// 
	PlugIn,				// プラグインによるアウトライン解析
	FileTree,			// ファイルツリー
	Default = -1,		// 
	UnknownOutlineType	= 99,
	Tree = 100,			// 汎用ツリー
	TreeTagJump = 101,	// 汎用ツリー(タグジャンプ付き)
	ClassTree = 200,	// 汎用ツリー(クラス)
	List = 300,			// 汎用リスト
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
	EolType			eDefaultEoltype;				// デフォルト改行コード
	bool			bDefaultBom;					// デフォルトBOM
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
	size_t				nIdx;
	int					id;
	TCHAR				szTypeName[64];				// タイプ属性：名称
	TCHAR				szTypeExts[MAX_TYPES_EXTS];	// タイプ属性：拡張子リスト
	TextWrappingMethod	nTextWrapMethod;			// テキストの折り返し方法
	size_t				nMaxLineKetas;				// 折り返し桁数
	size_t				nColumnSpace;				// 文字と文字の隙間
	int					nLineSpace;					// 行間のすきま
	size_t				nTabSpace;					// TABの文字数
	TabArrowType		bTabArrow;					// タブ矢印表示
	EDIT_CHAR			szTabViewString[8 + 1];		// TAB表示文字列
	bool				bInsSpace;					// スペースの挿入
	int					nKeywordSetIdx[MAX_KEYWORDSET_PER_TYPE];	// キーワードセット

	LineComment			lineComment;					// 行コメントデリミタ
	BlockComment		blockComments[2];				// ブロックコメントデリミタ

	StringLiteralType	stringType;						// 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""]['']
	bool				bStringLineOnly;				// 文字列は行内のみ
	bool				bStringEndLine;					// (終了文字列がない場合)行末まで色分け
	HereDocType			nHeredocType;
	wchar_t				szIndentChars[64];				// その他のインデント対象文字

	size_t				nColorInfoArrNum;				// 色設定配列の有効数
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
	int					nVertLineIdx[MAX_VERTLINES];	// 指定桁縦線
	int 				nNoteLineOffset;				// ノート線のオフセット

	bool				bWordWrap;						// 英文ワードラップをする
	bool				bKinsokuHead;					// 行頭禁則をする
	bool				bKinsokuTail;					// 行末禁則をする
	bool				bKinsokuRet;					// 改行文字のぶら下げ
	bool				bKinsokuKuto;					// 句読点のぶらさげ
	bool				bKinsokuHide;					// ぶら下げを隠す
	wchar_t				szKinsokuHead[200];				// 行頭禁則文字
	wchar_t				szKinsokuTail[200];				// 行頭禁則文字
	wchar_t				szKinsokuKuto[200];				// 句読点ぶらさげ文字

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
	int					nImeState;						// 初期IME状態

	// 補完のタイプ別設定
	SFilePath			szHokanFile;					// 入力補完 単語ファイル
	int					nHokanType;						// 入力補完 種別(プラグイン)
	// ファイル内からの入力補完機能
	bool				bUseHokanByFile;				// 入力補完 開いているファイル内から候補を探す
	bool				bUseHokanByKeyword;				// 強調キーワードから入力補完
	
	bool				bHokanLoHiCase;					// 入力補完機能：英大文字小文字を同一視する

	SFilePath			szExtHelp;						// 外部ヘルプ１
	SFilePath			szExtHtmlHelp;					// 外部HTMLヘルプ
	bool				bHtmlHelpIsSingle;				// HtmlHelpビューアはひとつ

	bool				bChkEnterAtEnd;					// 保存時に改行コードの混在を警告する

	EncodingConfig		encoding;						// エンコードオプション


	bool				bUseRegexKeyword;								// 正規表現キーワードを使うか
	DWORD				nRegexKeyMagicNumber;							// 正規表現キーワード更新マジックナンバー
	RegexKeywordInfo	regexKeywordArr[MAX_REGEX_KEYWORD];				// 正規表現キーワード
	wchar_t				regexKeywordList[MAX_REGEX_KEYWORDLISTLEN];		// 正規表現キーワード

	bool				bUseKeywordHelp;				// キーワード辞書セレクト機能を使うか
	int					nKeyHelpNum;					// キーワード辞書の冊数
	KeyHelpInfo			keyHelpArr[MAX_KEYHELP_FILE];	// キーワード辞書ファイル
	bool				bUseKeyHelpAllSearch;			// ヒットした次の辞書も検索(&A)
	bool				bUseKeyHelpKeyDisp;				// 1行目にキーワードも表示する(&W)
	bool				bUseKeyHelpPrefix;				// 選択範囲で前方一致検索(&P)

	bool				bAutoIndent;					// オートインデント
	bool				bAutoIndent_ZENSPACE;			// 日本語空白もインデント
	bool				bRTrimPrevLine;					// 改行時に末尾の空白を削除
	int					nIndentLayout;					// 折り返しは2行目以降を字下げ表示

	bool				bUseDocumentIcon;				// ファイルに関連づけられたアイコンを使う

	bool				bUseTypeFont;					// タイプ別フォントの使用
	LOGFONT				lf;								// フォント
	INT					nPointSize;						// フォントサイズ（1/10ポイント単位）

	TypeConfig()
		:
		nMaxLineKetas(10) //	画面折り返し幅がTAB幅以下にならないことを初期値でも保証する
	{
	}

	int			nLineNumWidth;					// 行番号の最小桁数
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
	void InitTypeConfig(size_t nIdx, TypeConfig&);
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
bool C_IsSpace(wchar_t c, bool bExtEol)
{
	return (
		L'\t' == c ||
		L' ' == c ||
		WCODE::IsLineDelimiter(c, bExtEol)
	);
}

