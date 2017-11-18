#pragma once

#include "recent/Recent.h"
#include "dlg/Dialog.h"

class EditWnd;
class ImageListMgr;

class MainToolBar {
public:
	MainToolBar(EditWnd& owner);

	void Create(ImageListMgr* pIcons);

	// �쐬�E�j��
	void CreateToolBar(void);		// �c�[���o�[�쐬
	void DestroyToolBar(void);		// �c�[���o�[�j��

	// ���b�Z�[�W
	bool EatMessage(MSG* msg);		// ���b�Z�[�W�����B�Ȃ񂩏��������Ȃ� true ��Ԃ��B
	void ProcSearchBox(MSG*);		// �����R���{�{�b�N�X�̃��b�Z�[�W����

	// �C�x���g
	void OnToolbarTimer(void);		// �^�C�}�[�̏��� 20060128 aroka
	void UpdateToolbar(void);		// �c�[���o�[�̕\�����X�V����		// 2008.09.23 nasukoji

	// �`��
	LPARAM ToolBarOwnerDraw(LPNMCUSTOMDRAW pnmh);

	// ���L�f�[�^�Ƃ̓���
	void AcceptSharedSearchKey();

	// �擾
	HWND GetToolbarHwnd() const	{ return hwndToolBar; }
	HWND GetRebarHwnd() const	{ return hwndReBar; }
	HWND GetSearchHwnd() const	{ return hwndSearchBox; }
	size_t GetSearchKey(std::wstring&); // �����L�[���擾�B�߂�l�͌����L�[�̕������B

	// ����
	void SetFocusSearchBox(void) const;		// �c�[���o�[�����{�b�N�X�փt�H�[�J�X���ړ�		2006.06.04 yukihane

private:
	EditWnd&	owner;
    HWND		hwndToolBar;

	// �q�E�B���h�E
    HWND		hwndReBar;		// Rebar �E�B���h�E	//@@@ 2006.06.17 ryoji
	HWND		hwndSearchBox;	// �����R���{�{�b�N�X

	// �t�H���g
	HFONT		hFontSearchBox;	// �����R���{�{�b�N�X�̃t�H���g

	ComboBoxItemDeleter	comboDel;
	RecentSearch		recentSearch;
	ImageListMgr*		pIcons;
};

