// 分割線ウィンドウクラス

#include "StdAfx.h"
#include "window/SplitterWnd.h"
#include "window/SplitBoxWnd.h"
#include "window/EditWnd.h"
#include "view/EditView.h"
#include "env/DllSharedData.h"


SplitterWnd::SplitterWnd()
	:
	Wnd(_T("::SplitterWnd")),
	pEditWnd(NULL),
	nAllSplitRows(1),					// 分割行数
	nAllSplitCols(1),					// 分割桁数
	nVSplitPos(0),					// 垂直分割位置
	nHSplitPos(0),					// 水平分割位置
	nChildWndCount(0),
	bDragging(0),						// 分割バーをドラッグ中か
	nDragPosX(0),						// ドラッグ位置Ｘ
	nDragPosY(0),						// ドラッグ位置Ｙ
	nActivePane(0)					// アクティブなペイン 0-3
{
	// 共有データ構造体のアドレスを返す
	pShareData = &GetDllShareData();

	hcurOld = NULL;					// もとのマウスカーソル

	for (int v=0; v<MAXCOUNTOFVIEW; ++v) {
		childWndArr[v] = NULL;		// 子ウィンドウ配列
	}
	return;
}


SplitterWnd::~SplitterWnd()
{
}


// 初期化
HWND SplitterWnd::Create(HINSTANCE hInstance, HWND hwndParent, EditWnd* pEditWnd)
{
	LPCTSTR pszClassName = _T("SplitterWndClass");
	
	// 初期化
	this->pEditWnd = pEditWnd;

	// ウィンドウクラス作成
	ATOM atWork;
	atWork = RegisterWC(
		hInstance,
		NULL,// Handle to the class icon.
		NULL,	// Handle to a small icon
		NULL,// Handle to the class cursor.
		(HBRUSH)NULL,// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE(MYDOCUMENT)*/,// Pointer to a null-terminated 
				// character string that specifies the resource name of the class menu,
				// as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);
	if (atWork == 0) {
		ErrorMessage(NULL, LS(STR_ERR_CSPLITTER01));
	}

	// 基底クラスメンバ呼び出し
	return Wnd::Create(
		hwndParent,
		0, // extended window style
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName,	// pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		CW_USEDEFAULT, // window width
		0, // window height
		NULL // handle to menu, or child-window identifier
	);
}


/* 子ウィンドウの設定
	@param hwndEditViewArr [in] HWND配列 NULL終端
*/
void SplitterWnd::SetChildWndArr(HWND* hwndEditViewArr)
{
	int v = 0;
	for (; v<MAXCOUNTOFVIEW && hwndEditViewArr[v]; ++v) {
		childWndArr[v] = hwndEditViewArr[v];				// 子ウィンドウ配列
	}
	nChildWndCount = v;
	// 残りはNULLで埋める
	for (; v<MAXCOUNTOFVIEW; ++v) {
		childWndArr[v] = NULL;
	}

	return;
}


// 分割フレーム描画
void SplitterWnd::DrawFrame(HDC hdc, RECT* prc)
{
	SplitBoxWnd::Draw3dRect(hdc, prc->left, prc->top, prc->right, prc->bottom,
		::GetSysColor(COLOR_3DSHADOW),
		::GetSysColor(COLOR_3DHILIGHT)
	);
	SplitBoxWnd::Draw3dRect(hdc, prc->left + 1, prc->top + 1, prc->right - 2, prc->bottom - 2,
		RGB(0, 0, 0),
		::GetSysColor(COLOR_3DFACE)
	);
	return;
}


// 分割トラッカーの表示
void SplitterWnd::DrawSplitter(int xPos, int yPos, int bEraseOld)
{
	RECT		rc;
	RECT		rc2;
	int			nTrackerWidth = 6;

	HDC hdc = ::GetDC(GetHwnd());
	HBRUSH hBrush = ::CreateSolidBrush(RGB(255, 255, 255));
	HBRUSH hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);
	::SetROP2(hdc, R2_XORPEN);
	::SetBkMode(hdc, TRANSPARENT);
	::GetClientRect(GetHwnd(), &rc);

	if (bEraseOld) {
		if (bDragging & 1) {	// 分割バーをドラッグ中か
			rc2.left = -1;
			rc2.top = nDragPosY;
			rc2.right = rc.right;
			rc2.bottom = rc2.top + nTrackerWidth;
			::Rectangle(hdc, rc2.left, rc2.top, rc2.right, rc2.bottom);
		}
		if (bDragging & 2) {	// 分割バーをドラッグ中か
			rc2.left = nDragPosX;
			rc2.top = 0;
			rc2.right = rc2.left + nTrackerWidth;
			rc2.bottom = rc.bottom;
			::Rectangle(hdc, rc2.left, rc2.top, rc2.right, rc2.bottom);
		}
	}

	nDragPosX = xPos;
	nDragPosY = yPos;
	if (bDragging & 1) {	// 分割バーをドラッグ中か
		rc2.left = -1;
		rc2.top = nDragPosY;
		rc2.right = rc.right;
		rc2.bottom = rc2.top + nTrackerWidth;
		::Rectangle(hdc, rc2.left, rc2.top, rc2.right, rc2.bottom);
	}
	if (bDragging & 2) {	// 分割バーをドラッグ中か
		rc2.left = nDragPosX;
		rc2.top = 0;
		rc2.right = rc2.left + nTrackerWidth;
		rc2.bottom = rc.bottom;
		::Rectangle(hdc, rc2.left, rc2.top, rc2.right, rc2.bottom);
	}

	::SelectObject(hdc, hBrushOld);
	::DeleteObject(hBrush);
	::ReleaseDC(GetHwnd(), hdc);
	return;
}


// 分割バーへのヒットテスト
int SplitterWnd::HitTestSplitter(int xPos, int yPos)
{
	int		nFrameWidth = 3;
	int		nMargin = 2;

	if (nAllSplitRows == 1 && nAllSplitCols == 1) {
		return 0;
	}else
	if (nAllSplitRows == 2 && nAllSplitCols == 1) {
		if (nVSplitPos - nMargin < yPos && yPos < nVSplitPos + nFrameWidth + nMargin) {
			return 1;
		}else {
			return 0;
		}
	}else
	if (nAllSplitRows == 1 && nAllSplitCols == 2) {
		if (nHSplitPos - nMargin < xPos && xPos < nHSplitPos + nFrameWidth + nMargin) {
			return 2;
		}else {
			return 0;
		}
	}else {
		if (nVSplitPos - nMargin < yPos && yPos < nVSplitPos + nFrameWidth + nMargin &&
			nHSplitPos - nMargin < xPos && xPos < nHSplitPos + nFrameWidth + nMargin
		) {
			return 3;
		}else
		if (nVSplitPos - nMargin < yPos && yPos < nVSplitPos + nFrameWidth + nMargin) {
			return 1;
		}else
		if (nHSplitPos - nMargin < xPos && xPos < nHSplitPos + nFrameWidth + nMargin) {
			return 2;
		}else {
			return 0;
		}
	}
}

/*! ウィンドウの分割
	@param nHorizontal 水平クライアント座標 1以上で分割 0:分割しない  -1: 前の設定を保持
	@param nVertical   垂直クライアント座標 1以上で分割 0:分割しない  -1: 前の設定を保持
*/
void SplitterWnd::DoSplit(int nHorizontal, int nVertical)
{
	int			nActivePane;
	int			nLimit = 32;
	RECT		rc;
	int			nAllSplitRowsOld = nAllSplitRows;	// 分割行数
	int			nAllSplitColsOld = nAllSplitCols;	// 分割桁数
	EditView*	pViewArr[MAXCOUNTOFVIEW];
	bool		bSizeBox;
	
	bool bVUp = false;
	bool bHUp = false;

	if (nHorizontal == -1 && nVertical == -1) {
		nVertical = nVSplitPos;		// 垂直分割位置
		nHorizontal = nHSplitPos;		// 水平分割位置
	}

	if (nVertical != 0 || nHorizontal != 0) {
		// 分割指示。まだ未作成なら2つ目以降のビューを作成します
		// 今のところは分割数に関係なく4つまで一度に作ります。
		pEditWnd->CreateEditViewBySplit(2*2);
	}
	/*
	|| ファンクションキーを下に表示している場合はサイズボックスを表示しない
	|| ステータスパーを表示している場合はサイズボックスを表示しない
	*/
	if (!pEditWnd
		|| (
			pEditWnd->funcKeyWnd.GetHwnd()
	 		&& pShareData->common.window.nFuncKeyWnd_Place == 1	// ファンクションキー表示位置／0:上 1:下
	 	)
	) {
		bSizeBox = false;
	}else if (pEditWnd->tabWnd.GetHwnd()
		&& pShareData->common.tabBar.eTabPosition == TabPosition::Bottom
	) {
		bSizeBox = false;
	}else {
		bSizeBox = true;
		// ステータスパーを表示している場合はサイズボックスを表示しない
		if (pEditWnd->statusBar.GetStatusHwnd()) {
			bSizeBox = false;
		}
	}
	if (pEditWnd->dlgFuncList.GetHwnd()) {
		DockSideType eDockSideFL = pEditWnd->dlgFuncList.GetDockSide();
		if (eDockSideFL == DockSideType::Right || eDockSideFL == DockSideType::Bottom) {
			bSizeBox = false;
		}
	}
	if (pEditWnd->GetMiniMap().GetHwnd()) {
		bSizeBox = false;
	}
	// メインウィンドウが最大化されている場合はサイズボックスを表示しない
	WINDOWPLACEMENT	wp;
	wp.length = sizeof(wp);
	::GetWindowPlacement(GetParentHwnd(), &wp);
	if (wp.showCmd == SW_SHOWMAXIMIZED) {
		bSizeBox = false;
	}

	int v;
	for (v=0; v<nChildWndCount; ++v) {
		pViewArr[v] = (EditView*)::GetWindowLongPtr(childWndArr[v], 0);
	}
	::GetClientRect(GetHwnd(), &rc);
	if (nHorizontal < nLimit) {
		if (nHorizontal > 0) {
			bHUp = true;
		}
		nHorizontal = 0;
	}
	if (nHorizontal > rc.right - nLimit * 2) {
		nHorizontal = 0;
	}
	if (nVertical < nLimit) {
		if (nVertical > 0) {
			bVUp = true;
		}
		nVertical = 0;
	}
	if (nVertical > rc.bottom - nLimit * 2) {
		nVertical = 0;
	}
	nVSplitPos = nVertical;		// 垂直分割位置
	nHSplitPos = nHorizontal;		// 水平分割位置

	if (nVertical == 0 && nHorizontal == 0) {
		nAllSplitRows = 1;	// 分割行数
		nAllSplitCols = 1;	// 分割桁数
		if (childWndArr[0]) ::ShowWindow(childWndArr[0], SW_SHOW);
		if (childWndArr[1]) ::ShowWindow(childWndArr[1], SW_HIDE);
		if (childWndArr[2]) ::ShowWindow(childWndArr[2], SW_HIDE);
		if (childWndArr[3]) ::ShowWindow(childWndArr[3], SW_HIDE);

		if (pViewArr[0]) pViewArr[0]->SplitBoxOnOff(true, true, bSizeBox);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
//		if (pViewArr[1]) pViewArr[1]->SplitBoxOnOff(false, false, false);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
//		if (pViewArr[2]) pViewArr[2]->SplitBoxOnOff(false, false, false);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
//		if (pViewArr[3]) pViewArr[3]->SplitBoxOnOff(false, false, false);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ

		OnSize(0, 0, 0, 0);

		if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
		}else
		if (nAllSplitRowsOld > 1 && nAllSplitColsOld == 1) {
			if (bVUp) {
				// ペインの表示状態を他のビューにコピー
				if (pViewArr[2] && pViewArr[0]) {
					pViewArr[2]->CopyViewStatus(pViewArr[0]);
				}
			}else {
				// ペインの表示状態を他のビューにコピー
				if (this->nActivePane != 0 &&
					pViewArr[this->nActivePane] && pViewArr[0]
				) {
					pViewArr[this->nActivePane]->CopyViewStatus(pViewArr[0]);
				}
			}
		}else
		if (nAllSplitRowsOld == 1 && nAllSplitColsOld > 1) {
			if (bHUp) {
				// ペインの表示状態を他のビューにコピー
				if (pViewArr[1] && pViewArr[0]) {
					pViewArr[1]->CopyViewStatus(pViewArr[0]);
				}
			}else {
				// ペインの表示状態を他のビューにコピー
				if (this->nActivePane != 0 &&
					pViewArr[this->nActivePane] && pViewArr[0]
				) {
					pViewArr[this->nActivePane]->CopyViewStatus(pViewArr[0]);
				}
			}
		}else {
			if (!bVUp && !bHUp) {
				// ペインの表示状態を他のビューにコピー
				if (this->nActivePane != 0 &&
					pViewArr[this->nActivePane] && pViewArr[0]
				) {
					pViewArr[this->nActivePane]->CopyViewStatus(pViewArr[0]);
				}
			}else
			if (bVUp && !bHUp) {
				// ペインの表示状態を他のビューにコピー
				if (pViewArr[2] && pViewArr[0]) {
					pViewArr[2]->CopyViewStatus(pViewArr[0]);
				}
			}else
			if (!bVUp && bHUp) {
				// ペインの表示状態を他のビューにコピー
				if (pViewArr[1] && pViewArr[0]) {
					pViewArr[1]->CopyViewStatus(pViewArr[0]);
				}
			}else {
				// ペインの表示状態を他のビューにコピー
				if (pViewArr[3] && pViewArr[0]) {
					pViewArr[3]->CopyViewStatus(pViewArr[0]);
				}
			}
		}
		nActivePane = 0;
	}else
	if (nVertical > 0 &&  nHorizontal == 0) {
		nAllSplitRows = 2;	// 分割行数
		nAllSplitCols = 1;	// 分割桁数

		if (childWndArr[0]) ::ShowWindow(childWndArr[0], SW_SHOW);
		if (childWndArr[1]) ::ShowWindow(childWndArr[1], SW_HIDE);
		if (childWndArr[2]) ::ShowWindow(childWndArr[2], SW_SHOW);
		if (childWndArr[3]) ::ShowWindow(childWndArr[3], SW_HIDE);
		if (pViewArr[0]) pViewArr[0]->SplitBoxOnOff(false, false, false);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
//		if (pViewArr[1]) pViewArr[1]->SplitBoxOnOff(false, false, false);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		if (pViewArr[2]) pViewArr[2]->SplitBoxOnOff(false, true, bSizeBox);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
//		if (pViewArr[3]) pViewArr[3]->SplitBoxOnOff(false, false, false);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ

		OnSize(0, 0, 0, 0);

		if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
			// 上下に分割したとき
			// ペインの表示状態を他のビューにコピー
			if (pViewArr[0] && pViewArr[2]) {
				pViewArr[0]->CopyViewStatus(pViewArr[2]);
			}
			pViewArr[2]->GetTextArea().SetViewTopLine(pViewArr[0]->GetTextArea().GetViewTopLine() + pViewArr[0]->GetTextArea().nViewRowNum);
		}else if (nAllSplitRowsOld > 1 && nAllSplitColsOld == 1) {
		}else if (nAllSplitRowsOld == 1 && nAllSplitColsOld > 1) {
		}else {
			if (bHUp) {
				// ペインの表示状態を他のビューにコピー
				if (pViewArr[1] && pViewArr[0]) {
					pViewArr[1]->CopyViewStatus(pViewArr[0]);
				}
				// ペインの表示状態を他のビューにコピー
				if (pViewArr[3] && pViewArr[2]) {
					pViewArr[3]->CopyViewStatus(pViewArr[2]);
				}
			}else {
				// ペインの表示状態を他のビューにコピー
				if (this->nActivePane != 0 &&
					this->nActivePane != 2 &&
					pViewArr[0] &&
					pViewArr[1] &&
					pViewArr[2] &&
					pViewArr[3]
				) {
					pViewArr[1]->CopyViewStatus(pViewArr[0]);
					pViewArr[3]->CopyViewStatus(pViewArr[2]);
				}
			}
		}
		if (this->nActivePane == 0 || this->nActivePane == 1) {
			// 分割無しからの切替時のみ従来コードを実行してアクティブペインを決める。
			// それ以外の場合はペイン0をアクティブにする。
			// 従来は、上下に分割しておいて、
			// ・上下分割バーを動かす
			// ・ステータスバーなど各種バーの表示／非表示を切り替える
			// ・設定画面をOKで閉じる
			// ・左右も分割して左右分割を解除する
			// といった操作をするだけで下のペインがアクティブ化されることがあった。
			// （シンプルに0固定にしてしまっても良い気はするけれど．．．）
			nActivePane = 0;
			if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
				if (pViewArr[2]->GetTextArea().GetViewTopLine() < pViewArr[2]->GetCaret().GetCaretLayoutPos().y) {
					nActivePane = 2;
				}else {
					nActivePane = 0;
				}
			}
		}else {
			nActivePane = 2;
		}
	}else if (nVertical == 0 &&  nHorizontal > 0) {
		nAllSplitRows = 1;	// 分割行数
		nAllSplitCols = 2;	// 分割桁数

		if (childWndArr[0]) ::ShowWindow(childWndArr[0], SW_SHOW);
		if (childWndArr[1]) ::ShowWindow(childWndArr[1], SW_SHOW);
		if (childWndArr[2]) ::ShowWindow(childWndArr[2], SW_HIDE);
		if (childWndArr[3]) ::ShowWindow(childWndArr[3], SW_HIDE);
		if (pViewArr[0]) pViewArr[0]->SplitBoxOnOff(false, false, false);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		if (pViewArr[1]) pViewArr[1]->SplitBoxOnOff(true, false, bSizeBox);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
//		if (pViewArr[2]) pViewArr[2]->SplitBoxOnOff(false, false);			// 縦・横の分割ボックスのＯＮ／ＯＦＦ
//		if (pViewArr[3]) pViewArr[3]->SplitBoxOnOff(false, false);			// 縦・横の分割ボックスのＯＮ／ＯＦＦ

		OnSize(0, 0, 0, 0);

		if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
			// ペインの表示状態を他のビューにコピー
			if (pViewArr[0] && pViewArr[1]) {
				pViewArr[0]->CopyViewStatus(pViewArr[1]);
			}
		}else
		if (nAllSplitRowsOld > 1 && nAllSplitColsOld == 1) {
		}else
		if (nAllSplitRowsOld == 1 && nAllSplitColsOld > 1) {
		}else {
			if (bVUp) {
				// ペインの表示状態を他のビューにコピー
				if (pViewArr[2] && pViewArr[0]) {
					pViewArr[2]->CopyViewStatus(pViewArr[0]);
				}
				// ペインの表示状態を他のビューにコピー
				if (pViewArr[3] && pViewArr[1]) {
					pViewArr[3]->CopyViewStatus(pViewArr[1]);
				}
			}else {
				// ペインの表示状態を他のビューにコピー
				if (this->nActivePane != 0 &&
					this->nActivePane != 1 &&
					pViewArr[0] &&
					pViewArr[1] &&
					pViewArr[2] &&
					pViewArr[3]
				) {
					pViewArr[2]->CopyViewStatus(pViewArr[0]);
					pViewArr[3]->CopyViewStatus(pViewArr[1]);
				}
			}
		}
		if (this->nActivePane == 0 || this->nActivePane == 2) {
			nActivePane = 0;
		}else {
			nActivePane = 1;
		}
	}else {
		nAllSplitRows = 2;	// 分割行数
		nAllSplitCols = 2;	// 分割桁数
		if (childWndArr[0]) { ::ShowWindow(childWndArr[0], SW_SHOW); }
		if (childWndArr[1]) { ::ShowWindow(childWndArr[1], SW_SHOW); }
		if (childWndArr[2]) { ::ShowWindow(childWndArr[2], SW_SHOW); }
		if (childWndArr[3]) { ::ShowWindow(childWndArr[3], SW_SHOW); }
		if (pViewArr[0]) { pViewArr[0]->SplitBoxOnOff(false, false, false);}	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		if (pViewArr[1]) { pViewArr[1]->SplitBoxOnOff(false, false, false);}	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		if (pViewArr[2]) { pViewArr[2]->SplitBoxOnOff(false, false, false);}	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		if (pViewArr[3]) { pViewArr[3]->SplitBoxOnOff(false, false, bSizeBox);}	// 縦・横の分割ボックスのＯＮ／ＯＦＦ

		OnSize(0, 0, 0, 0);

		if (nAllSplitRowsOld == 1 && nAllSplitColsOld == 1) {
			// ペインの表示状態を他のビューにコピー
			if (pViewArr[0] && pViewArr[1]) {
				pViewArr[0]->CopyViewStatus(pViewArr[1]);
			}
			// ペインの表示状態を他のビューにコピー
			if (pViewArr[0] && pViewArr[2]) {
				pViewArr[0]->CopyViewStatus(pViewArr[2]);
			}
			// ペインの表示状態を他のビューにコピー
			if (pViewArr[0] && pViewArr[3]) {
				pViewArr[0]->CopyViewStatus(pViewArr[3]);
			}
		}else
		if (nAllSplitRowsOld > 1 && nAllSplitColsOld == 1) {
			// ペインの表示状態を他のビューにコピー
			if (pViewArr[0] && pViewArr[1]) {
				pViewArr[0]->CopyViewStatus(pViewArr[1]);
			}
			// ペインの表示状態を他のビューにコピー
			if (pViewArr[2] && pViewArr[3]) {
				pViewArr[2]->CopyViewStatus(pViewArr[3]);
			}
		}else
		if (nAllSplitRowsOld == 1 && nAllSplitColsOld > 1) {
			// ペインの表示状態を他のビューにコピー
			if (pViewArr[0] && pViewArr[2]) {
				pViewArr[0]->CopyViewStatus(pViewArr[2]);
			}
			// ペインの表示状態を他のビューにコピー
			if (pViewArr[1] && pViewArr[3]) {
				pViewArr[1]->CopyViewStatus(pViewArr[3]);
			}
		}else {
		}
		nActivePane = this->nActivePane;
	}
	OnSize(0, 0, 0, 0);

	// アクティブになったことをペインに通知
	if (childWndArr[nActivePane]) {
		::PostMessage(childWndArr[nActivePane], MYWM_SETACTIVEPANE, 0, 0);
	}

	return;
}

// アクティブペインの設定
void SplitterWnd::SetActivePane(int nIndex)
{
	assert(nIndex < MAXCOUNTOFVIEW);
	nActivePane = nIndex;
	return;
}


// 縦分割ＯＮ／ＯＦＦ
void SplitterWnd::VSplitOnOff(void)
{
	RECT rc;
	::GetClientRect(GetHwnd(), &rc);

	if (nAllSplitRows == 1 && nAllSplitCols == 1) {
		DoSplit(0, rc.bottom / 2);
	}else
	if (nAllSplitRows == 1 && nAllSplitCols > 1) {
		DoSplit(nHSplitPos, rc.bottom / 2);
	}else
	if (nAllSplitRows > 1 && nAllSplitCols == 1) {
		DoSplit(0, 0);
	}else {
		DoSplit(nHSplitPos, 0);
	}
	return;
}


// 横分割ＯＮ／ＯＦＦ
void SplitterWnd::HSplitOnOff(void)
{
	RECT rc;
	::GetClientRect(GetHwnd(), &rc);

	if (nAllSplitRows == 1 && nAllSplitCols == 1) {
		DoSplit(rc.right / 2, 0);
	}else
	if (nAllSplitRows == 1 && nAllSplitCols > 1) {
		DoSplit(0, 0);
	}else
	if (nAllSplitRows > 1 && nAllSplitCols == 1) {
		DoSplit(rc.right / 2 , nVSplitPos);
	}else {
		DoSplit(0, nVSplitPos);
	}
	return;
}


// 縦横分割ＯＮ／ＯＦＦ
void SplitterWnd::VHSplitOnOff(void)
{
	int		nX;
	int		nY;
	RECT	rc;
	::GetClientRect(GetHwnd(), &rc);

	if (nAllSplitRows > 1 && nAllSplitCols > 1) {
		nX = 0;
		nY = 0;
	}else {
		if (nAllSplitRows == 1) {
			nY = rc.bottom / 2;
		}else {
			nY = nVSplitPos;
		}
		if (nAllSplitCols == 1) {
			nX = rc.right / 2;
		}else {
			nX = nHSplitPos;
		}
	}
	DoSplit(nX, nY);

	return;
}


// 前のペインを返す
int SplitterWnd::GetPrevPane(void)
{
	int nPane;
	nPane = -1;
	if (nAllSplitRows == 1 &&	nAllSplitCols == 1) {
		nPane = -1;
	}else
	if (nAllSplitRows == 2 &&	nAllSplitCols == 1) {
		switch (nActivePane) {
		case 0:
			nPane = -1;
			break;
		case 2:
			nPane = 0;
			break;
		}
	}else
	if (nAllSplitRows == 1 &&	nAllSplitCols == 2) {
		switch (nActivePane) {
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		}
	}else {
		switch (nActivePane) {
		case 0:
			nPane = -1;
			break;
		case 1:
			nPane = 0;
			break;
		case 2:
			nPane = 1;
			break;
		case 3:
			nPane = 2;
			break;
		}
	}
	return nPane;
}


// 次のペインを返す
int SplitterWnd::GetNextPane(void)
{
	int nPane;
	nPane = -1;
	if (nAllSplitRows == 1 &&	nAllSplitCols == 1) {
		nPane = -1;
	}else
	if (nAllSplitRows == 2 &&	nAllSplitCols == 1) {
		switch (nActivePane) {
		case 0:
			nPane = 2;
			break;
		case 2:
			nPane = -1;
			break;
		}
	}else
	if (nAllSplitRows == 1 &&	nAllSplitCols == 2) {
		switch (nActivePane) {
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = -1;
			break;
		}
	}else {
		switch (nActivePane) {
		case 0:
			nPane = 1;
			break;
		case 1:
			nPane = 2;
			break;
		case 2:
			nPane = 3;
			break;
		case 3:
			nPane = -1;
			break;
		}
	}
	return nPane;
}


// 最初のペインを返す
int SplitterWnd::GetFirstPane(void)
{
	return 0;
}


// 最後のペインを返す
int SplitterWnd::GetLastPane(void)
{
	int nPane;
	if (nAllSplitRows == 1 &&	nAllSplitCols == 1) {
		nPane = 0;
	}else
	if (nAllSplitRows == 1 &&	nAllSplitCols == 2) {
		nPane = 1;
	}else
	if (nAllSplitRows == 2 &&	nAllSplitCols == 1) {
		nPane = 2;
	}else {
		nPane = 3;
	}
	return nPane;
}


// 描画処理
LRESULT SplitterWnd::OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	RECT		rcFrame;
	int			nFrameWidth = 3;
	HBRUSH		hBrush;
	hdc = ::BeginPaint(hwnd, &ps);
	::GetClientRect(GetHwnd(), &rc);
	hBrush = ::CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
	if (nAllSplitRows > 1) {
		::SetRect(&rcFrame, rc.left, nVSplitPos, rc.right, nVSplitPos + nFrameWidth);
		::FillRect(hdc, &rcFrame, hBrush);
	}
	if (nAllSplitCols > 1) {
		::SetRect(&rcFrame, nHSplitPos, rc.top, nHSplitPos + nFrameWidth, rc.bottom);
		::FillRect(hdc, &rcFrame, hBrush);
	}
	::DeleteObject(hBrush);
	::EndPaint(hwnd, &ps);
	return 0L;
}


// ウィンドウサイズの変更処理
LRESULT SplitterWnd::OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EditView*	pViewArr[MAXCOUNTOFVIEW];
	int			nFrameWidth = 3;
	bool		bSizeBox;
	for (int i=0; i<nChildWndCount; ++i) {
		pViewArr[i] = (EditView*)::GetWindowLongPtr(childWndArr[i], 0);
	}

	/*
	|| ファンクションキーを下に表示している場合はサイズボックスを表示しない
	|| ステータスパーを表示している場合はサイズボックスを表示しない
	*/
	if (!pEditWnd
	 	|| (
	 		pEditWnd->funcKeyWnd.GetHwnd()
	  		&& pShareData->common.window.nFuncKeyWnd_Place == 1	// ファンクションキー表示位置／0:上 1:下
	 	)
	) {
		bSizeBox = false;
	}else if (pEditWnd->tabWnd.GetHwnd()
		&& pShareData->common.tabBar.eTabPosition == TabPosition::Bottom
	) {
		bSizeBox = false;
	}else {
		bSizeBox = true;
		// ステータスパーを表示している場合はサイズボックスを表示しない
		if (pEditWnd->statusBar.GetStatusHwnd()) {
			bSizeBox = false;
		}
	}
	if (pEditWnd->dlgFuncList.GetHwnd()) {
		DockSideType eDockSideFL = pEditWnd->dlgFuncList.GetDockSide();
		if (eDockSideFL == DockSideType::Right || eDockSideFL == DockSideType::Bottom) {
			bSizeBox = false;
		}
	}
	if (pEditWnd->GetMiniMap().GetHwnd()) {
		bSizeBox = false;
	}

	// メインウィンドウが最大化されている場合はサイズボックスを表示しない
	WINDOWPLACEMENT	wp;
	wp.length = sizeof(wp);
	::GetWindowPlacement(GetParentHwnd(), &wp);
	if (wp.showCmd == SW_SHOWMAXIMIZED) {
		bSizeBox = false;
	}

	RECT rcClient;
	::GetClientRect(GetHwnd(), &rcClient);

	if (nAllSplitRows == 1 && nAllSplitCols == 1) {
		if (childWndArr[0]) {
			::MoveWindow(childWndArr[0], 0, 0, rcClient.right,  rcClient.bottom, TRUE);		// 子ウィンドウ配列

			pViewArr[0]->SplitBoxOnOff(true, true, bSizeBox);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		}
	}else
	if (nAllSplitRows == 2 && nAllSplitCols == 1) {
		if (childWndArr[0]) {
			::MoveWindow(childWndArr[0], 0, 0, rcClient.right,  nVSplitPos, TRUE);		// 子ウィンドウ配列
			pViewArr[0]->SplitBoxOnOff(false, false, false);		// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		}
		if (childWndArr[2]) {
			::MoveWindow(childWndArr[2], 0, nVSplitPos + nFrameWidth, rcClient.right, rcClient.bottom - (nVSplitPos + nFrameWidth), TRUE);			// 子ウィンドウ配列
			pViewArr[2]->SplitBoxOnOff(false, true, bSizeBox);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		}
	}else
	if (nAllSplitRows == 1 && nAllSplitCols == 2) {
		if (childWndArr[0]) {
			::MoveWindow(childWndArr[0], 0, 0, nHSplitPos, rcClient.bottom, TRUE);		// 子ウィンドウ配列
			pViewArr[0]->SplitBoxOnOff(false, false, false);		// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		}
		if (childWndArr[1]) {
			::MoveWindow(childWndArr[1], nHSplitPos + nFrameWidth, 0, rcClient.right - (nHSplitPos + nFrameWidth),  rcClient.bottom, TRUE);			// 子ウィンドウ配列
			pViewArr[1]->SplitBoxOnOff(true, false, bSizeBox);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		}
	}else {
		if (childWndArr[0]) {
			::MoveWindow(childWndArr[0], 0, 0, nHSplitPos,  nVSplitPos, TRUE);			// 子ウィンドウ配列
			pViewArr[0]->SplitBoxOnOff(false, false, false);		// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		}
		if (childWndArr[1]) {
			::MoveWindow(childWndArr[1], nHSplitPos + nFrameWidth, 0, rcClient.right - (nHSplitPos + nFrameWidth),  nVSplitPos, TRUE);				// 子ウィンドウ配列
			pViewArr[1]->SplitBoxOnOff(false, false, false);		// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		}
		if (childWndArr[2]) {
			::MoveWindow(childWndArr[2], 0, nVSplitPos + nFrameWidth , nHSplitPos,  rcClient.bottom - (nVSplitPos + nFrameWidth), TRUE);			// 子ウィンドウ配列
			pViewArr[2]->SplitBoxOnOff(false, false, false);		// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		}
		if (childWndArr[3]) {
			::MoveWindow(childWndArr[3], nHSplitPos + nFrameWidth, nVSplitPos + nFrameWidth, rcClient.right - (nHSplitPos + nFrameWidth),  rcClient.bottom - (nVSplitPos + nFrameWidth), TRUE);			// 子ウィンドウ配列
			pViewArr[3]->SplitBoxOnOff(false, false, bSizeBox);	// 縦・横の分割ボックスのＯＮ／ＯＦＦ
		}
	}
	return 0L;
}


// マウス移動時の処理
LRESULT SplitterWnd::OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int xPos = (int)(short)LOWORD(lParam);
	int yPos = (int)(short)HIWORD(lParam);
	int nHit = HitTestSplitter(xPos, yPos);
	switch (nHit) {
	case 1:
		::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
		break;
	case 2:
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
		break;
	case 3:
		::SetCursor(::LoadCursor(NULL, IDC_SIZEALL));
		break;
	}
	if (bDragging != 0) {		// 分割バーをドラッグ中か
		RECT rc;
		::GetClientRect(GetHwnd(), &rc);
		if (xPos < 1) {
			xPos = 1;
		}
		if (xPos > rc.right - 6) {
			xPos = rc.right - 6;
		}
		if (yPos < 1) {
			yPos = 1;
		}
		if (yPos > rc.bottom - 6) {
			yPos = rc.bottom - 6;
		}
		// 分割トラッカーの表示
		DrawSplitter(xPos, yPos, TRUE);
//		MYTRACE(_T("xPos=%d yPos=%d \n"), xPos, yPos);
	}
	return 0L;
}


// マウス左ボタン押下時の処理
LRESULT SplitterWnd::OnLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int xPos = (int)(short)LOWORD(lParam);
	int yPos = (int)(short)HIWORD(lParam);
	::SetFocus(GetParentHwnd());
	// 分割バーへのヒットテスト
	int nHit = HitTestSplitter(xPos, yPos);
	if (nHit != 0) {
		bDragging = nHit;	// 分割バーをドラッグ中か
		::SetCapture(GetHwnd());
	}
	// 分割トラッカーの表示
	DrawSplitter(xPos, yPos, FALSE);

	return 0L;
}



// マウス左ボタン解放時の処理
LRESULT SplitterWnd::OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (bDragging) {
		// 分割トラッカーの表示
		DrawSplitter(nDragPosX, nDragPosY, FALSE);
		int bDraggingOld = bDragging;
		bDragging = 0;
		::ReleaseCapture();
		if (hcurOld) {
			::SetCursor(hcurOld);
		}
		int nX;
		int nY;
		// ウィンドウの分割
		if (nAllSplitRows == 1) {
			nY = 0;
		}else {
			nY = nDragPosY;
		}
		if (nAllSplitCols == 1) {
			nX = 0;
		}else {
			nX = nDragPosX;
		}
		if (bDraggingOld == 1) {
			DoSplit(nHSplitPos, nY);
		}else
		if (bDraggingOld == 2) {
			DoSplit(nX, nVSplitPos);
		}else
		if (bDraggingOld == 3) {
			DoSplit(nX, nY);
		}
	}
	return 0L;
}



// マウス左ボタンダブルクリック時の処理
LRESULT SplitterWnd::OnLButtonDblClk(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nX;
	int nY;
	int	xPos = (int)(short)LOWORD(lParam);
	int	yPos = (int)(short)HIWORD(lParam);
	int	nHit = HitTestSplitter(xPos, yPos);
	if (nHit == 1) {
		if (nAllSplitCols == 1) {
			nX = 0;
		}else {
			nX = nHSplitPos;
		}
		DoSplit(nX , 0);
	}else
	if (nHit == 2) {
		if (nAllSplitRows == 1) {
			nY = 0;
		}else {
			nY = nVSplitPos;
		}
		DoSplit(0 , nY);
	}else
	if (nHit == 3) {
		DoSplit(0 , 0);
	}
	OnMouseMove(GetHwnd(), 0, 0, MAKELONG(xPos, yPos));
	return 0L;
}


// アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF)
LRESULT SplitterWnd::DispatchEvent_WM_APP(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nPosX;
	int nPosY;
	switch (uMsg) {
	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE(_T("MYWM_DOSPLIT nPosX=%d nPosY=%d\n"), nPosX, nPosY);

		// ウィンドウの分割
		if (nHSplitPos != 0) {
			nPosX = nHSplitPos;
		}
		if (nVSplitPos != 0) {
			nPosY = nVSplitPos;
		}
		DoSplit(nPosX , nPosY);
		break;
	case MYWM_SETACTIVEPANE:
		SetActivePane((int)wParam);
		break;
	}
	return 0L;
}

