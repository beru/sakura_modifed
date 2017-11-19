#pragma once

#include "util/design_template.h"
#include "config/maxdata.h"

class AppNodeGroupHandle;

// �ҏW�E�B���h�E�m�[�h
struct EditNode {
	ptrdiff_t		nIndex;
	int				nGroup;						// �O���[�vID
	HWND			hWnd;
	int				nId;						// ����Id
	WIN_CHAR		szTabCaption[_MAX_PATH];	// �^�u�E�B���h�E�p�F�L���v�V������
	SFilePath		szFilePath;					// �^�u�E�B���h�E�p�F�t�@�C����
	bool			bIsGrep;					// Grep�̃E�B���h�E��
	UINT			showCmdRestore;				// ���̃T�C�Y�ɖ߂��Ƃ��̃T�C�Y���
	bool			bClosing;					// �I�������i�u�Ō�̃t�@�C������Ă�(����)���c���v�p�j

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
	size_t		nEditArrNum;
	EditNode	pEditArr[MAX_EDITWINDOWS];
	LONG		nSequences;			// �E�B���h�E�A��
	LONG		nNonameSequences;	// ����A��
	LONG		nGroupSequences;	// �^�u�O���[�v�A��
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
	EditNode* GetEditNodeAt(size_t nIndex);					// �w��ʒu�̕ҏW�E�B���h�E�����擾����
	bool AddEditWndList(HWND);								// �ҏW�E�B���h�E�̓o�^
	void DeleteEditWndList(HWND);							// �ҏW�E�B���h�E���X�g����̍폜
	bool RequestCloseEditor(EditNode* pWndArr, size_t nArrCnt, bool bExit, bool bCheckConfirm, HWND hWndFrom);
															// �������̃E�B���h�E�֏I���v�����o��

	int GetEditorWindowsNum(bool bExcludeClosing = true);				// ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�

	// �S�E�B���h�E�ꊇ����
	bool PostMessageToAllEditors(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast);	// �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����
	bool SendMessageToAllEditors(UINT uMsg, WPARAM wParam, LPARAM lParam, HWND hWndLast);	// �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂�

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
	bool ReorderTab(HWND hSrcTab, HWND hDstTab);				// �^�u�ړ��ɔ����E�B���h�E�̕��ёւ�
	HWND SeparateGroup(HWND hwndSrc, HWND hwndDst, bool bSrcIsTop, int notifygroups[]);	// �^�u�����ɔ����E�B���h�E����

	// �������
	size_t GetOpenedWindowArr(EditNode** , bool, bool bGSort = false);				// ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ�

protected:
	size_t _GetOpenedWindowArrCore(EditNode** , bool, bool bGSort = false);			// ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ��i�R�A�������j

public:
	static bool IsSameGroup(HWND hWnd1, HWND hWnd2);					// ����O���[�v���ǂ����𒲂ׂ�
	int GetFreeGroupId(void);											// �󂢂Ă���O���[�v�ԍ����擾����
	HWND GetNextTab(HWND hWndCur);										// Close �������̎���Window���擾����(�^�u�܂Ƃߕ\���̏ꍇ)
};


inline AppNodeGroupHandle EditNode::GetGroup() const { if (this) return nGroup; else return 0; }

inline bool EditNode::IsTopInGroup() const { return this && (AppNodeGroupHandle(nGroup).GetEditNodeAt(0) == this); }

inline AppNodeHandle::AppNodeHandle(HWND hwnd)
{
	pNodeRef = AppNodeManager::getInstance().GetEditNode(hwnd);
}

