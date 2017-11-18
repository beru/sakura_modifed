#include "StdAfx.h"
#include "types/Type.h"
#include "view/colors/EColorIndexType.h"

// �ݒ�t�@�C��
// Windows�W����ini, inf, cnf�t�@�C����sakura�L�[���[�h�ݒ�t�@�C��.kwd, �F�ݒ�t�@�C��.col ���ǂ߂�悤�ɂ���
void CType_Ini::InitTypeConfigImp(TypeConfig& type)
{
	// ���O�Ɗg���q
	_tcscpy(type.szTypeName, _T("�ݒ�t�@�C��"));
	_tcscpy(type.szTypeExts, _T("ini,inf,cnf,kwd,col"));
	
	// �ݒ�
	type.lineComment.CopyTo(0, L"//", -1);				// �s�R�����g�f���~�^
	type.lineComment.CopyTo(1, L";", -1);					// �s�R�����g�f���~�^2
	type.eDefaultOutline = OutlineType::Text;				// �A�E�g���C����͕��@
	type.colorInfoArr[COLORIDX_SSTRING].bDisp = false;	// �V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	type.colorInfoArr[COLORIDX_WSTRING].bDisp = false;	// �_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
}

