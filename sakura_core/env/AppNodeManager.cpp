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
static bool s_bSort;	// �\�[�g�w��
static bool s_bGSort;	// �O���[�v�w��

/*! @brief CShareData::pEditArr�ی�pMutex

	�����̃G�f�B�^���񓯊��Ɉ�ē��삵�Ă���Ƃ��ł��ACShareData::pEditArr��
	���S�ɑ���ł���悤���쒆��Mutex��Lock()����B

	@par�i�񓯊���ē���̗�j
		�����̃E�B���h�E��\�����Ă��ăO���[�v����L���ɂ����^�X�N�o�[�Łu�O���[�v�����v����������Ƃ�

	@par�i�ی삷��ӏ��̗�j
		CShareData::AddEditWndList(): �G���g���̒ǉ��^���ёւ�
		CShareData::DeleteEditWndList(): �G���g���̍폜
		CShareData::GetOpenedWindowArr(): �z��̃R�s�[�쐬

	����ɂǂ��ɂł������ƃf�b�h���b�N����댯������̂œ����Ƃ��͐T�d�ɁB
	�iLock()���Ԓ���SendMessage()�Ȃǂő��E�B���h�E�̑��������Ɗ댯����j
	CShareData::pEditArr�𒼐ڎQ�Ƃ�����ύX����悤�ȉӏ��ɂ͐��ݓI�Ȋ댯�����邪�A
	�Θb�^�ŏ������삵�Ă���͈͂ł���΂܂����͋N���Ȃ��B

	@date 2007.07.05 ryoji �V�K����
	@date 2007.07.07 genta CShareData�̃����o�ֈړ�
*/
static Mutex g_editArrMutex(FALSE, GSTR_MUTEX_SAKURA_EDITARR);

// GetOpenedWindowArr�p�\�[�g�֐�
static bool __cdecl cmpGetOpenedWindowArr(const EditNodeEx& e1, const EditNodeEx& e2)
{
	// �قȂ�O���[�v�̂Ƃ��̓O���[�v��r����
	int nGroup1;
	int nGroup2;

	if (s_bGSort) {
		// �I���W�i���̃O���[�v�ԍ��̂ق�������
		nGroup1 = e1.p->nGroup;
		nGroup2 = e2.p->nGroup;
	}else {
		// �O���[�v��MRU�ԍ��̂ق�������
		nGroup1 = e1.nGroupMru;
		nGroup2 = e2.nGroupMru;
	}
	if (nGroup1 != nGroup2) {
		return nGroup1 < nGroup2;	// �O���[�v��r
	}

	// �O���[�v��r���s���Ȃ������Ƃ��̓E�B���h�E��r����
	if (s_bSort) {
		return e1.p->nIndex < e2.p->nIndex;	// �E�B���h�E�ԍ���r
	}
	return e1.p < e2.p;	// �E�B���h�EMRU��r�i�\�[�g���Ȃ��j
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
	for (int i=0; i<pShare->nodes.nEditArrNum; ++i) {
		if (nGroup == 0 || nGroup == pShare->nodes.pEditArr[i].nGroup) {
			if (IsSakuraMainWindow(pShare->nodes.pEditArr[i].hWnd)) {
				if (iIndex == nIndex) {
					return &pShare->nodes.pEditArr[i];
				}
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
bool AppNodeGroupHandle::AddEditWndList(HWND hWnd)
{
	DllSharedData* pShare = &GetDllShareData();

	TabWndNotifyType subCommand = TabWndNotifyType::Add;
	EditNode editNode = {0};
	editNode.hWnd = hWnd;

	{	// 2007.07.07 genta Lock�̈�
		LockGuard<Mutex> guard(g_editArrMutex);

		RecentEditNode	recentEditNode;

		// �o�^�ς݂��H
		int nIndex = recentEditNode.FindItemByHwnd(hWnd);
		if (nIndex != -1) {
			// ��������ȏ�o�^�ł��Ȃ����H
			if (recentEditNode.GetItemCount() >= recentEditNode.GetArrayCount()) {
				recentEditNode.Terminate();
				return false;
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
			pShare->nodes.nSequences++;
			::SetWindowLongPtr(hWnd, sizeof(LONG_PTR) , (LONG_PTR)pShare->nodes.nSequences);

			// �A�Ԃ��X�V����B
			editNode.nIndex = pShare->nodes.nSequences;
			editNode.nId = -1;

			// �^�u�O���[�v�A��
			if (nGroup > 0) {
				editNode.nGroup = nGroup;	// �w��̃O���[�v
				if (pShare->nodes.nGroupSequences < nGroup) {
					// �w��O���[�v�����݂�Group Sequences�𒴂��Ă����ꍇ�̕␳
					pShare->nodes.nGroupSequences = nGroup;
				}
			}else {
				EditNode* p = recentEditNode.GetItem(0);
				if (!p) {
					editNode.nGroup = ++pShare->nodes.nGroupSequences;	// �V�K�O���[�v
				}else {
					editNode.nGroup = p->nGroup;	// �ŋ߃A�N�e�B�u�̃O���[�v
				}
			}

			editNode.showCmdRestore = ::IsZoomed(hWnd)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
			editNode.bClosing = false;
		}

		// �ǉ��܂��͐擪�Ɉړ�����B
		recentEditNode.AppendItem(&editNode);
		recentEditNode.Terminate();
	}	// 2007.07.07 genta Lock�̈�I���

	// �E�B���h�E�o�^���b�Z�[�W���u���[�h�L���X�g����B
	AppNodeGroupHandle(hWnd).PostMessageToAllEditors(MYWM_TAB_WINDOW_NOTIFY, (WPARAM)subCommand, (LPARAM)hWnd, hWnd);

	return true;
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
	AppNodeGroupHandle(nGroup).PostMessageToAllEditors(MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TabWndNotifyType::Delete, (LPARAM)hWnd, hWnd);
}

/** �������̃E�B���h�E�֏I���v�����o��

	@param pWndArr [in] EditNode�̔z��BhWnd��NULL�̗v�f�͏������Ȃ�
	@param nArrCnt [in] pWndArr�̒���
	@param bExit [in] true: �ҏW�̑S�I�� / false: ���ׂĕ���
	@param bCheckConfirm [in] false:�����E�B���h�E�����Ƃ��̌x�����o���Ȃ� / true:�x�����o���i�ݒ�ɂ��j
	@param hWndFrom [in] �I���v�����̃E�B���h�E�i�x�����b�Z�[�W�̐e�ƂȂ�j

	@date 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�
	@date 2007.06.22 ryoji nGroup������ǉ�
	@date 2008.11.22 syat �S�ā��������ɕύX�B�����E�B���h�E����鎞�̌x�����b�Z�[�W��ǉ�
	@date 2013.03.09 Uchi �I���v���ɗv�����̃E�B���h�E��n��
*/
bool AppNodeGroupHandle::RequestCloseEditor(EditNode* pWndArr, int nArrCnt, bool bExit, bool bCheckConfirm, HWND hWndFrom)
{
	// �N���[�Y�ΏۃE�B���h�E�𒲂ׂ�
	int iGroup = -1;
	HWND hWndLast = NULL;
	int nCloseCount = 0;
	for (int i=0; i<nArrCnt; ++i) {
		auto& wnd = pWndArr[i];
		if (nGroup == 0 || nGroup == wnd.nGroup) {
			if (IsSakuraMainWindow(wnd.hWnd)) {
				++nCloseCount;
				if (iGroup == -1) {
					iGroup = wnd.nGroup;	// �ŏ��ɕ���O���[�v
					hWndLast = wnd.hWnd;
				}else if (iGroup == wnd.nGroup) {
					hWndLast = wnd.hWnd;	// �ŏ��ɕ���O���[�v�̍Ō�̃E�B���h�E
				}
			}
		}
	}

	if (bCheckConfirm && GetDllShareData().common.general.bCloseAllConfirm) {	// [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����
		if (1 < nCloseCount) {
			if (::MYMESSAGEBOX(
				hWndFrom,
				MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
				GSTR_APPNAME,
				LS(STR_ERR_CSHAREDATA19)
				) != IDYES
			) {
				return false;
			}
		}
	}

	// �A�N�e�B�u������E�B���h�E�����߂�
	// �E���b�Z�[�W��\�����Ă��Ȃ��Ԃ͂��̐���E�B���h�E���A�N�e�B�u�ɕۂ悤�ɂ���
	// �E������G�f�B�^���ۑ��m�F�̃��b�Z�[�W��\������ꍇ�́A���̐���E�B���h�E�ɃA�N�e�B�u���v���iMYWM_ALLOWACTIVATE�j���o���ăA�N�e�B�u�ɂ��Ă��炤
	// �E�^�u�O���[�v�\�����ǂ����Ȃǂ̏����ɉ����āA��������ŏ����ɂ���̂ɓs���̗ǂ��E�B���h�E�������őI�����Ă���
	HWND hWndActive;
	bool bTabGroup = (GetDllShareData().common.tabBar.bDispTabWnd && !GetDllShareData().common.tabBar.bDispTabWndMultiWin);
	if (bTabGroup) {
		hWndActive = hWndLast;	// �Ō�ɕ���E�B���h�E���S��
	}else {
		hWndActive = GetDllShareData().handles.hwndTray;	// �^�X�N�g���C���S��
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
		if (nGroup == 0 || nGroup == wnd.nGroup) {
			if (IsSakuraMainWindow(wnd.hWnd)) {
				// �^�u�O���[�v�\���Ŏ��ɕ���̂��A�N�e�B�u������E�B���h�E�̏ꍇ�A
				// �A�N�e�B�u������E�B���h�E�����̃O���[�v�̍Ō�̃E�B���h�E�ɐؑւ���
				if (bTabGroup && wnd.hWnd == hWndActive) {
					iGroup = -1;
					hWndActive = IsSakuraMainWindow(hWndFrom) ? hWndFrom: NULL;	// ��ԍŌ�p
					for (int j=i+1; j<nArrCnt; ++j) {
						auto& wnd2 = pWndArr[j];
						if (nGroup == 0 || nGroup == wnd2.nGroup) {
							if (IsSakuraMainWindow(wnd2.hWnd)) {
								if (iGroup == -1) {
									iGroup = wnd2.nGroup;	// ���ɕ���O���[�v
									hWndActive = wnd2.hWnd;
								}else if (iGroup == wnd2.nGroup) {
									hWndActive = wnd2.hWnd;	// ���ɕ���O���[�v�̍Ō�̃E�B���h�E
								}else {
									break;
								}
							}
						}
					}
				}
				DWORD dwPid;
				::GetWindowThreadProcessId(wnd.hWnd, &dwPid);
				::SendMessage(hWndActive, MYWM_ALLOWACTIVATE, dwPid, 0);	// �A�N�e�B�u���̋����˗�����
				if (!::SendMessage(wnd.hWnd, MYWM_CLOSE, bExit ? PM_CLOSE_EXIT : 0, (LPARAM)hWndActive)) {	// 2007.02.13 ryoji bExit�������p��
					return false;
				}
			}
		}
	}

	return true;
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
	auto& appNodeMgr = AppNodeManager::getInstance();
	for (int i=0; i<pShare->nodes.nEditArrNum; ++i) {
		auto& node = pShare->nodes.pEditArr[i];
		if (IsSakuraMainWindow(node.hWnd)) {
			if (1
				&& nGroup != 0
				&& nGroup != appNodeMgr.GetEditNode(node.hWnd)->GetGroup()
			) {
				continue;
			}
			if (1
				&& bExcludeClosing
				&& node.bClosing
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
	size_t n = AppNodeManager::getInstance().GetOpenedWindowArr(&pWndArr, false);
	if (n == 0) {
		return true;
	}

	// hWndLast�ȊO�ւ̃��b�Z�[�W
	for (size_t i=0; i<n; ++i) {
		// Jan. 24, 2005 genta hWndLast == NULL�̂Ƃ��Ƀ��b�Z�[�W��������悤��
		auto& node = pWndArr[i];
		if (!hWndLast || node.hWnd != hWndLast) {
			if (nGroup == 0 || nGroup == node.nGroup) {
				if (IsSakuraMainWindow(node.hWnd)) {
					// ���b�Z�[�W���|�X�g
					func(node.hWnd, uMsg, wParam, lParam);
				}
			}
		}
	}

	// hWndLast�ւ̃��b�Z�[�W
	for (size_t i=0; i<n; ++i) {
		auto& node = pWndArr[i];
		if (hWndLast == node.hWnd) {
			if (nGroup == 0 || nGroup == node.nGroup) {
				if (IsSakuraMainWindow(node.hWnd)) {
					// ���b�Z�[�W���|�X�g
					func(node.hWnd, uMsg, wParam, lParam);
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
		nGroup,
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
		nGroup,
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
	auto& nodes = pShare->nodes;
	int nGroup = ++nodes.nGroupSequences;
	for (int i=0; i<nodes.nEditArrNum; ++i) {
		auto& node = nodes.pEditArr[i];
		if (IsSakuraMainWindow(node.hWnd)) {
			node.nGroup = nGroup;
		}
	}
}

/** �ҏW�E�B���h�E�����擾����

	@date 2007.06.20 ryoji

	@warning ���̊֐���pEditArr���̗v�f�ւ̃|�C���^��Ԃ��D
	pEditArr���ύX���ꂽ��ł̓A�N�Z�X���Ȃ��悤���ӂ��K�v�D

	@note NULL��Ԃ��ꍇ������̂Ŗ߂�l�̃`�F�b�N���K�v�ł�
*/
EditNode* AppNodeManager::GetEditNode(HWND hWnd)
{
	DllSharedData* pShare = &GetDllShareData();
	auto& nodes = pShare->nodes;
	for (int i=0; i<nodes.nEditArrNum; ++i) {
		auto& node = nodes.pEditArr[i];
		if (hWnd == node.hWnd) {
			if (IsSakuraMainWindow(node.hWnd)) {
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
		if (editNode->nId == -1) {
			pShare->nodes.nNonameSequences++;
			editNode->nId = pShare->nodes.nNonameSequences;
		}
		return editNode->nId;
	}
	return -1;
}


/** ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ�

	@param[out] ppEditNode �z����󂯎��|�C���^
		�߂�l��0�̏ꍇ��NULL���Ԃ���邪�C��������҂��Ȃ����ƁD
		�܂��C�s�v�ɂȂ�����delete[]���Ȃ��Ă͂Ȃ�Ȃ��D
	@param[in] bSort true: �\�[�g���� / false: �\�[�g����
	@param[in] bGSort true: �O���[�v�\�[�g���� / false: �O���[�v�\�[�g����

	���Ƃ̕ҏW�E�B���h�E���X�g�̓\�[�g���Ȃ���΃E�B���h�E��MRU���ɕ���ł���
	-------------------------------------------------
	bSort	bGSort	��������
	-------------------------------------------------
	false	false	�O���[�vMRU���|�E�B���h�EMRU��
	true	false	�O���[�vMRU���|�E�B���h�E�ԍ���
	false	true	�O���[�v�ԍ����|�E�B���h�EMRU��
	true	true	�O���[�v�ԍ����|�E�B���h�E�ԍ���
	-------------------------------------------------

	@return �z��̗v�f����Ԃ�
	@note �v�f��>0 �̏ꍇ�͌Ăяo�����Ŕz���delete[]���Ă�������

	@date 2003.06.28 MIK CRecent���p�ŏ�������
	@date 2007.06.20 ryoji bGroup�����ǉ��A�\�[�g���������O�̂��̂���qsort�ɕύX
*/
size_t AppNodeManager::GetOpenedWindowArr(EditNode** ppEditNode, bool bSort, bool bGSort/* = false */)
{
	LockGuard<Mutex> guard(g_editArrMutex);
	size_t nRet = _GetOpenedWindowArrCore(ppEditNode, bSort, bGSort);
	return nRet;
}

// GetOpenedWindowArr�֐��R�A������
size_t AppNodeManager::_GetOpenedWindowArrCore(EditNode** ppEditNode, bool bSort, bool bGSort/* = false */)
{
	DllSharedData* pShare = &GetDllShareData();
	auto& nodes = pShare->nodes;

	// �ҏW�E�B���h�E�����擾����B
	*ppEditNode = nullptr;
	if (nodes.nEditArrNum <= 0) {
		return 0;
	}

	// �ҏW�E�B���h�E���X�g�i�[�̈���쐬����B
	*ppEditNode = new EditNode[nodes.nEditArrNum];
	if (!(*ppEditNode)) {
		return 0;
	}

	// �g�����X�g���쐬����
	// �\�[�g�����p�̊g�����X�g
	std::vector<EditNodeEx> nodesEx(nodes.nEditArrNum);
	EditNodeEx*	pNode = &nodesEx[0];
	// �g�����X�g�̊e�v�f�ɕҏW�E�B���h�E���X�g�̊e�v�f�ւ̃|�C���^���i�[����
	size_t nRowNum = 0;	// �ҏW�E�B���h�E��
	for (int i=0; i<nodes.nEditArrNum; ++i) {
		auto& node = nodes.pEditArr[i];
		if (IsSakuraMainWindow(node.hWnd)) {
			pNode[nRowNum].p = &node;	// �|�C���^�i�[
			pNode[nRowNum].nGroupMru = -1;	// �O���[�v�P�ʂ�MRU�ԍ�������
			++nRowNum;
		}
	}
	if (nRowNum == 0) {
		delete[] (*ppEditNode);
		*ppEditNode = nullptr;
		return 0;
	}

	// �g�����X�g��ŃO���[�v�P�ʂ�MRU�ԍ�������
	if (!bGSort) {
		int iGroupMru = 0;	// �O���[�v�P�ʂ�MRU�ԍ�
		int nGroup = -1;
		for (size_t i=0; i<nRowNum; ++i) {
			if (pNode[i].nGroupMru == -1
				&& nGroup != pNode[i].p->nGroup
			) {
				nGroup = pNode[i].p->nGroup;
				++iGroupMru;
				pNode[i].nGroupMru = iGroupMru;	// MRU�ԍ��t�^
				// ����O���[�v�̃E�B���h�E�ɓ���MRU�ԍ�������
				for (size_t j=i+1; j<nRowNum; ++j) {
					if (pNode[j].p->nGroup == nGroup)
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
	std::sort(nodesEx.begin(), nodesEx.begin()+nRowNum, cmpGetOpenedWindowArr);

	// �g�����X�g�̃\�[�g���ʂ����ƂɕҏW�E�B���h�E���X�g�i�[�̈�Ɍ��ʂ��i�[����
	for (size_t i=0; i<nRowNum; ++i) {
		(*ppEditNode)[i] = *pNode[i].p;

		// �C���f�b�N�X��t����B
		// ���̃C���f�b�N�X�� pEditArr �̔z��ԍ��ł��B
		(*ppEditNode)[i].nIndex = pNode[i].p - nodes.pEditArr;	// �|�C���^���Z���z��ԍ�
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
	size_t nCount = _GetOpenedWindowArrCore(&p, true);	// ���b�N�͎����ł���Ă���̂Œ��ڃR�A���Ăяo��
	for (size_t i=0; i<nCount; ++i) {
		if (hwndSrc == p[i].hWnd) {
			nSrcTab = i;
		}
		if (hwndDst == p[i].hWnd) {
			nDstTab = i;
		}
	}

	if (0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab) {
		if (p) {
			delete[] p;
		}
		return false;
	}

	// �^�u�̏��������ւ��邽�߂ɃE�B���h�E�̃C���f�b�N�X�����ւ���
	ptrdiff_t nArr0, nArr1;
	ptrdiff_t nIndex;

	nArr0 = p[nDstTab].nIndex;
	auto& nodes = pShare->nodes;
	nIndex = nodes.pEditArr[nArr0].nIndex;
	if (nSrcTab < nDstTab) {
		// �^�u���������[�e�[�g
		for (int i=nDstTab-1; i>=nSrcTab; --i) {
			nArr1 = p[i].nIndex;
			nodes.pEditArr[nArr0].nIndex = nodes.pEditArr[nArr1].nIndex;
			nArr0 = nArr1;
		}
	}else {
		// �^�u�E�������[�e�[�g
		for (int i=nDstTab+1; i<=nSrcTab; ++i) {
			nArr1 = p[i].nIndex;
			nodes.pEditArr[nArr0].nIndex = nodes.pEditArr[nArr1].nIndex;
			nArr0 = nArr1;
		}
	}
	nodes.pEditArr[nArr0].nIndex = nIndex;

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
	int nSrcGroup = pSrcEditNode->nGroup;
	int nDstGroup;
	if (!pDstEditNode) {
		hwndDst = NULL;
		nDstGroup = ++pShare->nodes.nGroupSequences;	// �V�K�O���[�v
	}else {
		nDstGroup = pDstEditNode->nGroup;	// �����O���[�v
	}

	pSrcEditNode->nGroup = nDstGroup;
	pSrcEditNode->nIndex = ++pShare->nodes.nSequences;	// �^�u���т̍Ō�i�N�����̍Ō�j�ɂ����Ă���

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
	auto& nodeMgr = AppNodeManager::getInstance();
	AppNodeGroupHandle group1 = nodeMgr.GetEditNode(hWnd1)->GetGroup();
	AppNodeGroupHandle group2 = nodeMgr.GetEditNode(hWnd2)->GetGroup();
	if (group1.IsValidGroup() && group1 == group2) {
		return true;
	}

	return false;
}

// �󂢂Ă���O���[�v�ԍ����擾����
int AppNodeManager::GetFreeGroupId(void)
{
	DllSharedData* pShare = &GetDllShareData();
	return ++pShare->nodes.nGroupSequences;	// �V�K�O���[�v
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
	auto& tabBar = GetDllShareData().common.tabBar;
	if (1
		&& tabBar.bDispTabWnd
		&& !tabBar.bDispTabWndMultiWin
	) {
		int nGroup = 0;
		bool bFound = false;
		EditNode* p = nullptr;
		size_t nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&p, false, false);
		if (nCount > 1) {
			// search Group No.
			for (size_t i=0; i<nCount; ++i) {
				if (p[i].GetHwnd() == hWndCur) {
					nGroup = p[i].nGroup;
					break;
				}
			}
			// Search Next Window
			for (size_t i=0; i<nCount; ++i) {
				if (p[i].nGroup == nGroup) {
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
			delete[] p;
		}
	}

	return hWnd;
}

