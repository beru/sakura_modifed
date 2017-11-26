#include "StdAfx.h"
#include "ImageListMgr.h"
#include "env/CommonSetting.h"
#include "util/module.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h"

const uint32_t MAX_X = MAX_TOOLBAR_ICON_X;
const uint32_t MAX_Y = MAX_TOOLBAR_ICON_Y;

ImageListMgr::ImageListMgr()
	:
	cx(16),
	cy(16),
	cTrans(RGB(0, 0, 0)),
	hIconBitmap(NULL),
	nIconCount(MAX_TOOLBAR_ICON_COUNT)
{
}

/*!	領域を指定色で塗りつぶす */
static
void FillSolidRect(
	HDC hdc,
	int x,
	int y,
	int cx,
	int cy,
	COLORREF clr
	)
{
//	ASSERT_VALID(this);
//	ASSERT(hDC);

	RECT rect;
	::SetBkColor(hdc, clr);
	::SetRect(&rect, x, y, x + cx, y + cy);
	::ExtTextOutW_AnyBuild(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
}

//	Destructor
ImageListMgr::~ImageListMgr()
{
	// Image Listの代わりに描画用bitmapを解放
	if (hIconBitmap) {
		DeleteObject(hIconBitmap);
	}
}

/*
	@brief Image Listの作成
	
	リソースまたはファイルからbitmapを読み込んで
	描画用に保持する．
	
	@param hInstance [in] bitmapリソースを持つインスタンス
*/
bool ImageListMgr::Create(HINSTANCE hInstance)
{
	MY_RUNNINGTIMER(runningTimer, "CImageListMgr::Create");
	if (hIconBitmap) {	//	既に構築済みなら無視する
		return true;
	}

	HBITMAP	hRscbmp;			//	リソースから読み込んだひとかたまりのBitmap
	HBITMAP	hFOldbmp = NULL;	//	SetObjectで得られた1つ前のハンドルを保持する
	HDC		dcFrom = 0;			//	描画用
	int		nRetPos;			//	後処理用
	cx = cy  = 16;

	nRetPos = 0;
	do {
		TCHAR szPath[_MAX_PATH];
		GetInidirOrExedir(szPath, FN_TOOL_BMP);
		hRscbmp = (HBITMAP)::LoadImage(NULL, szPath, IMAGE_BITMAP, 0, 0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);

		if (!hRscbmp) {	// ローカルファイルの読み込み失敗時はリソースから取得
			//	このブロック内は従来の処理
			//	リソースからBitmapを読み込む
			hRscbmp = (HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_MYTOOL), IMAGE_BITMAP, 0, 0,
				LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );
			if (!hRscbmp) {
				//	正常終了と同じコードだとdcFromを不正に解放してしまう
				nRetPos = 2;
				break;
			}
		}
		hIconBitmap = hRscbmp;

		//	透過色を得るためにDCにマップする
		dcFrom = CreateCompatibleDC(0);	//	転送元用
		if (!dcFrom) {
			nRetPos = 1;
			break;
		}

		//	まずbitmapをdcにmapする
		//	こうすることでCreateCompatibleBitmapで
		//	hRscbmpと同じ形式のbitmapを作れる．
		//	単にCreateCompatibleDC(0)で取得したdcや
		//	スクリーンのDCに対してCreateCompatibleBitmapを
		//	使うとモノクロBitmapになる．
		hFOldbmp = (HBITMAP)SelectObject(dcFrom, hRscbmp);
		if (!hFOldbmp) {
			nRetPos = 4;
			break;
		}

		cTrans = GetPixel(dcFrom, 0, 0);//	取得した画像の(0,0)の色を背景色として使う
		
		//	もはや処理とは無関係だが，後学のためにコメントのみ残しておこう
		//---------------------------------------------------------
		//	BitmapがMemoryDCにAssignされている間はbitmapハンドルを
		//	使っても正しいbitmapが取得できない．
		//	つまり，DCへの描画命令を発行してもその場でBitmapに
		//	反映されるわけではない．
		//	BitmapをDCから取り外して初めて内容の保証ができる

		//	DCのmap/unmapが速度に大きく影響するため，
		//	横長のBitmapを作って一括登録するように変更
		//	これによって250msecくらい速度が改善される．
		//---------------------------------------------------------

	}while (0);	//	1回しか通らない. breakでここまで飛ぶ

	//	後処理
	switch (nRetPos) {
	case 0:
		//	hRscBmpをdcFromから切り離しておく必要がある
		//	アイコン描画変更時に過って削除されていた
		SelectObject(dcFrom, hFOldbmp);
	case 4:
		DeleteDC(dcFrom);
	case 2:
	case 1:
		//	hRscbmpは hIconBitmap としてオブジェクトと
		//	同じだけ保持されるので解放してはならない
		break;
	}

	return nRetPos == 0;
}


/*! ビットマップの表示 灰色を透明描画 */
void ImageListMgr::MyBitBlt(
	HDC drawdc, 
	int nXDest, 
	int nYDest, 
	int nWidth, 
	int nHeight, 
	HBITMAP bmp, 
	int nXSrc, 
	int nYSrc,
	COLORREF colToTransParent	// BMPの中の透明にする色
	) const
{
//	HBRUSH	brShadow, brHilight;
	// create a monochrome memory DC
	HDC hdcMask = CreateCompatibleDC(drawdc);
	HBITMAP bmpMask = CreateCompatibleBitmap(hdcMask, nWidth, nHeight);
	HBITMAP bmpMaskOld = (HBITMAP)SelectObject(hdcMask, bmpMask);
	// 元ビットマップ用DC
	HDC hdcMem = ::CreateCompatibleDC(drawdc);
	HBITMAP bmpMemOld = (HBITMAP)::SelectObject(hdcMem, bmp);
	// 作業用DC
	HDC hdcMem2 = ::CreateCompatibleDC(drawdc);
	HBITMAP bmpMem2 = CreateCompatibleBitmap(drawdc, nWidth, nHeight);
	HBITMAP bmpMem2Old = (HBITMAP)SelectObject(hdcMem2, bmpMem2);
	
	// build a mask
	SetBkColor(hdcMem, colToTransParent);
	BitBlt(hdcMask, 0, 0, nWidth, nHeight, hdcMem, nXSrc, nYSrc, SRCCOPY);

	// マスク描画(透明にしない部分だけ黒く描画)
	::SetBkColor(drawdc, RGB(255, 255, 255) /* colBkColor */);
	::SetTextColor(drawdc, RGB(0, 0, 0));
	::BitBlt(drawdc, nXDest, nYDest, nWidth, nHeight, hdcMask, 0, 0, SRCAND /* SRCCOPY */); 

	// ビットマップ描画(透明にする色を黒くしてマスクとOR描画)
	::SetBkColor(hdcMem2, colToTransParent/*RGB(0, 0, 0)*/);
	::SetTextColor(hdcMem2, RGB(0, 0, 0));
	::BitBlt(hdcMem2, 0, 0, nWidth, nHeight, hdcMask, 0, 0, SRCCOPY);
	::BitBlt(hdcMem2, 0, 0, nWidth, nHeight, hdcMem, nXSrc, nYSrc, SRCINVERT/*SRCPAINT*/);
	::BitBlt(drawdc, nXDest, nYDest, nWidth, nHeight, hdcMem2,  0, 0, /*SRCCOPY*/SRCPAINT);

	::SelectObject(hdcMask, bmpMaskOld);
	::DeleteObject(bmpMask);
	::DeleteDC(hdcMask);
	::SelectObject(hdcMem, bmpMemOld);
	::DeleteDC(hdcMem);
	::SelectObject(hdcMem2, bmpMem2Old);
	::DeleteObject(bmpMem2);
	::DeleteDC(hdcMem2);
	return;
}

/*! メニューアイコンの淡色表示 */
void ImageListMgr::DitherBlt2(
	HDC drawdc,
	int nXDest,
	int nYDest,
	int nWidth, 
    int nHeight,
    HBITMAP bmp,
    int nXSrc,
    int nYSrc
    ) const
{

	//COLORREF colToTransParent = RGB(192, 192, 192);	// BMPの中の透明にする色
	COLORREF colToTransParent = cTrans;

	// create a monochrome memory DC
	HDC hdcMask = CreateCompatibleDC(drawdc);
	HBITMAP bmpMask = CreateCompatibleBitmap(hdcMask, nWidth, nHeight);
	HBITMAP bmpMaskOld = (HBITMAP)SelectObject(hdcMask, bmpMask);

	HDC hdcMem = CreateCompatibleDC(drawdc);
	HBITMAP bmpMemOld = (HBITMAP)SelectObject(hdcMem, bmp);

	//	hdcMemに書き込むと元のbitmapを破壊してしまう
	HDC hdcMem2 = ::CreateCompatibleDC(drawdc);
	HBITMAP bmpMem2 = CreateCompatibleBitmap(drawdc, nWidth, nHeight);
	HBITMAP bmpMem2Old = (HBITMAP)SelectObject(hdcMem2, bmpMem2);

	// build a mask
	SetBkColor(hdcMem, colToTransParent);
	BitBlt(hdcMask, 0, 0, nWidth, nHeight, hdcMem, nXSrc, nYSrc, SRCCOPY);
	SetBkColor(hdcMem, RGB(255, 255, 255));
	BitBlt(hdcMask, 0, 0, nWidth, nHeight, hdcMem, nXSrc, nYSrc, SRCPAINT);

	// Copy the image from the toolbar into the memory DC
	// and draw it (grayed) back into the toolbar.
    // SK: Looks better on the old shell
	COLORREF coltxOld = ::SetTextColor(drawdc, RGB(0, 0, 0));
	COLORREF colbkOld = ::SetBkColor(drawdc, RGB(255, 255, 255));
	::SetBkColor(hdcMem2, RGB(0, 0, 0));
#if 0
	::SetTextColor(hdcMem2, ::GetSysColor(COLOR_BTNHILIGHT));
	::BitBlt(hdcMem2, 0, 0, nWidth, nHeight, hdcMask, 0, 0, SRCCOPY);
	::BitBlt(drawdc, nXDest + 1, nYDest + 1, nWidth, nHeight, hdcMask, 0, 0, SRCAND);
	::BitBlt(drawdc, nXDest + 1, nYDest + 1, nWidth, nHeight, hdcMem2, 0, 0, SRCPAINT);
	::SetTextColor(hdcMem2, ::GetSysColor(COLOR_BTNSHADOW));
#else
	::SetTextColor(hdcMem2, (::GetSysColor(COLOR_BTNSHADOW) != ::GetSysColor(COLOR_BTNFACE) ? ::GetSysColor(COLOR_3DSHADOW) : ::GetSysColor(COLOR_BTNHILIGHT)));
#endif
	::BitBlt(hdcMem2, 0, 0, nWidth, nHeight, hdcMask, 0, 0, SRCCOPY);
	::BitBlt(drawdc, nXDest, nYDest, nWidth, nHeight, hdcMask, 0, 0, SRCAND);
	::BitBlt(drawdc, nXDest, nYDest, nWidth, nHeight, hdcMem2, 0, 0, SRCPAINT);
	::SetTextColor(drawdc, coltxOld);
	::SetBkColor(drawdc, colbkOld);

	// reset DCs
	SelectObject(hdcMask, bmpMaskOld);
	DeleteDC(hdcMask);

	SelectObject(hdcMem, bmpMemOld);
	DeleteDC(hdcMem);

	::SelectObject(hdcMem2, bmpMem2Old);
	::DeleteObject(bmpMem2);
	::DeleteDC(hdcMem2);

	DeleteObject(bmpMask);
	return;

}

/*! @brief アイコンの描画

	指定されたDCの指定された座標にアイコンを描画する．

	@param index [in] 描画するアイコン番号
	@param dc [in] 描画するDevice Context
	@param x [in] 描画するX座標
	@param y [in] 描画するY座標
	@param fstyle [in] 描画スタイル
	@param bgColor [in] 背景色(透明部分の描画用)

	@note 描画スタイルとして有効なのは，ILD_NORMAL, ILD_MASK
*/
bool ImageListMgr::Draw(
	int index,
	HDC dc,
	int x,
	int y,
	int fstyle
	) const
{
	if (!hIconBitmap) {
		return false;
	}
	if (index < 0) {
		return false;
	}

	if (fstyle == ILD_MASK) {
		DitherBlt2(dc, x, y, cx, cy, hIconBitmap,
			(index % MAX_X) * cx, (index / MAX_X) * cy);
	}else {
		MyBitBlt(dc, x, y, cx, cy, hIconBitmap,
			(index % MAX_X) * cx, (index / MAX_X) * cy, cTrans);
	}
	return true;
}

/*!	アイコン数を返す */
size_t ImageListMgr::Count() const
{
	return nIconCount;
//	return MAX_X * MAX_Y;
}

/*!	アイコンを追加してそのIDを返す */
int ImageListMgr::Add(const TCHAR* szPath)
{
	if ((nIconCount % MAX_X) == 0) {
		Extend();
	}
	size_t index = nIconCount;
	++nIconCount;

	// アイコンを読み込む
	HBITMAP hExtBmp = (HBITMAP)::LoadImage(NULL, szPath, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	if (!hExtBmp) {
		return -1;
	}

	// hIconBitmapにコピーする
	HDC hDestDC = ::CreateCompatibleDC(0);
	HBITMAP hOldDestBmp = (HBITMAP)::SelectObject(hDestDC, hIconBitmap);

	HDC hExtDC = ::CreateCompatibleDC(0);
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hExtDC, hExtBmp);
	COLORREF cTrans = GetPixel(hExtDC, 0, 0);//	取得した画像の(0,0)の色を背景色として使う
	::SelectObject(hExtDC, hOldBmp);
	::DeleteDC(hExtDC);

	MyBitBlt(
		hDestDC,
		(int)((index % MAX_X) * cx),
		(int)((index / MAX_X) * cy),
		cx, cy, hExtBmp, 0, 0, cTrans
	);

	::SelectObject(hDestDC, hOldDestBmp);
	::DeleteDC(hDestDC);
	::DeleteObject(hExtBmp);

	return (int)index;
}

// ビットマップを一行（MAX_X個）拡張する
void ImageListMgr::Extend(bool bExtend)
{
	size_t curY = nIconCount / MAX_X;
	if (curY < MAX_Y) {
		curY = MAX_Y;
	}

	HDC hSrcDC = ::CreateCompatibleDC(0);
	HBITMAP hSrcBmpOld = (HBITMAP)::SelectObject(hSrcDC, hIconBitmap);

	// 1行拡張したビットマップを作成
	HDC hDestDC = ::CreateCompatibleDC(hSrcDC);
	HBITMAP hDestBmp = ::CreateCompatibleBitmap(hSrcDC, (int)(MAX_X * cx), (int)(curY + (bExtend ? 1 : 0)) * cy);
	HBITMAP hDestBmpOld = (HBITMAP)::SelectObject(hDestDC, hDestBmp);

	::BitBlt(hDestDC, 0, 0, (int)(MAX_X * cx), (int)(curY * cy), hSrcDC, 0, 0, SRCCOPY);

	// 拡張した部分は透過色で塗る
	if (bExtend) {
		FillSolidRect(hDestDC, 0, (int)(curY * cy), MAX_X * cx, cy, cTrans);
	}

	::SelectObject(hSrcDC, hSrcBmpOld);
	::DeleteObject(hIconBitmap);
	::DeleteDC(hSrcDC);

	::SelectObject(hDestDC, hDestBmpOld);
	::DeleteDC(hDestDC);

	// ビットマップの差し替え
	hIconBitmap = hDestBmp;
}

void ImageListMgr::ResetExtend()
{
	nIconCount = MAX_TOOLBAR_ICON_COUNT;
	Extend(false);
}

