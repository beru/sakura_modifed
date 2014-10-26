/*!	@file
	@brief Plugin�I�u�W�F�N�g

*/
/*
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

#include "stdafx.h"
#include "plugin/CPluginIfObj.h"

// �R�}���h���
MacroFuncInfo CPluginIfObj::m_MacroFuncInfoCommandArr[] = {
	// ID									�֐���							����										�߂�l�̌^	m_pszData
	{EFunctionCode(F_PL_SETOPTION),			LTEXT("SetOption"),				{VT_BSTR, VT_BSTR, VT_VARIANT, VT_EMPTY},	VT_EMPTY,	NULL }, // �I�v�V�����t�@�C���ɒl������
	{EFunctionCode(F_PL_ADDCOMMAND),		LTEXT("AddCommand"),			{VT_BSTR, VT_BSTR, VT_BSTR, VT_EMPTY},		VT_EMPTY,	NULL }, // �R�}���h��ǉ�����
	// �I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

// �֐����
MacroFuncInfo CPluginIfObj::m_MacroFuncInfoArr[] = {
	// ID									�֐���							����										�߂�l�̌^	m_pszData
	{EFunctionCode(F_PL_GETPLUGINDIR),		LTEXT("GetPluginDir"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // �v���O�C���t�H���_�p�X���擾����
	{EFunctionCode(F_PL_GETDEF),			LTEXT("GetDef"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, // �ݒ�t�@�C������l��ǂ�
	{EFunctionCode(F_PL_GETOPTION),			LTEXT("GetOption"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, // �I�v�V�����t�@�C������l��ǂ�
	{EFunctionCode(F_PL_GETCOMMANDNO),		LTEXT("GetCommandNo"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �I�v�V�����t�@�C������l��ǂ�
	{EFunctionCode(F_PL_GETSTRING),			LTEXT("GetString"),				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // �ݒ�t�@�C�����當�����ǂ�
	// �I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

