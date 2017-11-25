/*!	@file
	@brief ���ʒ�`
*/

#pragma once

//////////////////////////////////////////////////////////////
#ifndef STRICT
#define STRICT
#endif

#include <Windows.h>
#include <tchar.h>

// �E�B���h�E��ID
#define IDW_STATUSBAR 123

#define IDM_SELWINDOW		10000
#define IDM_SELMRU			11000
#define IDM_SELOPENFOLDER	12000

#include "charset/charset.h"

// �_�C�A���O�\�����@	 �A�E�g���C���E�B���h�E�p�ɍ쐬
enum class ShowDialogType {
	Normal,
	Reload,
	Toggle,
};

// �I��̈�`��p�p�����[�^
extern const COLORREF	SELECTEDAREA_RGB;
extern const int		SELECTEDAREA_ROP2;


// �^�u�E�B���h�E�p���b�Z�[�W�T�u�R�}���h
enum class TabWndNotifyType {
	Refresh,		// �ĕ\��
	Add,			// �E�B���h�E�o�^
	Delete,			// �E�B���h�E�폜
	Reorder,		// �E�B���h�E�����ύX
	Rename,			// �t�@�C�����ύX
	Enable,			// �^�u���[�h�L����
	Disable,		// �^�u���[�h������
	Adjust,			// �E�B���h�E�ʒu���킹
};

// �o�[�̕\���E��\��
enum class BarChangeNotifyType {
	Toolbar,		// �c�[���o�[
	FuncKey,		// �t�@���N�V�����L�[
	Tab,			// �^�u
	StatusBar,		// �X�e�[�^�X�o�[
	MiniMap,		//�~�j�}�b�v
};

// �^�u�Ŏg���J�X�^�����j���[�̃C���f�b�N�X
#define	CUSTMENU_INDEX_FOR_TABWND		24
// �E�N���b�N���j���[�Ŏg���J�X�^�����j���[�̃C���f�b�N�X
#define	CUSTMENU_INDEX_FOR_RBUTTONUP	0


// �F�^�C�v
#define COLOR_ATTRIB_FORCE_DISP		0x00000001
#define COLOR_ATTRIB_NO_TEXT		0x00000010
#define COLOR_ATTRIB_NO_BACK		0x00000020
#define COLOR_ATTRIB_NO_BOLD		0x00000100
#define COLOR_ATTRIB_NO_UNDERLINE	0x00000200
//#define COLOR_ATTRIB_NO_ITALIC		0x00000400	�\��l
#define COLOR_ATTRIB_NO_EFFECTS		0x00000F00

struct ColorAttributeData {
	const TCHAR*	szName;
	unsigned int	fAttribute;
};
extern const ColorAttributeData g_ColorAttributeArr[];

// �ݒ�l�̏���E����
// ���[���̍���
const int IDC_SPIN_nRulerHeight_MIN = 2;
const int IDC_SPIN_nRulerHeight_MAX = 32;

/**	�}�E�X�N���b�N�ƃL�[��`�̑Ή� */
enum class MouseFunctionType {
	DoubleClick,	// �_�u���N���b�N
	RightClick,		// �E�N���b�N
	CenterClick,	// ���N���b�N
	LeftSideClick,	// ���T�C�h�N���b�N
	RightSideClick,	// �E�T�C�h�N���b�N
	TripleClick,	// �g���v���N���b�N
	QuadrapleClick,	// �N�A�h���v���N���b�N
	WheelUp,		// �z�C�[���A�b�v
	WheelDown,		// �z�C�[���_�E��
	WheelLeft,		// �z�C�[����
	WheelRight,		// �z�C�[���E
	KeyBegin,		// �}�E�X�ւ̊��蓖�Č����{���̃L�[���蓖�Đ擪INDEX
};

// �e�L�X�g�̐܂�Ԃ����@
enum class TextWrappingMethod {
	NoWrapping,		// �܂�Ԃ��Ȃ��iScrollBar���e�L�X�g���ɍ��킹��j
	SettingWidth,	// �w�茅�Ő܂�Ԃ�
	WindowWidth,	// �E�[�Ő܂�Ԃ�
};

// �����J�E���g���@
enum class SelectCountMode {
	Toggle,		// �����J�E���g���@���g�O��
	ByChar,		// �������ŃJ�E���g
	ByByte,		// �o�C�g���ŃJ�E���g
};

// ��������
enum class SearchDirection {
	Backward,	// �O������ (�O������)
	Forward,	// ������� (��������) (����)
};

struct SearchOption {
//	SearchDirection	eDirection;
//	bool	bPrevOrNext;	// false==�O������ true==�������
	bool	bRegularExp;	// true==���K�\��
	bool	bLoHiCase;		// true==�p�啶���������̋��
	bool	bWordOnly;		// true==�P��̂݌���

	SearchOption()
		:
		bRegularExp(false),
		bLoHiCase(false),
		bWordOnly(false)
	{
	}
	SearchOption(
		bool bRegularExp,
		bool bLoHiCase,
		bool bWordOnly
	)
		:
		bRegularExp(bRegularExp),
		bLoHiCase(bLoHiCase),
		bWordOnly(bWordOnly)
	{
	}
	void Reset() {
		bRegularExp = false;
		bLoHiCase   = false;
		bWordOnly   = false;
	}

	// ���Z�q
	bool operator == (const SearchOption& rhs) const {
		// �Ƃ肠����memcmp�ł�����
		return memcmp(this, &rhs, sizeof(*this)) == 0;
	}
	bool operator != (const SearchOption& rhs) const {
		return !operator == (rhs);
	}

};

// EditWnd�̃C���X�^���X�ւ̃|�C���^�������ɕۑ����Ă���
class EditWnd;
extern EditWnd* g_pcEditWnd;

HINSTANCE G_AppInstance();

