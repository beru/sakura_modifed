/*!	@file
	@brief 印刷プレビュー管理クラス
*/
#pragma once

#include <Windows.h>
#include "basis/SakuraBasis.h"
#include "util/design_template.h"
#include "Print.h"

class ColorStrategy;
class ColorStrategyPool;
class DlgCancel;
class EditWnd;
class Layout;
class LayoutMgr;

class PrintPreview {
// メンバ関数宣言
public:
	/*
	||  コンストラクタ
	*/
	PrintPreview(class EditWnd& parentWnd);
	~PrintPreview();
	
	/*
	||	イベント
	*/
	//	Window Messages
	LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);	// 描画処理
	LRESULT OnSize(WPARAM, LPARAM);				// WM_SIZE 処理
	LRESULT OnVScroll(WPARAM wParam, LPARAM lParam);
	LRESULT OnHScroll(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);

	//	User Messages
	void OnChangeSetting();
	void OnChangePrintSetting(void);
	void OnPreviewGoPage(int nPage);	// プレビュー ページ指定
	void OnPreviewGoPreviousPage() { OnPreviewGoPage(nCurPageNum - 1); }		//	前のページへ
	void OnPreviewGoNextPage() { OnPreviewGoPage(nCurPageNum + 1); }		//	前のページへ
	void OnPreviewGoDirectPage(void);
	void OnPreviewZoom(BOOL bZoomUp);
	void OnPrint(void);	// 印刷実行
	bool OnPrintPageSetting(void);
	void OnCheckAntialias(void);

	/*
	||	コントロール
	*/
	//	スクロールバー
	void InitPreviewScrollBar(void);
	
	//	PrintPreviewバー（画面上部のコントロール）
	void CreatePrintPreviewControls(void);
	void DestroyPrintPreviewControls(void);

	void SetFocusToPrintPreviewBar(void);
	HWND GetPrintPreviewBarHANDLE(void) { return hwndPrintPreviewBar;	}
	HWND GetPrintPreviewBarHANDLE_Safe() const { if (!this) return NULL; else return hwndPrintPreviewBar; } // thisがNULLでも実行できる版。2007.10.29 kobake
	
	//	PrintPreviewバーのメッセージ処理。
	//	まずPrintPreviewBar_DlgProcにメッセージが届き、DispatchEvent_PPBに転送する仕組み
	static INT_PTR CALLBACK PrintPreviewBar_DlgProc(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);
	INT_PTR DispatchEvent_PPB(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam 		// second message parameter
	);

protected:
	/*
	||	描画。
	||	DrawXXXXX()は、現在のフォントを半角フォントに設定してから呼び出すこと。
	||	また、DrawXXXXX()から抜けてきたときは、半角フォントに設定されていることを期待してよい。
	||	フォントは、半角フォントと全角フォントしかないことも期待してよい。
	*/
	void DrawHeaderFooter(HDC hdc, const Rect& rect , bool bHeader);
	ColorStrategy* DrawPageTextFirst(int nPageNum);
	ColorStrategy* DrawPageText(HDC, int, int, int nPageNum, DlgCancel*, ColorStrategy* pStrategyStart);

	// 印刷／プレビュー 行描画
	ColorStrategy* Print_DrawLine(
		HDC				hdc,
		POINT			ptDraw,		// 描画座標。HDC内部単位。
		const wchar_t*	pLine,
		size_t			nDocLineLen,
		size_t			nLineStart,
		size_t			nLineLen,
		size_t			nIndent,	// 折り返しインデント桁数
		const Layout*	pLayout = nullptr,	// 色付用Layout
		ColorStrategy*	pStrategyStart = nullptr
	);

	// 印刷／プレビュー ブロック描画
	void Print_DrawBlock(
		HDC				hdc,
		POINT			ptDraw,		// 描画座標。HDC内部単位。
		const wchar_t*	pPhysicalLine,
		int				nBlockLen,
		int				nKind,		// 0:半角, 1:全角
		const Layout*	pLayout,	// 色設定用Layout
		int				nColorIndex,
		int				nBgnPhysical,
		int				nLayoutX,
		int				nDx,
		const int*		pDxArray
	);

	// 指定ロジック位置のColorStrategyを取得
	ColorStrategy* GetColorStrategy(
		const StringRef&	stringLine,
		size_t				iLogic,
		ColorStrategy*		pStrategy,
		bool&				bChange
	);

	// 印刷用フォントを作成する
	void CreateFonts(HDC hdc);
	// 印刷用フォントを破棄する
	void DestroyFonts();

public:
	//	フォント列挙
	static int CALLBACK MyEnumFontFamProc(
		ENUMLOGFONT*	pelf,		// pointer to logical-font data
		NEWTEXTMETRIC*	pntm,		// pointer to physical-font data
		int				nFontType,	// type of font
		LPARAM			lParam 		// address of application-defined data
	);

	/*
	||	アクセサ
	*/
	void SetPrintSetting(PrintSetting* pPrintSetting) {
		sPrintSetting = *pPrintSetting;
		this->pPrintSetting = &sPrintSetting;
		this->pPrintSettingOrg = pPrintSetting;
	}
	BOOL GetDefaultPrinterInfo() { return print.GetDefaultPrinter(&pPrintSetting->mdmDevMode); }
	int  GetCurPageNum() { return nCurPageNum; }	// 現在のページ
	int  GetAllPageNum() { return nAllPageNum; }	// 現在のページ
	
	/*
	||	ヘッダ・フッタ
	*/
	void SetHeader(char* pszWork[]);	//	&fなどを登録
	void SetFooter(char* pszWork[]);	//	&p/&Pなどを登録

protected:
	void SetPreviewFontHan(const LOGFONT* lf);
	void SetPreviewFontZen(const LOGFONT* lf);

// メンバ変数宣言
public:
	// none

protected:
	EditWnd&		parentWnd;	//	親のEditEnd。

	HDC				hdcCompatDC;		// 再描画用コンパチブルDC
	HBITMAP			hbmpCompatBMP;		// 再描画用メモリBMP
	HBITMAP			hbmpCompatBMPOld;	// 再描画用メモリBMP(OLD)
	int				nbmpCompatScale;	// BMPの画面の10(COMPAT_BMP_BASE)ピクセル幅あたりのBMPのピクセル幅

	//	コントロール制御用
	//	操作バー
	HWND			hwndPrintPreviewBar;	// 印刷プレビュー 操作バー
	//	スクロールバー
	int				nPreviewVScrollPos;	// 印刷プレビュー：スクロール位置縦
	int				nPreviewHScrollPos;	// 印刷プレビュー：スクロール位置横
	BOOL			SCROLLBAR_HORZ;
	BOOL			SCROLLBAR_VERT;
	HWND			hwndVScrollBar;		// 垂直スクロールバーウィンドウハンドル
	HWND			hwndHScrollBar;		// 水平スクロールバーウィンドウハンドル
	//	サイズボックス
	HWND			hwndSizeBox;		// サイズボックスウィンドウハンドル
	BOOL			sizeBoxCanMove;		// サイズボックスウィンドウハンドルを動かせるかどうか

	//	表示
	int				nPreview_Zoom;	// 印刷プレビュー：倍率

	//	印刷位置を決定するための変数
	int				nPreview_ViewWidth;			// 印刷プレビュー：ビュー幅(ピクセル)
	int				nPreview_ViewHeight;		// 印刷プレビュー：ビュー高さ(ピクセル)
	int				nPreview_ViewMarginLeft;	// 印刷プレビュー：ビュー左端と用紙の間隔(1/10mm単位)
	int				nPreview_ViewMarginTop;		// 印刷プレビュー：ビュー左端と用紙の間隔(1/10mm単位)
	short			nPreview_PaperAllWidth;		// 用紙幅(1/10mm単位)
	short			nPreview_PaperAllHeight;	// 用紙高さ(1/10mm単位)
	short			nPreview_PaperWidth;		// 用紙印刷有効幅(1/10mm単位)
	short			nPreview_PaperHeight;		// 用紙印刷有効高さ(1/10mm単位)
	short			nPreview_PaperOffsetLeft;	// 用紙余白左端(1/10mm単位)
	short			nPreview_PaperOffsetTop;	// 用紙余白上端(1/10mm単位)
	int				bPreview_EnableColumns;		// 印字可能桁数/ページ
	int				bPreview_EnableLines;		// 印字可能行数/ページ
	int				nPreview_LineNumberColumns;	// 行番号エリアの幅（文字数）
	WORD			nAllPageNum;				// 全ページ数
	WORD			nCurPageNum;				// 現在のページ

	PrintSetting*	pPrintSetting;				// 現在の印刷設定(キャッシュへのポインタ)
	PrintSetting*	pPrintSettingOrg;			// 現在の印刷設定(共有データ)
	PrintSetting	sPrintSetting;				// 現在の印刷設定(キャッシュ)
	LOGFONT			lfPreviewHan;				// プレビュー用フォント
	LOGFONT			lfPreviewZen;				// プレビュー用フォント

	HFONT			hFontHan;					// 印刷用半角フォントハンドル
	HFONT			hFontHan_b;					// 印刷用半角フォントハンドル 太字
	HFONT			hFontHan_u;					// 印刷用半角フォントハンドル 下線
	HFONT			hFontHan_bu;				// 印刷用半角フォントハンドル 太字、下線
	HFONT			hFontZen;					// 印刷用全角フォントハンドル
	HFONT			hFontZen_b;					// 印刷用全角フォントハンドル 太字
	HFONT			hFontZen_u;					// 印刷用全角フォントハンドル 下線
	HFONT			hFontZen_bu;				// 印刷用全角フォントハンドル 太字、下線
	int				nAscentHan;					// 半角文字のアセント（文字高/基準ラインからの高さ）
	int				nAscentZen;					// 全角文字のアセント（文字高/基準ラインからの高さ）

	ColorStrategyPool*	pool;					// 色定義管理情報

public:
	class LayoutMgr*	pLayoutMgr_Print;		// 印刷用のレイアウト管理情報
protected:
	TypeConfig typePrint;

	// プレビューから出ても現在のプリンタ情報を記憶しておけるようにstaticにする 2003.05.02 かろと 
	static Print	print;						// 現在のプリンタ情報

	bool			bLockSetting;				// 設定のロック
	bool			bDemandUpdateSetting;		// 設定の更新要求

private:
	DISALLOW_COPY_AND_ASSIGN(PrintPreview);
};

