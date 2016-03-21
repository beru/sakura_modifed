/*!	@file
	@brief ���ʒ�`

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, Stonee, genta, jepro, MIK
	Copyright (C) 2001, jepro, hor, MIK
	Copyright (C) 2002, MIK, genta, aroka, YAZAKI, Moca, KK, novice
	Copyright (C) 2003, MIK, genta, zenryaku, naoh
	Copyright (C) 2004, Kazika
	Copyright (C) 2005, MIK, Moca, genta
	Copyright (C) 2006, aroka, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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

// �_�C�A���O�\�����@	 �A�E�g���C���E�B���h�E�p�ɍ쐬 20060201 aroka
enum class ShowDialogType {
	Normal,
	Reload,
	Toggle,
};

// �I��̈�`��p�p�����[�^
extern const COLORREF	SELECTEDAREA_RGB;
extern const int		SELECTEDAREA_ROP2;


//@@@ From Here 2003.05.31 MIK
// �^�u�E�B���h�E�p���b�Z�[�W�T�u�R�}���h
enum class TabWndNotifyType {
	Refresh,		// �ĕ\��
	Add,			// �E�B���h�E�o�^
	Delete,			// �E�B���h�E�폜
	Reorder,		// �E�B���h�E�����ύX
	Rename,			// �t�@�C�����ύX
	Enable,			// �^�u���[�h�L����	// 2004.07.14 Kazika �ǉ�
	Disable,		// �^�u���[�h������	// 2004.08.27 Kazika �ǉ�
	Adjust,			// �E�B���h�E�ʒu���킹	// 2007.04.03 ryoji �ǉ�
};

// �o�[�̕\���E��\��
enum class BarChangeNotifyType {
	Toolbar,		// �c�[���o�[
	FuncKey,		// �t�@���N�V�����L�[
	Tab,			// �^�u
	StatusBar,		// �X�e�[�^�X�o�[
	MiniMap,		//�~�j�}�b�v
};
//@@@ To Here 2003.05.31 MIK

// �^�u�Ŏg���J�X�^�����j���[�̃C���f�b�N�X	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_TABWND		24
// �E�N���b�N���j���[�Ŏg���J�X�^�����j���[�̃C���f�b�N�X	//@@@ 2003.06.13 MIK
#define	CUSTMENU_INDEX_FOR_RBUTTONUP	0


// �F�^�C�v
//@@@ From Here 2006.12.18 ryoji
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

//@@@ To Here 2006.12.18 ryoji

// �ݒ�l�̏���E����
// ���[���̍���
const int IDC_SPIN_nRulerHeight_MIN = 2;
const int IDC_SPIN_nRulerHeight_MAX = 32;

/**	�}�E�X�N���b�N�ƃL�[��`�̑Ή�

	@date 2007.11.04 genta �V�K�쐬�D���l����Ɣ͈̓T�C�Y��`�̂���
*/
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

// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@
enum class TextWrappingMethod {
	NoWrapping,		// �܂�Ԃ��Ȃ��iScrollBar���e�L�X�g���ɍ��킹��j
	SettingWidth,	// �w�茅�Ő܂�Ԃ�
	WindowWidth,	// �E�[�Ő܂�Ԃ�
};

// 2009.07.06 syat	�����J�E���g���@
enum class SelectCountMode {
	Toggle,		// �����J�E���g���@���g�O��
	ByChar,		// �������ŃJ�E���g
	ByByte,		// �o�C�g���ŃJ�E���g
};

// 2007.09.06 kobake �ǉ�
// ��������
enum class SearchDirection {
	Backward,	// �O������ (�O������)
	Forward,	// ������� (��������) (����)
};

// 2007.09.06 kobake �ǉ�
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

// 2007.10.02 kobake EditWnd�̃C���X�^���X�ւ̃|�C���^�������ɕۑ����Ă���
class EditWnd;
extern EditWnd* g_pcEditWnd;

HINSTANCE G_AppInstance();

