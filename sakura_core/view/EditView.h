#pragma once

#include <Windows.h>
#include <ObjIdl.h>  // LPDATAOBJECT
#include <ShellAPI.h>  // HDROP
#include "TextMetrics.h"
#include "TextDrawer.h"
#include "TextArea.h"
#include "Caret.h"
#include "ViewCalc.h" // parent
#include "EditView_Paint.h"	// parent
#include "ViewParser.h"
#include "ViewSelect.h"
#include "SearchAgent.h"
#include "view/colors/EColorIndexType.h"
#include "window/TipWnd.h"
#include "window/AutoScrollWnd.h"
#include "DicMgr.h"
#include "extmodule/Bregexp.h"
#include "Eol.h"				// EolType
#include "cmd/ViewCommander.h"
#include "mfclike/MyWnd.h"		// parent
#include "doc/DocListener.h"	// parent
#include "basis/SakuraBasis.h"	// LogicInt, LayoutInt
#include "util/container.h"		// vector_ex
#include "util/design_template.h"

class ViewFont;
class Ruler;
class DropTarget;
class OpeBlk;///
class SplitBoxWnd;///
class RegexKeyword;///
class AutoMarkMgr;
class EditDoc;
class Layout;
class Migemo;
struct ColorStrategyInfo;
struct Color3Setting;
class OutputAdapter;

// struct DispPos; //	誰かがincludeしてます
// class CColorStrategy;	// 誰かがincludeしてます
class Color_Found;

#ifndef IDM_COPYDICINFO
#define IDM_COPYDICINFO 2000
#endif
#ifndef IDM_JUMPDICT
#define IDM_JUMPDICT 2001
#endif

///	マウスからコマンドが実行された場合の上位ビット
const int CMD_FROM_MOUSE = 2;


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 文書ウィンドウの管理
	
	1つの文書ウィンドウにつき1つのEditDocオブジェクトが割り当てられ、
	1つのEditDocオブジェクトにつき、4つのCEditViweオブジェクトが割り当てられる。
	ウィンドウメッセージの処理、コマンドメッセージの処理、
	画面表示などを行う。
*/

class EditView :
	public ViewCalc, //$$ これが親クラスである必要は無いが、このクラスのメソッド呼び出しが多いので、暫定的に親クラスとする。
	public EditView_Paint,
	public MyWnd,
	public DocListenerEx
{
public:
	const EditDoc& GetDocument() const {
		return *pEditDoc;
	}
	EditDoc& GetDocument() {
		return *pEditDoc;
	}
public:
	// 背景にビットマップを使用するかどうか
	bool IsBkBitmap() const { return pEditDoc->hBackImg != NULL; }

public:
	EditView& GetEditView() override {
		return *this;
	}
	const EditView& GetEditView() const override {
		return *this;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        生成と破棄                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// Constructors
	EditView(EditWnd& editWnd);
	~EditView();
	void Close();
	// 初期化系メンバ関数
	BOOL Create(
		HWND		hwndParent,	// 親
		EditDoc&	editDoc,	// 参照するドキュメント
		int			nMyIndex,	// ビューのインデックス
		BOOL		bShow,		// 作成時に表示するかどうか
		bool		bMiniMap
	);
	void CopyViewStatus(EditView*) const;					// 自分の表示状態を他のビューにコピー

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      クリップボード                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 取得
	bool MyGetClipboardData(NativeW&, bool*, bool* = nullptr);			// クリップボードからデータを取得

	// 設定
	bool MySetClipboardData(const char*, size_t, bool bColumnSelect, bool = false);	// クリップボードにデータを設定
	bool MySetClipboardData(const wchar_t*, size_t, bool bColumnSelect, bool = false);	// クリップボードにデータを設定

	// 利用
	void CopyCurLine(bool bAddCRLFWhenCopy, EolType neweol, bool bEnableLineModePaste);	// カーソル行をクリップボードにコピーする
	void CopySelectedAllLines(const wchar_t*, bool);			// 選択範囲内の全行をクリップボードにコピーする


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         イベント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ドキュメントイベント
	void OnAfterLoad(const LoadInfo& loadInfo);
	// メッセージディスパッチャ
	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	//
	void OnChangeSetting();								// 設定変更を反映させる
	void OnPaint(HDC, PAINTSTRUCT *, BOOL);				// 通常の描画処理
	void OnPaint2( HDC, PAINTSTRUCT *, BOOL );			// 通常の描画処理
	void DrawBackImage(HDC hdc, RECT& rcPaint, HDC hdcBgImg);
	void OnTimer(HWND, UINT, UINT_PTR, DWORD);
	// ウィンドウ
	void OnSize(int, int);								// ウィンドウサイズの変更処理
	void OnMove(int, int, int, int);
	// フォーカス
	void OnSetFocus(void);
	void OnKillFocus(void);
	// スクロール
	int OnVScroll(int, int);					// 垂直スクロールバーメッセージ処理
	int OnHScroll(int, int);					// 水平スクロールバーメッセージ処理
	// マウス
	void OnLBUTTONDOWN(WPARAM, int, int);			// マウス左ボタン押下
	void OnMOUSEMOVE(WPARAM, int, int);				// マウス移動のメッセージ処理
	void OnLBUTTONUP(WPARAM, int, int);				// マウス左ボタン開放のメッセージ処理
	void OnLBUTTONDBLCLK(WPARAM, int , int);		// マウス左ボタンダブルクリック
	void OnRBUTTONDOWN(WPARAM, int, int);			// マウス右ボタン押下
	void OnRBUTTONUP(WPARAM, int, int);				// マウス右ボタン開放
	void OnMBUTTONDOWN(WPARAM, int, int);			// マウス中ボタン押下
	void OnMBUTTONUP(WPARAM, int, int);				// マウス中ボタン開放
	void OnXLBUTTONDOWN(WPARAM, int, int);			// マウスサイドボタン1押下
	void OnXLBUTTONUP(WPARAM, int, int);			// マウスサイドボタン1開放
	void OnXRBUTTONDOWN(WPARAM, int, int);			// マウスサイドボタン2押下
	void OnXRBUTTONUP(WPARAM, int, int);			// マウスサイドボタン2開放
	LRESULT OnMOUSEWHEEL(WPARAM, LPARAM);			// 垂直マウスホイールのメッセージ処理
	LRESULT OnMOUSEHWHEEL(WPARAM, LPARAM);			// 水平マウスホイールのメッセージ処理
	LRESULT OnMOUSEWHEEL2(WPARAM, LPARAM, bool, EFunctionCode);		// マウスホイールのメッセージ処理
	bool IsSpecialScrollMode(int);					// キー・マウスボタン状態よりスクロールモードを判定する

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	// ロジック行を1行描画
	bool DrawLogicLine(
		HDC				hdc,			// [in]     作画対象
		DispPos*		pDispPos,		// [in/out] 描画する箇所、描画元ソース
		int				nLineTo			// [in]     作画終了するレイアウト行番号
	);

	// レイアウト行を1行描画
	bool DrawLayoutLine(ColorStrategyInfo& csInfo);

	// 色分け
public:
	Color3Setting GetColorIndex(const Layout* pLayout, int nLineNum, int nIndex, ColorStrategyInfo& csInfo, bool bPrev = false);	// 指定位置のColorIndexの取得
	void SetCurrentColor(Graphics& gr, EColorIndexType, EColorIndexType, EColorIndexType);
	COLORREF GetTextColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2);
	COLORREF GetBackColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2);

	// 画面バッファ
protected:
	bool CreateOrUpdateCompatibleBitmap(int cx, int cy);	// メモリBMPを作成または更新
	void UseCompatibleDC(BOOL fCache);
public:
	void DeleteCompatibleBitmap();							// メモリBMPを削除

public:
	void DispTextSelected(HDC hdc, int nLineNum, const Point& ptXY, int nX_Layout);	// テキスト反転
	void RedrawAll();										// フォーカス移動時の再描画
	void Redraw();											// 再描画
	void RedrawLines(int top, int bottom);
	void CaretUnderLineON(bool, bool, bool);				// カーソル行アンダーラインのON
	void CaretUnderLineOFF(bool, bool, bool, bool);			// カーソル行アンダーラインのOFF
	bool GetDrawSwitch() const {
		return bDrawSwitch;
	}
	bool SetDrawSwitch(bool b) {
		bool bOld = bDrawSwitch;
		bDrawSwitch = b;
		return bOld;
	}
	bool IsDrawCursorVLinePos(int);
	void DrawBracketCursorLine(bool);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        スクロール                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AdjustScrollBars();											// スクロールバーの状態を更新する
	BOOL CreateScrollBar();												// スクロールバー作成
	void DestroyScrollBar();											// スクロールバー破棄
	size_t GetWrapOverhang(void) const;								// 折り返し桁以後のぶら下げ余白計算
	int ViewColNumToWrapColNum(int nViewColNum) const;				//「右端で折り返す」用にビューの桁数から折り返し桁数を計算する
	size_t GetRightEdgeForScrollBar(void);							// スクロールバー制御用に右端座標を取得する

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           IME                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool IsImeON(void);	// IME ONか
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        スクロール                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int  ScrollAtV(int);								// 指定上端行位置へスクロール
	int  ScrollAtH(int);								// 指定左端桁位置へスクロール
	int  ScrollByV(int vl) { return ScrollAtV((int)GetTextArea().GetViewTopLine() + vl); }	// 指定行スクロール
	int  ScrollByH(int hl) { return ScrollAtH((int)GetTextArea().GetViewLeftCol() + hl); }	// 指定桁スクロール
	void ScrollDraw(int, int, const RECT&, const RECT&, const RECT&);
	void MiniMapRedraw(bool);
public:
	void SyncScrollV(int);										// 垂直同期スクロール
	void SyncScrollH(int);										// 水平同期スクロール

	void SetBracketPairPos(bool);										// 対括弧の強調表示位置設定

	void AutoScrollEnter();
	void AutoScrollExit();
	void AutoScrollMove(Point& point);
	void AutoScrollOnTimer();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        過去の遺産                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void SetIMECompFormPos(void);								// IME編集エリアの位置を変更
	void SetIMECompFormFont(void);								// IME編集エリアの表示フォントを変更
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       テキスト選択                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool GetSelectedDataSimple(NativeW&);// 選択範囲のデータを取得
	bool GetSelectedDataOne(NativeW& memBuf, size_t nMaxLen);
	bool GetSelectedData(NativeW*, bool, const wchar_t*, bool, bool bAddCRLFWhenCopy, EolType neweol = EolType::Unknown);	// 選択範囲のデータを取得
	int IsCurrentPositionSelected(Point ptCaretPos);					// 指定カーソル位置が選択エリア内にあるか
	int IsCurrentPositionSelectedTEST(const Point& ptCaretPos, const Range& select) const; // 指定カーソル位置が選択エリア内にあるか
	// 行桁指定によるカーソル移動(選択領域を考慮)
	void MoveCursorSelecting(Point ptWk_CaretPos, bool bSelect, int = _CARETMARGINRATE);
	void ConvSelectedArea(EFunctionCode);									// 選択エリアのテキストを指定方法で変換
	// 指定位置または指定範囲がテキストの存在しないエリアかチェックする
	bool IsEmptyArea(Point ptFrom, Point ptTo = Point(-1, -1), bool bSelect = false, bool bBoxSelect = false) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         各種判定                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool IsCurrentPositionURL(const Point& ptCaretPos, Range* pUrlRange, std::wstring* pwstrURL); // カーソル位置にURLが有る場合のその範囲を調べる
	bool CheckTripleClick(Point ptMouse);							// トリプルクリックをチェックする
	
	bool ExecCmd(const TCHAR*, int, const TCHAR*, OutputAdapter* = nullptr) ;			// 子プロセスの標準出力をリダイレクトする
	void AddToCmdArr(const TCHAR*);
	bool ChangeCurRegexp(bool bRedrawIfChanged = true);			// 正規表現の検索パターンを必要に応じて更新する(ライブラリが使用できないときは false を返す)
	void SendStatusMessage(const TCHAR* msg);					// 検索／置換／ブックマーク検索時の状態をステータスバーに表示する
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode, bool bDocumentFeed = false);	// 再変換用構造体を設定する
	LRESULT SetSelectionFromReonvert(const RECONVERTSTRING* pReconv, bool bUnicode);				// 再変換用構造体の情報を元に選択範囲を変更する

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           D&D                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public: // テスト用にアクセス属性を変更
	// IDropTarget実装
	STDMETHODIMP DragEnter(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragLeave(void);
	STDMETHODIMP Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHODIMP PostMyDropFiles(LPDATAOBJECT pDataObject);		// 独自ドロップファイルメッセージをポストする
	void OnMyDropFiles(HDROP hDrop);								// 独自ドロップファイルメッセージ処理
	CLIPFORMAT GetAvailableClipFormat(LPDATAOBJECT pDataObject);
	DWORD TranslateDropEffect(CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect);
	bool IsDragSource(void);

	void _SetDragMode(bool b) {
		bDragMode = b;
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           編集                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 指定位置の指定長データ削除
	void DeleteData2(
		const Point&	ptCaretPos,
		size_t			nDelLen,
		NativeW*		pMem
	);

	// 現在位置のデータ削除
	void DeleteData(bool bRedraw);

	// 現在位置にデータを挿入
	void InsertData_CEditView(
		Point	ptInsertPos,
		const wchar_t*	pData,
		size_t			nDataLen,
		Point*	pptNewPos,	// 挿入された部分の次の位置のデータ位置
		bool			bRedraw
	);

	// データ置換 削除&挿入にも使える
	void ReplaceData_CEditView(
		const Range&	delRange,			// 削除範囲。レイアウト単位。
		const wchar_t*	pInsData,			// 挿入するデータ
		size_t			nInsDataLen,		// 挿入するデータの長さ
		bool			bRedraw,
		OpeBlk*			pOpeBlk,
		bool			bFastMode = false,
		const Range*	psDelRangeLogicFast = nullptr
	);
	void ReplaceData_CEditView2(
		const Range&	delRange,			// 削除範囲。ロジック単位。
		const wchar_t*	pInsData,			// 挿入するデータ
		size_t			nInsDataLen,		// 挿入するデータの長さ
		bool			bRedraw,
		OpeBlk*			pOpeBlk,
		bool			bFastMode = false
	);
	bool ReplaceData_CEditView3(
		Range		delRange,			// 削除範囲。レイアウト単位。
		OpeLineData*	pMemCopyOfDeleted,	// 削除されたデータのコピー(nullptr可能)
		OpeLineData*	pInsData,
		bool			bRedraw,
		OpeBlk*			pOpeBlk,
		int				nDelSeq,
		int*			pnInsSeq,
		bool			bFastMode = false,
		const Range*	psDelRangeLogicFast = nullptr
	);
	void RTrimPrevLine(void);		// 前の行にある末尾の空白を削除

	// 挿入モードの設定・取得
	bool IsInsMode() const;
	void SetInsMode(bool);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           検索                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// インクリメンタルサーチ関係
	void TranslateCommand_isearch(EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM&);
	bool ProcessCommand_isearch(int, bool, LPARAM, LPARAM, LPARAM, LPARAM);

	void TranslateCommand_grep(EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM&);

	bool IsISearchEnabled(int nCommand) const;

	bool KeySearchCore(const NativeW* pMemCurText);
	bool MiniMapCursorLineTip(POINT* po, RECT* rc, bool* pbHide);

	/*!	EditView::KeywordHelpSearchDictのコール元指定用ローカルID */
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	// EditView::OnTimer
		LID_SKH_POPUPMENU_R = 2,	// EditView::CreatePopUpMenu_R
	};
	BOOL KeywordHelpSearchDict(LID_SKH nID, POINT* po, RECT* rc);

	size_t IsSearchString(const StringRef& str, size_t, int*, int*) const;	// 現在位置が検索文字列に該当するか

	void GetCurrentTextForSearch(NativeW&, bool bStripMaxPath = true, bool bTrimSpaceTab = false);			// 現在カーソル位置単語または選択範囲より検索等のキーを取得
	bool GetCurrentTextForSearchDlg(NativeW&, bool bGetHistory = false);		// 現在カーソル位置単語または選択範囲より検索等のキーを取得（ダイアログ用）

private:
	// インクリメンタルサーチ 
	void ISearchEnter(int mode, SearchDirection direction);
	void ISearchExit();
	void ISearchExec(DWORD wChar);
	void ISearchExec(LPCWSTR pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(NativeT* msg) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           括弧                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool  SearchBracket(const Point& ptPos, Point* pptLayoutNew, int* mode);	// 対括弧の検索
	bool  SearchBracketForward(Point ptPos, Point* pptLayoutNew,
						const wchar_t* upChar, const wchar_t* dnChar, int mode);	//	対括弧の前方検索
	bool  SearchBracketBackward(Point ptPos, Point* pptLayoutNew,
						const wchar_t* dnChar, const wchar_t* upChar, int mode);	//	対括弧の後方検索
	void DrawBracketPair(bool);								// 対括弧の強調表示
	bool IsBracket(const wchar_t*, int, int);					// 括弧判定

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           補完                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 支援
	void PreprocessCommand_hokan(int nCommand);
	void PostprocessCommand_hokan(void);

	// 補完ウィンドウを表示する。Ctrl+Spaceや、文字の入力/削除時に呼び出されます。
	void ShowHokanMgr(NativeW& memData, bool bAutoDecided);

	size_t HokanSearchByFile(const wchar_t*, bool, vector_ex<std::wstring>&, int);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ジャンプ                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool TagJumpSub(
		const TCHAR* pszJumpToFile,
		Point ptJumpTo,
		bool bClose = false,
		bool bRelFromIni = false,
		bool* pbJumpToSelf = nullptr
	);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         メニュー                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int	CreatePopUpMenu_R(void);		// ポップアップメニュー(右クリック)
	int	CreatePopUpMenuSub(HMENU hMenu, int nMenuIdx, int* pParentMenus);		// ポップアップメニュー



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           DIFF                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AnalyzeDiffInfo(const char*, int);	// DIFF情報の解析
	bool MakeDiffTmpFile(TCHAR*, HWND, EncodingType, bool);		// DIFF一時ファイル作成
	bool MakeDiffTmpFile2(TCHAR*, const TCHAR*, EncodingType, EncodingType);
	void ViewDiffInfo(const TCHAR*, const TCHAR*, int, bool);		// DIFF差分表示

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           履歴                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AddCurrentLineToHistory(void);	// 現在行を履歴に追加する


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          その他                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool OPEN_ExtFromtoExt(bool, bool, const TCHAR* [], const TCHAR* [], int, int, const TCHAR*); // 指定拡張子のファイルに対応するファイルを開く補助関数
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode(int* newKetas);
	void SmartIndent_CPP(wchar_t);				// C/C++スマートインデント処理
	// コマンド操作
	void SetFont(void);							// フォントの変更
	void SplitBoxOnOff(bool, bool, bool);		// 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ

	bool  ShowKeywordHelp(POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin);	// 補完ウィンドウ用のキーワードヘルプ表示
	void SetUndoBuffer(bool bPaintLineNumber = false);			// アンドゥバッファの処理
	HWND StartProgress();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         アクセサ                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 主要構成部品アクセス
	TextArea& GetTextArea() { assert(pTextArea); return *pTextArea; }
	const TextArea& GetTextArea() const { assert(pTextArea); return *pTextArea; }
	Caret& GetCaret() { assert(pCaret); return *pCaret; }
	const Caret& GetCaret() const { assert(pCaret); return *pCaret; }
	Ruler& GetRuler() { assert(pRuler); return *pRuler; }
	const Ruler& GetRuler() const { assert(pRuler); return *pRuler; }

	// 主要属性アクセス
	TextMetrics& GetTextMetrics() { return textMetrics; }
	const TextMetrics& GetTextMetrics() const { return textMetrics; }
	ViewSelect& GetSelectionInfo() { return viewSelect; }
	const ViewSelect& GetSelectionInfo() const { return viewSelect; }

	// 主要オブジェクトアクセス
	ViewFont& GetFontset() { assert(pViewFont); return *pViewFont; }
	const ViewFont& GetFontset() const { assert(pViewFont); return *pViewFont; }

	// 主要ヘルパアクセス
	const ViewParser& GetParser() const { return parser; }
	const TextDrawer& GetTextDrawer() const { return textDrawer; }
	ViewCommander& GetCommander() { return commander; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 参照
	EditWnd&			editWnd;	// ウィンドウ
	EditDoc*			pEditDoc;	// ドキュメント
	const TypeConfig*	pTypeData;

	// 主要構成部品
	TextArea*		pTextArea;
	Caret*			pCaret;
	Ruler*			pRuler;

	// 主要属性
	TextMetrics		textMetrics;
	ViewSelect		viewSelect;

	// 主要オブジェクト
	ViewFont*		pViewFont;

	// 主要ヘルパ
	ViewParser		parser;
	TextDrawer		textDrawer;
	ViewCommander	commander;

public:
	// ウィンドウ
	HWND			hwndParent;			// 親ウィンドウハンドル
	HWND			hwndVScrollBar;		// 垂直スクロールバーウィンドウハンドル
	int				nVScrollRate;		// 垂直スクロールバーの縮尺
	HWND			hwndHScrollBar;		// 水平スクロールバーウィンドウハンドル
	HWND			hwndSizeBox;		// サイズボックスウィンドウハンドル
	SplitBoxWnd*	pcsbwVSplitBox;		// 垂直分割ボックス
	SplitBoxWnd*	pcsbwHSplitBox;		// 水平分割ボックス
	AutoScrollWnd	autoScrollWnd;		// オートスクロール

public:
	// 描画
	bool			bDrawSwitch;
	COLORREF		crBack;				// テキストの背景色
	COLORREF		crBack2;			// テキストの背景(キャレット用)
	int				nOldUnderLineY;		// 前回作画したカーソルアンダーラインの位置 0未満=非表示
	int				nOldUnderLineYBg;
	int				nOldUnderLineYMargin;
	int				nOldUnderLineYHeight;
	int				nOldUnderLineYHeightReal;
	int				nOldCursorLineX;		// 前回作画したカーソル位置縦線の位置
	int				nOldCursorVLineWidth;	// カーソル位置縦線の太さ(px)

public:
	// 画面バッファ
	HDC				hdcCompatDC;		// 再描画用コンパチブルＤＣ
	HBITMAP			hbmpCompatBMP;		// 再描画用メモリＢＭＰ
	HBITMAP			hbmpCompatBMPOld;	// 再描画用メモリＢＭＰ(OLD)
	int				nCompatBMPWidth;	// 再作画用メモリＢＭＰの幅
	int				nCompatBMPHeight;	// 再作画用メモリＢＭＰの高さ

public:
	// D&D
	DropTarget*		pDropTarget;
	bool			bDragMode;					// 選択テキストのドラッグ中か
	CLIPFORMAT		cfDragData;					// ドラッグデータのクリップ形式
	bool			bDragBoxData;				// ドラッグデータは矩形か
	Point			ptCaretPos_DragEnter;		// ドラッグ開始時のカーソル位置
	int				nCaretPosX_Prev_DragEnter;	// ドラッグ開始時のX座標記憶

	// 括弧
	Point			ptBracketCaretPos_PHY;		// 前カーソル位置の括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	Point			ptBracketPairPos_PHY;		// 対括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	bool			bDrawBracketPairFlag;		// 対括弧の強調表示を行なうか

	// マウス
	bool			bActivateByMouse;		// マウスによるアクティベート
	DWORD			dwTripleClickCheck;		// トリプルクリックチェック用時刻
	Point			mouseDownPos;			// クリック時のマウス座標
	int				nWheelDelta;			// ホイール変化量
	EFunctionCode	eWheelScroll; 			// スクロールの種類
	int				nMousePouse;			// マウス停止時間
	Point			mousePousePos;			// マウスの停止位置
	bool			bHideMouse;

	int				nAutoScrollMode;			// オートスクロールモード
	bool			bAutoScrollDragMode;		// ドラッグモード
	Point			autoScrollMousePos;			// オートスクロールのマウス基準位置
	bool			bAutoScrollVertical;		// 垂直スクロール可
	bool			bAutoScrollHorizontal;		// 水平スクロール可

	// 検索
	SearchStringPattern searchPattern;
	mutable Bregexp		curRegexp;				// コンパイルデータ
	bool				bCurSrchKeyMark;		// 検索文字列のマーク
	bool				bCurSearchUpdate;		// コンパイルデータ更新要求
	int					nCurSearchKeySequence;	// 検索キーシーケンス
	std::wstring		strCurSearchKey;		// 検索文字列
	SearchOption		curSearchOption;		// 検索／置換  オプション
	Point				ptSrchStartPos_PHY;		// 検索/置換開始時のカーソル位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	bool				bSearch;				// 検索/置換開始位置を登録するか
	SearchDirection		nISearchDirection;
	int					nISearchMode;
	bool				bISearchWrap;
	bool				bISearchFlagHistory[256];
	int					nISearchHistoryCount;
	bool				bISearchFirst;
	Range				searchHistory[256];

	// マクロ
	bool			bExecutingKeyMacro;		// キーボードマクロの実行中
	bool			bCommandRunning;		// コマンドの実行中

	// 入力補完
	bool			bHokan;					//	補完中か？＝補完ウィンドウが表示されているか？かな？

	// 編集
	bool			bDoing_UndoRedo;		// Undo, Redoの実行中か

	// 辞書Tip関連
	DWORD			dwTipTimer;				// Tip起動タイマー
	TipWnd			tipWnd;					// Tip表示ウィンドウ
	POINT			poTipCurPos;			// Tip起動時のマウスカーソル位置
	bool			bInMenuLoop;			// メニュー モーダル ループに入っています
	DicMgr			dicMgr;					// 辞書マネージャ

	TCHAR			szComposition[512];		// IMR_DOCUMENTFEED用入力中文字列データ

	// IME
private:
	UINT			uMSIMEReconvertMsg;
	UINT			uATOKReconvertMsg;
public:
	UINT			uWM_MSIME_RECONVERTREQUEST;
private:
	int				nLastReconvLine;		// 再変換情報保存用;
	ptrdiff_t		nLastReconvIndex;		// 再変換情報保存用;

public:
	// ATOK専用再変換のAPI
	typedef BOOL (WINAPI *FP_ATOK_RECONV)(HIMC, int, PRECONVERTSTRING, DWORD);
	HMODULE			hAtokModule;
	FP_ATOK_RECONV	AT_ImmSetReconvertString;

	// その他
	AutoMarkMgr*	pHistory;			//	Jump履歴
	RegexKeyword*	pRegexKeyword;		//
	int				nMyIndex;			// 分割状態
	Migemo*			pMigemo;
	bool			bMiniMap;
	bool			bMiniMapMouseDown;
	int				nPageViewTop;
	int				nPageViewBottom;

private:
	DISALLOW_COPY_AND_ASSIGN(EditView);
};

class OutputAdapter
{
public:
	OutputAdapter(){};
	virtual  ~OutputAdapter(){};

	virtual bool OutputW(const wchar_t* pBuf, int size = -1) = 0;
	virtual bool OutputA(const char* pBuf, int size = -1) = 0;
	virtual bool IsEnableRunningDlg(){ return true; }
	virtual bool IsActiveDebugWindow(){ return true; }
};


