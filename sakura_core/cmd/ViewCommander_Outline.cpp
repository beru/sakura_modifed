#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "outline/FuncInfoArr.h"
#include "plugin/JackManager.h"
#include "plugin/OutlineIfObj.h"
#include "sakura_rc.h"

// ViewCommander�N���X�̃R�}���h(�����n �A�E�g���C�����)�֐��Q

/*!	�A�E�g���C����� */
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

	OutlineType nOutlineType = _nOutlineType;

//	if (bCheckOnly) {
//		return TRUE;
//	}

	static FuncInfoArr funcInfoArr;
//	int		nLine;
//	int		nListType;
	std::tstring titleOverride;				// �v���O�C���ɂ��_�C�A���O�^�C�g���㏑��

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
	OutlineType nListType = nOutlineType;

	auto& docOutline = GetDocument().docOutline;
	switch (nOutlineType) {
	case OutlineType::C:			// C/C++ �� MakeFuncList_C
	case OutlineType::CPP:			docOutline.MakeFuncList_C(&funcInfoArr);break;
	case OutlineType::PLSQL:		docOutline.MakeFuncList_PLSQL(&funcInfoArr);break;
	case OutlineType::Java:			docOutline.MakeFuncList_Java(&funcInfoArr);break;
	case OutlineType::Cobol:		docOutline.MakeTopicList_cobol(&funcInfoArr);break;
	case OutlineType::Asm:			docOutline.MakeTopicList_asm(&funcInfoArr);break;
	case OutlineType::Perl:			docOutline.MakeFuncList_Perl(&funcInfoArr);break;
	case OutlineType::VisualBasic:	docOutline.MakeFuncList_VisualBasic(&funcInfoArr);break;
	case OutlineType::WZText:		docOutline.MakeTopicList_wztxt(&funcInfoArr);break;		// �K�w�t�e�L�X�g �A�E�g���C�����
	case OutlineType::HTML:			docOutline.MakeTopicList_html(&funcInfoArr);break;		// HTML �A�E�g���C�����
	case OutlineType::TeX:			docOutline.MakeTopicList_tex(&funcInfoArr);break;		// TeX �A�E�g���C�����
	case OutlineType::BookMark:		docOutline.MakeFuncList_BookMark(&funcInfoArr);break;	// 
	case OutlineType::RuleFile:		docOutline.MakeFuncList_RuleFile(&funcInfoArr, titleOverride);break;	// �A�E�g���C����͂Ƀ��[���t�@�C���𓱓�
	case OutlineType::Python:		docOutline.MakeFuncList_python(&funcInfoArr);break;		// 
	case OutlineType::Erlang:		docOutline.MakeFuncList_Erlang(&funcInfoArr);break;		// 
	case OutlineType::FileTree:	/* ���ɉ������Ȃ�*/ ;break;	// 
	case OutlineType::Text:
		// fall though
		// �����ɂ͉�������Ă͂����Ȃ�
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
	Point poCaret = GetCaret().GetCaretLayoutPos();
	if (!dlgFuncList.GetHwnd()) {
		dlgFuncList.DoModeless(
			G_AppInstance(),
			view.GetHwnd(),
			(LPARAM)&view,
			&funcInfoArr,
			poCaret.y + 1,
			poCaret.x + 1,
			nOutlineType,
			nListType,
			view.pTypeData->bLineNumIsCRLF	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
		);
	}else {
		// �A�N�e�B�u�ɂ���
		dlgFuncList.Redraw(nOutlineType, nListType, &funcInfoArr, poCaret.y + 1, poCaret.x + 1);
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

