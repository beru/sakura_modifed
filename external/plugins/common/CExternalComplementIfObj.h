/*!	@file
	@brief �⊮�v���O�C���N���X
*/
/*
	Copyright (C) 2013, Plugins developers

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
#ifndef CEXTERNALCOMPLEMENTIFOBJ_H_901F8FB9_701F_4249_A77E_629296545585
#define CEXTERNALCOMPLEMENTIFOBJ_H_901F8FB9_701F_4249_A77E_629296545585

#include <Windows.h>
#include "CExternalIfObj.h"

///////////////////////////////////////////////////////////////////////////////
/*
	�v���O�C�����ɃA�N�Z�X����
*/
class CExternalComplementIfObj : public CExternalIfObj
{
public:
	CExternalComplementIfObj(){}
	virtual ~CExternalComplementIfObj(){}

public:
	virtual LPCWSTR GetName(){
		return L"Complement";
	}

public:	//CComplementIfObj
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					//���R�}���h�͈ȉ��ɒǉ�����
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST,	//���֐��͈ȉ��ɒǉ�����
		F_CM_GETCURRENTWORD,					//�⊮�Ώۂ̕�������擾
		F_CM_GETOPTION,							//�I�v�V�������擾
		F_CM_ADDLIST,							//���ɒǉ�
	};

	//Command
	//Function
	WideString GetCurrentWord();
	int GetOption();
	int AddList(LPCWSTR arg1);

	//Aliases
	int AddList(const WideString& arg1);
};

#endif	//CEXTERNALCOMPLEMENTIFOBJ_H_901F8FB9_701F_4249_A77E_629296545585
