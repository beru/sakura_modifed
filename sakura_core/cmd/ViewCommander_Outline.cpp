/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�����n �A�E�g���C�����)�֐��Q

	2012/12/17	ViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, hor
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, zenryaku
	Copyright (C) 2006, aroka
	Copyright (C) 2007, genta, kobake
	Copyright (C) 2009, genta
	Copyright (C) 2011, syat

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "outline/FuncInfoArr.h"
#include "plugin/JackManager.h"
#include "plugin/OutlineIfObj.h"
#include "sakura_rc.h"


/*!	�A�E�g���C�����
	
	2002/3/13 YAZAKI nOutlineType��nListType�𓝍��B
*/
// �g�O���p�̃t���O�ɕύX 20060201 aroka
bool ViewCommander::Command_FuncList(
	ShowDialogType nAction,
	OutlineType _nOutlineType = OutlineType::Default
	)
{
	static bool bIsProcessing = false;	// �A�E�g���C����͏������t���O

	// �A�E�g���C���v���O�C�����ł�Editor.Outline�Ăяo���ɂ��ē����֎~����
	if (bIsProcessing) return false;

	bIsProcessing = true;

	// ���v���Z�X���O�ʂɂ��邩�ǂ������ׂ�
	DWORD dwPid2;
	DWORD dwPid1 = ::GetCurrentProcessId();
	::GetWindowThreadProcessId(::GetForegroundWindow(), &dwPid2);
	bool bForeground = (dwPid1 == dwPid2);

	OutlineType nOutlineType = _nOutlineType; // 2007.11.29 kobake

//	if (bCheckOnly) {
//		return TRUE;
//	}

	static FuncInfoArr funcInfoArr;
//	int		nLine;
//	int		nListType;
	std::tstring titleOverride;				// �v���O�C���ɂ��_�C�A���O�^�C�g���㏑��

	// 2001.12.03 hor & 2002.3.13 YAZAKI
	if (nOutlineType == OutlineType::Default) {
		// �^�C�v�ʂɐݒ肳�ꂽ�A�E�g���C����͕��@
		nOutlineType = view.pTypeData->eDefaultOutline;
		if (nOutlineType == OutlineType::CPP) {
			if (CheckEXT(GetDocument().docFile.GetFilePath(), _T("c"))) {
				nOutlineType = OutlineType::C;	// �����C�֐��ꗗ���X�g�r���[�ɂȂ�
			}
		}
	}

	auto& dlgFuncList = GetEditWindow().dlgFuncList;
	if (dlgFuncList.GetHwnd() && nAction != ShowDialogType::Reload) {
		switch (nAction) {
		case ShowDialogType::Normal: // �A�N�e�B�u�ɂ���
			// �J���Ă�����̂Ǝ�ʂ������Ȃ�Active�ɂ��邾���D�قȂ�΍ĉ��
			dlgFuncList.SyncColor();
			if (dlgFuncList.CheckListType(nOutlineType)) {
				if (bForeground) {
					::SetFocus(dlgFuncList.GetHwnd());
				}
				bIsProcessing = false;
				return true;
			}
			break;
		case ShowDialogType::Toggle: // ����
			// �J���Ă�����̂Ǝ�ʂ������Ȃ����D�قȂ�΍ĉ��
			if (dlgFuncList.CheckListType(nOutlineType)) {
				if (dlgFuncList.IsDocking())
					::DestroyWindow(dlgFuncList.GetHwnd());
				else
					::SendMessage(dlgFuncList.GetHwnd(), WM_CLOSE, 0, 0);
				bIsProcessing = false;
				return true;
			}
			break;
		default:
			break;
		}
	}

	// ��͌��ʃf�[�^����ɂ���
	funcInfoArr.Empty();
	OutlineType nListType = nOutlineType;			// 2011.06.25 syat

	auto& docOutline = GetDocument().docOutline;
	switch (nOutlineType) {
	case OutlineType::C:			// C/C++ �� MakeFuncList_C
	case OutlineType::CPP:			docOutline.MakeFuncList_C(&funcInfoArr);break;
	case OutlineType::PLSQL:		docOutline.MakeFuncList_PLSQL(&funcInfoArr);break;
	case OutlineType::Java:			docOutline.MakeFuncList_Java(&funcInfoArr);break;
	case OutlineType::Cobol:		docOutline.MakeTopicList_cobol(&funcInfoArr);break;
	case OutlineType::Asm:			docOutline.MakeTopicList_asm(&funcInfoArr);break;
	case OutlineType::Perl:			docOutline.MakeFuncList_Perl(&funcInfoArr);break;	// Sep. 8, 2000 genta
	case OutlineType::VisualBasic:	docOutline.MakeFuncList_VisualBasic(&funcInfoArr);break;	// June 23, 2001 N.Nakatani
	case OutlineType::WZText:		docOutline.MakeTopicList_wztxt(&funcInfoArr);break;		// 2003.05.20 zenryaku �K�w�t�e�L�X�g �A�E�g���C�����
	case OutlineType::HTML:			docOutline.MakeTopicList_html(&funcInfoArr);break;		// 2003.05.20 zenryaku HTML �A�E�g���C�����
	case OutlineType::TeX:			docOutline.MakeTopicList_tex(&funcInfoArr);break;		// 2003.07.20 naoh TeX �A�E�g���C�����
	case OutlineType::BookMark:		docOutline.MakeFuncList_BookMark(&funcInfoArr);break;	// 2001.12.03 hor
	case OutlineType::RuleFile:		docOutline.MakeFuncList_RuleFile(&funcInfoArr, titleOverride);break;	// 2002.04.01 YAZAKI �A�E�g���C����͂Ƀ��[���t�@�C���𓱓�
//	case OUTLINE_UNKNOWN:	// Jul. 08, 2001 JEPRO �g��Ȃ��悤�ɕύX
	case OutlineType::Python:		docOutline.MakeFuncList_python(&funcInfoArr);break;		// 2007.02.08 genta
	case OutlineType::Erlang:		docOutline.MakeFuncList_Erlang(&funcInfoArr);break;		// 2009.08.10 genta
	case OutlineType::FileTree:	/* ���ɉ������Ȃ�*/ ;break;	// 2013.12.08 Moca
	case OutlineType::Text:
		// fall though
		// �����ɂ͉�������Ă͂����Ȃ� 2007.02.28 genta ���ӏ���
	default:
		// �v���O�C�����猟������
		{
			Plug::Array plugs;
			JackManager::getInstance().GetUsablePlug(PP_OUTLINE, (PlugId)nOutlineType, &plugs);

			if (plugs.size() > 0) {
				assert_warning(plugs.size() == 1);
				// �C���^�t�F�[�X�I�u�W�F�N�g����
				WSHIfObj::List params;
				OutlineIfObj* objOutline = new OutlineIfObj(funcInfoArr);
				objOutline->AddRef();
				params.push_back(objOutline);
				// �v���O�C���Ăяo��
				(*plugs.begin())->Invoke(view, params);

				nListType = objOutline->nListType;			// �_�C�A���O�̕\�����@�����㏑��
				titleOverride = objOutline->sOutlineTitle;	// �_�C�A���O�^�C�g�����㏑��

				objOutline->Release();
				break;
			}
		}

		// ����ȊO
		docOutline.MakeTopicList_txt(&funcInfoArr);
		break;
	}

	// ��͑Ώۃt�@�C����
	_tcscpy(funcInfoArr.szFilePath, GetDocument().docFile.GetFilePath());

	// �A�E�g���C�� �_�C�A���O�̕\��
	LayoutPoint poCaret = GetCaret().GetCaretLayoutPos();
	if (!dlgFuncList.GetHwnd()) {
		dlgFuncList.DoModeless(
			G_AppInstance(),
			view.GetHwnd(),
			(LPARAM)&view,
			&funcInfoArr,
			poCaret.GetY2() + 1,
			poCaret.GetX2() + 1,
			nOutlineType,
			nListType,
			view.pTypeData->bLineNumIsCRLF	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
		);
	}else {
		// �A�N�e�B�u�ɂ���
		dlgFuncList.Redraw(nOutlineType, nListType, &funcInfoArr, poCaret.GetY2() + 1, poCaret.GetX2() + 1);
		if (bForeground) {
			::SetFocus(dlgFuncList.GetHwnd());
		}
	}

	// �_�C�A���O�^�C�g�����㏑��
	if (!titleOverride.empty()) {
		dlgFuncList.SetWindowText(titleOverride.c_str());
	}

	bIsProcessing = false;
	return true;
}

