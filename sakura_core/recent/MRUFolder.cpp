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
	m_pShareData = &GetDllShareData();
}

// �f�X�g���N�^
MruFolder::~MruFolder()
{
	m_recentFolder.Terminate();
}

/*!
	�t�H���_�������j���[�̍쐬
	
	@param pMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@return �����������j���[�̃n���h��

	2010/5/21 Uchi �g�ݒ���
*/
HMENU MruFolder::CreateMenu(MenuDrawer* pMenuDrawer) const
{
	HMENU hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	return CreateMenu(hMenuPopUp, pMenuDrawer);
}

/*!
	�t�H���_�������j���[�̍쐬
	
	@param �ǉ����郁�j���[�̃n���h��
	@param pMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@author Norio Nakantani
	@return ���j���[�̃n���h��
*/
HMENU MruFolder::CreateMenu(HMENU	hMenuPopUp, MenuDrawer* pMenuDrawer) const
{
	TCHAR szMenu[_MAX_PATH * 2 + 10];				//	���j���[�L���v�V����

	NONCLIENTMETRICS met;
	met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
	DCFont dcFont(met.lfMenuFont);

	FileNameManager::getInstance()->TransformFileName_MakeCache();
	for (int i=0; i<m_recentFolder.GetItemCount(); ++i) {
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if (i >= m_recentFolder.GetViewCount()) {
			break;
		}

		const TCHAR* pszFolder = m_recentFolder.GetItemText(i);
		bool bFavorite = m_recentFolder.IsFavorite(i);
		bool bFavoriteLabel = bFavorite && !m_pShareData->m_common.window.bMenuIcon;
		FileNameManager::getInstance()->GetMenuFullLabel(szMenu, _countof(szMenu), true, pszFolder, -1, false, CODE_NONE, bFavoriteLabel, i, true, dcFont.GetHDC());

		// ���j���[�ɒǉ�
		pMenuDrawer->MyAppendMenu(
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
	for (int i=0; i<m_recentFolder.GetItemCount(); ++i) {
		// �u���ʐݒ�v���u�S�ʁv���u�t�H���_�̗���MAX�v�𔽉f
		if (i >= m_recentFolder.GetViewCount()) {
			break;
		}
		ret.push_back(m_recentFolder.GetItemText(i));
	}
	return ret;
}

int MruFolder::Length() const
{
	return m_recentFolder.GetItemCount();
}

void MruFolder::ClearAll()
{
	m_recentFolder.DeleteAllItem();
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
	if (m_recentFolder.FindItemByText(pszFolder) == -1) {
		int nSize = m_pShareData->m_history.m_aExceptMRU.size();
		for (int i=0; i<nSize; ++i) {
			TCHAR szExceptMRU[_MAX_PATH];
			FileNameManager::ExpandMetaToFolder(m_pShareData->m_history.m_aExceptMRU[i], szExceptMRU, _countof(szExceptMRU));
			if (_tcsistr(pszFolder, szExceptMRU)) {
				return;
			}
		}
	}

	m_recentFolder.AppendItem(pszFolder);
}

const TCHAR* MruFolder::GetPath(int num) const
{
	return m_recentFolder.GetItemText(num);
}

