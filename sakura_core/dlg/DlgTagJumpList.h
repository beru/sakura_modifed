/*!	@file
	@brief �^�O�W�����v���X�g�_�C�A���O�{�b�N�X
*/

#pragma once

#include "dlg/Dialog.h"
#include "recent/RecentTagjumpKeyword.h"

// �^�O�t�@�C����
#define TAG_FILENAME_T _T("tags")

class SortedTagJumpList;

/*!	@brief �_�C���N�g�^�O�W�����v���ꗗ�_�C�A���O

	�_�C���N�g�^�O�W�����v�ŕ����̌�₪����ꍇ�y��
	�L�[���[�h�w��^�O�W�����v�̂��߂̃_�C�A���O�{�b�N�X����
*/
class DlgTagJumpList : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgTagJumpList(bool bDirectTagJump);
	~DlgTagJumpList();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// ���[�_���_�C�A���O�̕\�� 

//	bool AddParamA(const char*, const char*, int, const char*, const char*, int depth, int baseDirId);	// �o�^
	bool GetSelectedParam(TCHAR* s0, TCHAR* s1, int* n2, TCHAR* s3, TCHAR* s4, int* depth, TCHAR* fileBase);	// �擾
	void SetFileName(const TCHAR* pszFileName);
	void SetKeyword(const wchar_t* pszKeyword);
	int  FindDirectTagJump();

	bool GetSelectedFullPathAndLine(TCHAR* fullPath, int count, int* lineNum, int* depth);

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	BOOL	OnBnClicked(int);
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
	BOOL	OnSize(WPARAM wParam, LPARAM lParam);
	BOOL	OnMove(WPARAM wParam, LPARAM lParam);
	BOOL	OnMinMaxInfo(LPARAM lParam);
	BOOL	OnNotify(WPARAM wParam, LPARAM lParam);
	// @@ 2005.03.31 MIK �L�[���[�h���̓G���A�̃C�x���g����
	BOOL	OnCbnSelChange(HWND hwndCtl, int wID);
	BOOL	OnCbnEditChange(HWND hwndCtl, int wID);
	//BOOL	OnEnChange(HWND hwndCtl, int wID);
	BOOL	OnTimer(WPARAM wParam);
	LPVOID	GetHelpIdTable(void);

private:
	void	StopTimer(void);
	void	StartTimer(int);

	void	SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int		GetData(void);	// �_�C�A���O�f�[�^�̎擾
	void	UpdateData(bool);

	TCHAR*	GetNameByType(const TCHAR type, const TCHAR* name);	// �^�C�v�𖼑O�ɕϊ�����B
	int		SearchBestTag(void);	// �����Ƃ��m���̍������ȃC���f�b�N�X��Ԃ��B
	const TCHAR* GetFileName(void);
	const TCHAR* GetFilePath(void) { return pszFileName ? pszFileName : _T(""); }
	void Empty(void);
	void SetTextDir();
	void FindNext(bool);
	void find_key(const wchar_t* keyword);
	int find_key_core(int, const wchar_t*, bool, bool, bool, bool, int);
	
	bool IsDirectTagJump();
	
	void ClearPrevFindInfo();
	bool GetFullPathAndLine(int index, TCHAR *fullPath, int count, int *lineNum, int *depth);


	// depth���犮�S�p�X��(���΃p�X/��΃p�X)���쐬����
	static TCHAR* GetFullPathFromDepth(TCHAR*, int, TCHAR*, const TCHAR*, int);
	static TCHAR* CopyDirDir(TCHAR* dest, const TCHAR* target, const TCHAR* base);
public:
	static int CalcMaxUpDirectory(const TCHAR*);
	static TCHAR* DirUp(TCHAR* dir);

private:

	struct TagFindState {
		int   nDepth;
		int   nMatchAll;
		int   nNextMode;
		int   nLoop;
		bool  bJumpPath;
		TCHAR szCurPath[1024];
	};
	
	bool	bDirectTagJump;

	int			nIndex;				// �I�����ꂽ�v�f�ԍ�
	TCHAR*		pszFileName;		// �ҏW���̃t�@�C����
	wchar_t*	pszKeyword;			// �L�[���[�h(DoModal��lParam != 0���w�肵���ꍇ�Ɏw��ł���)
	int			nLoop;				// �����̂ڂ��K�w��
	SortedTagJumpList*	pList;		// �^�O�W�����v���
	UINT	nTimerId;				// �^�C�}�ԍ�
	bool	bTagJumpICase;			// �啶���������𓯈ꎋ
	bool	bTagJumpAnyWhere;		// ������̓r���Ƀ}�b�`
	bool	bTagJumpExactMatch;		// ���S��v(��ʖ���)

	int 	nTop;					// �y�[�W�߂���̕\���̐擪(0�J�n)
	bool	bNextItem;				// �܂����Ƀq�b�g������̂�����

	// �i�荞�݌����p
	TagFindState* psFindPrev;		// �O��̍Ō�Ɍ����������
	TagFindState* psFind0Match;		// �O���1��Hit���Ȃ������Ō��tags

	NativeW	strOldKeyword;			// �O��̃L�[���[�h
	bool	bOldTagJumpICase;		// �O��̑啶���������𓯈ꎋ
	bool	bOldTagJumpAnyWhere;	// �O��̕�����̓r���Ƀ}�b�`

	ComboBoxItemDeleter		comboDel;
	RecentTagJumpKeyword	recentKeyword;
	
	POINT	ptDefaultSize;
	RECT	rcItems[11];

private:
	DISALLOW_COPY_AND_ASSIGN(DlgTagJumpList);
};


