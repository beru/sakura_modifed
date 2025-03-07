#pragma once

#include "Wnd.h"

struct DllSharedData;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/

#define MAXCOUNTOFVIEW	4

/*!
	@brief 分割線ウィンドウクラス
	
	４分割ウィンドウの管理と分割線の描画を行う。
*/
class SplitterWnd : public Wnd {
public:
	/*
	||  Constructors
	*/
	SplitterWnd();
	~SplitterWnd();
private:
	/*
	||  Attributes & Operations
	*/
	DllSharedData*	pShareData;
	EditWnd*		pEditWnd;
	int				nAllSplitRows;		// 分割行数
	int				nAllSplitCols;		// 分割桁数
	int				nVSplitPos;			// 垂直分割位置
	int				nHSplitPos;			// 水平分割位置
	HWND			childWndArr[MAXCOUNTOFVIEW];		// 子ウィンドウ配列
	int				nChildWndCount;		// 有効な子ウィンドウ配列の数
	HCURSOR			hcurOld;			// もとのマウスカーソル
	int				bDragging;			// 分割バーをドラッグ中か
	int				nDragPosX;			// ドラッグ位置Ｘ
	int				nDragPosY;			// ドラッグ位置Ｙ
	int				nActivePane;		// アクティブなペイン
public:
	HWND Create(HINSTANCE, HWND, EditWnd* pEditWnd);	// 初期化
	void SetChildWndArr(HWND*);	// 子ウィンドウの設定 
	void DoSplit(int, int);		// ウィンドウの分割
	void SetActivePane(int);	// アクティブペインの設定
	int GetPrevPane(void);		// 前のペインを返す
	int GetNextPane(void);		// 次のペインを返す
	int GetFirstPane(void);		// 最初のペインを返す
	int GetLastPane(void);		// 最後のペインを返す

	void VSplitOnOff(void);		// 縦分割ＯＮ／ＯＦＦ
	void HSplitOnOff(void);		// 横分割ＯＮ／ＯＦＦ
	void VHSplitOnOff(void);	// 縦横分割ＯＮ／ＯＦＦ
//	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// ダイアログのメッセージ処理
	int GetAllSplitRows() { return nAllSplitRows;}
	int GetAllSplitCols() { return nAllSplitCols;}
protected:
	// 仮想関数
	virtual LRESULT DispatchEvent_WM_APP(HWND, UINT, WPARAM, LPARAM);	// アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF)

	// 仮想関数 メッセージ処理 詳しくは実装を参照
	virtual LRESULT OnSize(HWND, UINT, WPARAM, LPARAM);				// ウィンドウサイズの変更処理
	virtual LRESULT OnPaint(HWND, UINT, WPARAM, LPARAM);			// 描画処理
	virtual LRESULT OnMouseMove(HWND, UINT, WPARAM, LPARAM);		// マウス移動時の処理
	virtual LRESULT OnLButtonDown(HWND, UINT, WPARAM, LPARAM);		// マウス左ボタン押下時の処理
	virtual LRESULT OnLButtonUp(HWND, UINT, WPARAM, LPARAM);		// マウス左ボタン解放時の処理
	virtual LRESULT OnLButtonDblClk(HWND, UINT, WPARAM, LPARAM);	// マウス左ボタンダブルクリック時の処理
	/*
	||  実装ヘルパ関数
	*/
	void DrawFrame(HDC , RECT*);			// 分割フレーム描画
	int HitTestSplitter(int, int);		// 分割バーへのヒットテスト
	void DrawSplitter(int, int, int);	// 分割トラッカーの表示

};



