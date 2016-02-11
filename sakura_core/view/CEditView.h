/*!	@file
	@brief 文書ウィンドウの管理

	@author Norio Nakatani
	@date	1998/03/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, asa-o, MIK, hor, Misaka, Stonee, YAZAKI
	Copyright (C) 2002, genta, hor, YAZAKI, Azumaiya, KK, novice, minfu, ai, aroka, MIK
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, Moca, novice, Kazika, isearch
	Copyright (C) 2005, genta, Moca, MIK, ryoji, maru
	Copyright (C) 2006, genta, aroka, fon, yukihane, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji

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

#include <Windows.h>
#include <ObjIdl.h>  // LPDATAOBJECT
#include <ShellAPI.h>  // HDROP
#include "CTextMetrics.h"
#include "CTextDrawer.h"
#include "CTextArea.h"
#include "CCaret.h"
#include "CViewCalc.h" // parent
#include "CEditView_Paint.h"	// parent
#include "CViewParser.h"
#include "CViewSelect.h"
#include "CSearchAgent.h"
#include "view/colors/EColorIndexType.h"
#include "window/CTipWnd.h"
#include "window/CAutoScrollWnd.h"
#include "CDicMgr.h"
//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
#include "extmodule/CBregexp.h"
#include "CEol.h"				// EolType
#include "cmd/CViewCommander.h"
#include "mfclike/CMyWnd.h"		// parent
#include "doc/CDocListener.h"	// parent
#include "basis/SakuraBasis.h"	// LogicInt, LayoutInt
#include "util/container.h"		// vector_ex
#include "util/design_template.h"

class ViewFont;
class Ruler;
class DropTarget; /// 2002/2/3 aroka ヘッダ軽量化
class OpeBlk;///
class SplitBoxWnd;///
class RegexKeyword;///
class AutoMarkMgr; /// 2002/2/3 aroka ヘッダ軽量化 to here
class EditDoc;	//	2002/5/13 YAZAKI ヘッダ軽量化
class Layout;	//	2002/5/13 YAZAKI ヘッダ軽量化
class Migemo;	// 2004.09.14 isearch
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
#define IDM_JUMPDICT 2001	// 2006.04.10 fon
#endif

///	マウスからコマンドが実行された場合の上位ビット
///	@date 2006.05.19 genta
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
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
// 2007.08.25 kobake 文字間隔配列の機能をTextMetricsに移動
// 2007.10.02 kobake Command_TRIM2をCConvertに移動

class EditView :
	public ViewCalc, //$$ これが親クラスである必要は無いが、このクラスのメソッド呼び出しが多いので、暫定的に親クラスとする。
	public EditView_Paint,
	public MyWnd,
	public DocListenerEx
{
public:
	const EditDoc* GetDocument() const {
		return m_pcEditDoc;
	}
	EditDoc* GetDocument() {
		return m_pcEditDoc;
	}
public:
	//! 背景にビットマップを使用するかどうか
	//! 2010.10.03 背景実装
	bool IsBkBitmap() const { return m_pcEditDoc->m_hBackImg != NULL; }

public:
	EditView* GetEditView() {
		return this;
	}
	const EditView* GetEditView() const {
		return this;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        生成と破棄                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// Constructors
	EditView(EditWnd* pcEditWnd);
	~EditView();
	void Close();
	// 初期化系メンバ関数
	BOOL Create(
		HWND		hwndParent,	//!< 親
		EditDoc*	pcEditDoc,	//!< 参照するドキュメント
		int			nMyIndex,	//!< ビューのインデックス
		BOOL		bShow,		//!< 作成時に表示するかどうか
		bool		bMiniMap
	);
	void CopyViewStatus(EditView*) const;					// 自分の表示状態を他のビューにコピー

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      クリップボード                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 取得
	bool MyGetClipboardData(NativeW&, bool*, bool* = NULL);			// クリップボードからデータを取得

	// 設定
	bool MySetClipboardData(const ACHAR*, int, bool bColumnSelect, bool = false);	// クリップボードにデータを設定
	bool MySetClipboardData(const WCHAR*, int, bool bColumnSelect, bool = false);	// クリップボードにデータを設定

	// 利用
	void CopyCurLine(bool bAddCRLFWhenCopy, EolType neweol, bool bEnableLineModePaste);	// カーソル行をクリップボードにコピーする	// 2007.10.08 ryoji
	void CopySelectedAllLines(const wchar_t*, bool);			// 選択範囲内の全行をクリップボードにコピーする


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         イベント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ドキュメントイベント
	void OnAfterLoad(const LoadInfo& sLoadInfo);
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
	LayoutInt OnVScroll(int, int);					// 垂直スクロールバーメッセージ処理
	LayoutInt OnHScroll(int, int);					// 水平スクロールバーメッセージ処理
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
	void OnXLBUTTONUP(WPARAM, int, int);			// マウスサイドボタン1開放		// 2009.01.17 nasukoji
	void OnXRBUTTONDOWN(WPARAM, int, int);			// マウスサイドボタン2押下
	void OnXRBUTTONUP(WPARAM, int, int);			// マウスサイドボタン2開放		// 2009.01.17 nasukoji
	LRESULT OnMOUSEWHEEL(WPARAM, LPARAM);			//!< 垂直マウスホイールのメッセージ処理
	LRESULT OnMOUSEHWHEEL(WPARAM, LPARAM);			//!< 水平マウスホイールのメッセージ処理
	LRESULT OnMOUSEWHEEL2(WPARAM, LPARAM, bool, EFunctionCode);		//!< マウスホイールのメッセージ処理
	bool IsSpecialScrollMode(int);					// キー・マウスボタン状態よりスクロールモードを判定する		// 2009.01.17 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2006.05.14 Moca  互換BMPによる画面バッファ
	// 2007.09.30 genta CompatibleDC操作関数
protected:
	//! ロジック行を1行描画
	bool DrawLogicLine(
		HDC				hdc,			//!< [in]     作画対象
		DispPos*		pDispPos,		//!< [in/out] 描画する箇所、描画元ソース
		LayoutInt		nLineTo			//!< [in]     作画終了するレイアウト行番号
	);

	//! レイアウト行を1行描画
	bool DrawLayoutLine(ColorStrategyInfo* pInfo);

	// 色分け
public:
	Color3Setting GetColorIndex(const Layout* pcLayout, LayoutYInt nLineNum, int nIndex, ColorStrategyInfo* pInfo, bool bPrev = false);	// 指定位置のColorIndexの取得 02/12/13 ai
	void SetCurrentColor(Graphics& gr, EColorIndexType, EColorIndexType, EColorIndexType);
	COLORREF GetTextColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2);
	COLORREF GetBackColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2);

	// 画面バッファ
protected:
	bool CreateOrUpdateCompatibleBitmap(int cx, int cy);	//!< メモリBMPを作成または更新
	void UseCompatibleDC(BOOL fCache);
public:
	void DeleteCompatibleBitmap();							//!< メモリBMPを削除

public:
	void DispTextSelected(HDC hdc, LayoutInt nLineNum, const Point& ptXY, LayoutInt nX_Layout);	// テキスト反転
	void RedrawAll();										// フォーカス移動時の再描画
	void Redraw();											// 2001/06/21 asa-o 再描画
	void RedrawLines( LayoutYInt top, LayoutYInt bottom );
	void CaretUnderLineON(bool, bool, bool);				// カーソル行アンダーラインのON
	void CaretUnderLineOFF(bool, bool, bool, bool);			// カーソル行アンダーラインのOFF
	bool GetDrawSwitch() const {
		return m_bDrawSWITCH;
	}
	bool SetDrawSwitch(bool b) {
		bool bOld = m_bDrawSWITCH;
		m_bDrawSWITCH = b;
		return bOld;
	}
	bool IsDrawCursorVLinePos(int);
	void DrawBracketCursorLine(bool);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        スクロール                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AdjustScrollBars();											// スクロールバーの状態を更新する
	BOOL CreateScrollBar();												// スクロールバー作成	// 2006.12.19 ryoji
	void DestroyScrollBar();											// スクロールバー破棄	// 2006.12.19 ryoji
	LayoutInt GetWrapOverhang(void) const;								// 折り返し桁以後のぶら下げ余白計算	// 2008.06.08 ryoji
	LayoutInt ViewColNumToWrapColNum(LayoutInt nViewColNum) const;	//「右端で折り返す」用にビューの桁数から折り返し桁数を計算する	// 2008.06.08 ryoji
	LayoutInt GetRightEdgeForScrollBar(void);							// スクロールバー制御用に右端座標を取得する		// 2009.08.28 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           IME                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Aug. 25, 2002 genta protected->publicに移動
	bool IsImeON(void);	// IME ONか	// 2006.12.04 ryoji
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        スクロール                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LayoutInt  ScrollAtV(LayoutInt);								// 指定上端行位置へスクロール
	LayoutInt  ScrollAtH(LayoutInt);								// 指定左端桁位置へスクロール
	//	From Here Sep. 11, 2004 genta ずれ維持の同期スクロール
	LayoutInt  ScrollByV(LayoutInt vl) {	return ScrollAtV(GetTextArea().GetViewTopLine() + vl);}	// 指定行スクロール
	LayoutInt  ScrollByH(LayoutInt hl) {	return ScrollAtH(GetTextArea().GetViewLeftCol() + hl);}	// 指定桁スクロール
	void ScrollDraw(LayoutInt, LayoutInt, const RECT&, const RECT&, const RECT&);
	void MiniMapRedraw(bool);
public:
	void SyncScrollV(LayoutInt);										// 垂直同期スクロール
	void SyncScrollH(LayoutInt);										// 水平同期スクロール

	void SetBracketPairPos(bool);										// 対括弧の強調表示位置設定 03/02/18 ai

	void AutoScrollEnter();
	void AutoScrollExit();
	void AutoScrollMove(Point& point);
	void AutoScrollOnTimer();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        過去の遺産                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void SetIMECompFormPos(void);								// IME編集エリアの位置を変更
	void SetIMECompFormFont(void);							// IME編集エリアの表示フォントを変更
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       テキスト選択                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 2002/01/19 novice public属性に変更
	bool GetSelectedDataSimple(NativeW&);// 選択範囲のデータを取得
	bool GetSelectedDataOne(NativeW& cmemBuf, int nMaxLen);
	bool GetSelectedData(NativeW*, bool, const wchar_t*, bool, bool bAddCRLFWhenCopy, EolType neweol = EolType::Unknown);	// 選択範囲のデータを取得
	int IsCurrentPositionSelected(LayoutPoint ptCaretPos);					// 指定カーソル位置が選択エリア内にあるか
	int IsCurrentPositionSelectedTEST(const LayoutPoint& ptCaretPos, const LayoutRange& sSelect) const; // 指定カーソル位置が選択エリア内にあるか
	// 2006.07.09 genta 行桁指定によるカーソル移動(選択領域を考慮)
	void MoveCursorSelecting(LayoutPoint ptWk_CaretPos, bool bSelect, int = _CARETMARGINRATE);
	void ConvSelectedArea(EFunctionCode);								// 選択エリアのテキストを指定方法で変換
	//!指定位置または指定範囲がテキストの存在しないエリアかチェックする		// 2008.08.03 nasukoji
	bool IsEmptyArea(LayoutPoint ptFrom, LayoutPoint ptTo = LayoutPoint(LayoutInt(-1), LayoutInt(-1)), bool bSelect = false, bool bBoxSelect = false) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         各種判定                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool IsCurrentPositionURL(const LayoutPoint& ptCaretPos, LogicRange* pUrlRange, std::wstring* pwstrURL); // カーソル位置にURLが有る場合のその範囲を調べる
	bool CheckTripleClick(Point ptMouse);							// トリプルクリックをチェックする	// 2007.10.02 nasukoji
	
	bool ExecCmd(const TCHAR*, int, const TCHAR*, OutputAdapter* = NULL) ;			// 子プロセスの標準出力をリダイレクトする
	void AddToCmdArr(const TCHAR*);
	BOOL ChangeCurRegexp(bool bRedrawIfChanged = true);									// 2002.01.16 hor 正規表現の検索パターンを必要に応じて更新する(ライブラリが使用できないときはFALSEを返す)
	void SendStatusMessage(const TCHAR* msg);					// 2002.01.26 hor 検索／置換／ブックマーク検索時の状態をステータスバーに表示する
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode, bool bDocumentFeed = false);	// 再変換用構造体を設定する 2002.04.09 minfu
	LRESULT SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode);				// 再変換用構造体の情報を元に選択範囲を変更する 2002.04.09 minfu

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           D&D                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public: // テスト用にアクセス属性を変更
	// IDropTarget実装
	STDMETHODIMP DragEnter(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD);
	STDMETHODIMP DragLeave(void);
	STDMETHODIMP Drop(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
	STDMETHODIMP PostMyDropFiles(LPDATAOBJECT pDataObject);		// 独自ドロップファイルメッセージをポストする	// 2008.06.20 ryoji
	void OnMyDropFiles(HDROP hDrop);								// 独自ドロップファイルメッセージ処理			// 2008.06.20 ryoji
	CLIPFORMAT GetAvailableClipFormat(LPDATAOBJECT pDataObject);
	DWORD TranslateDropEffect(CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect);
	bool IsDragSource(void);

	void _SetDragMode(bool b) {
		m_bDragMode = b;
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           編集                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 指定位置の指定長データ削除
	void DeleteData2(
		const LayoutPoint&	ptCaretPos,
		LogicInt			nDelLen,
		NativeW*			pcMem
	);

	// 現在位置のデータ削除
	void DeleteData(bool bRedraw);

	// 現在位置にデータを挿入
	void InsertData_CEditView(
		LayoutPoint	ptInsertPos,
		const wchar_t*	pData,
		int				nDataLen,
		LayoutPoint*	pptNewPos,	// 挿入された部分の次の位置のデータ位置
		bool			bRedraw
	);

	// データ置換 削除&挿入にも使える
	void ReplaceData_CEditView(
		const LayoutRange&	sDelRange,			// 削除範囲。レイアウト単位。
		const wchar_t*		pInsData,			// 挿入するデータ
		LogicInt			nInsDataLen,		// 挿入するデータの長さ
		bool				bRedraw,
		OpeBlk*			pcOpeBlk,
		bool				bFastMode = false,
		const LogicRange*	psDelRangeLogicFast = NULL
	);
	void ReplaceData_CEditView2(
		const LogicRange&	sDelRange,			// 削除範囲。ロジック単位。
		const wchar_t*		pInsData,			// 挿入するデータ
		LogicInt			nInsDataLen,		// 挿入するデータの長さ
		bool				bRedraw,
		OpeBlk*			pcOpeBlk,
		bool				bFastMode = false
	);
	bool ReplaceData_CEditView3(
		LayoutRange	sDelRange,			// 削除範囲。レイアウト単位。
		OpeLineData*	pcmemCopyOfDeleted,	// 削除されたデータのコピー(NULL可能)
		OpeLineData*	pInsData,
		bool			bRedraw,
		OpeBlk*		pcOpeBlk,
		int				nDelSeq,
		int*			pnInsSeq,
		bool			bFastMode = false,
		const LogicRange*	psDelRangeLogicFast = NULL
	);
	void RTrimPrevLine(void);		// 2005.10.11 ryoji 前の行にある末尾の空白を削除

	//	Oct. 2, 2005 genta 挿入モードの設定・取得
	bool IsInsMode() const;
	void SetInsMode(bool);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           検索                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 2004.10.13 インクリメンタルサーチ関係
	void TranslateCommand_isearch(EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM&);
	bool ProcessCommand_isearch(int, bool, LPARAM, LPARAM, LPARAM, LPARAM);

	//	Jan. 10, 2005 genta HandleCommandからgrep関連処理を分離
	void TranslateCommand_grep(EFunctionCode&, bool&, LPARAM&, LPARAM&, LPARAM&, LPARAM&);

	//	Jan. 10, 2005 インクリメンタルサーチ
	bool IsISearchEnabled(int nCommand) const;

	bool KeySearchCore(const NativeW* pcmemCurText);	// 2006.04.10 fon
	bool MiniMapCursorLineTip(POINT* po, RECT* rc, bool* pbHide);

	/*!	EditView::KeyWordHelpSearchDictのコール元指定用ローカルID
		@date 2006.04.10 fon 新規作成
	*/
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	//!< EditView::OnTimer
		LID_SKH_POPUPMENU_R = 2,	//!< EditView::CreatePopUpMenu_R
	};
	BOOL KeyWordHelpSearchDict(LID_SKH nID, POINT* po, RECT* rc);	// 2006.04.10 fon

	int IsSearchString(const StringRef& cStr, LogicInt, LogicInt*, LogicInt*) const;	// 現在位置が検索文字列に該当するか	// 2002.02.08 hor 引数追加

	void GetCurrentTextForSearch(NativeW&, bool bStripMaxPath = true, bool bTrimSpaceTab = false);			// 現在カーソル位置単語または選択範囲より検索等のキーを取得
	bool GetCurrentTextForSearchDlg(NativeW&, bool bGetHistory = false);		// 現在カーソル位置単語または選択範囲より検索等のキーを取得（ダイアログ用） 2006.08.23 ryoji

private:
	// インクリメンタルサーチ 
	// 2004.10.24 isearch migemo
	void ISearchEnter(int mode, SearchDirection direction);
	void ISearchExit();
	void ISearchExec(DWORD wChar);
	void ISearchExec(LPCWSTR pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(CNativeT* msg) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           括弧                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Jun. 16, 2000 genta
	bool  SearchBracket(const LayoutPoint& ptPos, LayoutPoint* pptLayoutNew, int* mode);	// 対括弧の検索		// modeの追加 02/09/18 ai
	bool  SearchBracketForward(LogicPoint ptPos, LayoutPoint* pptLayoutNew,
						const wchar_t* upChar, const wchar_t* dnChar, int mode);	//	対括弧の前方検索	// modeの追加 02/09/19 ai
	bool  SearchBracketBackward(LogicPoint ptPos, LayoutPoint* pptLayoutNew,
						const wchar_t* dnChar, const wchar_t* upChar, int mode);	//	対括弧の後方検索	// modeの追加 02/09/19 ai
	void DrawBracketPair(bool);								// 対括弧の強調表示 02/09/18 ai
	bool IsBracket(const wchar_t*, LogicInt, LogicInt);					// 括弧判定 03/01/09 ai

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           補完                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 支援
	//	Jan. 10, 2005 genta HandleCommandから補完関連処理を分離
	void PreprocessCommand_hokan(int nCommand);
	void PostprocessCommand_hokan(void);

	// 補完ウィンドウを表示する。Ctrl+Spaceや、文字の入力/削除時に呼び出されます。 YAZAKI 2002/03/11
	void ShowHokanMgr(NativeW& cmemData, bool bAutoDecided);

	int HokanSearchByFile(const wchar_t*, bool, vector_ex<std::wstring>&, int); // 2003.06.25 Moca


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ジャンプ                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//@@@ 2003.04.13 MIK, Apr. 21, 2003 genta bClose追加
	//	Feb. 17, 2007 genta 相対パスの基準ディレクトリ指示を追加
	bool TagJumpSub(
		const TCHAR* pszJumpToFile,
		Point ptJumpTo,
		bool bClose = false,
		bool bRelFromIni = false,
		bool* pbJumpToSelf = NULL
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
	void AnalyzeDiffInfo(const char*, int);	// DIFF情報の解析	//@@@ 2002.05.25 MIK
	BOOL MakeDiffTmpFile(TCHAR*, HWND, ECodeType, bool);		// DIFF一時ファイル作成	//@@@ 2002.05.28 MIK	// 2005.10.29 maru
	BOOL MakeDiffTmpFile2(TCHAR*, const TCHAR*, ECodeType, ECodeType);
	void ViewDiffInfo(const TCHAR*, const TCHAR*, int, bool);		// DIFF差分表示		// 2005.10.29 maru

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           履歴                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	// 現在行を履歴に追加する


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          その他                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool OPEN_ExtFromtoExt(bool, bool, const TCHAR* [], const TCHAR* [], int, int, const TCHAR*); // 指定拡張子のファイルに対応するファイルを開く補助関数 // 2003.08.12 Moca
	//	Jan.  8, 2006 genta 折り返しトグル動作判定
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode(LayoutInt* newKetas);
	void SmartIndent_CPP(wchar_t);				// C/C++スマートインデント処理
	// コマンド操作
	void SetFont(void);							// フォントの変更
	void SplitBoxOnOff(bool, bool, bool);		// 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ

//	2001/06/18 asa-o
	bool  ShowKeywordHelp(POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin);	// 補完ウィンドウ用のキーワードヘルプ表示
	void SetUndoBuffer(bool bPaintLineNumber = false);			// アンドゥバッファの処理
	HWND StartProgress();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         アクセサ                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 主要構成部品アクセス
	TextArea& GetTextArea() { assert(m_pcTextArea); return *m_pcTextArea; }
	const TextArea& GetTextArea() const { assert(m_pcTextArea); return *m_pcTextArea; }
	Caret& GetCaret() { assert(m_pcCaret); return *m_pcCaret; }
	const Caret& GetCaret() const { assert(m_pcCaret); return *m_pcCaret; }
	Ruler& GetRuler() { assert(m_pcRuler); return *m_pcRuler; }
	const Ruler& GetRuler() const { assert(m_pcRuler); return *m_pcRuler; }

	// 主要属性アクセス
	TextMetrics& GetTextMetrics() { return m_cTextMetrics; }
	const TextMetrics& GetTextMetrics() const { return m_cTextMetrics; }
	ViewSelect& GetSelectionInfo() { return m_cViewSelect; }
	const ViewSelect& GetSelectionInfo() const { return m_cViewSelect; }

	// 主要オブジェクトアクセス
	ViewFont& GetFontset() { assert(m_pcViewFont); return *m_pcViewFont; }
	const ViewFont& GetFontset() const { assert(m_pcViewFont); return *m_pcViewFont; }

	// 主要ヘルパアクセス
	const ViewParser& GetParser() const { return m_cParser; }
	const TextDrawer& GetTextDrawer() const { return m_cTextDrawer; }
	ViewCommander& GetCommander() { return m_cCommander; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 参照
	EditWnd*			m_pcEditWnd;	//!< ウィンドウ
	EditDoc*			m_pcEditDoc;	//!< ドキュメント
	const TypeConfig*	m_pTypeData;

	// 主要構成部品
	TextArea*		m_pcTextArea;
	Caret*			m_pcCaret;
	Ruler*			m_pcRuler;

	// 主要属性
	TextMetrics	m_cTextMetrics;
	ViewSelect		m_cViewSelect;

	// 主要オブジェクト
	ViewFont*		m_pcViewFont;

	// 主要ヘルパ
	ViewParser		m_cParser;
	TextDrawer		m_cTextDrawer;
	ViewCommander	m_cCommander;

public:
	// ウィンドウ
	HWND			m_hwndParent;		// 親ウィンドウハンドル
	HWND			m_hwndVScrollBar;	// 垂直スクロールバーウィンドウハンドル
	int				m_nVScrollRate;		// 垂直スクロールバーの縮尺
	HWND			m_hwndHScrollBar;	// 水平スクロールバーウィンドウハンドル
	HWND			m_hwndSizeBox;		// サイズボックスウィンドウハンドル
	SplitBoxWnd*	m_pcsbwVSplitBox;	// 垂直分割ボックス
	SplitBoxWnd*	m_pcsbwHSplitBox;	// 水平分割ボックス
	AutoScrollWnd	m_cAutoScrollWnd;	//!< オートスクロール

public:
	// 描画
	bool			m_bDrawSWITCH;
	COLORREF		m_crBack;				// テキストの背景色			// 2006.12.07 ryoji
	COLORREF		m_crBack2;				// テキストの背景(キャレット用)
	LayoutInt		m_nOldUnderLineY;		// 前回作画したカーソルアンダーラインの位置 0未満=非表示
	LayoutInt		m_nOldUnderLineYBg;
	int				m_nOldUnderLineYMargin;
	int				m_nOldUnderLineYHeight;
	int				m_nOldUnderLineYHeightReal;
	int				m_nOldCursorLineX;		// 前回作画したカーソル位置縦線の位置 // 2007.09.09 Moca
	int				m_nOldCursorVLineWidth;	// カーソル位置縦線の太さ(px)

public:
	// 画面バッファ
	HDC				m_hdcCompatDC;		// 再描画用コンパチブルＤＣ
	HBITMAP			m_hbmpCompatBMP;	// 再描画用メモリＢＭＰ
	HBITMAP			m_hbmpCompatBMPOld;	// 再描画用メモリＢＭＰ(OLD)
	int				m_nCompatBMPWidth;  // 再作画用メモリＢＭＰの幅	// 2007.09.09 Moca 互換BMPによる画面バッファ
	int				m_nCompatBMPHeight; // 再作画用メモリＢＭＰの高さ	// 2007.09.09 Moca 互換BMPによる画面バッファ

public:
	// D&D
	DropTarget*	m_pcDropTarget;
	bool			m_bDragMode;					// 選択テキストのドラッグ中か
	CLIPFORMAT		m_cfDragData;					// ドラッグデータのクリップ形式	// 2008.06.20 ryoji
	bool			m_bDragBoxData;					// ドラッグデータは矩形か
	LayoutPoint	m_ptCaretPos_DragEnter;			// ドラッグ開始時のカーソル位置	// 2007.12.09 ryoji
	LayoutInt		m_nCaretPosX_Prev_DragEnter;	// ドラッグ開始時のX座標記憶	// 2007.12.09 ryoji

	// 括弧
	LogicPoint		m_ptBracketCaretPos_PHY;	// 前カーソル位置の括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	LogicPoint		m_ptBracketPairPos_PHY;		// 対括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	bool			m_bDrawBracketPairFlag;		// 対括弧の強調表示を行なうか						// 03/02/18 ai

	// マウス
	bool			m_bActivateByMouse;		//!< マウスによるアクティベート	// 2007.10.02 nasukoji
	DWORD			m_dwTripleClickCheck;	//!< トリプルクリックチェック用時刻	// 2007.10.02 nasukoji
	Point		m_cMouseDownPos;		//!< クリック時のマウス座標
	int				m_nWheelDelta;			//!< ホイール変化量
	EFunctionCode	m_eWheelScroll; 		//!< スクロールの種類
	int				m_nMousePouse;			// マウス停止時間
	Point		m_cMousePousePos;		// マウスの停止位置
	bool			m_bHideMouse;

	int				m_nAutoScrollMode;			//!< オートスクロールモード
	bool			m_bAutoScrollDragMode;		//!< ドラッグモード
	Point		m_cAutoScrollMousePos;		//!< オートスクロールのマウス基準位置
	bool			m_bAutoScrollVertical;		//!< 垂直スクロール可
	bool			m_bAutoScrollHorizontal;	//!< 水平スクロール可

	// 検索
	SearchStringPattern m_sSearchPattern;
	mutable Bregexp	m_CurRegexp;				// コンパイルデータ
	bool				m_bCurSrchKeyMark;			// 検索文字列のマーク
	bool				m_bCurSearchUpdate;			// コンパイルデータ更新要求
	int					m_nCurSearchKeySequence;	// 検索キーシーケンス
	std::wstring		m_strCurSearchKey;			// 検索文字列
	SearchOption		m_curSearchOption;			// 検索／置換  オプション
	LogicPoint			m_ptSrchStartPos_PHY;		// 検索/置換開始時のカーソル位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	bool				m_bSearch;					// 検索/置換開始位置を登録するか											// 02/06/26 ai
	SearchDirection	m_nISearchDirection;
	int					m_nISearchMode;
	bool				m_bISearchWrap;
	bool				m_bISearchFlagHistory[256];
	int					m_nISearchHistoryCount;
	bool				m_bISearchFirst;
	LayoutRange		m_sISearchHistory[256];

	// マクロ
	bool			m_bExecutingKeyMacro;	// キーボードマクロの実行中
	bool			m_bCommandRunning;		// コマンドの実行中

	// 入力補完
	bool			m_bHokan;				//	補完中か？＝補完ウィンドウが表示されているか？かな？

	// 編集
	bool			m_bDoing_UndoRedo;		// アンドゥ・リドゥの実行中か

	// 辞書Tip関連
	DWORD			m_dwTipTimer;			// Tip起動タイマー
	TipWnd			m_cTipWnd;				// Tip表示ウィンドウ
	POINT			m_poTipCurPos;			// Tip起動時のマウスカーソル位置
	bool			m_bInMenuLoop;			// メニュー モーダル ループに入っています
	DicMgr			m_cDicMgr;				// 辞書マネージャ

	TCHAR			m_szComposition[512];	// IMR_DOCUMENTFEED用入力中文字列データ

	// IME
private:
	UINT			m_uMSIMEReconvertMsg;
	UINT			m_uATOKReconvertMsg;
public:
	UINT			m_uWM_MSIME_RECONVERTREQUEST;
private:
	int				m_nLastReconvLine;             // 2002.04.09 minfu 再変換情報保存用;
	int				m_nLastReconvIndex;            // 2002.04.09 minfu 再変換情報保存用;

public:
	// ATOK専用再変換のAPI
	typedef BOOL (WINAPI *FP_ATOK_RECONV)(HIMC, int, PRECONVERTSTRING, DWORD);
	HMODULE			m_hAtokModule;
	FP_ATOK_RECONV	m_AT_ImmSetReconvertString;

	// その他
	AutoMarkMgr*	m_cHistory;	//	Jump履歴
	RegexKeyword*	m_cRegexKeyword;	//@@@ 2001.11.17 add MIK
	int				m_nMyIndex;	// 分割状態
	Migemo*		m_pcmigemo;
	bool			m_bMiniMap;
	bool			m_bMiniMapMouseDown;
	LayoutInt		m_nPageViewTop;
	LayoutInt		m_nPageViewBottom;

private:
	DISALLOW_COPY_AND_ASSIGN(EditView);
};



class OutputAdapter
{
public:
	OutputAdapter(){};
	virtual  ~OutputAdapter(){};

	virtual bool OutputW(const WCHAR* pBuf, int size = -1) = 0;
	virtual bool OutputA(const ACHAR* pBuf, int size = -1) = 0;
	virtual bool IsEnableRunningDlg(){ return true; }
	virtual bool IsActiveDebugWindow(){ return true; }
};


