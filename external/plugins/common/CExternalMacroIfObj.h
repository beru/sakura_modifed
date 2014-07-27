/*!	@file
	@brief �}�N���v���O�C���N���X
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
#ifndef CEXTERNALMACROIFOBJ_H_366C3BF9_4627_4DA3_A802_C684C5E6FF90
#define CEXTERNALMACROIFOBJ_H_366C3BF9_4627_4DA3_A802_C684C5E6FF90

#include <Windows.h>
#include "CExternalIfObj.h"

///////////////////////////////////////////////////////////////////////////////
/*
	�v���O�C�����ɃA�N�Z�X����
*/
class CExternalMacroIfObj : public CExternalIfObj
{
public:
	CExternalMacroIfObj(){}
	virtual ~CExternalMacroIfObj(){}

public:
	virtual LPCWSTR GetName(){
		return L"Macro";
	}

	enum tagModeID {
		MACRO_MODE_CREATOR = 0,
		MACRO_MODE_EXEC
	};

public:	//CExternalMacroIfObj
	enum FuncId {
		F_MA_COMMAND_FIRST = 0,					//���R�}���h�͈ȉ��ɒǉ�����
		F_MA_SET_MATCH,							//�g���q����v�������Ƃ�����
		F_MA_FUNCTION_FIRST = F_FUNCTION_FIRST,	//���֐��͈ȉ��ɒǉ�����
		F_MA_GET_MODE,							//���[�h���擾����
		F_MA_GET_FLAGS,							//flags���擾����
		F_MA_GET_EXT,							//Ext���擾����
		F_MA_GET_SOURCE,						//Source���擾����
		F_MA_GET_INDEX,							//�}�N��Index���擾����
	};

	//Command
	void SetMatch(const int nMatch);
	//Function
	int GetMode();
	int GetFlags();
	WideString GetExt();
	WideString GetSource();
	int GetIndex();

	//Aliases
};

#endif	//CEXTERNALMACROIFOBJ_H_366C3BF9_4627_4DA3_A802_C684C5E6FF90
