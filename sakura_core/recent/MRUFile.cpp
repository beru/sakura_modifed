/*!	@file
	@brief MRU���X�g�ƌĂ΂�郊�X�g���Ǘ�����

	@author YAZAKI
	@date 2001/12/23  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, YAZAKI
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, MIK
	Copyright (C) 2006, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <ShlObj.h>
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/FileNameManager.h"
#include "uiparts/MenuDrawer.h"	//	����ł����̂��H
#include "window/EditWnd.h"
#include "util/string_ex2.h"
#include "util/window.h"

/*!	�R���X�g���N�^
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
MruFile::MruFile()
{
	//	�������B
	m_pShareData = &GetDllShareData();
}

// �f�X�g���N�^
MruFile::~MruFile()
{
	m_recentFile.Terminate();
}

/*!
	�t�@�C���������j���[�̍쐬
	
	@param pMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@author Norio Nakantani
	@return �����������j���[�̃n���h��

	2010/5/21 Uchi �g�ݒ���
*/
HMENU MruFile::CreateMenu(MenuDrawer* pMenuDrawer) const
{
	//	�󃁃j���[�����
	HMENU hMenuPopUp = ::CreatePopupMenu();	// Jan. 29, 2002 genta
	return CreateMenu(hMenuPopUp, pMenuDrawer);
}
/*!
	�t�@�C���������j���[�̍쐬
	
	@param �ǉ����郁�j���[�̃n���h��
	@param pMenuDrawer [in] (out?) ���j���[�쐬�ŗp����MenuDrawer
	
	@author Norio Nakantani
	@return �����������j���[�̃n���h��

	2010/5/21 Uchi �g�ݒ���
*/
HMENU MruFile::CreateMenu(HMENU hMenuPopUp, MenuDrawer* pMenuDrawer) const
{
	TCHAR szMenu[_MAX_PATH * 2 + 10];				//	���j���[�L���v�V����
	const BOOL bMenuIcon = m_pShareData->common.window.bMenuIcon;
	FileNameManager::getInstance().TransformFileName_MakeCache();

	NONCLIENTMETRICS met;
	met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
	DCFont dcFont(met.lfMenuFont);

	for (int i=0; i<m_recentFile.GetItemCount(); ++i) {
		//	�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if (i >= m_recentFile.GetViewCount()) {
			break;
		}
		
		// MRU���X�g�̒��ɂ���J����Ă��Ȃ��t�@�C��

		const EditInfo* p = m_recentFile.GetItem(i);
		bool bFavorite = m_recentFile.IsFavorite(i);
		bool bFavoriteLabel = bFavorite && !bMenuIcon;
		FileNameManager::getInstance().GetMenuFullLabel_MRU(szMenu, _countof(szMenu), p, -1, bFavoriteLabel, i, dcFont.GetHDC());

		// ���j���[�ɒǉ��B
		pMenuDrawer->MyAppendMenu(
			hMenuPopUp,
			MF_BYPOSITION | MF_STRING,
			IDM_SELMRU + i,
			szMenu,
			_T(""),
			TRUE,
			bFavorite ? F_FAVORITE : -1
		);
	}
	return hMenuPopUp;
}

BOOL MruFile::DestroyMenu(HMENU hMenuPopUp) const
{
	return ::DestroyMenu(hMenuPopUp);
}

/*!
	�t�@�C�������̈ꗗ��Ԃ�
	
	@param ppszMRU [out] ������ւ̃|�C���^���X�g���i�[����D
	�Ō�̗v�f�̎��ɂ�NULL������D
	�\�ߌĂяo�����ōő�l+1�̗̈���m�ۂ��Ă������ƁD
*/
std::vector<LPCTSTR> MruFile::GetPathList() const
{
	std::vector<LPCTSTR> ret;
	for (int i=0; i<m_recentFile.GetItemCount(); ++i) {
		//�u���ʐݒ�v���u�S�ʁv���u�t�@�C���̗���MAX�v�𔽉f
		if (i >= m_recentFile.GetViewCount()) {
			break;
		}
		ret.push_back(m_recentFile.GetItemText(i));
	}
	return ret;
}

// �A�C�e������Ԃ�
int MruFile::Length(void) const
{
	return m_recentFile.GetItemCount();
}

/*!
	�t�@�C�������̃N���A
*/
void MruFile::ClearAll(void)
{
	m_recentFile.DeleteAllItem();
}

/*!
	�t�@�C�����̎擾
	
	@param num [in] ����ԍ�(0~)
	@param pfi [out] �\���̂ւ̃|�C���^�i�[��
	
	@retval TRUE �f�[�^���i�[���ꂽ
	@retval FALSE �������Ȃ��ԍ����w�肳�ꂽ�D�f�[�^�͊i�[����Ȃ������D
*/
bool MruFile::GetEditInfo(int num, EditInfo* pfi) const
{
	const EditInfo*	p = m_recentFile.GetItem(num);
	if (!p) {
		return false;
	}

	*pfi = *p;

	return true;
}

/*!
	�w�肳�ꂽ���O�̃t�@�C����MRU���X�g�ɑ��݂��邩���ׂ�B���݂���Ȃ�΃t�@�C������Ԃ��B

	@param pszPath [in] ��������t�@�C����
	@param pfi [out] �f�[�^�����������Ƃ��Ƀt�@�C�������i�[����̈�B
		�Ăяo�����ŗ̈�����炩���ߗp�ӂ���K�v������B
	@retval TRUE  �t�@�C�������������Bpfi�Ƀt�@�C����񂪊i�[����Ă���B
	@retval FALSE �w�肳�ꂽ�t�@�C����MRU List�ɖ����B

	@date 2001.12.26 CShareData::IsExistInMRUList����ړ������B�iYAZAKI�j
*/
bool MruFile::GetEditInfo(const TCHAR* pszPath, EditInfo* pfi) const
{
	const EditInfo*	p = m_recentFile.GetItem(m_recentFile.FindItemByPath(pszPath));
	if (!p) {
		return false;
	}

	*pfi = *p;

	return true;
}

/*!	@brief MRU���X�g�ւ̓o�^

	@param pEditInfo [in] �ǉ�����t�@�C���̏��

	�Y���t�@�C���������[�o�u���f�B�X�N��ɂ���ꍇ�ɂ�MRU List�ւ̓o�^�͍s��Ȃ��B

	@date 2001.03.29 MIK �����[�o�u���f�B�X�N��̃t�@�C����o�^���Ȃ��悤�ɂ����B
	@date 2001.12.26 YAZAKI CShareData::AddMRUList����ړ�
*/
void MruFile::Add(EditInfo* pEditInfo)
{
	//	�t�@�C������������Ζ���
	if (!pEditInfo || pEditInfo->szPath[0] == L'\0') {
		return;
	}
	
	// ���łɓo�^����Ă���ꍇ�́A���O�w��𖳎�����
	if (m_recentFile.FindItemByPath(pEditInfo->szPath) == -1) {
		int nSize = m_pShareData->history.m_aExceptMRU.size();
		for (int i=0; i<nSize; ++i) {
			TCHAR szExceptMRU[_MAX_PATH];
			FileNameManager::ExpandMetaToFolder(m_pShareData->history.m_aExceptMRU[i], szExceptMRU, _countof(szExceptMRU));
			if (_tcsistr(pEditInfo->szPath,  szExceptMRU)) {
				return;
			}
		}
	}
	EditInfo tmpEditInfo = *pEditInfo;
	tmpEditInfo.bIsModified = false; // �ύX�t���O�𖳌���

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szFolder[_MAX_PATH + 1];	//	�h���C�u�{�t�H���_
	
	_tsplitpath(pEditInfo->szPath, szDrive, szDir, NULL, NULL);	//	�h���C�u�ƃt�H���_�����o���B

	//	Jan.  10, 2006 genta USB��������Removable media�ƔF�������悤�Ȃ̂ŁC
	//	�ꉞ����������D
	//	�����[�o�u���Ȃ��o�^�H
	//if (/* �u�����[�o�u���Ȃ�o�^���Ȃ��v�I�� && */ ! IsLocalDrive(szDrive)) {
	//	return;
	//}

	//	szFolder�쐬
	_tcscpy(szFolder, szDrive);
	_tcscat(szFolder, szDir);

	//	Folder���AMruFolder�ɓo�^
	MruFolder mruFolder;
	mruFolder.Add(szFolder);

	m_recentFile.AppendItem(&tmpEditInfo);
	
	::SHAddToRecentDocs(SHARD_PATH, to_wchar(pEditInfo->szPath));
}

// EOF
