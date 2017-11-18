#pragma once

#include <Windows.h>

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
// 砂時計カーソルクラス
/*!
	オブジェクトの存続している間カーソル形状を砂時計にする．
	オブジェクトが破棄されるとカーソル形状は元に戻る
*/
class WaitCursor {
public:
	/*
	||  Constructors
	*/
	WaitCursor(HWND, bool bEnable = true);
	~WaitCursor();

	bool IsEnable() { return bEnable; }
private: // 2002/2/10 aroka
	HCURSOR	hCursor;
	HCURSOR	hCursorOld;
	bool	bEnable;
};

