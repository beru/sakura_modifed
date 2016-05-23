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

#include "basis/SakuraBasis.h"
#include "config/maxdata.h"
#include "types/Type.h"


/*! �t�@�C�����

	@date 2002.03.07 genta szDocType�ǉ�
	@date 2003.01.26 aroka nWindowSizeX/Y nWindowOriginX/Y�ǉ�
*/
struct EditInfo {
	// �t�@�C��
	TCHAR			szPath[_MAX_PATH];					// �t�@�C����
	EncodingType	nCharCode;							// �����R�[�h���
	bool			bBom;								// BOM(GetFileInfo)
	TCHAR			szDocType[MAX_DOCTYPE_LEN + 1];		// �����^�C�v
	int 			nTypeId;							// �����^�C�v(MRU)

	// �\����
	int nViewTopLine;							// �\����̈�ԏ�̍s(0�J�n)
	int nViewLeftCol;							// �\����̈�ԍ��̌�(0�J�n)

	// �L�����b�g
	LogicPoint ptCursor;								// �L�����b�g�ʒu

	// �e����
	bool		bIsModified;							// �ύX�t���O

	// GREP���[�h
	bool		bIsGrep;								// Grep�̃E�B���h�E��
	wchar_t		szGrepKey[1024];

	// �f�o�b�O���j�^ (�A�E�g�v�b�g�E�B���h�E) ���[�h
	bool		bIsDebug;								// �f�o�b�O���j�^���[�h (�A�E�g�v�b�g�E�B���h�E) ��

	// �u�b�N�}�[�N���
	wchar_t		szMarkLines[MAX_MARKLINES_LEN + 1];		// �u�b�N�}�[�N�̕����s���X�g

	// �E�B���h�E
	int			nWindowSizeX;							// �E�B���h�E  ��(�s�N�Z����)
	int			nWindowSizeY;							// �E�B���h�E  ����(�s�N�Z����)
	int			nWindowOriginX;							// �E�B���h�E  �����ʒu(�s�N�Z�����E�}�C�i�X�l���L��)
	int			nWindowOriginY;							// �E�B���h�E  �����ʒu(�s�N�Z�����E�}�C�i�X�l���L��)
	
	// Mar. 7, 2002 genta
	// Constructor �m���ɏ��������邽��
	EditInfo()
		:
		nCharCode(CODE_AUTODETECT),
		bBom(false),
		nTypeId(-1),
		nViewTopLine(-1),
		nViewLeftCol(-1),
		ptCursor(-1, -1),
		bIsModified(false),
		bIsGrep(false),
		bIsDebug(false),
		nWindowSizeX(-1),
		nWindowSizeY(-1),
		nWindowOriginX(CW_USEDEFAULT),	//	2004.05.13 Moca �g�w�薳���h��-1����CW_USEDEFAULT�ɕύX
		nWindowOriginY(CW_USEDEFAULT)
	{
		szPath[0] = '\0';
		szMarkLines[0] = L'\0';
		szDocType[0] = '\0';
	}
};

