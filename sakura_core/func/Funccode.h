/*!	@file
	@brief �@�\�ԍ���`
*/

#pragma once

/*
�����̈�����
  CEditView::HandleCommand
  CMacro::HandleFunction
  MacroFuncInfo CSMacroMgr::macroFuncInfoArr[]
���Q��
*/

#include "Funccode_enum.h"

#ifndef UINT16
#define UINT16 WORD
#endif
#ifndef uint16_t
typedef UINT16 uint16_t;
#endif

// �@�\�ꗗ�Ɋւ���f�[�^�錾
namespace nsFuncCode {
	extern const uint16_t		ppszFuncKind[];
	extern const size_t			nFuncKindNum;
	extern const int			pnFuncListNumArr[];
	extern const EFunctionCode*	ppnFuncListArr[];
	extern const size_t			nFincListNumArrNum;

	extern const EFunctionCode	pnFuncList_Special[];
	extern const size_t			nFuncList_Special_Num;
};
///////////////////////////////////////////////////////////////////////


// �@�\�ԍ��ɑΉ������w���v�g�s�b�NID��Ԃ�
int FuncID_To_HelpContextID(EFunctionCode nFuncID);

class EditDoc;
struct DllSharedData;

bool IsFuncEnable(const EditDoc&, const DllSharedData&, EFunctionCode);	// �@�\�����p�\�����ׂ�
bool IsFuncChecked(const EditDoc&, const DllSharedData&, EFunctionCode);	// �@�\���`�F�b�N��Ԃ����ׂ�

