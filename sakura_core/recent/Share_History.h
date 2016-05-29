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

#include "config/maxdata.h"

// ���L���������\����
struct Share_History {
	//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRU���o�R���Ă��������B
	size_t				nMRUArrNum;
	EditInfo			fiMRUArr[MAX_MRU];
	bool				bMRUArrFavorite[MAX_MRU];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRUFolder���o�R���Ă��������B
	size_t							nOPENFOLDERArrNum;
	StaticString<TCHAR,_MAX_PATH>	szOPENFOLDERArr[MAX_OPENFOLDER];
	bool							bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	// ���C�ɓ���	//@@@ 2003.04.08 MIK

	// MRU���O���X�g�ꗗ
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_MRU,  const TCHAR* >	aExceptMRU;

	// MRU�ȊO�̏��
	SFilePath													szIMPORTFOLDER;	// �C���|�[�g�f�B���N�g���̗���
	StaticVector< StaticString<TCHAR, MAX_CMDLEN>, MAX_CMDARR >	aCommands;		// �O���R�}���h���s����
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_CMDARR >	aCurDirs;		// �J�����g�f�B���N�g������
};

