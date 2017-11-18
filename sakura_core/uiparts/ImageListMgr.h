#pragma once

#include "_main/global.h"

/*! @brief ImageListの管理

	アイコンイメージを管理する

	@note イメージリストへのビットマップの登録でBitbltを行う部分が
		VAIOと相性が悪くブルースクリーンが発生していた．
		またイメージリストがIE3以前のcommon componentに含まれていないために
		初期Win95でイメージの表示ができなかった．それらを回避するためにImageListの使用をやめて
		当初の独自描画に戻した．
*/
class ImageListMgr {
public:

	//	constructor
	ImageListMgr();
	~ImageListMgr();

	bool Create(HINSTANCE hInstance);	//	生成
	
	/*! @brief アイコンの描画
	
		指定されたDCの指定された座標にアイコンを描画する．
	
		@param index [in] 描画するアイコン番号
		@param dc [in] 描画するDevice Context
		@param x [in] 描画するX座標
		@param y [in] 描画するY座標
		@param fstyle [in] 描画スタイル
	*/
	bool Draw(int index, HDC dc, int x, int y, int fstyle) const	//	描画
	;
	
	// アイコン数を返す
	size_t Count(void) const;	//	アイコン数
	
	// アイコンの幅
	int  GetCx(void) const { return cx; }
	// アイコンの高さ
	int  GetCy(void) const { return cy; }
	
	// アイコンを追加する
	int Add(const TCHAR* szPath);

	// アイコンの追加を元に戻す
	void ResetExtend();

	/*!
		イメージのToolBarへの登録
	
		@param hToolBar [in] 登録するToolBar
		@param id [in] 登録する先頭アイコン番号
	*/
	void  SetToolBarImages(HWND hToolBar, int id = 0) const {}

protected:
	int cx;			// width of icon
	int cy;			// height of icon
	/*!	@brief 透過色
	
		描画を自前で行うため，透過色を覚えておく必要がある．
	*/
	COLORREF cTrans;
	
	/*! アイコン用ビットマップを保持する	*/
	HBITMAP hIconBitmap;

	size_t nIconCount;	// アイコンの個数

	//	オリジナルテキストエディタからの描画関数
	void MyBitBlt(HDC drawdc, int nXDest, int nYDest, 
					int nWidth, int nHeight, HBITMAP bmp,
					int nXSrc, int nYSrc, COLORREF colToTransParent) const;
	void DitherBlt2(HDC drawdc, int nXDest, int nYDest, int nWidth, 
                        int nHeight, HBITMAP bmp, int nXSrc, int nYSrc) const;

	// ビットマップを一行拡張する
	void Extend(bool = true);

};

