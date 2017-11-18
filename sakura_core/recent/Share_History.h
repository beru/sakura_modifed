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

