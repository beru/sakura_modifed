/*!	@file
	@brief WSH�C���^�t�F�[�X�I�u�W�F�N�g��{�N���X

	@date 2009.10.29 syat CWSH.cpp����؂�o��
*/
/*
	Copyright (C) 2002, �S, genta
	Copyright (C) 2003, FILE
	Copyright (C) 2004, genta
	Copyright (C) 2005, FILE, zenryaku
	Copyright (C) 2009, syat

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

#include "StdAfx.h"
#include "macro/WSHIfObj.h"
#include "macro/SMacroMgr.h" // MacroFuncInfo
#include "Funccode_enum.h" // EFunctionCode::FA_FROMMACRO
#include "util/other_util.h" // auto_array_ptr


// �R�}���h�E�֐�����������
void WSHIfObj::ReadyMethods(
	EditView* pView,
	int flags
	)
{
	this->m_pView = pView;
	// 2007.07.20 genta : �R�}���h�ɍ������ރt���O��n��
	ReadyCommands(GetMacroCommandInfo(), flags | FA_FROMMACRO);
	ReadyCommands(GetMacroFuncInfo(), 0);
	/* WSHIfObj���p�������T�u�N���X����ReadyMethods���Ăяo�����ꍇ�A
	 * �T�u�N���X��GetMacroCommandInfo,GetMacroFuncInfo���Ăяo�����B */
}

/** WSH�}�N���G���W���փR�}���h�o�^���s��

	@date 2007.07.20 genta flags�ǉ��Dflag�̓R�}���h�o�^�i�K�ō����Ă����D
*/
void WSHIfObj::ReadyCommands(
	MacroFuncInfo* Info,
	int flags
	)
{
	while (Info->nFuncID != -1) {	// Aug. 29, 2002 genta �Ԑl�̒l���ύX���ꂽ�̂ł������ύX
		wchar_t FuncName[256];
		wcscpy(FuncName, Info->pszFuncName);

		int ArgCount = 0;
		if (Info->pData) {
			ArgCount = Info->pData->nArgMinSize;
		}else {
			for (int i=0; i<4; ++i) {
				if (Info->varArguments[i] != VT_EMPTY) {
					++ArgCount;
				}
			}
		}
		VARTYPE* varArgTmp = NULL;
		VARTYPE* varArg = Info->varArguments;
		if (4 < ArgCount) {
			varArgTmp = varArg = new VARTYPE[ArgCount];
			for (int i=0; i<ArgCount; ++i) {
				if (i < 4) {
					varArg[i] = Info->varArguments[i];
				}else {
					varArg[i] = Info->pData->pVarArgEx[i-4];
				}
			}
		}
		// 2007.07.21 genta : flag���������l��o�^����
		this->AddMethod(
			FuncName,
			(Info->nFuncID | flags),
			varArg,
			ArgCount,
			Info->varResult,
			reinterpret_cast<CIfObjMethod>(&WSHIfObj::MacroCommand)
			/* WSHIfObj���p�������T�u�N���X����ReadyCommands���Ăяo�����ꍇ�A
			 * �T�u�N���X��MacroCommand���Ăяo�����B */
		);
		delete [] varArgTmp;
		++Info;
	}
}

/*!
	�}�N���R�}���h�̎��s

	@date 2005.06.27 zenryaku �߂�l�̎󂯎�肪�����Ă��G���[�ɂ����Ɋ֐������s����
	@date 2013.06.07 Moca 5�ȏ�̈����̎������̂��C���BNUL���܂ޕ�����Ή�
*/
HRESULT WSHIfObj::MacroCommand(
	int IntID,
	DISPPARAMS* Arguments,
	VARIANT* Result,
	void* Data
	)
{
	int ArgCount = Arguments->cArgs;

	const EFunctionCode ID = static_cast<EFunctionCode>(IntID);
	//	2007.07.22 genta : �R�}���h�͉���16�r�b�g�̂�
	if (LOWORD(ID) >= F_FUNCTION_FIRST) {
		VARIANT ret; // 2005.06.27 zenryaku �߂�l�̎󂯎�肪�����Ă��֐������s����
		VariantInit(&ret);

		// 2011.3.18 syat �����̏����𐳂������ɂ���
		auto_array_ptr<VARIANTARG> rgvargParam(new VARIANTARG[ArgCount]);
		for (int i=0; i<ArgCount; ++i) {
			::VariantInit(&rgvargParam[ArgCount - i - 1]);
			::VariantCopy(&rgvargParam[ArgCount - i - 1], &Arguments->rgvarg[i]);
		}

		// 2009.9.5 syat HandleFunction�̓T�u�N���X�ŃI�[�o�[���C�h����
		bool r = HandleFunction(m_pView, ID, &rgvargParam[0], ArgCount, ret);
		if (Result) {::VariantCopyInd(Result, &ret);}
		VariantClear(&ret);
		for (int i=0; i<ArgCount; ++i) {
			::VariantClear(&rgvargParam[i]);
		}
		return r ? S_OK : E_FAIL;
	}else {
		// �Œ�4�͊m��
		int argCountMin = t_max(4, ArgCount);
		//	Nov. 29, 2005 FILE �����𕶎���Ŏ擾����
		auto_array_ptr<LPWSTR> StrArgs(new LPWSTR[argCountMin]);
		auto_array_ptr<int> strLengths(new int[argCountMin]);
		for (int i=ArgCount; i<argCountMin; ++i) {
			StrArgs[i] = NULL;
			strLengths[i] = 0;
		}
		WCHAR* s = NULL;							// �������K�{
		Variant varCopy;							// VT_BYREF���ƍ���̂ŃR�s�[�p
		int Len;
		for (int i=0; i<ArgCount; ++i) {
			if (VariantChangeType(&varCopy.data, &(Arguments->rgvarg[i]), 0, VT_BSTR) == S_OK) {
				Wrap(&varCopy.data.bstrVal)->GetW(&s, &Len);
			}else {
				s = new WCHAR[1];
				s[0] = 0;
				Len = 0;
			}
			StrArgs[ArgCount - i - 1] = s;			// DISPPARAMS�͈����̏������t�]���Ă��邽�ߐ��������ɒ���
			strLengths[ArgCount - i - 1] = Len;
		}

		// 2009.10.29 syat HandleCommand�̓T�u�N���X�ŃI�[�o�[���C�h����
		HandleCommand(m_pView, ID, const_cast<WCHAR const **>(&StrArgs[0]), &strLengths[0], ArgCount);

		//	Nov. 29, 2005 FILE �z��̔j���Ȃ̂ŁA[����]��ǉ�
		for (int j=0; j<ArgCount; ++j) {
			delete [] StrArgs[j];
		}
		return S_OK;
	}
}

