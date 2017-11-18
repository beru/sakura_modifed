#include "StdAfx.h"
#include "types/Type.h"
#include "doc/DocOutline.h"
#include "view/colors/EColorIndexType.h"

void CType_Basis::InitTypeConfigImp(TypeConfig& type)
{
	// ���O�Ɗg���q
	_tcscpy(type.szTypeName, _T("��{"));
	_tcscpy(type.szTypeExts, _T(""));

	// �ݒ�
	type.nMaxLineKetas = MAXLINEKETAS;			// �܂�Ԃ�����
	type.eDefaultOutline = OutlineType::Text;				// �A�E�g���C����͕��@
	type.colorInfoArr[COLORIDX_SSTRING].bDisp = false;	// �V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�	// Oct. 17, 2000 JEPRO
	type.colorInfoArr[COLORIDX_WSTRING].bDisp = false;	// �_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�	// Sept. 4, 2000 JEPRO
}

