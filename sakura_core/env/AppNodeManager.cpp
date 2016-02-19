/*!	@file
	@brief �A�v���P�[�V�����m�[�h�}�l�[�W��

	@author kobake
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2009, syat
	Copyright (C) 2011, syat
	Copyright (C) 2012, syat, Uchi
	Copyright (C) 2013, Moca, Uchi

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
#include "StdAfx.h"
#include "env/AppNodeManager.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/SakuraEnvironment.h"
#include "recent/RecentEditNode.h"
#include "util/window.h"
#include "_main/Mutex.h"


// GetOpenedWindowArr�p�ÓI�ϐ��^�\����
static BOOL s_bSort;	// �\�[�g�w��
static BOOL s_bGSort;	// �O���[�v�w��

/*! @brief CShareData::m_pEditArr�ی�pMutex

	�����̃G�f�B�^���񓯊��Ɉ�ē��삵�Ă���Ƃ��ł��ACShareData::m_pEditArr��
	���S�ɑ���ł���悤���쒆��Mutex��Lock()����B

	@par�i�񓯊���ē���̗�j
		�����̃E�B���h�E��\�����Ă��ăO���[�v����L���ɂ����^�X�N�o�[�Łu�O���[�v�����v����������Ƃ�

	@par�i�ی삷��ӏ��̗�j
		CShareData::AddEditWndList(): �G���g���̒ǉ��^���ёւ�
		CShareData::DeleteEditWndList(): �G���g���̍폜
		CShareData::GetOpenedWindowArr(): �z��̃R�s�[�쐬

	����ɂǂ��ɂł������ƃf�b�h���b�N����댯������̂œ����Ƃ��͐T�d�ɁB
	�iLock()���Ԓ���SendMessage()�Ȃǂő��E�B���h�E�̑��������Ɗ댯����j
	CShareData::m_pEditArr�𒼐ڎQ�Ƃ�����ύX����悤�ȉӏ��ɂ͐��ݓI�Ȋ댯�����邪�A
	�Θb�^�ŏ������삵�Ă���͈͂ł���΂܂����͋N���Ȃ��B

	@date 2007.07.05 ryoji �V�K����
	@date 2007.07.07 genta CShareData�̃����o�ֈړ�
*/
static Mutex g_editArrMutex(FALSE, GSTR_MUTEX_SAKURA_EDITARR);

// GetOpenedWindowArr�p�\�[�g�֐�
static int __cdecl cmpGetOpenedWindowArr(const void *e1, const void *e2)
{
	// �قȂ�O���[�v�̂Ƃ��̓O���[�v��r����
	int nGroup1;
	int nGroup2;

	if (s_bGSort) {
		// �I���W�i���̃O���[�v�ԍ��̂ق�������
		nGroup1 = ((EditNodeEx*)e1)->p->m_nGroup;
		nGroup2 = ((EditNodeEx*)e2)->p->m_nGroup;
	}else {
		// �O���[�v��MRU�ԍ��̂ق�������
		nGroup1 = ((EditNodeEx*)e1)->nGroupMru;
		nGroup2 = ((EditNodeEx*)e2)->nGroupMru;
	}
	if (nGroup1 != nGroup2) {
		return nGroup1 - nGroup2;	// �O���[�v��r
	}

	// �O���[�v��r���s���Ȃ������Ƃ��̓E�B���h�E��r����
	if (s_bSort) {
		return (((EditNodeEx*)e1)->p->m_nIndex - ((EditNodeEx*)e2)->p->m_nIndex);	// �E�B���h�E�ԍ���r
	}
	return (((EditNodeEx*)e1)->p - ((EditNodeEx*)e2)->p);	// �E�B���h�EMRU��r�i�\�[�g���Ȃ��j
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �O���[�v                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/** �w��ʒu�̕ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji
*/
EditNode* AppNodeGroupHandle::GetEditNodeAt(int nIndex)
{
	DllSharedData* pShare = &GetDllShareData();

	int iIndex = 0;
	for (int i = 0; i < pShare->m_nodes.m_nEditArrNum; ++i) {
		if (m_nGroup == 0 || m_nGroup == pShare->m_nodes.m_pEditArr[i].m_nGroup) {
			if (IsSakuraMainWindow(pShare->m_nodes.m_pEditArr[i].m_hWnd)) {
				if (iIndex == nIndex)
					return &pShare->m_nodes.m_pEditArr[i];
				++iIndex;
			}
		}
	}

	return NULL;
}


/** �ҏW�E�B���h�E���X�g�ւ̓o�^

	@param hWnd   [in] �o�^����ҏW�E�B���h�E�̃n���h��

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.06.20 ryoji �V�K�E�B���h�E�ɂ̓O���[�vID��t�^����
*/
BOOL AppNodeGroupHandle::AddEditWndList(HWND hWnd)
{
	DllSharedData* pShare = &GetDllShareData();

	TabWndNotifyType subCommand = TabWndNotifyType::Add;
	EditNode editNode = {0};
	editNode.m_hWnd = hWnd;

	{	// 2007.07.07 genta Lock�̈�
		LockGuard<Mutex> guard(g_editArrMutex);

		RecentEditNode	recentEditNode;

		// �o�^�ς݂��H
		int nIndex = recentEditNode.FindItemByHwnd(hWnd);
		if (nIndex != -1) {
			// ��������ȏ�o�^�ł��Ȃ����H
			if (recentEditNode.GetItemCount() >= recentEditNode.GetArrayCount()) {
				recentEditNode.Terminate();
				return FALSE;
			}
			subCommand = TabWndNotifyType::Reorder;

			// �ȑO�̏����R�s�[����B
			EditNode* p = recentEditNode.GetItem(nIndex);
			if (p) {
				editNode = *p;
			}
		}

		// �E�B���h�E�A��
		if (::GetWindowLongPtr(hWnd, sizeof(LONG_PTR)) == 0) {
			pShare->m_nodes.m_nSequences++;
			::SetWindowLongPtr(hWnd, sizeof(LONG_PTR) , (LONG_PTR)pShare->m_nodes.m_nSequences);

			// �A�Ԃ��X�V����B
			editNode.m_nIndex = pShare->m_nodes.m_nSequences;
			editNode.m_nId = -1;

			// �^�u�O���[�v�A��
			if (m_nGroup > 0) {
				editNode.m_nGroup = m_nGroup;	// �w��̃O���[�v
				if (pShare->m_nodes.m_nGroupSequences < m_nGroup) {
					// �w��O���[�v�����݂�Group Sequences�𒴂��Ă����ꍇ�̕␳
					pShare->m_nodes.m_nGroupSequences = m_nGroup;
				}
			}else {
				EditNode* p = recentEditNode.GetItem(0);
				if (!p) {
					editNode.m_nGroup = ++pShare->m_nodes.m_nGroupSequences;	// �V�K�O���[�v
				}else {
					editNode.m_nGroup = p->m_nGroup;	// �ŋ߃A�N�e�B�u�̃O���[�v
				}
			}

			editNode.m_showCmdRestore = ::IsZoomed(hWnd)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
			editNode.m_bClosing = FALSE;
		}

		// �ǉ��܂��͐擪�Ɉړ�����B
		recentEditNode.AppendItem(&editNode);
		recentEditNode.Terminate();
	}	// 2007.07.07 genta Lock�̈�I���

	// �E�B���h�E�o�^���b�Z�[�W���u���[�h�L���X�g����B
	AppNodeGroupHandle(hWnd).PostMessageToAllEditors(MYWM_TAB_WINDOW_NOTIFY, (WPARAM)subCommand, (LPARAM)hWnd, hWnd);

	return TRUE;
}


/** �ҏW�E�B���h�E���X�g����̍폜

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.07.05 ryoji mutex�ŕی�
*/
void AppNodeGroupHandle::DeleteEditWndList(HWND hWnd)
{
	// �E�B���h�E�����X�g����폜����B
	{	// 2007.07.07 genta Lock�̈�
		LockGuard<Mutex> guard(g_editArrMutex);

		RecentEditNode	recentEditNode;
		recentEditNode.DeleteItemByHwnd(hWnd);
		recentEditNode.Terminate();
	}

	// �E�B���h�E�폜���b�Z�[�W���u���[�h�L���X�g����B
	AppNodeGroupHandle(m_nGroup).PostMessageToAllEditors(MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TabWndNotifyType::Delete, (LPARAM)hWnd, hWnd);
}

/** �������̃E�B���h�E�֏I���v�����o��

	@param pWndArr [in] EditNode�̔z��Bm_hWnd��NULL�̗v�f�͏������Ȃ�
	@param nArrCnt [in] pWndArr�̒���
	@param bExit [in] TRUE: �ҏW�̑S�I�� / FALSE: ���ׂĕ���
	@param bCheckConfirm [in] FALSE:�����E�B���h�E�����Ƃ��̌x�����o���Ȃ� / TRUE:�x�����o���i�ݒ�ɂ��j
	@param hWndFrom [in] �I���v�����̃E�B���h�E�i�x�����b�Z�[�W�̐e�ƂȂ�j

	@date 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�
	@date 2007.06.22 ryoji nGroup������ǉ�
	@date 2008.11.22 syat �S�ā��������ɕύX�B�����E�B���h�E����鎞�̌x�����b�Z�[�W��ǉ�
	@date 2013.03.09 Uchi �I���v���ɗv�����̃E�B���h�E��n��
*/
BOOL AppNodeGroupHandle::RequestCloseEditor(EditNode* pWndArr, int nArrCnt, bool bExit, bool bCheckConfirm, HWND hWndFrom)
{
	// �N���[�Y�ΏۃE�B���h�E�𒲂ׂ�
	int iGroup = -1;
	HWND hWndLast = NULL;
	int nCloseCount = 0;
	for (int i=0; i<nArrCnt; ++i) {
		auto& wnd = pWndArr[i];
		if (m_nGroup == 0 || m_nGroup == wnd.m_nGroup) {
			if (IsSakuraMainWindow(wnd.m_hWnd)) {
				++nCloseCount;
				if (iGroup == -1) {
					iGroup = wnd.m_nGroup;	// �ŏ��ɕ���O���[�v
					hWndLast = wnd.m_hWnd;
				}else if (iGroup == wnd.m_nGroup) {
					hWndLast = wnd.m_hWnd;	// �ŏ��ɕ���O���[�v�̍Ō�̃E�B���h�E
				}
			}
		}
	}

	if (bCheckConfirm && GetDllShareData().m_common.m_general.m_bCloseAllConfirm) {	// [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����
		if (1 < nCloseCount) {
			if (::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				LS(STR_ERR_CSHAREDATA19)
				) != IDYES
			) {
				return FALSE;
			}
		}
	}

	// �A�N�e�B�u������E�B���h�E�����߂�
	// �E���b�Z�[�W��\�����Ă��Ȃ��Ԃ͂��̐���E�B���h�E���A�N�e�B�u�ɕۂ悤�ɂ���
	// �E������G�f�B�^���ۑ��m�F�̃��b�Z�[�W��\������ꍇ�́A���̐���E�B���h�E�ɃA�N�e�B�u���v���iMYWM_ALLOWACTIVATE�j���o���ăA�N�e�B�u�ɂ��Ă��炤
	// �E�^�u�O���[�v�\�����ǂ����Ȃǂ̏����ɉ����āA��������ŏ����ɂ���̂ɓs���̗ǂ��E�B���h�E�������őI�����Ă���
	HWND hWndActive;
	bool bTabGroup = (GetDllShareData().m_common.m_tabBar.m_bDispTabWnd && !GetDllShareData().m_common.m_tabBar.m_bDispTabWndMultiWin);
	if (bTabGroup) {
		hWndActive = hWndLast;	// �Ō�ɕ���E�B���h�E���S��
	}else {
		hWndActive = GetDllShareData().m_handles.m_hwndTray;	// �^�X�N�g���C���S��
	}

	// �A�N�e�B�u������E�B���h�E���A�N�e�B�u�ɂ��Ă���
	if (IsSakuraMainWindow(hWndActive)) {
		ActivateFrameWindow(hWndActive);	// �G�f�B�^�E�B���h�E
	}else {
		::SetForegroundWindow(hWndActive);	// �^�X�N�g���C
	}

	// �G�f�B�^�ւ̏I���v��
	for (int i=0; i<nArrCnt; ++i) {
		auto& wnd = pWndArr[i];
		if (m_nGroup == 0 || m_nGroup == wnd.m_nGroup) {
			if (IsSakuraMainWindow(wnd.m_hWnd)) {
				// �^�u�O���[�v�\���Ŏ��ɕ���̂��A�N�e�B�u������E�B���h�E�̏ꍇ�A
				// �A�N�e�B�u������E�B���h�E�����̃O���[�v�̍Ō�̃E�B���h�E�ɐؑւ���
				if (bTabGroup && wnd.m_hWnd == hWndActive) {
					iGroup = -1;
					hWndActive = IsSakuraMainWindow(hWndFrom) ? hWndFrom: NULL;	// ��ԍŌ�p
					for (int j=i+1; j<nArrCnt; ++j) {
						auto& wnd2 = pWndArr[j];
						if (m_nGroup == 0 || m_nGroup == wnd2.m_nGroup) {
							if (IsSakuraMainWindow(wnd2.m_hWnd)) {
								if (iGroup == -1) {
									iGroup = wnd2.m_nGroup;	// ���ɕ���O���[�v
									hWndActive = wnd2.m_hWnd;
								}else if (iGroup == wnd2.m_nGroup) {
									hWndActive = wnd2.m_hWnd;	// ���ɕ���O���[�v�̍Ō�̃E�B���h�E
								}else {
									break;
								}
							}
						}
					}
				}
				DWORD dwPid;
				::GetWindowThreadProcessId(wnd.m_hWnd, &dwPid);
				::SendMessage(hWndActive, MYWM_ALLOWACTIVATE, dwPid, 0);	// �A�N�e�B�u���̋����˗�����
				if (!::SendMessage(wnd.m_hWnd, MYWM_CLOSE, bExit ? PM_CLOSE_EXIT : 0, (LPARAM)hWndActive)) {	// 2007.02.13 ryoji bExit�������p��
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}


/** ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�

	@param bExcludeClosing [in] �I�����̕ҏW�E�B���h�E�̓J�E���g���Ȃ�

	@date 2007.06.22 ryoji nGroup������ǉ�
	@date 2008.04.19 ryoji bExcludeClosing������ǉ�
*/
int AppNodeGroupHandle::GetEditorWindowsNum(bool bExcludeClosing/* = true */)
{
	DllSharedData* pShare = &GetDllShareData();
	int cnt = 0;
	auto appNodeMgr = AppNodeManager::getInstance();
	for (int i=0; i<pShare->m_nodes.m_nEditArrNum; ++i) {
		auto& node = pShare->m_nodes.m_pEditArr[i];
		if (IsSakuraMainWindow(node.m_hWnd)) {
			if (1
				&& m_nGroup != 0
				&& m_nGroup != appNodeMgr->GetEditNode(node.m_hWnd)->GetGroup()
			) {
				continue;
			}
			if (1
				&& bExcludeClosing
				&& node.m_bClosing
			) {
				continue;
			}
			++cnt;
		}
	}
	return cnt;
}

template <typename Func>
bool relayMessageToAllEditors(
	UINT		uMsg,		// ���郁�b�Z�[�W
	WPARAM		wParam,		// ��1���b�Z�[�W �p�����[�^
	LPARAM		lParam,		// ��2���b�Z�[�W �p�����[�^
	HWND		hWndLast,	// �Ō�ɑ��肽���E�B���h�E
	int			nGroup,		// �w��O���[�v
	Func		func
	)
{
	EditNode* pWndArr;
	int n = AppNodeManager::getInstance()->GetOpenedWindowArr(&pWndArr, FALSE);
	if (n == 0) {
		return true;
	}

	// hWndLast�ȊO�ւ̃��b�Z�[�W
	for (int i=0; i<n; ++i) {
		// Jan. 24, 2005 genta hWndLast == NULL�̂Ƃ��Ƀ��b�Z�[�W��������悤��
		auto& node = pWndArr[i];
		if (!hWndLast || node.m_hWnd != hWndLast) {
			if (nGroup == 0 || nGroup == node.m_nGroup) {
				if (IsSakuraMainWindow(node.m_hWnd)) {
					// ���b�Z�[�W���|�X�g
					func(node.m_hWnd, uMsg, wParam, lParam);
				}
			}
		}
	}

	// hWndLast�ւ̃��b�Z�[�W
	for (int i=0; i<n; ++i) {
		auto& node = pWndArr[i];
		if (hWndLast == node.m_hWnd) {
			if (nGroup == 0 || nGroup == node.m_nGroup) {
				if (IsSakuraMainWindow(node.m_hWnd)) {
					// ���b�Z�[�W���|�X�g
					func(node.m_hWnd, uMsg, wParam, lParam);
				}
			}
		}
	}

	delete[] pWndArr;
	return true;
}

/** �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����

	@date 2005.01.24 genta hWndLast == NULL�̂Ƃ��S�����b�Z�[�W�������Ȃ�����
	@date 2007.06.22 ryoji nGroup������ǉ��A�O���[�v�P�ʂŏ��Ԃɑ���
*/
bool AppNodeGroupHandle::PostMessageToAllEditors(
	UINT		uMsg,		// �|�X�g���郁�b�Z�[�W
	WPARAM		wParam,		// ��1���b�Z�[�W �p�����[�^
	LPARAM		lParam,		// ��2���b�Z�[�W �p�����[�^
	HWND		hWndLast	// �Ō�ɑ��肽���E�B���h�E
	)
{
	return relayMessageToAllEditors(
		uMsg,
		wParam,
		lParam,
		hWndLast,
		m_nGroup,
		::PostMessage
	);
}

/** �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂�

	@date 2005.01.24 genta m_hWndLast == NULL�̂Ƃ��S�����b�Z�[�W�������Ȃ�����
	@date 2007.06.22 ryoji nGroup������ǉ��A�O���[�v�P�ʂŏ��Ԃɑ���
*/
bool AppNodeGroupHandle::SendMessageToAllEditors(
	UINT	uMsg,		// �|�X�g���郁�b�Z�[�W
	WPARAM	wParam,		// ��1���b�Z�[�W �p�����[�^
	LPARAM	lParam,		// ��2���b�Z�[�W �p�����[�^
	HWND	hWndLast	// �Ō�ɑ��肽���E�B���h�E
	)
{
	return relayMessageToAllEditors(
		uMsg,
		wParam,
		lParam,
		hWndLast,
		m_nGroup,
		::SendMessage
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �}�l�[�W��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/** �O���[�v��ID���Z�b�g����

	@date 2007.06.20 ryoji
*/
void AppNodeManager::ResetGroupId()
{
	DllSharedData* pShare = &GetDllShareData();
	auto& nodes = pShare->m_nodes;
	int nGroup = ++nodes.m_nGroupSequences;
	for (int i=0; i<nodes.m_nEditArrNum; ++i) {
		auto& node = nodes.m_pEditArr[i];
		if (IsSakuraMainWindow(node.m_hWnd)) {
			node.m_nGroup = nGroup;
		}
	}
}

/** �ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji

	@warning ���̊֐���m_pEditArr���̗v�f�ւ̃|�C���^��Ԃ��D
	m_pEditArr���ύX���ꂽ��ł̓A�N�Z�X���Ȃ��悤���ӂ��K�v�D

	@note NULL��Ԃ��ꍇ������̂Ŗ߂�l�̃`�F�b�N���K�v�ł�
*/
EditNode* AppNodeManager::GetEditNode(HWND hWnd)
{
	DllSharedData* pShare = &GetDllShareData();
	auto& nodes = pShare->m_nodes;
	for (int i=0; i<nodes.m_nEditArrNum; ++i) {
		auto& node = nodes.m_pEditArr[i];
		if (hWnd == node.m_hWnd) {
			if (IsSakuraMainWindow(node.m_hWnd)) {
				return &node;
			}
		}
	}
	return nullptr;
}


// ����ԍ��擾
int AppNodeManager::GetNoNameNumber(HWND hWnd)
{
	DllSharedData* pShare = &GetDllShareData();
	EditNode* editNode = GetEditNode(hWnd);
	if (editNode) {
		if (editNode->m_nId == -1) {
			pShare->m_nodes.m_nNonameSequences++;
			editNode->m_nId = pShare->m_nodes.m_nNonameSequences;
		}
		return editNode->m_nId;
	}
	return -1;
}


/** ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ�

	@param[out] ppEditNode �z����󂯎��|�C���^
		�߂�l��0�̏ꍇ��NULL���Ԃ���邪�C��������҂��Ȃ����ƁD
		�܂��C�s�v�ɂȂ�����delete []���Ȃ��Ă͂Ȃ�Ȃ��D
	@param[in] bSort TRUE: �\�[�g���� / FALSE: �\�[�g����
	@param[in]bGSort TRUE: �O���[�v�\�[�g���� / FALSE: �O���[�v�\�[�g����

	���Ƃ̕ҏW�E�B���h�E���X�g�̓\�[�g���Ȃ���΃E�B���h�E��MRU���ɕ���ł���
	-------------------------------------------------
	bSort	bGSort	��������
	-------------------------------------------------
	FALSE	FALSE	�O���[�vMRU���|�E�B���h�EMRU��
	TRUE	FALSE	�O���[�vMRU���|�E�B���h�E�ԍ���
	FALSE	TRUE	�O���[�v�ԍ����|�E�B���h�EMRU��
	TRUE	TRUE	�O���[�v�ԍ����|�E�B���h�E�ԍ���
	-------------------------------------------------

	@return �z��̗v�f����Ԃ�
	@note �v�f��>0 �̏ꍇ�͌Ăяo�����Ŕz���delete []���Ă�������

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.06.20 ryoji bGroup�����ǉ��A�\�[�g���������O�̂��̂���qsort�ɕύX
*/
int AppNodeManager::GetOpenedWindowArr(EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */)
{
	LockGuard<Mutex> guard(g_editArrMutex);
	int nRet = _GetOpenedWindowArrCore(ppEditNode, bSort, bGSort);
	return nRet;
}

// GetOpenedWindowArr�֐��R�A������
int AppNodeManager::_GetOpenedWindowArrCore(EditNode** ppEditNode, BOOL bSort, BOOL bGSort/* = FALSE */)
{
	DllSharedData* pShare = &GetDllShareData();
	auto& nodes = pShare->m_nodes;

	// �ҏW�E�B���h�E�����擾����B
	*ppEditNode = nullptr;
	if (nodes.m_nEditArrNum <= 0) {
		return 0;
	}

	// �ҏW�E�B���h�E���X�g�i�[�̈���쐬����B
	*ppEditNode = new EditNode[nodes.m_nEditArrNum];
	if (!(*ppEditNode)) {
		return 0;
	}

	// �g�����X�g���쐬����
	// �\�[�g�����p�̊g�����X�g
	std::vector<EditNodeEx> nodesEx(nodes.m_nEditArrNum);
	EditNodeEx*	pNode = &nodesEx[0];

	// �g�����X�g�̊e�v�f�ɕҏW�E�B���h�E���X�g�̊e�v�f�ւ̃|�C���^���i�[����
	int nRowNum = 0;	// �ҏW�E�B���h�E��
	for (int i=0; i<nodes.m_nEditArrNum; ++i) {
		auto& node = nodes.m_pEditArr[i];
		if (IsSakuraMainWindow(node.m_hWnd)) {
			pNode[nRowNum].p = &node;	// �|�C���^�i�[
			pNode[nRowNum].nGroupMru = -1;	// �O���[�v�P�ʂ�MRU�ԍ�������
			++nRowNum;
		}
	}
	if (nRowNum <= 0) {
		delete [](*ppEditNode);
		*ppEditNode = nullptr;
		return 0;
	}

	// �g�����X�g��ŃO���[�v�P�ʂ�MRU�ԍ�������
	if (!bGSort) {
		int iGroupMru = 0;	// �O���[�v�P�ʂ�MRU�ԍ�
		int nGroup = -1;
		for (int i=0; i<nRowNum; ++i) {
			if (pNode[i].nGroupMru == -1
				&& nGroup != pNode[i].p->m_nGroup
			) {
				nGroup = pNode[i].p->m_nGroup;
				++iGroupMru;
				pNode[i].nGroupMru = iGroupMru;	// MRU�ԍ��t�^
				// ����O���[�v�̃E�B���h�E�ɓ���MRU�ԍ�������
				for (int j=i+1; j<nRowNum; ++j) {
					if (pNode[j].p->m_nGroup == nGroup)
						pNode[j].nGroupMru = iGroupMru;
				}
			}
		}
	}

	// �g�����X�g���\�[�g����
	// Note. �O���[�v���P�����̏ꍇ�͏]���ibGSort ���������j�Ɠ������ʂ�������
	//       �i�O���[�v������ݒ�łȂ���΃O���[�v�͂P�j
	s_bSort = bSort;
	s_bGSort = bGSort;
	qsort(pNode, nRowNum, sizeof(EditNodeEx), cmpGetOpenedWindowArr);

	// �g�����X�g�̃\�[�g���ʂ����ƂɕҏW�E�B���h�E���X�g�i�[�̈�Ɍ��ʂ��i�[����
	for (int i=0; i<nRowNum; ++i) {
		(*ppEditNode)[i] = *pNode[i].p;

		// �C���f�b�N�X��t����B
		// ���̃C���f�b�N�X�� m_pEditArr �̔z��ԍ��ł��B
		(*ppEditNode)[i].m_nIndex = pNode[i].p - nodes.m_pEditArr;	// �|�C���^���Z���z��ԍ�
	}

	return nRowNum;
}

/** �E�B���h�E�̕��ёւ�

	@param[in] hwndSrc �ړ�����E�B���h�E
	@param[in] hwndDst �ړ���E�B���h�E

	@author ryoji
	@date 2007.07.07 genta �E�B���h�E�z�񑀍암��CTabWnd���ړ�
*/
bool AppNodeManager::ReorderTab(HWND hwndSrc, HWND hwndDst)
{
	DllSharedData* pShare = &GetDllShareData();
	EditNode* p = NULL;
	int nSrcTab = -1;
	int nDstTab = -1;
	LockGuard<Mutex> guard(g_editArrMutex);
	int nCount = _GetOpenedWindowArrCore(&p, TRUE);	// ���b�N�͎����ł���Ă���̂Œ��ڃR�A���Ăяo��
	for (int i=0; i<nCount; ++i) {
		if (hwndSrc == p[i].m_hWnd) {
			nSrcTab = i;
		}
		if (hwndDst == p[i].m_hWnd) {
			nDstTab = i;
		}
	}

	if (0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab) {
		if (p) {
			delete []p;
		}
		return false;
	}

	// �^�u�̏��������ւ��邽�߂ɃE�B���h�E�̃C���f�b�N�X�����ւ���
	int nArr0, nArr1;
	int	nIndex;

	nArr0 = p[nDstTab].m_nIndex;
	auto& nodes = pShare->m_nodes;
	nIndex = nodes.m_pEditArr[nArr0].m_nIndex;
	if (nSrcTab < nDstTab) {
		// �^�u���������[�e�[�g
		for (int i=nDstTab-1; i>=nSrcTab; --i) {
			nArr1 = p[i].m_nIndex;
			nodes.m_pEditArr[nArr0].m_nIndex = nodes.m_pEditArr[nArr1].m_nIndex;
			nArr0 = nArr1;
		}
	}else {
		// �^�u�E�������[�e�[�g
		for (int i=nDstTab+1; i<=nSrcTab; ++i) {
			nArr1 = p[i].m_nIndex;
			nodes.m_pEditArr[nArr0].m_nIndex = nodes.m_pEditArr[nArr1].m_nIndex;
			nArr0 = nArr1;
		}
	}
	nodes.m_pEditArr[nArr0].m_nIndex = nIndex;

	if (p) {
		delete[] p;
	}
	return true;
}

/** �^�u�ړ��ɔ����E�B���h�E����

	@param[in] hwndSrc �ړ�����E�B���h�E
	@param[in] hwndDst �ړ���E�B���h�E�D�V�K�Ɨ�����NULL�D
	@param[in] bSrcIsTop �ړ�����E�B���h�E�����E�B���h�E�Ȃ�true
	@param[in] notifygroups �^�u�̍X�V���K�v�ȃO���[�v�̃O���[�vID�Dint[2]���Ăяo�����ŗp�ӂ���D

	@return �X�V���ꂽhwndDst (�ړ��悪���ɕ���ꂽ�ꍇ�Ȃǂ�NULL�ɕύX����邱�Ƃ�����)

	@author ryoji
	@date 2007.07.07 genta CTabWnd::SeparateGroup()���Ɨ�
*/
HWND AppNodeManager::SeparateGroup(HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[])
{
	DllSharedData* pShare = &GetDllShareData();

	LockGuard<Mutex> guard(g_editArrMutex);

	EditNode* pSrcEditNode = GetEditNode(hwndSrc);
	EditNode* pDstEditNode = GetEditNode(hwndDst);
	int nSrcGroup = pSrcEditNode->m_nGroup;
	int nDstGroup;
	if (!pDstEditNode) {
		hwndDst = NULL;
		nDstGroup = ++pShare->m_nodes.m_nGroupSequences;	// �V�K�O���[�v
	}else {
		nDstGroup = pDstEditNode->m_nGroup;	// �����O���[�v
	}

	pSrcEditNode->m_nGroup = nDstGroup;
	pSrcEditNode->m_nIndex = ++pShare->m_nodes.m_nSequences;	// �^�u���т̍Ō�i�N�����̍Ō�j�ɂ����Ă���

	// ��\���̃^�u�������O���[�v�Ɉړ�����Ƃ��͔�\���̂܂܂ɂ���̂�
	// ���������擪�ɂ͂Ȃ�Ȃ��悤�A�K�v�Ȃ�擪�E�B���h�E�ƈʒu����������B
	if (!bSrcIsTop && pDstEditNode) {
		if (pSrcEditNode < pDstEditNode) {
			EditNode en = *pDstEditNode;
			*pDstEditNode = *pSrcEditNode;
			*pSrcEditNode = en;
		}
	}
	
	notifygroups[0] = nSrcGroup;
	notifygroups[1] = nDstGroup;
	
	return hwndDst;
}


/** ����O���[�v���ǂ����𒲂ׂ�

	@param[in] hWnd1 ��r����E�B���h�E1
	@param[in] hWnd2 ��r����E�B���h�E2
	
	@return 2�̃E�B���h�E������O���[�v�ɑ����Ă����true

	@date 2007.06.20 ryoji
*/
bool AppNodeManager::IsSameGroup(HWND hWnd1, HWND hWnd2)
{
	if (hWnd1 == hWnd2) {
		return true;
	}
	auto* pNodeMgr = AppNodeManager::getInstance();
	AppNodeGroupHandle group1 = pNodeMgr->GetEditNode(hWnd1)->GetGroup();
	AppNodeGroupHandle group2 = pNodeMgr->GetEditNode(hWnd2)->GetGroup();
	if (group1.IsValidGroup() && group1 == group2) {
		return true;
	}

	return false;
}

// �󂢂Ă���O���[�v�ԍ����擾����
int AppNodeManager::GetFreeGroupId(void)
{
	DllSharedData* pShare = &GetDllShareData();
	return ++pShare->m_nodes.m_nGroupSequences;	// �V�K�O���[�v
}

// Close �������̎���Window���擾����
//  (�^�u�܂Ƃߕ\���̏ꍇ)
//
// @param hWndCur [in] Close�Ώۂ̃E�B���h�E�n���h��
// @retval �N���[�Y��ړ�����E�B���h�E
//			NULL�̓^�u�܂Ƃߕ\���Ŗ������O���[�v�ɑ��ɃE�B���h�E�������ꍇ
//
// @date 2013.04.10 Uchi
// @date 2013.10.25 Moca ���̃E�B���h�E�́u1�O�̃A�N�e�B�u�ȃ^�u�v�ɂ���
//
HWND AppNodeManager::GetNextTab(HWND hWndCur)
{
	HWND hWnd = NULL;
	auto& tabBar = GetDllShareData().m_common.m_tabBar;
	if (1
		&& tabBar.m_bDispTabWnd
		&& !tabBar.m_bDispTabWndMultiWin
	) {
		int			nGroup = 0;
		bool		bFound = false;
		EditNode*	p = nullptr;
		int			nCount = AppNodeManager::getInstance()->GetOpenedWindowArr(&p, FALSE, FALSE);
		if (nCount > 1) {
			// search Group No.
			for (int i=0; i<nCount; ++i) {
				if (p[i].GetHwnd() == hWndCur) {
					nGroup = p[i].m_nGroup;
					break;
				}
			}
			// Search Next Window
			for (int i=0; i<nCount; ++i) {
				if (p[i].m_nGroup == nGroup) {
					if (p[i].GetHwnd() == hWndCur) {
						bFound= true;
					}else {
						if (!bFound && hWnd == NULL || bFound) {
							hWnd = p[i].GetHwnd();
						}
						if (bFound) {
							break;
						}
					}
				}
			}
		}
		if (p) {
			delete []p;
		}
	}

	return hWnd;
}

