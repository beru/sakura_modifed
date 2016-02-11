/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�����n �A�E�g���C�����)�֐��Q

	2012/12/17	CViewCommander.cpp���番��
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
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "outline/CFuncInfoArr.h"
#include "plugin/CJackManager.h"
#include "plugin/COutlineIfObj.h"
#include "sakura_rc.h"


/*!	�A�E�g���C�����
	
	2002/3/13 YAZAKI nOutlineType��nListType�𓝍��B
*/
// �g�O���p�̃t���O�ɕύX 20060201 aroka
bool ViewCommander::Command_FUNCLIST(
	int nAction,
	int _nOutlineType = OUTLINE_DEFAULT
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

	EOutlineType nOutlineType = (EOutlineType)_nOutlineType; // 2007.11.29 kobake

//	if (bCheckOnly) {
//		return TRUE;
//	}

	static FuncInfoArr	cFuncInfoArr;
//	int		nLine;
//	int		nListType;
	std::tstring sTitleOverride;				// �v���O�C���ɂ��_�C�A���O�^�C�g���㏑��

	// 2001.12.03 hor & 2002.3.13 YAZAKI
	if (nOutlineType == OUTLINE_DEFAULT) {
		// �^�C�v�ʂɐݒ肳�ꂽ�A�E�g���C����͕��@
		nOutlineType = m_pCommanderView->m_pTypeData->m_eDefaultOutline;
		if (nOutlineType == OUTLINE_CPP) {
			if (CheckEXT(GetDocument()->m_docFile.GetFilePath(), _T("c"))) {
				nOutlineType = OUTLINE_C;	// �����C�֐��ꗗ���X�g�r���[�ɂȂ�
			}
		}
	}

	auto& dlgFuncList = GetEditWindow()->m_dlgFuncList;
	if (dlgFuncList.GetHwnd() && nAction != (int)ShowDialogType::Reload) {
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
	cFuncInfoArr.Empty();
	int nListType = nOutlineType;			// 2011.06.25 syat

	auto& cDocOutline = GetDocument()->m_docOutline;
	switch (nOutlineType) {
	case OUTLINE_C:			// C/C++ �� MakeFuncList_C
	case OUTLINE_CPP:		cDocOutline.MakeFuncList_C(&cFuncInfoArr);break;
	case OUTLINE_PLSQL:		cDocOutline.MakeFuncList_PLSQL(&cFuncInfoArr);break;
	case OUTLINE_JAVA:		cDocOutline.MakeFuncList_Java(&cFuncInfoArr);break;
	case OUTLINE_COBOL:		cDocOutline.MakeTopicList_cobol(&cFuncInfoArr);break;
	case OUTLINE_ASM:		cDocOutline.MakeTopicList_asm(&cFuncInfoArr);break;
	case OUTLINE_PERL:		cDocOutline.MakeFuncList_Perl(&cFuncInfoArr);break;	// Sep. 8, 2000 genta
	case OUTLINE_VB:		cDocOutline.MakeFuncList_VisualBasic(&cFuncInfoArr);break;	// June 23, 2001 N.Nakatani
	case OUTLINE_WZTXT:		cDocOutline.MakeTopicList_wztxt(&cFuncInfoArr);break;		// 2003.05.20 zenryaku �K�w�t�e�L�X�g �A�E�g���C�����
	case OUTLINE_HTML:		cDocOutline.MakeTopicList_html(&cFuncInfoArr);break;		// 2003.05.20 zenryaku HTML �A�E�g���C�����
	case OUTLINE_TEX:		cDocOutline.MakeTopicList_tex(&cFuncInfoArr);break;		// 2003.07.20 naoh TeX �A�E�g���C�����
	case OUTLINE_BOOKMARK:	cDocOutline.MakeFuncList_BookMark(&cFuncInfoArr);break;	// 2001.12.03 hor
	case OUTLINE_FILE:		cDocOutline.MakeFuncList_RuleFile(&cFuncInfoArr, sTitleOverride);break;	// 2002.04.01 YAZAKI �A�E�g���C����͂Ƀ��[���t�@�C���𓱓�
//	case OUTLINE_UNKNOWN:	// Jul. 08, 2001 JEPRO �g��Ȃ��悤�ɕύX
	case OUTLINE_PYTHON:	cDocOutline.MakeFuncList_python(&cFuncInfoArr);break;		// 2007.02.08 genta
	case OUTLINE_ERLANG:	cDocOutline.MakeFuncList_Erlang(&cFuncInfoArr);break;		// 2009.08.10 genta
	case OUTLINE_FILETREE:	/* ���ɉ������Ȃ�*/ ;break;	// 2013.12.08 Moca
	case OUTLINE_TEXT:
		// fall though
		// �����ɂ͉�������Ă͂����Ȃ� 2007.02.28 genta ���ӏ���
	default:
		// �v���O�C�����猟������
		{
			Plug::Array plugs;
			JackManager::getInstance()->GetUsablePlug(PP_OUTLINE, nOutlineType, &plugs);

			if (plugs.size() > 0) {
				assert_warning(1 == plugs.size());
				// �C���^�t�F�[�X�I�u�W�F�N�g����
				WSHIfObj::List params;
				OutlineIfObj* objOutline = new OutlineIfObj(cFuncInfoArr);
				objOutline->AddRef();
				params.push_back(objOutline);
				// �v���O�C���Ăяo��
				(*plugs.begin())->Invoke(m_pCommanderView, params);

				nListType = objOutline->m_nListType;			// �_�C�A���O�̕\�����@�����㏑��
				sTitleOverride = objOutline->m_sOutlineTitle;	// �_�C�A���O�^�C�g�����㏑��

				objOutline->Release();
				break;
			}
		}

		// ����ȊO
		cDocOutline.MakeTopicList_txt(&cFuncInfoArr);
		break;
	}

	// ��͑Ώۃt�@�C����
	_tcscpy(cFuncInfoArr.m_szFilePath, GetDocument()->m_docFile.GetFilePath());

	// �A�E�g���C�� �_�C�A���O�̕\��
	LayoutPoint poCaret = GetCaret().GetCaretLayoutPos();
	if (!dlgFuncList.GetHwnd()) {
		dlgFuncList.DoModeless(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			(LPARAM)m_pCommanderView,
			&cFuncInfoArr,
			poCaret.GetY2() + LayoutInt(1),
			poCaret.GetX2() + LayoutInt(1),
			nOutlineType,
			nListType,
			m_pCommanderView->m_pTypeData->m_bLineNumIsCRLF	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
		);
	}else {
		// �A�N�e�B�u�ɂ���
		dlgFuncList.Redraw(nOutlineType, nListType, &cFuncInfoArr, poCaret.GetY2() + 1, poCaret.GetX2() + 1);
		if (bForeground) {
			::SetFocus(dlgFuncList.GetHwnd());
		}
	}

	// �_�C�A���O�^�C�g�����㏑��
	if (!sTitleOverride.empty()) {
		dlgFuncList.SetWindowText(sTitleOverride.c_str());
	}

	bIsProcessing = false;
	return true;
}

