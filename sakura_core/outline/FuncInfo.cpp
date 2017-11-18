/*!	@file
	@brief �A�E�g���C�����  �f�[�^�v�f
*/

#include "StdAfx.h"
#include "FuncInfo.h"

// FuncInfo�N���X�\�z
FuncInfo::FuncInfo(
	size_t	nFuncLineCRLF,		// �֐��̂���s(CRLF�P��)
	size_t	nFuncColCRLF,		// �֐��̂��錅(CRLF�P��)
	size_t	nFuncLineLAYOUT,	// �֐��̂���s(�܂�Ԃ��P��)
	size_t	nFuncColLAYOUT,		// �֐��̂��錅(�܂�Ԃ��P��)
	const TCHAR*	pszFuncName,		// �֐���
	const TCHAR*	pszFileName,
	int				nInfo				// �t�����
	)
	:
	nDepth(0) // �[��
{
	this->nFuncLineCRLF = nFuncLineCRLF;		// �֐��̂���s(CRLF�P��)
	this->nFuncColCRLF = nFuncColCRLF;			// �֐��̂��錅(CRLF�P��)
	this->nFuncLineLAYOUT = nFuncLineLAYOUT;	// �֐��̂���s(�܂�Ԃ��P��)
	this->nFuncColLAYOUT = nFuncColLAYOUT;		// �֐��̂��錅(�܂�Ԃ��P��)
	memFuncName.SetString(pszFuncName);
	if (pszFileName) {
		memFileName.SetString( pszFileName );
	}
	this->nInfo = nInfo;
}


// FuncInfo�N���X����
FuncInfo::~FuncInfo()
{

}

