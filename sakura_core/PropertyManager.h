#pragma once

#include "prop/PropCommon.h"
#include "typeprop/PropTypes.h"

class ImageListMgr;
class MenuDrawer;

class PropertyManager {
public:
	void Create(HWND, ImageListMgr*, MenuDrawer*);

	/*
	|| ÇªÇÃëº
	*/
	bool OpenPropertySheet(HWND hWnd, int nPageNum, bool bTrayProc);	// ã§í ê›íË
	bool OpenPropertySheetTypes(HWND hWnd, int nPageNum, TypeConfigNum nSettingType);	// É^ÉCÉvï ê›íË

private:
	HWND			hwndOwner;
	ImageListMgr*	pImageList;
	MenuDrawer*		pMenuDrawer;

	int				nPropComPageNum;
	int				nPropTypePageNum;
};

