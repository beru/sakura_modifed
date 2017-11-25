#include "StdAfx.h"
#include "TextArea.h"
#include "ViewFont.h"
#include "Ruler.h"
#include "EditView.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "doc/EditDoc.h"

//#define USE_LOG10			// この行のコメントを外すと行番号の最小桁数の計算にlog10()を用いる
#ifdef USE_LOG10
#include <math.h>
#endif

TextArea::TextArea(EditView& editView)
	:
	editView(editView)
{
	DllSharedData* pShareData = &GetDllShareData();

	nViewAlignLeft = 0;					// 表示域の左端座標
	nViewAlignLeftCols = 0;				// 行番号域の桁数
	nViewCx = 0;							// 表示域の幅
	nViewCy = 0;							// 表示域の高さ
	nViewColNum = 0;			// 表示域の桁数
	nViewRowNum = 0;			// 表示域の行数
	nViewTopLine = 0;			// 表示域の一番上の行
	nViewLeftCol = 0;			// 表示域の一番左の桁
	SetTopYohaku(pShareData->common.window.nRulerBottomSpace); 	// ルーラーとテキストの隙間
	SetLeftYohaku(pShareData->common.window.nLineNumRightSpace);
	nViewAlignTop = GetTopYohaku();		// 表示域の上端座標
}

TextArea::~TextArea()
{
}

void TextArea::CopyTextAreaStatus(TextArea* pDst) const
{
	pDst->SetAreaLeft				(this->GetAreaLeft());		// 表示域の左端座標
	pDst->nViewAlignLeftCols		= this->nViewAlignLeftCols;	// 行番号域の桁数
	pDst->SetAreaTop				(this->GetAreaTop());			// 表示域の上端座標
//	pDst->nViewCx					= nViewCx;					// 表示域の幅
//	pDst->nViewCy					= nViewCy;					// 表示域の高さ
//	pDst->nViewColNum				= this->nViewColNum;			// 表示域の桁数
//	pDst->nViewRowNum				= this->nViewRowNum;			// 表示域の行数
	pDst->SetViewTopLine			(this->GetViewTopLine());		// 表示域の一番上の行(0開始)
	pDst->SetViewLeftCol			(this->GetViewLeftCol());		// 表示域の一番左の桁(0開始)
}

// 表示域の再計算
void TextArea::UpdateViewColRowNums()
{
	auto& view = editView;
	// Note: マイナスの割り算は処理系依存です。
	// 0だとカーソルを設定できない・選択できないなど動作不良になるので1以上にする
	nViewColNum = t_max(1, t_max(0, nViewCx - 1) / view.GetTextMetrics().GetHankakuDx());	// 表示域の桁数
	nViewRowNum = t_max(1, t_max(0, nViewCy - 1) / view.GetTextMetrics().GetHankakuDy());	// 表示域の行数
}

//フォント変更の際、各種パラメータを計算し直す
void TextArea::UpdateAreaMetrics(HDC hdc)
{
	auto& view = editView;
	auto& textMetrics = view.GetTextMetrics();
	if (view.bMiniMap) {
		// 文字間隔
		textMetrics.SetHankakuDx(textMetrics.GetHankakuWidth());

		// 行間隔
		textMetrics.SetHankakuDy(textMetrics.GetHankakuHeight());
	}else {
		// 文字間隔
		textMetrics.SetHankakuDx(textMetrics.GetHankakuWidth() + (int)view.pTypeData->nColumnSpace);
	
		// 行間隔
		textMetrics.SetHankakuDy(textMetrics.GetHankakuHeight() + view.pTypeData->nLineSpace);
	}

	// 表示域の再計算
	// 2010.08.24 Dx/Dyを使うので後で設定
	UpdateViewColRowNums();
}

void TextArea::GenerateCharRect(RECT* rc, const DispPos& pos, int nHankakuNum) const
{
	auto& view = editView;

	rc->left   = pos.GetDrawPos().x;
	rc->right  = pos.GetDrawPos().x + view.GetTextMetrics().GetHankakuDx() * nHankakuNum;
	rc->top    = pos.GetDrawPos().y;
	rc->bottom = pos.GetDrawPos().y + view.GetTextMetrics().GetHankakuDy();
}

bool TextArea::TrimRectByArea(RECT* rc) const
{
	// 左はみ出し調整
	if (rc->left < GetAreaLeft()) {
		rc->left = GetAreaLeft();
	}

	if (rc->left >= rc->right) return false; // 左と右があべこべ
	if (rc->left >= GetAreaRight()) return false; // 画面外(右)
	if (rc->right <= GetAreaLeft()) return false; // 画面外(左)

	//$ 元動作踏襲：画面上下のはみ出し判定は省略

	return true;
}

bool TextArea::GenerateClipRect(RECT* rc, const DispPos& pos, int nHankakuNum) const
{
	GenerateCharRect(rc, pos, nHankakuNum);
	return TrimRectByArea(rc);
}

// 右の残りを表す矩形を生成する
bool TextArea::GenerateClipRectRight(RECT* rc, const DispPos& pos) const
{
	auto& view = editView;

	rc->left   = pos.GetDrawPos().x;
	rc->right  = GetAreaRight();
	rc->top    = pos.GetDrawPos().y;
	rc->bottom = pos.GetDrawPos().y + view.GetTextMetrics().GetHankakuDy();

	// 左はみ出し調整
	if (rc->left < GetAreaLeft()) {
		rc->left = GetAreaLeft();
	}

	if (rc->left >= rc->right) return false; // 左と右があべこべ
	if (rc->left >= GetAreaRight()) return false; // 画面外(右)
	if (rc->right <= GetAreaLeft()) return false; // 画面外(左)

	//$ 元動作踏襲：画面上下のはみ出し判定は省略

	return true;
}

bool TextArea::GenerateClipRectLine(RECT* rc, const DispPos& pos) const
{
	rc->left   = 0;
	rc->right  = GetAreaRight();
	rc->top    = pos.GetDrawPos().y;
	rc->bottom = pos.GetDrawPos().y + editView.GetTextMetrics().GetHankakuDy();
	return true;
}


// 行番号表示に必要な幅を設定。幅が変更された場合は true を返す
bool TextArea::DetectWidthOfLineNumberArea(bool bRedraw)
{
	auto& view = editView;

	int nViewAlignLeftNew;

	if (view.pTypeData->colorInfoArr[COLORIDX_GYOU].bDisp && !view.bMiniMap) {
		// 行番号表示に必要な桁数を計算
		int i = DetectWidthOfLineNumberArea_calculate(&view.pEditDoc->layoutMgr);
		nViewAlignLeftNew = view.GetTextMetrics().GetHankakuDx() * (i + 1);	// 表示域の左端座標
		nViewAlignLeftCols = i + 1;
	}else if (view.bMiniMap) {
		nViewAlignLeftNew = 4;
		nViewAlignLeftCols = 0;
	}else {
		nViewAlignLeftNew = 8;
		nViewAlignLeftCols = 0;
	}

	//	Sep 18, 2002 genta
	nViewAlignLeftNew += GetDllShareData().common.window.nLineNumRightSpace;
	if (nViewAlignLeftNew != GetAreaLeft()) {
		Rect			rc;
		SetAreaLeft(nViewAlignLeftNew);
		view.GetClientRect(&rc);
		int nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);		// 垂直スクロールバーの横幅
		nViewCx = rc.Width() - nCxVScroll - GetAreaLeft();	// 表示域の幅
		UpdateViewColRowNums();

		if (bRedraw && view.GetDrawSwitch()) {
			// 再描画
			view.GetCaret().underLine.Lock();
			view.Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Ruler | (int)PaintAreaType::Body, false); // メモリＤＣを使用してちらつきのない再描画
			view.GetCaret().underLine.UnLock();
			view.GetCaret().ShowEditCaret();
			/*
			PAINTSTRUCT		ps;
			HDC hdc = ::GetDC(view.hWnd);
			ps.rcPaint.left   = 0;
			ps.rcPaint.right  = GetAreaRight();
			ps.rcPaint.top    = 0;
			ps.rcPaint.bottom = GetAreaBottom();
			view.GetCaret().underLine.Lock();
			view.OnPaint(hdc, &ps, TRUE);	
			GetCaret().underLine.UnLock();
			view.GetCaret().ShowEditCaret();
			::ReleaseDC(hWnd, hdc);
			*/
		}
		view.GetRuler().SetRedrawFlag();
		return true;
	}else {
		return false;
	}
}


// 行番号表示に必要な桁数を計算
int TextArea::DetectWidthOfLineNumberArea_calculate(const LayoutMgr* pLayoutMgr, bool bLayout) const
{
	auto& view = editView;

	size_t nAllLines; //$$ 単位混在
	// 行番号の表示 false=折り返し単位／true=改行単位
	if (view.pTypeData->bLineNumIsCRLF && !bLayout) {
		nAllLines = view.pEditDoc->docLineMgr.GetLineCount();
	}else {
		nAllLines = pLayoutMgr->GetLineCount();
	}
	
	if (0 < nAllLines) {
		int i;

		// 行番号の桁数を決める
		// nLineNumWidthは純粋に数字の桁数を示し、先頭の空白を含まない（仕様変更）
#ifdef USE_LOG10
		// 表示している行数の桁数を求める
		nWork = (int)(log10( (double)nAllLines) +1);	// 10を底とする対数(小数点以下切り捨て)+1で桁数
		// 設定値と比較し、大きい方を取る
		i = nWork > view.pTypeData->nLineNumWidth ?
			nWork : view.pTypeData->nLineNumWidth;
		// 先頭の空白分を加算する
		return (i +1);
#else
		// 設定から行数を求める
		size_t nWork = 10;
		for (i=1; i<view.pTypeData->nLineNumWidth; ++i) {
			nWork *= 10;
		}
		// 表示している行数と比較し、大きい方の値を取る
		for (i= view.pTypeData->nLineNumWidth; i<LINENUMWIDTH_MAX; ++i) {
			if (nWork > nAllLines) {	// Oct. 18, 2003 genta 式を整理
				break;
			}
			nWork *= 10;
		}
		// 先頭の空白分を加算する
		return (i +1);
#endif
	}else {
		// 行番号が1桁のときと幅を合わせる
		return view.pTypeData->nLineNumWidth +1;
	}
}

void TextArea::TextArea_OnSize(
	const Size& sizeClient,		// ウィンドウのクライアントサイズ
	int nCxVScroll,				// 垂直スクロールバーの横幅
	int nCyHScroll				// 水平スクロールバーの縦幅
	)
{
	nViewCx = sizeClient.cx - nCxVScroll - GetAreaLeft(); // 表示域の幅
	nViewCy = sizeClient.cy - nCyHScroll - GetAreaTop();  // 表示域の高さ
	UpdateViewColRowNums();
}


int TextArea::GetDocumentLeftClientPointX() const
{
	return GetAreaLeft() - GetViewLeftCol() * editView.GetTextMetrics().GetHankakuDx();
}

// クライアント座標からレイアウト位置に変換する
void TextArea::ClientToLayout(Point ptClient, Point* pptLayout) const
{
	auto& view = editView;
	auto& tm = view.GetTextMetrics();
	int x = (int)GetViewLeftCol() + ((ptClient.x - GetAreaLeft()) / (int)tm.GetHankakuDx());
	int y = (int)GetViewTopLine() + ((ptClient.y - GetAreaTop()) / (int)tm.GetHankakuDy());
	pptLayout->Set(x, y);
}


// 行番号エリアも含む範囲
void TextArea::GenerateTopRect(RECT* rc, int nLineCount) const
{
	rc->left   = 0; //nViewAlignLeft;
	rc->right  = nViewAlignLeft + nViewCx;
	rc->top    = nViewAlignTop;
	rc->bottom = nViewAlignTop + nLineCount * editView.GetTextMetrics().GetHankakuDy();
}

// 行番号エリアも含む範囲
void TextArea::GenerateBottomRect(RECT* rc, int nLineCount) const
{
	rc->left   = 0; //nViewAlignLeft;
	rc->right  = nViewAlignLeft + nViewCx;
	rc->top    = nViewAlignTop  + nViewCy - nLineCount * editView.GetTextMetrics().GetHankakuDy();
	rc->bottom = nViewAlignTop  + nViewCy;
}

void TextArea::GenerateLeftRect(RECT* rc, int nColCount) const
{
	rc->left   = nViewAlignLeft;
	rc->right  = nViewAlignLeft + nColCount * editView.GetTextMetrics().GetHankakuDx();
	rc->top    = nViewAlignTop;
	rc->bottom = nViewAlignTop + nViewCy;
}

void TextArea::GenerateRightRect(RECT* rc, int nColCount) const
{
	rc->left   = nViewAlignLeft + nViewCx - nColCount * editView.GetTextMetrics().GetHankakuDx();
	rc->right  = nViewAlignLeft + nViewCx;
	rc->top    = nViewAlignTop;
	rc->bottom = nViewAlignTop  + nViewCy;
}

void TextArea::GenerateLineNumberRect(RECT* rc) const
{
	rc->left   = 0;
	rc->right  = nViewAlignLeft;
	rc->top    = 0;
	rc->bottom = nViewAlignTop + nViewCy;
}

void TextArea::GenerateTextAreaRect(RECT* rc) const
{
	rc->left   = 0;
	rc->right  = nViewAlignLeft + nViewCx;
	rc->top    = nViewAlignTop;
	rc->bottom = nViewAlignTop + nViewCy;
}


int TextArea::GenerateYPx(int nLineNum) const
{
	int nY = nLineNum - GetViewTopLine();
	int ret;
	if (nY < 0) {
		ret = GetAreaTop();
	}else if (nViewRowNum < nY) {
		ret = GetAreaBottom();
	}else {
		ret = GetAreaTop() + editView.GetTextMetrics().GetHankakuDy() * nY;
	}
	return ret;
}

