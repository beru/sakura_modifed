/*!	@file
@brief CViewCommander�N���X�̃R�}���h(�J�X�^�����j���[)�֐��Q

	2012/12/20	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2006, fon
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include <vector>

// �E�N���b�N���j���[
void CViewCommander::Command_MENU_RBUTTON(void)
{
//	HGLOBAL		hgClip;
//	char*		pszClip;
	// �|�b�v�A�b�v���j���[(�E�N���b�N)
	int nId = m_pCommanderView->CreatePopUpMenu_R();
	if (nId == 0) {
		return;
	}
	switch (nId) {
	case IDM_COPYDICINFO:
		{
			int nLength;
			const TCHAR* pszStr = m_pCommanderView->m_cTipWnd.m_cInfo.GetStringPtr(&nLength);
			std::vector<TCHAR> szWork(nLength + 1);
			TCHAR* pszWork = &szWork[0];
			auto_memcpy(pszWork, pszStr, nLength);
			pszWork[nLength] = _T('\0');

			// �����ڂƓ����悤�ɁA\n �� CR+LF�֕ϊ�����
			for (int i=0; i<nLength; ++i) {
				if (pszWork[i] == _T('\\') && pszWork[i + 1] == _T('n')) {
					pszWork[i] = WCODE::CR;
					pszWork[i+1] = WCODE::LF;
				}
			}
			// �N���b�v�{�[�h�Ƀf�[�^��ݒ�
			m_pCommanderView->MySetClipboardData(pszWork, nLength, false);
		}
		break;
	case IDM_JUMPDICT:
		// �L�[���[�h�����t�@�C�����J��
		if (m_pCommanderView->m_pTypeData->m_bUseKeyWordHelp) {		// �L�[���[�h�����Z���N�g���g�p����	// 2006.04.10 fon
			// Feb. 17, 2007 genta ���΃p�X�����s�t�@�C����ŊJ���悤��
			m_pCommanderView->TagJumpSub(
				m_pCommanderView->m_pTypeData->m_KeyHelpArr[m_pCommanderView->m_cTipWnd.m_nSearchDict].m_szPath,
				CMyPoint(1, m_pCommanderView->m_cTipWnd.m_nSearchLine),
				0,
				true
			);
		}
		break;

	default:
		// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
//		HandleCommand(nId, true, 0, 0, 0, 0);
		::PostMessage(GetMainWindow(), WM_COMMAND, MAKELONG(nId, 0),  (LPARAM)NULL);
		break;
	}
	return;
}


// �J�X�^�����j���[�\��
int CViewCommander::Command_CUSTMENU(int nMenuIdx)
{
	GetEditWindow()->GetMenuDrawer().ResetContents();

	// Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = GetDocument()->m_cFuncLookup;

	if (nMenuIdx < 0 || MAX_CUSTOM_MENU <= nMenuIdx) {
		return 0;
	}
	if (GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx] == 0) {
		return 0;
	}
	HMENU hMenu = ::CreatePopupMenu();
	return m_pCommanderView->CreatePopUpMenuSub(hMenu, nMenuIdx, NULL);
}

