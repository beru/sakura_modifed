#pragma once

#include "prop/PropCommon.h"
#include "typeprop/PropTypes.h"

class ImageListMgr;
class MenuDrawer;

class PropertyManager {
public:
	void Create(HWND, ImageListMgr*, MenuDrawer*);

	/*
	|| ���̑�
	*/
	bool OpenPropertySheet(HWND hWnd, int nPageNum, bool bTrayProc);	// ���ʐݒ�
	bool OpenPropertySheetTypes(HWND hWnd, int nPageNum, TypeConfigNum nSettingType);	// �^�C�v�ʐݒ�

private:
	HWND			hwndOwner;
	ImageListMgr*	pImageList;
	MenuDrawer*		pMenuDrawer;

	int				nPropComPageNum;
	int				nPropTypePageNum;
};

