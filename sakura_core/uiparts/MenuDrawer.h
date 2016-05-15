/*!	@file
	@brief ���j���[�Ǘ����\��

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, MIK
	Copyright (C) 2005, aroka, genta
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "Funccode_enum.h"

class MenuDrawer;

class ImageListMgr;// 2002/2/10 aroka
struct DllSharedData;

//#define MAX_MENUPOS	10
//	Jul. 2, 2005 genta : �}�N������������o�^����Ə���𒴂��Ă��܂��̂�
//	�����l��300���瑝�₷
// #define MAX_MENUITEMS	400 // 2011.11.23 Moca ����P�p

// �c�[���o�[�̊g��	//@@@ 2002.06.15 MIK
#define TBSTYLE_COMBOBOX	((BYTE)0x40)	// �c�[���o�[�ɃR���{�{�b�N�X
#ifndef TBSTYLE_DROPDOWN	// IE3�ȏ�
	#define TBSTYLE_DROPDOWN	0x0008
#endif
#ifndef TB_SETEXTENDEDSTYLE	// IE4�ȏ�
	#define TB_SETEXTENDEDSTYLE     (WM_USER + 84)  // For TBSTYLE_EX_*
#endif
#ifndef TBSTYLE_EX_DRAWDDARROWS	// IE4�ȏ�
	#define TBSTYLE_EX_DRAWDDARROWS 0x00000001
#endif

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief ���j���[�\�����Ǘ�

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 20050809 aroka �N���X�O������A�N�Z�X����Ȃ������o��private�ɂ����B
*/
class MenuDrawer {
public:
	/*
	||  Constructors
	*/
	MenuDrawer();
	~MenuDrawer();
	void Create(HINSTANCE, HWND, ImageListMgr*);

	/*
	||  Attributes & Operations
	*/
	void ResetContents(void);
	//void MyAppendMenu(HMENU , int , int , const char*, BOOL = TRUE);	/* ���j���[���ڂ�ǉ� */
	void MyAppendMenu(HMENU hMenu, int nFlag, UINT_PTR nFuncId, const TCHAR*     pszLabel, const TCHAR*     pszKey, bool bAddKeyStr = true, int nForceIconId = -1);	/* ���j���[���ڂ�ǉ� */	// ���C�ɓ���	//@@@ 2003.04.08 MIK	// add pszKey	2010/5/17 Uchi
	void MyAppendMenu(HMENU hMenu, int nFlag, UINT_PTR nFuncId, const NOT_TCHAR* pszLabel, const NOT_TCHAR* pszKey, bool bAddKeyStr = true, int nForceIconId = -1) {
		MyAppendMenu(hMenu, nFlag, nFuncId, to_tchar(pszLabel), to_tchar(pszKey), bAddKeyStr, nForceIconId);
	}
	void MyAppendMenuSep(HMENU hMenu, int nFlag, int nFuncId, const TCHAR* pszLabel, bool bAddKeyStr = true, int nForceIconId = -1) {
		MyAppendMenu(hMenu, nFlag, nFuncId, pszLabel, _T(""), bAddKeyStr, nForceIconId);
	}
	int MeasureItem(int, int*);	/* ���j���[�A�C�e���̕`��T�C�Y���v�Z */
	void DrawItem(DRAWITEMSTRUCT*);	/* ���j���[�A�C�e���`�� */
	void EndDrawMenu();
	LRESULT OnMenuChar(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	int FindToolbarNoFromCommandId(int idCommand, bool bOnlyFunc = true)const; // �c�[���o�[No�̎擾
	int GetIconIdByFuncId(int nIndex) const;

	TBBUTTON getButton(int nToolBarNo) const; // 20050809 aroka
	void AddToolButton(int iBitmap, int iCommand);	// �c�[���o�[�{�^����ǉ����� 2009.11.14 syat
	
	// iBitmap�ɑΉ�����萔
	static const int TOOLBAR_ICON_MACRO_INTERNAL = 384;		// �O���}�N������A�C�R��
	static const int TOOLBAR_ICON_PLUGCOMMAND_DEFAULT = 283;// �v���O�C���R�}���h����A�C�R��
	// tbMyButton��index�ɑΉ�����萔
	static const int TOOLBAR_BUTTON_F_SEPARATOR = 0;		// �Z�p���[�^�i�_�~�[�j
	static const int TOOLBAR_BUTTON_F_TOOLBARWRAP = 384;	// �c�[���o�[�ܕԂ��A�C�R���i�_�~�[�j

private:
	void DeleteCompDC();
	int FindIndexFromCommandId(int idCommand, bool bOnlyFunc = true) const;  /* �c�[���o�[Index�̎擾 */// 20050809 aroka
	int Find(int nFuncID);
	const TCHAR* GetLabel(int nFuncID);
	TCHAR GetAccelCharFromLabel(const TCHAR* pszLabel);
	int ToolbarNoToIndex(int nToolbarNo) const;

private:
	DllSharedData*	pShareData;

	HINSTANCE		hInstance;
	HWND			hWndOwner;

//@@@ 2002.01.03 YAZAKI tbMyButton�Ȃǂ�CShareData����MenuDrawer�ֈړ��������Ƃɂ��C���B
// 2009.11.14 syat �v���O�C���R�}���h���I�ǉ��̂���vector��
	std::vector<TBBUTTON>	tbMyButton;	// �c�[���o�[�̃{�^��
	int				nMyButtonNum;
	int				nMyButtonFixSize;	// �Œ蕔���̍ő吔
	
	// 2011.11.18 MenuItem��vector��
	struct MyMenuItemInfo {
		int				nBitmapIdx;
		int				nFuncId;
		NativeT			memLabel;
	};
	std::vector<MyMenuItemInfo> menuItems;
	int				nMenuHeight;
	int				nMenuFontHeight;
	HFONT			hFontMenu;
	HBITMAP			hCompBitmap;
	HBITMAP			hCompBitmapOld;
	HDC				hCompDC;
	int				nCompBitmapHeight;
	int				nCompBitmapWidth;

public:
	// 2010.01.30 syat �A�C�R���C���[�W���X�g��private->public
	//	Oct. 16, 2000 genta
	ImageListMgr* pIcons;	//	Image List

protected:
	/*
	||  �����w���p�֐�
	*/
	int GetData(void);	/* �_�C�A���O�f�[�^�̎擾 */

//@@@ 2002.01.03 YAZAKI tbMyButton�Ȃǂ�CShareData����MenuDrawer�ֈړ��������Ƃɂ��C���B
	void SetTBBUTTONVal(TBBUTTON*, int, int, BYTE, BYTE, DWORD_PTR, INT_PTR) const;	/* TBBUTTON�\���̂Ƀf�[�^���Z�b�g */
};

