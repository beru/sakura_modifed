/*
	Copyright (C) 2008, kobake

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

#include "util/design_template.h"
#include "config/maxdata.h"

class AppNodeGroupHandle;

// �ҏW�E�B���h�E�m�[�h
struct EditNode {
	int				nIndex;
	int				nGroup;						// �O���[�vID							//@@@ 2007.06.20 ryoji
	HWND			hWnd;
	int				nId;						// ����Id
	WIN_CHAR		szTabCaption[_MAX_PATH];	// �^�u�E�B���h�E�p�F�L���v�V������		//@@@ 2003.05.31 MIK
	SFilePath		szFilePath;					// �^�u�E�B���h�E�p�F�t�@�C����			//@@@ 2006.01.28 ryoji
	bool			bIsGrep;					// Grep�̃E�B���h�E��					//@@@ 2006.01.28 ryoji
	UINT			showCmdRestore;				// ���̃T�C�Y�ɖ߂��Ƃ��̃T�C�Y���		//@@@ 2007.06.20 ryoji
	BOOL			bClosing;					// �I�������i�u�Ō�̃t�@�C������Ă�(����)���c���v�p�j	//@@@ 2007.06.20 ryoji

	HWND GetHwnd() const { return GetSafeHwnd(); }
	HWND GetSafeHwnd() const { if (this) return hWnd; else return NULL; }
	int GetId() const { return GetSafeId(); }
	int GetSafeId() const { if (this) return nId; else return 0; }
	AppNodeGroupHandle GetGroup() const;
	bool IsTopInGroup() const;
};

// �g���\����
struct EditNodeEx {
	EditNode*	p;			// �ҏW�E�B���h�E�z��v�f�ւ̃|�C���^
	int			nGroupMru;	// �O���[�v�P�ʂ�MRU�ԍ�
};


// ���L���������\����
struct Share_Nodes {
	int			nEditArrNum;		// short->int�ɏC��	//@@@ 2003.05.31 MIK
	EditNode	pEditArr[MAX_EDITWINDOWS];	// �ő�l�C��	@@@ 2003.05.31 MIK
	LONG		nSequences;			// �E�B���h�E�A��
	LONG		nNonameSequences;	// ����A��
	LONG		nGroupSequences;	// �^�u�O���[�v�A��	// 2007.06.20 ryoji
};


// �m�[�h�A�N�Z�T
class AppNodeHandle {
public:
	AppNodeHandle(HWND hwnd);
	EditNode* operator->() { return pNodeRef; }
private:
	EditNode* pNodeRef;
};

// �O���[�v�A�N�Z�T
class AppNodeGroupHandle {
public:
	AppNodeGroupHandle(int nGroupId) : nGroup(nGroupId) { }
	AppNodeGroupHandle(HWND hwnd) { nGroup = AppNodeHandle(hwnd)->GetGroup(); }

	EditNode* GetTopEditNode() { return GetEditNodeAt(0); }	//
	EditNode* GetEditNodeAt(int nIndex);					// �w��ʒu�̕ҏW�E�B���h�E�����擾����
	bool AddEditWndList(HWND);								// �ҏW�E�B���h�E�̓o�^	// 2007.06.26 ryoji nGroup�����ǉ�
	void DeleteEditWndList(HWND);							// �ҏW�E�B���h�E���X�g����̍폜
	bool RequestCloseEditor(EditNode* pWndArr, int nArrCnt, bool bExit, bool bCheckConfirm, HWND hWndFrom);
															// �������̃E�B���h�E�֏I���v�����o��	// 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�	// 2007.06.20 ryoji nGroup�����ǉ�

	int GetEditorWindowsNum(bool bExcludeClosing = true);				// ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�		// 2007.06.20 ryoji nGroup�����ǉ�	// 2008.04.19 ryoji bExcludeClosing�����ǉ�

	// �S�E�B���h�E�ꊇ����
	bool PostMessageToAllEditors(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast);	// �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����	// 2007.06.20 ryoji nGroup�����ǉ�
	bool SendMessageToAllEditors(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast);	// �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂�		// 2007.06.20 ryoji nGroup�����ǉ�

public:
	bool operator == (const AppNodeGroupHandle& rhs) const { return nGroup == rhs.nGroup; }
	bool IsValidGroup() const { return nGroup >= 0; }
	operator int() const { return nGroup; }

private:
	int nGroup;
};


class AppNodeManager : public TSingleton<AppNodeManager> {
	friend class TSingleton<AppNodeManager>;
	AppNodeManager() {}

public:
	// �O���[�v
	void ResetGroupId();									// �O���[�v��ID���Z�b�g����

	// �E�B���h�E�n���h�� �� �m�[�h�@�ϊ�
	EditNode* GetEditNode(HWND hWnd);							// �ҏW�E�B���h�E�����擾����
	int GetNoNameNumber(HWND);

	// �^�u
	bool ReorderTab(HWND hSrcTab, HWND hDstTab);				// �^�u�ړ��ɔ����E�B���h�E�̕��ёւ� 2007.07.07 genta
	HWND SeparateGroup(HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[]);	// �^�u�����ɔ����E�B���h�E���� 2007.07.07 genta

	// �������
	int GetOpenedWindowArr(EditNode** , bool, bool bGSort = false);				// ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ�

protected:
	int _GetOpenedWindowArrCore(EditNode** , bool, bool bGSort = false);			// ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ��i�R�A�������j

public:
	static bool IsSameGroup(HWND hWnd1, HWND hWnd2);					// ����O���[�v���ǂ����𒲂ׂ�
	int GetFreeGroupId(void);											// �󂢂Ă���O���[�v�ԍ����擾����
	HWND GetNextTab(HWND hWndCur);										// Close �������̎���Window���擾����(�^�u�܂Ƃߕ\���̏ꍇ)	2013/4/10 Uchi
};


inline AppNodeGroupHandle EditNode::GetGroup() const { if (this) return nGroup; else return 0; }

inline bool EditNode::IsTopInGroup() const { return this && (AppNodeGroupHandle(nGroup).GetEditNodeAt(0) == this); }

inline AppNodeHandle::AppNodeHandle(HWND hwnd)
{
	pNodeRef = AppNodeManager::getInstance().GetEditNode(hwnd);
}

