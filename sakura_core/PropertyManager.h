#pragma once

#include "prop/PropCommon.h"
#include "typeprop/PropTypes.h"

class ImageListMgr;
class MenuDrawer;

class PropertyManager {
public:
	void Create(HWND, ImageListMgr*, MenuDrawer*);

	/*
	|| その他
	*/
	bool OpenPropertySheet(HWND hWnd, int nPageNum, bool bTrayProc);	// 共通設定
	bool OpenPropertySheetTypes(HWND hWnd, int nPageNum, TypeConfigNum nSettingType);	// タイプ別設定

private:
	HWND			hwndOwner;
	ImageListMgr*	pImageList;
	MenuDrawer*		pMenuDrawer;

	int				nPropComPageNum;
	int				nPropTypePageNum;
};

