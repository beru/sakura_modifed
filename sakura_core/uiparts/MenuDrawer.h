#pragma once

#include "Funccode_enum.h"

class MenuDrawer;

class ImageListMgr;
struct DllSharedData;

// �c�[���o�[�̊g��
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
	void MyAppendMenu(HMENU hMenu, int nFlag, UINT_PTR nFuncId, const TCHAR*     pszLabel, const TCHAR*     pszKey, bool bAddKeyStr = true, int nForceIconId = -1);	/* ���j���[���ڂ�ǉ� */
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

	TBBUTTON getButton(int nToolBarNo) const;
	void AddToolButton(int iBitmap, int iCommand);	// �c�[���o�[�{�^����ǉ�����
	
	// iBitmap�ɑΉ�����萔
	static const int TOOLBAR_ICON_MACRO_INTERNAL = 384;		// �O���}�N������A�C�R��
	static const int TOOLBAR_ICON_PLUGCOMMAND_DEFAULT = 283;// �v���O�C���R�}���h����A�C�R��
	// tbMyButton��index�ɑΉ�����萔
	static const int TOOLBAR_BUTTON_F_SEPARATOR = 0;		// �Z�p���[�^�i�_�~�[�j
	static const int TOOLBAR_BUTTON_F_TOOLBARWRAP = 384;	// �c�[���o�[�ܕԂ��A�C�R���i�_�~�[�j

private:
	void DeleteCompDC();
	int FindIndexFromCommandId(int idCommand, bool bOnlyFunc = true) const;  /* �c�[���o�[Index�̎擾 */
	int Find(int nFuncID);
	const TCHAR* GetLabel(int nFuncID);
	TCHAR GetAccelCharFromLabel(const TCHAR* pszLabel);
	int ToolbarNoToIndex(int nToolbarNo) const;

private:
	DllSharedData*	pShareData;

	HINSTANCE		hInstance;
	HWND			hWndOwner;

	std::vector<TBBUTTON>	tbMyButton;	// �c�[���o�[�̃{�^��
	size_t nMyButtonNum;
	size_t nMyButtonFixSize;	// �Œ蕔���̍ő吔
	
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
	ImageListMgr* pIcons;	//	Image List

protected:
	/*
	||  �����w���p�֐�
	*/
	int GetData(void);	/* �_�C�A���O�f�[�^�̎擾 */

	void SetTBBUTTONVal(TBBUTTON*, int, int, BYTE, BYTE, DWORD_PTR, INT_PTR) const;	/* TBBUTTON�\���̂Ƀf�[�^���Z�b�g */
};

