/*!	@file
	@brief Outline�I�u�W�F�N�g

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
#pragma once

#include "macro/WSHIfObj.h"
#include "outline/FuncInfo.h"	// FUNCINFO_INFOMASK

class OutlineIfObj : public WSHIfObj {
	// �^��`
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					// ���R�}���h�͈ȉ��ɒǉ�����
		F_OL_ADDFUNCINFO,						// �A�E�g���C����͂ɒǉ�����
		F_OL_ADDFUNCINFO2,						// �A�E�g���C����͂ɒǉ�����i�[���w��j
		F_OL_SETTITLE,							// �A�E�g���C���_�C�A���O�^�C�g�����w��
		F_OL_SETLISTTYPE,						// �A�E�g���C�����X�g��ʂ��w��
		F_OL_SETLABEL,							// ���x����������w��
		F_OL_ADDFUNCINFO3,						//�A�E�g���C����͂ɒǉ�����i�t�@�C�����j
		F_OL_ADDFUNCINFO4,						//�A�E�g���C����͂ɒǉ�����i�[���w��A�t�@�C�����j
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST	//���֐��͈ȉ��ɒǉ�����
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// �R���X�g���N�^
public:
	OutlineIfObj(FuncInfoArr& funcInfoArr)
		:
		WSHIfObj(L"Outline", false),
		nListType(OutlineType::PlugIn),
		funcInfoArr(funcInfoArr)
	{
	}

	// �f�X�g���N�^
public:
	~OutlineIfObj() {}

	// ����
public:
	// �R�}���h�����擾����
	MacroFuncInfoArray GetMacroCommandInfo() const { return macroFuncInfoCommandArr; }
	// �֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const { return macroFuncInfoArr; }
	// �֐�����������
	bool HandleFunction(
		EditView& view,
		EFunctionCode index,
		const VARIANT* arguments,
		const int argSize,
		VARIANT& result
	) {
		return false;
	}
	// �R�}���h����������
	bool HandleCommand(
		EditView& view,
		EFunctionCode index,
		const WCHAR* arguments[],
		const int argLengths[],
		const int argSize
		)
	{
		switch (index) {
		case F_OL_ADDFUNCINFO:			// �A�E�g���C����͂ɒǉ�����
		case F_OL_ADDFUNCINFO2:			// �A�E�g���C����͂ɒǉ�����i�[���w��j
		case F_OL_ADDFUNCINFO3:			//�A�E�g���C����͂ɒǉ�����i�t�@�C�����j
		case F_OL_ADDFUNCINFO4:			//�A�E�g���C����͂ɒǉ�����i�t�@�C����/�[���w��j
			{
				if (!arguments[0]) return false;
				if (!arguments[1]) return false;
				if (!arguments[2]) return false;
				if (!arguments[3]) return false;
				Point ptLogic( _wtoi(arguments[1])-1, _wtoi(arguments[0])-1 );
				Point ptLayout;
				if (ptLogic.x < 0 || ptLogic.y < 0) {
					ptLayout.x = ptLogic.x;
					ptLayout.y = ptLogic.y;
				}else {
					view.GetDocument().layoutMgr.LogicToLayout( ptLogic, &ptLayout );
				}
				int nParam = _wtoi(arguments[3]);
				if (index == F_OL_ADDFUNCINFO) {
					funcInfoArr.AppendData(
						ptLogic.GetY() + 1,
						ptLogic.GetX() + 1, 
						ptLayout.GetY() + 1,
						ptLayout.GetX() + 1,
						arguments[2],
						NULL,
						nParam
					);
				}else if (index == F_OL_ADDFUNCINFO2) {
					int nDepth = nParam & FUNCINFO_INFOMASK;
					nParam -= nDepth;
					funcInfoArr.AppendData(
						ptLogic.GetY() + 1,
						ptLogic.GetX() + 1,
						ptLayout.GetY() + 1,
						ptLayout.GetX() + 1,
						arguments[2],
						NULL,
						nParam,
						nDepth
					);
				}else if (index == F_OL_ADDFUNCINFO3) {
					if (argSize < 5 || arguments[4] == NULL) { return false; }
					funcInfoArr.AppendData(
						ptLogic.GetY() + 1,
						ptLogic.GetX() + 1,
						ptLayout.GetY() + 1,
						ptLayout.GetX() + 1,
						arguments[2],
						arguments[4],
						nParam
					);
				}else if (index == F_OL_ADDFUNCINFO4) {
					if (argSize < 5 || arguments[4] == NULL) { return false; }
					int nDepth = nParam & FUNCINFO_INFOMASK;
					nParam -= nDepth;
					funcInfoArr.AppendData(
						ptLogic.GetY() + 1,
						ptLogic.GetX() + 1,
						ptLayout.GetY() + 1,
						ptLayout.GetX() + 1,
						arguments[2],
						arguments[4],
						nParam,
						nDepth
					);
				}

			}
			break;
		case F_OL_SETTITLE:				// �A�E�g���C���_�C�A���O�^�C�g�����w��
			if (!arguments[0]) return false;
			sOutlineTitle = to_tchar(arguments[0]);
			break;
		case F_OL_SETLISTTYPE:			// �A�E�g���C�����X�g��ʂ��w��
			if (!arguments[0]) return false;
			nListType = (OutlineType)_wtol(arguments[0]);
			break;
		case F_OL_SETLABEL:				// ���x����������w��
			if (!arguments[0] || !arguments[1]) {
				return false;
			}
			{
				std::wstring sLabel = arguments[1];
				funcInfoArr.SetAppendText(_wtol(arguments[0]), sLabel, true);
			}
			break;
		default:
			return false;
		}
		return true;
	}

	// �����o�ϐ�
public:
	tstring sOutlineTitle;
	OutlineType nListType;
private:
	FuncInfoArr& funcInfoArr;
	static MacroFuncInfo macroFuncInfoCommandArr[];	// �R�}���h���(�߂�l�Ȃ�)
	static MacroFuncInfo macroFuncInfoArr[];	// �֐����(�߂�l����)
};

VARTYPE g_OutlineIfObj_MacroArgEx_s[] = {VT_BSTR};
MacroFuncInfoEx g_OutlineIfObj_FuncInfoEx_s = {5, 5, g_OutlineIfObj_MacroArgEx_s};

// �R�}���h���
MacroFuncInfo OutlineIfObj::macroFuncInfoCommandArr[] = {
	// ID									�֐���							����										�߂�l�̌^	pszData
	{EFunctionCode(F_OL_ADDFUNCINFO),		LTEXT("AddFuncInfo"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	nullptr }, // �A�E�g���C����͂ɒǉ�����
	{EFunctionCode(F_OL_ADDFUNCINFO2),		LTEXT("AddFuncInfo2"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	nullptr }, // �A�E�g���C����͂ɒǉ�����i�[���w��j
	{EFunctionCode(F_OL_SETTITLE),			LTEXT("SetTitle"),				{VT_BSTR, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr },	// �A�E�g���C���_�C�A���O�^�C�g�����w��
	{EFunctionCode(F_OL_SETLISTTYPE),		LTEXT("SetListType"),			{VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},		VT_EMPTY,	nullptr }, // �A�E�g���C�����X�g��ʂ��w��
	{EFunctionCode(F_OL_SETLABEL),			LTEXT("SetLabel"),				{VT_I4, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_EMPTY,	nullptr }, // ���x����������w��
	{EFunctionCode(F_OL_ADDFUNCINFO3),		LTEXT("AddFuncInfo3"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	&g_OutlineIfObj_FuncInfoEx_s }, //�A�E�g���C����͂ɒǉ�����i�t�@�C�����j
	{EFunctionCode(F_OL_ADDFUNCINFO4),		LTEXT("AddFuncInfo4"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	&g_OutlineIfObj_FuncInfoEx_s }, //�A�E�g���C����͂ɒǉ�����i�t�@�C�����A�[���w��j
	// �I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
};

// �֐����
MacroFuncInfo OutlineIfObj::macroFuncInfoArr[] = {
	//ID									�֐���							����										�߂�l�̌^	pszData
	//	�I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
};

