/*!	@file
	@brief �X�}�[�g�C���f���g�v���O�C���N���X
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
#ifndef CEXTERNALSMARTINDENTIFOBJ_H_366C3BF9_4627_4DA3_A802_C684C5E6FF99
#define CEXTERNALSMARTINDENTIFOBJ_H_366C3BF9_4627_4DA3_A802_C684C5E6FF99

#include <Windows.h>
#include "CExternalIfObj.h"

///////////////////////////////////////////////////////////////////////////////
/*
	�v���O�C�����ɃA�N�Z�X����
*/
class CExternalSmartIndentIfObj : public CExternalIfObj
{
public:
	CExternalSmartIndentIfObj(){}
	virtual ~CExternalSmartIndentIfObj(){}

public:
	virtual LPCWSTR GetName(){
		return L"Indent";
	}

public:	//CSmartindentIfObj
	enum FuncId {
		F_SI_COMMAND_FIRST = 0,					//���R�}���h�͈ȉ��ɒǉ�����
		F_SI_FUNCTION_FIRST = F_FUNCTION_FIRST,	//���֐��͈ȉ��ɒǉ�����
		F_SI_GETCHAR							//���������L�[���擾����
	};

	//Command
	//Function
	WideString GetChar();
};

#endif	//CEXTERNALSMARTINDENTIFOBJ_H_366C3BF9_4627_4DA3_A802_C684C5E6FF99
