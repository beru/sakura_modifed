/*!	@file
	@brief 文字列共通定義
*/

#include "StdAfx.h"
#include "global.h"
#include "window/EditWnd.h"
#include "NormalProcess.h"

// EditWndのインスタンスへのポインタをここに保存しておく
EditWnd* g_pcEditWnd = nullptr;


// 選択領域描画用パラメータ
const COLORREF	SELECTEDAREA_RGB = RGB(255, 255, 255);
const int		SELECTEDAREA_ROP2 = R2_XORPEN;


HINSTANCE G_AppInstance()
{
	return Process::getInstance()->GetProcessInstance();
}

