/*!	@file
	@brief DIFF�����\���_�C�A���O�{�b�N�X

	@author MIK
	@date 2002.5.27
*/
/*
	Copyright (C) 2002, MIK
	Copyright (C) 2004, genta

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

class DlgDiff;

#pragma once

#include "dlg/Dialog.h"
/*!
	@brief DIFF�����\���_�C�A���O�{�b�N�X
*/
// Feb. 28, 2004 genta �Ō�ɑI������Ă����ԍ���ۑ�����
class DlgDiff : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgDiff();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal( HINSTANCE, HWND, LPARAM, const TCHAR* );	// ���[�_���_�C�A���O�̕\��

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnBnClicked(int);
	BOOL	OnLbnSelChange(HWND hwndCtl, int wID);
	BOOL	OnLbnDblclk(int wID);
	BOOL	OnEnChange(HWND hwndCtl, int wID);
	LPVOID	GetHelpIdTable(void);
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// �W���ȊO�̃��b�Z�[�W��ߑ�����
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnMove(WPARAM wParam, LPARAM lParam);
	BOOL OnMinMaxInfo(LPARAM lParam);

	void	SetData(void);	// �_�C�A���O�f�[�^�̐ݒ�
	int		GetData(void);	// �_�C�A���O�f�[�^�̎擾

private:
	int			nIndexSave;		// �Ō�ɑI������Ă����ԍ�
	POINT		ptDefaultSize;
	RECT		rcItems[22];

public:
	SFilePath	szFile1;			// ���t�@�C��
	SFilePath	szFile2;			// ����t�@�C��
	bool		bIsModifiedDst;		// ����t�@�C���X�V��
	EncodingType	nCodeTypeDst;	// ����t�@�C���̕����R�[�h
	bool		bBomDst;			// ����t�@�C����BOM
	int			nDiffFlgOpt;		// DIFF�I�v�V����
	HWND		hWnd_Dst;			// ����E�B���h�E�n���h��

};


