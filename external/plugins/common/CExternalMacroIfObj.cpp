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

#include "stdafx.h"
#include "CExternalMacroIfObj.h"

///////////////////////////////////////////////////////////////////////////////
//Command
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_SET_MATCH
	�g���q����v�������Ƃ��Z�b�g����B
	@param[in]	arg1	0:�g���q�s��v, 1:�g���q��v
*/
void CExternalMacroIfObj::SetMatch(const int arg1)
{
	DEFINE_WCHAR_PARAMS(Arguments);
	SET_INT_ARG(0, arg1);
	HandleCommand(F_MA_SET_MATCH, Arguments, ArgLengths, 1);
}

///////////////////////////////////////////////////////////////////////////////
//Function
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*	F_MA_GET_MODE
	�}�N�����s���[�h���擾����B
	@retval	0	�g���q�m�F�v���@�g���q����v������SetMatch(1)���s���B
	@retval	1	�}�N�����s�v��
*/
int CExternalMacroIfObj::GetMode()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_MODE, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_GET_FLAGS
	@return	�}�N�����s����ID�}�X�N�l
	�ʏ��Editor�v���O�C�����Ŏ����I�ɃZ�b�g���Ă���邽�߉������Ȃ��Ă悢�B
*/
int CExternalMacroIfObj::GetFlags()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_FLAGS, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_GET_EXT
	@return	���s�}�N���̊g���q
*/
WideString CExternalMacroIfObj::GetExt()
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_EXT, Arguments, 0, &Result)){
		result = (LPCTSTR)Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_GET_SOURCE
	@return	���s�}�N���\�[�X�R�[�h
*/
WideString CExternalMacroIfObj::GetSource()
{
	WideString result = L"";
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_SOURCE, Arguments, 0, &Result)){
		result = (LPCTSTR)Result.bstrVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}

///////////////////////////////////////////////////////////////////////////////
/*!	F_MA_GET_INDEX
	@return	�}�N�����s����Index�l (CSMacroMgr.h���Q��)
	-1: �W���}�N��(�L�[�}�N��)
	-2: �ꎞ�}�N��(���O���w�肵�ă}�N�����s)
	-3: �����ȃ}�N��
	0�`: �}�N���ԍ�
*/
int CExternalMacroIfObj::GetIndex()
{
	int result = 0;
	DEFINE_PARAMS(Arguments, Result);
	if(HandleFunction(F_MA_GET_INDEX, Arguments, 0, &Result)){
		result = Result.lVal;
	}
	CLEAR_PARAMS(Arguments, Result);
	return result;
}
