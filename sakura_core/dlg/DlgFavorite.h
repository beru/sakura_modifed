/*!	@file
	@brief �����̊Ǘ��_�C�A���O�{�b�N�X

	@author MIK
	@date 2003.4.8
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2010, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#pragma once

#include "dlg/Dialog.h"
#include "recent/Recent.h"

// �����Ƃ��C�ɓ���̊Ǘ��v�_�C�A���O
// �A�N�Z�X���@�F[�ݒ�] - [�����̊Ǘ�]
class DlgFavorite : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgFavorite();
	~DlgFavorite();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// ���[�_���_�C�A���O�̕\��

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	BOOL	OnBnClicked(int);
	BOOL	OnNotify(WPARAM wParam, LPARAM lParam);
	BOOL	OnActivate(WPARAM wParam, LPARAM lParam);
	LPVOID	GetHelpIdTable(void);
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// �W���ȊO�̃��b�Z�[�W��ߑ�����
	BOOL	OnSize(WPARAM wParam, LPARAM lParam);
	BOOL	OnMove(WPARAM wParam, LPARAM lParam);
	BOOL	OnMinMaxInfo(LPARAM lParam);

	void	SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int		GetData(void);	// �_�C�A���O�f�[�^�̎擾

	void	TabSelectChange(bool);
	bool	RefreshList(void);
	void	SetDataOne(int nIndex, int nLvItemIndex);	// �_�C�A���O�f�[�^�̐ݒ�
	bool	RefreshListOne(int nIndex);
	//void	ChangeSlider(int nIndex);
	void	UpdateUIState();
	
	void    GetFavorite(int nIndex);
	int     DeleteSelected();
	void	AddItem();
	void	EditItem();
	void	RightMenu(POINT&);

private:
	RecentFile			recentFile;
	RecentFolder		recentFolder;
	RecentExceptMRU		recentExceptMRU;
	RecentSearch		recentSearch;
	RecentReplace		recentReplace;
	RecentGrepFile		recentGrepFile;
	RecentGrepFolder	recentGrepFolder;
	RecentCmd			recentCmd;
	RecentCurDir		recentCurDir;

	enum {
		// �Ǘ���
		FAVORITE_INFO_MAX = 10 // �Ǘ��� +1(�ԕ�)
	};

	struct FavoriteInfo {
		Recent*	pRecent;			// �I�u�W�F�N�g�ւ̃|�C���^
		std::tstring	strCaption;	// �L���v�V����
		const TCHAR*	pszCaption;	// �L���v�V����
		int			nId;				// �R���g���[����ID
		bool		bHaveFavorite;	// ���C�ɓ���������Ă��邩�H
		bool		bHaveView;		// �\�����ύX�@�\�������Ă��邩�H
		bool		bFilePath;		// �t�@�C��/�t�H���_���H
		bool		bEditable;		// �ҏW�\
		bool		bAddExcept;		// ���O�֒ǉ�
		int			nViewCount;		// �J�����g�̕\����
		FavoriteInfo():
			pRecent(nullptr)
			,pszCaption(NULL)
			,nId(0)
			,bHaveFavorite(false)
			,bHaveView(false)
			,bFilePath(false)
			,bEditable(false)
			,bAddExcept(false)
			,nViewCount(0)
		{};
	};

	struct ListViewSortInfo {
		HWND	hListView;		// ���X�g�r���[�� HWND
		int		nSortColumn;	// �\�[�g�� -1�Ŗ��w��
		bool	bSortAscending; // �\�[�g������
	};

	FavoriteInfo        aFavoriteInfo[FAVORITE_INFO_MAX];
	ListViewSortInfo    aListViewInfo[FAVORITE_INFO_MAX];
	POINT				ptDefaultSize;
	RECT				rcListDefault;
	RECT				rcItems[10];

	int		nCurrentTab;
	int		nExceptTab;
	TCHAR	szMsg[1024];

	static void  ListViewSort(ListViewSortInfo&, const Recent* , int, bool);
};

