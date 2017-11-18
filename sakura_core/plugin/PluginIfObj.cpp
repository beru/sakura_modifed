/*!	@file
	@brief Plugin�I�u�W�F�N�g
*/
#include "stdafx.h"
#include "plugin/PluginIfObj.h"

// �R�}���h���
MacroFuncInfo PluginIfObj::macroFuncInfoCommandArr[] = {
	// ID									�֐���							����										�߂�l�̌^	pszData
	{EFunctionCode(F_PL_SETOPTION),			LTEXT("SetOption"),				{VT_BSTR, VT_BSTR, VT_VARIANT, VT_EMPTY},	VT_EMPTY,	NULL }, // �I�v�V�����t�@�C���ɒl������
	{EFunctionCode(F_PL_ADDCOMMAND),		LTEXT("AddCommand"),			{VT_BSTR, VT_BSTR, VT_BSTR, VT_EMPTY},		VT_EMPTY,	NULL }, // �R�}���h��ǉ�����
	// �I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

// �֐����
MacroFuncInfo PluginIfObj::macroFuncInfoArr[] = {
	// ID									�֐���							����										�߂�l�̌^	pszData
	{EFunctionCode(F_PL_GETPLUGINDIR),		LTEXT("GetPluginDir"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // �v���O�C���t�H���_�p�X���擾����
	{EFunctionCode(F_PL_GETDEF),			LTEXT("GetDef"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, // �ݒ�t�@�C������l��ǂ�
	{EFunctionCode(F_PL_GETOPTION),			LTEXT("GetOption"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, // �I�v�V�����t�@�C������l��ǂ�
	{EFunctionCode(F_PL_GETCOMMANDNO),		LTEXT("GetCommandNo"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �I�v�V�����t�@�C������l��ǂ�
	{EFunctionCode(F_PL_GETSTRING),			LTEXT("GetString"),				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // �ݒ�t�@�C�����當�����ǂ�
	// �I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

