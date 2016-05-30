/*!	@file
	@brief MRU���X�g�ƌĂ΂�郊�X�g���Ǘ�����

	@author YAZAKI
	@date 2001/12/23  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "MRUFolder.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "uiparts/MenuDrawer.h"	//	����ł����̂��H
#include "util/string_ex2.h"
#include "util/window.h"

/*!	�R���X�g���N�^

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
MruFolder::MruFolder()
{
	// �������B
	pShareData = &GetDllShareData();
}

// �f�X�g���N�^
MruFolder::~MruFolder()
{
	recentFolder.Terminate();
}

/*!
	�t�H���_�������j���[�̍쐬
	
	@param pMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@return �����������j���[�̃n���h��

	2010/5/21 Uchi �g�ݒ���
*/
HMENU MruFolder::CreateMenu(MenuDrawer& menuDrawer) const
{
	HMENU hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	return CreateMenu(hMenuPopUp, menuDrawer);
}

/*!
	�t�H���_�������j���[�̍쐬
	
	@param �ǉ����郁�j���[�̃n���h��
	@param pMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@author Norio Nakantani
	@return ���j���[�̃n���h��
*/
HMENU MruFolder::CreateMenu(HMENU	hMenuPopUp, MenuDrawer& menuDrawer) const
{
	TCHAR szMenu[_MAX_PATH * 2 + 10];				//	���j���[�L���v�V����

	NONCLIENTMETRICS met;
	met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
	DCFont dcFont(met.lfMenuFont);

	FileNameManager::getInstance().TransformFileName_MakeCache();
	for (size_t i=0; i<recentFolder.GetItemCount(); ++i) {
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if (i >= recentFolder.GetViewCount()) {
			break;
		}

		const TCHAR* pszFolder = recentFolder.GetItemText(i);
		bool bFavorite = recentFolder.IsFavorite(i);
		bool bFavoriteLabel = bFavorite && !pShareData->common.window.bMenuIcon;
		FileNameManager::getInstance().GetMenuFullLabel(szMenu, _countof(szMenu), true, pszFolder, -1, false, CODE_NONE, bFavoriteLabel, i, true, dcFont.GetHDC());

		// ���j���[�ɒǉ�
		menuDrawer.MyAppendMenu(
			hMenuPopUp,
			MF_BYPOSITION | MF_STRING,
			IDM_SELOPENFOLDER + i,
			szMenu,
			_T(""),
			TRUE,
			bFavorite ? F_FAVORITE : -1
		);
	}
	return hMenuPopUp;
}

std::vector<LPCTSTR> MruFolder::GetPathList() const
{
	std::vector<LPCTSTR> ret;
	for (size_t i=0; i<recentFolder.GetItemCount(); ++i) {
		// �u���ʐݒ�v���u�S�ʁv���u�t�H���_�̗���MAX�v�𔽉f
		if (i >= recentFolder.GetViewCount()) {
			break;
		}
		ret.push_back(recentFolder.GetItemText(i));
	}
	return ret;
}

size_t MruFolder::Length() const
{
	return recentFolder.GetItemCount();
}

void MruFolder::ClearAll()
{
	recentFolder.DeleteAllItem();
}

/*	@brief �J�����t�H���_ ���X�g�ւ̓o�^

	@date 2001.12.26  CShareData::AddOPENFOLDERList����ړ������B�iYAZAKI�j
*/
void MruFolder::Add(const TCHAR* pszFolder)
{
	if (!pszFolder
	 || pszFolder[0] == _T('\0')
	) {	//	������0�Ȃ�r���B
		return;
	}

	// ���łɓo�^����Ă���ꍇ�́A���O�w��𖳎�����
	if (recentFolder.FindItemByText(pszFolder) == -1) {
		size_t nSize = pShareData->history.aExceptMRU.size();
		for (size_t i=0; i<nSize; ++i) {
			TCHAR szExceptMRU[_MAX_PATH];
			FileNameManager::ExpandMetaToFolder(pShareData->history.aExceptMRU[i], szExceptMRU, _countof(szExceptMRU));
			if (_tcsistr(pszFolder, szExceptMRU)) {
				return;
			}
		}
	}

	recentFolder.AppendItem(pszFolder);
}

const TCHAR* MruFolder::GetPath(int num) const
{
	return recentFolder.GetItemText(num);
}

