/*!	@file
	�L�[�{�[�h�}�N��

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, hor
	Copyright (C) 2002, YAZAKI, aroka, genta, Moca, hor
	Copyright (C) 2003, �S, ryoji, Moca
	Copyright (C) 2004, genta, zenryaku
	Copyright (C) 2005, MIK, genta, maru, zenryaku, FILE
	Copyright (C) 2006, �����, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2011, syat

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

#include "StdAfx.h"
#include "func/Funccode.h"
#include "Macro.h"
#include "_main/ControlTray.h"
#include "cmd/ViewCommander_inline.h"
#include "view/EditView.h"		// 2002/2/10 aroka
#include "macro/SMacroMgr.h"	// 2002/2/10 aroka
#include "doc/EditDoc.h"		// 2002/5/13 YAZAKI �w�b�_����
#include "_os/OleTypes.h"		// 2003-02-21 �S
#include "io/TextStream.h"
#include "window/EditWnd.h"
#include "env/SakuraEnvironment.h"
#include "dlg/DlgInput1.h"
#include "dlg/DlgOpenFile.h"
#include "util/format.h"
#include "util/shell.h"
#include "util/ole_convert.h"
#include "util/os.h"
#include "uiparts/WaitCursor.h"

Macro::Macro(EFunctionCode nFuncID)
{
	nFuncID = nFuncID;
	pNext = nullptr;
	pParamTop = pParamBot = nullptr;
}

Macro::~Macro(void)
{
	ClearMacroParam();
}

void Macro::ClearMacroParam()
{
	MacroParam* p = pParamTop;
	MacroParam* del_p;
	while (p) {
		del_p = p;
		p = p->pNext;
		delete del_p;
	}
	pParamTop = nullptr;
	pParamBot = nullptr;
	return;
}

/*	�����̌^�U�蕪��
	�@�\ID�ɂ���āA���҂���^�͈قȂ�܂��B
	�����ŁA�����̌^���@�\ID�ɂ���ĐU�蕪���āAAddParam���܂��傤�B
	���Ƃ��΁AF_INSTEXT_W��1�߁A2�߂̈����͕�����A3�߂̈�����int�������肷��̂��A�����ł��܂��U�蕪�����邱�Ƃ����҂��Ă��܂��B

	lParam�́AHandleCommand��param�ɒl��n���Ă���R�}���h�̏ꍇ�ɂ̂ݎg���܂��B
*/
void Macro::AddLParam(
	const LPARAM* lParams,
	const EditView& editView
	)
{
	int nOption = 0;
	LPARAM lParam = lParams[0];
	switch (nFuncID) {
	case F_GOLOGICALLINETOP_BOX:
	case F_GOLINETOP_BOX:
	case F_GOLINEEND_BOX:
	case F_HalfPageUp_BOX:
	case F_HalfPageDown_BOX:
	case F_1PageUp_BOX:
	case F_1PageDown_BOX:
		nOption = 1;
	case F_UP_BOX:
	case F_DOWN_BOX:
	case F_LEFT_BOX:
	case F_RIGHT_BOX:
	case F_UP2_BOX:
	case F_DOWN2_BOX:
	case F_WORDLEFT_BOX:
	case F_WORDRIGHT_BOX:
	case F_GOFILETOP_BOX:
	case F_GOFILEEND_BOX:
		{
			if (nOption == 1) {
				switch (nFuncID) {
				case F_HalfPageUp_BOX:
				case F_HalfPageDown_BOX:
					if (lParam == 0) {
						AddIntParam(editView.GetTextArea().nViewRowNum / 2);
					}else {
						AddIntParam(lParam);
					}
					break;
				case F_1PageUp_BOX:
				case F_1PageDown_BOX:
					if (lParam == 0) {
						AddIntParam(editView.GetTextArea().nViewRowNum - 1);
					}else {
						AddIntParam(lParam);
					}
					break;
				default:
					AddIntParam( lParam );
					break;
				}
			}
			int nParamOption;
			if (nOption == 1) {
				nParamOption = lParams[1];
			}else {
				nParamOption = lParam;
			}
			if (nParamOption == 0) {
				if (GetDllShareData().common.edit.bBoxSelectLock) {
					nParamOption = 0x01;
				}else {
					nParamOption = 0x02;
				}
			}
			AddIntParam( nParamOption );
		}
		break;
	case F_INSTEXT_W:
	case F_FILEOPEN:
	case F_EXECEXTMACRO:
		{
			AddStringParam((const wchar_t*)lParam);	// lParam��ǉ��B
		}
		break;

	case F_EXECMD:
		{
			AddStringParam((const wchar_t*)lParam);	// lParam��ǉ��B
			AddIntParam((int)lParams[1]);
			if (lParams[2] != 0) {
				AddStringParam((const wchar_t*)lParams[2]);
			}
		}
		break;

	case F_JUMP:	// �w��s�փW�����v�i������PL/SQL�R���p�C���G���[�s�ւ̃W�����v�͖��Ή��j
		{
			AddIntParam(editView.editWnd.dlgJump.nLineNum);
			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().bLineNumIsCRLF_ForJump		? 0x01 : 0x00;
			lFlag |= editView.editWnd.dlgJump.bPLSQL	? 0x02 : 0x00;
			AddIntParam(lFlag);
		}
		break;

	case F_BOOKMARK_PATTERN:	//2002.02.08 hor
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		{
			AddStringParam(editView.strCurSearchKey.c_str());	// lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= editView.curSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= editView.curSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= editView.curSearchOption.bRegularExp		? 0x04 : 0x00;
			lFlag |= GetDllShareData().common.search.bNotifyNotFound				? 0x08 : 0x00;
			lFlag |= GetDllShareData().common.search.bAutoCloseDlgFind			? 0x10 : 0x00;
			lFlag |= GetDllShareData().common.search.bSearchAll					? 0x20 : 0x00;
			AddIntParam(lFlag);
		}
		break;
	case F_REPLACE:
	case F_REPLACE_ALL:
		{
			AddStringParam(editView.strCurSearchKey.c_str());	// lParam��ǉ��B
			AddStringParam(editView.editWnd.dlgReplace.strText2.c_str());	// lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= editView.curSearchOption.bWordOnly		? 0x01 : 0x00;
			lFlag |= editView.curSearchOption.bLoHiCase		? 0x02 : 0x00;
			lFlag |= editView.curSearchOption.bRegularExp	? 0x04 : 0x00;
			lFlag |= GetDllShareData().common.search.bNotifyNotFound				? 0x08 : 0x00;
			lFlag |= GetDllShareData().common.search.bAutoCloseDlgFind			? 0x10 : 0x00;
			lFlag |= GetDllShareData().common.search.bSearchAll					? 0x20 : 0x00;
			lFlag |= editView.editWnd.dlgReplace.bPaste					? 0x40 : 0x00;	// CShareData�ɓ���Ȃ��Ă����́H
			lFlag |= GetDllShareData().common.search.bSelectedArea				? 0x80 : 0x00;	// �u�����鎞�͑I�ׂȂ�
			lFlag |= editView.editWnd.dlgReplace.nReplaceTarget << 8;	// 8bit�V�t�g�i0x100�Ŋ|���Z�j
			lFlag |= GetDllShareData().common.search.bConsecutiveAll				? 0x0400: 0x00;	// 2007.01.16 ryoji
			AddIntParam(lFlag);
		}
		break;
	case F_GREP_REPLACE:
	case F_GREP:
		{
			DlgGrep* pDlgGrep;
			DlgGrepReplace* pDlgGrepRep;
			if (nFuncID == F_GREP) {
				pDlgGrep = &editView.editWnd.dlgGrep;
				pDlgGrepRep = nullptr;
				AddStringParam( pDlgGrep->strText.c_str() );
			}else {
				pDlgGrep = pDlgGrepRep = &editView.editWnd.dlgGrepReplace;
				AddStringParam( pDlgGrep->strText.c_str() );
				AddStringParam( editView.editWnd.dlgGrepReplace.strText2.c_str() );
			}
			AddStringParam(GetDllShareData().searchKeywords.grepFiles[0]);	// lParam��ǉ��B
			AddStringParam(GetDllShareData().searchKeywords.grepFolders[0]);	// lParam��ǉ��B

			LPARAM lFlag = 0x00;
			lFlag |= GetDllShareData().common.search.bGrepSubFolder				? 0x01 : 0x00;
			// ���̕ҏW���̃e�L�X�g���猟������(0x02.������)
			lFlag |= pDlgGrep->searchOption.bLoHiCase		? 0x04 : 0x00;
			lFlag |= pDlgGrep->searchOption.bRegularExp	? 0x08 : 0x00;
			lFlag |= (GetDllShareData().common.search.nGrepCharSet == CODE_AUTODETECT) ? 0x10 : 0x00;	// 2002/09/21 Moca ���ʌ݊����̂��߂̏���
			lFlag |= GetDllShareData().common.search.nGrepOutputLineType == 1	? 0x20 : 0x00;
			lFlag |= GetDllShareData().common.search.nGrepOutputLineType == 2	? 0x400000 : 0x00;	// 2014.09.23 �ۃq�b�g�s
			lFlag |= (GetDllShareData().common.search.nGrepOutputStyle == 2)		? 0x40 : 0x00;	// CShareData�ɓ���Ȃ��Ă����́H
			lFlag |= (GetDllShareData().common.search.nGrepOutputStyle == 3)		? 0x80 : 0x00;
			EncodingType code = GetDllShareData().common.search.nGrepCharSet;
			if (IsValidCodeType(code) || code == CODE_AUTODETECT) {
				lFlag |= code << 8;
			}
			lFlag |= pDlgGrep->searchOption.bWordOnly								? 0x10000 : 0x00;
			lFlag |= GetDllShareData().common.search.bGrepOutputFileOnly			? 0x20000 : 0x00;
			lFlag |= GetDllShareData().common.search.bGrepOutputBaseFolder		? 0x40000 : 0x00;
			lFlag |= GetDllShareData().common.search.bGrepSeparateFolder			? 0x80000 : 0x00;
			if (nFuncID == F_GREP_REPLACE) {
				lFlag |= pDlgGrepRep->bPaste											? 0x100000 : 0x00;
				lFlag |= GetDllShareData().common.search.bGrepBackup				? 0x200000 : 0x00;
			}
			AddIntParam(lFlag);
			AddIntParam(code);
		}
		break;
	// ���l�p�����[�^��ǉ�
	case F_WCHAR:
	case F_CTRL_CODE:
		AddIntParam(lParam); // �������R�[�h���n�����
		break;
	case F_CHGMOD_EOL:
		{
			// EOL�^�C�v�l���}�N�������l�ɕϊ�����	// 2009.08.18 ryoji
			int nFlag;
			switch ((EolType)lParam) {
			case EolType::CRLF:	nFlag = 1; break;
//			case EOL_LFCR:	nFlag = 2; break;
			case EolType::LF:	nFlag = 3; break;
			case EolType::CR:	nFlag = 4; break;
			case EolType::NEL:	nFlag = 5; break;
			case EolType::LS:	nFlag = 6; break;
			case EolType::PS:	nFlag = 7; break;
			default:		nFlag = 0; break;
			}
			AddIntParam(nFlag);
		}
		break;
	case F_SETFONTSIZE:
		{
			AddIntParam(lParam);
			AddIntParam(lParams[1]);
			AddIntParam(lParams[2]);
		}
		break;
	// 2014.01.15 PageUp/Down�n�ǉ�
	case F_HalfPageUp:
	case F_HalfPageUp_Sel:
	case F_HalfPageDown:
	case F_HalfPageDown_Sel:
		if (lParam == 0) {
			AddIntParam(editView.GetTextArea().nViewRowNum / 2);
		}else {
			AddIntParam( lParam );
		}
		break;
	case F_1PageUp:
	case F_1PageUp_Sel:
	case F_1PageDown:
	case F_1PageDown_Sel:
		if (lParam == 0) {
			AddIntParam(editView.GetTextArea().nViewRowNum - 1);
		}else {
			AddIntParam( lParam );
		}
		break;

	// �W�����p�����[�^��ǉ�
	default:
		AddIntParam(lParam);
		break;
	}
}

void MacroParam::SetStringParam( const WCHAR* szParam, int nLength )
{
	Clear();
	size_t nLen;
	if (nLength == -1) {
		nLen = auto_strlen( szParam );
	}else {
		nLen = nLength;
	}
	pData = new WCHAR[nLen + 1];
	auto_memcpy( pData, szParam, nLen );
	pData[nLen] = LTEXT('\0');
	nDataLen = nLen;
	type = MacroParamType::Str;
}

void MacroParam::SetIntParam(const int nParam)
{
	Clear();
	pData = new WCHAR[16];	//	���l�i�[�i�ő�16���j�p
	_itow(nParam, pData, 10);
	nDataLen = auto_strlen(pData);
	type = MacroParamType::Int;
}

// �����ɕ������ǉ��B
void Macro::AddStringParam(const WCHAR* szParam, int nLength)
{
	MacroParam* param = new MacroParam();

	param->SetStringParam( szParam, nLength );

	// ���X�g�̐�������ۂ�
	if (pParamTop) {
		pParamBot->pNext = param; 
		pParamBot = param;
	}else {
		pParamTop = param;
		pParamBot = pParamTop;
	}
}

// �����ɐ��l��ǉ�
void Macro::AddIntParam(const int nParam)
{
	MacroParam* param = new MacroParam();

	param->SetIntParam( nParam );

	// ���X�g�̐�������ۂ�
	if (pParamTop) {
		pParamBot->pNext = param; 
		pParamBot = param;
	}else {
		pParamTop = param;
		pParamBot = pParamTop;
	}
}

/**	�R�}���h�����s����ieditView.GetCommander().HandleCommand�𔭍s����j
	nFuncID�ɂ���āA�����̌^�𐳊m�ɓn���Ă����܂��傤�B
	
	@note
	paramArr�͉����̃|�C���^�i�A�h���X�j��LONG�ł���킵���l�ɂȂ�܂��B
	������char*�̂Ƃ��́AparamArr[i]�����̂܂�HandleCommand�ɓn���Ă��܂��܂���B
	������int�̂Ƃ��́A*((int*)paramArr[i])�Ƃ��ēn���܂��傤�B
	
	���Ƃ��΁AF_INSTEXT_W��1�߁A2�߂̈����͕�����A3�߂̈�����int�A4�߂̈����������B�������肷��ꍇ�́A���̂悤�ɂ��܂��傤�B
	editView.GetCommander().HandleCommand(nFuncID, true, paramArr[0], paramArr[1], *((int*)paramArr[2]), 0);
	
	@date 2007.07.20 genta : flags�ǉ��DFA_FROMMACRO��flags�Ɋ܂߂ēn�����̂Ƃ���D
		(1�R�}���h���s���ɖ��񉉎Z����K�v�͂Ȃ��̂�)
*/
bool Macro::Exec(EditView& editView, int flags) const
{
	const int maxArg = 8;
	const WCHAR* paramArr[maxArg] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	int paramLenArr[maxArg] = {0, 0, 0, 0, 0, 0, 0, 0};

	MacroParam* p = pParamTop;
	int i = 0;
	for (i=0; i<maxArg; ++i) {
		if (!p) break;	// p���������break;
		paramArr[i] = p->pData;
		paramLenArr[i] = wcslen(paramArr[i]);
		p = p->pNext;
	}
	return Macro::HandleCommand(editView, (EFunctionCode)(nFuncID | flags), paramArr, paramLenArr, i);
}

WCHAR* Macro::GetParamAt(MacroParam* p, int index)
{
	MacroParam* x = p;
	int i = 0;
	while (i < index) {
		if (!x) {
			return NULL;
		}
		x = x->pNext;
		++i;
	}
	if (!x) {
		return NULL;
	}
	return x->pData;
}

int Macro::GetParamCount() const
{
	MacroParam* p = pParamTop;
	int n = 0;
	while (p) {
		++n;
		p = p->pNext;
	}
	return n;
}

static inline int wtoi_def(
	const WCHAR* arg,
	int def_val
	)
{
	return (!arg ? def_val: _wtoi(arg));
}

static inline const WCHAR* wtow_def(
	const WCHAR* arg,
	const WCHAR* def_val
	)
{
	return (!arg ? def_val: arg);
}

/*	Macro���Č����邽�߂̏���hFile�ɏ����o���܂��B

	InsText("�Ȃ�Ƃ�");
	�̂悤�ɁB
	AddLParam�ȊO��CKeyMacroMgr::LoadKeyMacro�ɂ���Ă�Macro���쐬�����_�ɒ���
*/
void Macro::Save(HINSTANCE hInstance, TextOutputStream& out) const
{
	WCHAR			szFuncName[1024];
	WCHAR			szFuncNameJapanese[500];
	size_t			nTextLen;
	const WCHAR*	pText;
	NativeW			memWork;

	// 2002.2.2 YAZAKI SMacroMgr�ɗ���
	if (SMacroMgr::GetFuncInfoByID( hInstance, nFuncID, szFuncName, szFuncNameJapanese)){
		// 2014.01.24 Moca �}�N�������o����type��ǉ����ē���
		out.WriteF( L"S_%ls(", szFuncName );
		MacroParam* pParam = pParamTop;
		while (pParam) {
			if (pParam != pParamTop) {
				out.WriteString( L", " );
			}
			switch (pParam->type) {
			case MacroParamType::Int:
				out.WriteString( pParam->pData );
				break;
			case MacroParamType::Str:
				pText = pParam->pData;
				nTextLen = pParam->nDataLen;
				memWork.SetString( pText, nTextLen );
				memWork.Replace( L"\\", L"\\\\" );
				memWork.Replace( L"\'", L"\\\'" );
				memWork.Replace( L"\r", L"\\\r" );
				memWork.Replace( L"\n", L"\\\n" );
				memWork.Replace( L"\t", L"\\\t" );
				memWork.Replace( L"\0", 1, L"\\u0000", 6 );
				const wchar_t u0085[] = {0x85, 0};
				memWork.Replace( u0085, L"\\u0085" );
				memWork.Replace( L"\u2028", L"\\u2028" );
				memWork.Replace( L"\u2029", L"\\u2029" );
				for (int c=0; c<0x20; ++c) {
					size_t nLen = memWork.GetStringLength();
					const wchar_t* p = memWork.GetStringPtr();
					for (size_t i=0; i<nLen; ++i) {
						if (p[i] == c) {
							wchar_t from[2];
							wchar_t to[7];
							from[0] = c;
							from[1] = L'\0';
							auto_sprintf( to, L"\\u%4x", c );
							memWork.Replace( from, to );
							break;
						}
					}
				}
				const wchar_t u007f[] = {0x7f, 0};
				memWork.Replace( u007f, L"\\u007f" );
				out.WriteString( L"'" );
				out.WriteString( memWork.GetStringPtr(), memWork.GetStringLength() );
				out.WriteString( L"'" );
				break;
			}
			pParam = pParam->pNext;
		}
		out.WriteF( L");\t// %ls\r\n", szFuncNameJapanese );
		return;
	}
	out.WriteF( LSW(STR_ERR_DLGMACRO01) );
}

/**	�}�N�������ϊ�

	Macro�R�}���h��editView.GetCommander().HandleCommand�Ɉ����n���D
	�������Ȃ��}�N���������C�}�N����HandleCommand�ł̑Ή��������Œ�`����K�v������D

	@param pEditView	[in]	����Ώ�EditView
	@param index	[in] ����16bit: �@�\ID, ��ʃ��[�h�͂��̂܂�Macro::HandleCommand()�ɓn���D
	@param arguments [in] ����
	@param argSize	[in] �����̐�
	
	@date 2007.07.08 genta index�̃R�}���h�ԍ������ʃ��[�h�ɐ���
*/
bool Macro::HandleCommand(
	EditView&			editView,
	const EFunctionCode	index,
	const WCHAR*		arguments[],
	const int			argLengths[],
	const int			argSize
	)
{
	std::tstring EXEC_ERROR_TITLE_string = LS(STR_ERR_DLGMACRO02);
	const TCHAR* EXEC_ERROR_TITLE = EXEC_ERROR_TITLE_string.c_str();
	int nOptions = 0;

	switch (LOWORD(index)) {
	case F_WCHAR:		// �������́B���l�͕����R�[�h
	case F_IME_CHAR:	// ���{�����
	case F_CTRL_CODE:
		// Jun. 16, 2002 genta
		if (!arguments[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO03)
			);
			return false;
		}
	case F_PASTE:	// 2011.06.26 Moca
	case F_PASTEBOX:	// 2011.06.26 Moca
	case F_TEXTWRAPMETHOD:	// �e�L�X�g�̐܂�Ԃ����@�̎w��B���l�́A0x0�i�܂�Ԃ��Ȃ��j�A0x1�i�w�茅�Ő܂�Ԃ��j�A0x2�i�E�[�Ő܂�Ԃ��j	// 2008.05.30 nasukoji
	case F_GOLINETOP:	// �s���Ɉړ��B���l�́A0x0�i�f�t�H���g�j�A0x1�i�󔒂𖳎����Đ擪�Ɉړ��j�A0x2�i����`�j�A0x4�i�I�����Ĉړ��j�A0x8�i���s�P�ʂŐ擪�Ɉړ��j
	case F_GOLINETOP_SEL:
	case F_GOLINEEND:	// �s���Ɉړ�
	case F_GOLINEEND_SEL:
	case F_SELECT_COUNT_MODE:	// �����J�E���g�̕��@���w��B���l�́A0x0�i�ύX�����擾�̂݁j�A0x1�i�������j�A0x2�i�o�C�g���j�A0x3�i�������̃o�C�g���g�O���j	// 2009.07.06 syat
	case F_OUTLINE:	// �A�E�g���C����͂̃A�N�V�������w��B���l�́A0x0�i��ʕ\���j�A0x1�i��ʕ\�����ĉ�́j�A0x2�i��ʕ\���g�O���j
	case F_CHANGETYPE:
	case F_TOGGLE_KEY_SEARCH:
		// ��ڂ̈��������l�B
	case F_WHEELUP:
	case F_WHEELDOWN:
	case F_WHEELLEFT:
	case F_WHEELRIGHT:
	case F_WHEELPAGEUP:
	case F_WHEELPAGEDOWN:
	case F_WHEELPAGELEFT:
	case F_WHEELPAGERIGHT:
	case F_HalfPageUp:
	case F_HalfPageDown:
	case F_HalfPageUp_Sel:
	case F_HalfPageDown_Sel:
	case F_1PageUp:
	case F_1PageDown:
	case F_1PageUp_Sel:
	case F_1PageDown_Sel:
		editView.GetCommander().HandleCommand(
			index,
			true,
			(arguments[0] ? _wtoi(arguments[0]) : 0),
			0,
			0,
			0
		);
		break;
	case F_UP_BOX:
	case F_DOWN_BOX:
	case F_LEFT_BOX:
	case F_RIGHT_BOX:
	case F_UP2_BOX:
	case F_DOWN2_BOX:
	case F_WORDLEFT_BOX:
	case F_WORDRIGHT_BOX:
	case F_GOFILETOP_BOX:
	case F_GOFILEEND_BOX:
		nOptions = 1;
	case F_GOLOGICALLINETOP_BOX:
	case F_GOLINETOP_BOX:
	case F_GOLINEEND_BOX:
	case F_HalfPageUp_BOX:
	case F_HalfPageDown_BOX:
	case F_1PageUp_BOX:
	case F_1PageDown_BOX:
		{
			// 0: ���ʐݒ�
			// 1: true(�}�N���̃f�t�H���g�l)
			// 2: false
			// �}�N���̃f�t�H���g�l��true(1)�����AEditView���̃f�t�H���g�͋��ʐݒ�(0)
			int nBoxLock = wtoi_def(arguments[nOptions == 1 ? 0 : 1], 1);
			if (nOptions == 1) {
				editView.GetCommander().HandleCommand(
					index, true, nBoxLock, 0, 0, 0);
			}else {
				editView.GetCommander().HandleCommand(
					index, true, wtoi_def(arguments[0], 0), nBoxLock, 0, 0);
			}
		}
		break;
	case F_CHGMOD_EOL:	// ���͉��s�R�[�h�w��BEolType�̐��l���w��B2003.06.23 Moca
		// Jun. 16, 2002 genta
		if (!arguments[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO03_1)
			);
			return false;
		}
		{
			// �}�N�������l��EOL�^�C�v�l�ɕϊ�����	// 2009.08.18 ryoji
			EolType nEol;
			switch (arguments[0] ? _wtoi(arguments[0]) : 0) {
			case 1:		nEol = EolType::CRLF; break;
//			case 2:		nEol = EOL_LFCR; break;
			case 3:		nEol = EolType::LF; break;
			case 4:		nEol = EolType::CR; break;
			case 5:		nEol = EolType::NEL; break;
			case 6:		nEol = EolType::LS; break;
			case 7:		nEol = EolType::PS; break;
			default:	nEol = EolType::None; break;
			}
			if (nEol != EolType::None) {
				editView.GetCommander().HandleCommand(index, true, (int)nEol, 0, 0, 0);
			}
		}
		break;
	case F_SET_QUOTESTRING:	// Jan. 29, 2005 genta �ǉ� �e�L�X�g����1�����}�N���͂����ɓ������Ă������D
		{
			if (!arguments[0]) {
				::MYMESSAGEBOX(
					NULL,
					MB_OK | MB_ICONSTOP | MB_TOPMOST,
					EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO04)
				);
				return false;
			} {
				editView.GetCommander().HandleCommand(index, true, (LPARAM)arguments[0], 0, 0, 0);	// �W��
			}
		}
		break;
	case F_INSTEXT_W:		// �e�L�X�g�}��
	case F_ADDTAIL_W:		// ���̑���̓L�[�{�[�h����ł͑��݂��Ȃ��̂ŕۑ����邱�Ƃ��ł��Ȃ��H
	case F_INSBOXTEXT:
		// ��ڂ̈�����������B
		if (!arguments[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO04)
			);
			return false;
		}
		{
			int len = argLengths[0];
			editView.GetCommander().HandleCommand(index, true, (LPARAM)arguments[0], len, 0, 0);	// �W��
		}
		break;
	// ��ځA��ڂƂ������͐��l
	case F_CHG_CHARSET:
		{
			int		nCharSet = (!arguments[0] || arguments[0][0] == '\0') ? CODE_NONE : _wtoi(arguments[0]);
			BOOL	bBOM = (!arguments[1]) ? FALSE : (_wtoi(arguments[1]) != 0);
			editView.GetCommander().HandleCommand(index, true, (LPARAM)nCharSet, (LPARAM)bBOM, 0, 0);
		}
		break;
	case F_JUMP:		// �w��s�փW�����v�i������PL/SQL�R���p�C���G���[�s�ւ̃W�����v�͖��Ή��j
		// arguments[0]�փW�����v�B�I�v�V������arguments[1]�ɁB
		//		******** �ȉ��u�s�ԍ��̒P�ʁv ********
		//		0x00	�܂�Ԃ��P�ʂ̍s�ԍ�
		//		0x01	���s�P�ʂ̍s�ԍ�
		//		**************************************
		//		0x02	PL/SQL�R���p�C���G���[�s����������
		//		����`	�e�L�X�g�́��s�ڂ��u���b�N��1�s�ڂƂ���
		//		����`	���o���ꂽPL/SQL�p�b�P�[�W�̃u���b�N����I��
		if (!arguments[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO05)
			);
			return false;
		}
		{
			editView.editWnd.dlgJump.nLineNum = _wtoi(arguments[0]);	// �W�����v��
			LPARAM lFlag = arguments[1] ? _wtoi(arguments[1]) : 1; // �f�t�H���g1
			GetDllShareData().bLineNumIsCRLF_ForJump = ((lFlag & 0x01) != 0);
			editView.editWnd.dlgJump.bPLSQL = lFlag & 0x02 ? 1 : 0;
			editView.GetCommander().HandleCommand(index, true, 0, 0, 0, 0);	// �W��
		}
		break;
	// ��ڂ̈����͕�����A��ڂ̈����͐��l
	case F_BOOKMARK_PATTERN:	// 2002.02.08 hor
		if (!arguments[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO06)
			);
			return false;
		}
		// NO BREAK
	case F_SEARCH_NEXT:
	case F_SEARCH_PREV:
		//	arguments[0] �������B(�ȗ����A���̌���������E�I�v�V�������g��)
		//	arguments[1]:�I�v�V���� (�ȗ����A0�݂̂Ȃ�)
		//		0x01	�P��P�ʂŒT��
		//		0x02	�p�啶���Ə���������ʂ���
		//		0x04	���K�\��
		//		0x08	������Ȃ��Ƃ��Ƀ��b�Z�[�W��\��
		//		0x10	�����_�C�A���O�������I�ɕ���
		//		0x20	�擪�i�����j����Č�������
		//		0x800	(�}�N����p)�����L�[�𗚗��ɓo�^���Ȃ�
		//		0x1000	(�}�N����p)�����I�v�V���������ɖ߂�
		{
			LPARAM lFlag = arguments[1] ? _wtoi(arguments[1]) : 0;
			SearchOption searchOption;
			searchOption.bWordOnly			= ((lFlag & 0x01) != 0);
			searchOption.bLoHiCase			= ((lFlag & 0x02) != 0);
			searchOption.bRegularExp		= ((lFlag & 0x04) != 0);
			bool bAddHistory = ((lFlag & 0x800) == 0);
			bool bBackupFlag = ((lFlag & 0x1000) != 0);
			CommonSetting_Search backupFlags;
			SearchOption backupLocalFlags;
			std::wstring backupStr;
			bool backupKeyMark;
			int nBackupSearchKeySequence;
			if (bBackupFlag) {
				backupFlags = GetDllShareData().common.search;
				backupLocalFlags = editView.curSearchOption;
				backupStr = editView.strCurSearchKey;
				backupKeyMark = editView.bCurSrchKeyMark;
				nBackupSearchKeySequence = editView.nCurSearchKeySequence;
				bAddHistory = false;
			}
			const WCHAR* pszSearchKey = wtow_def(arguments[0], L"");
			size_t nLen = wcslen( pszSearchKey );
			if (0 < nLen) {
				// ���K�\��
				if (lFlag & 0x04
					&& !CheckRegexpSyntax(arguments[0], NULL, true)
				) {
					break;
				}

				// ����������
				if (nLen < _MAX_PATH && bAddHistory) {
					SearchKeywordManager().AddToSearchKeys(arguments[0]);
					GetDllShareData().common.search.searchOption = searchOption;
				}
				editView.strCurSearchKey = arguments[0];
				editView.curSearchOption = searchOption;
				editView.bCurSearchUpdate = true;
				editView.nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;
			}
			// �ݒ�l�o�b�N�A�b�v
			// �}�N���p�����[�^���ݒ�l�ϊ�
			GetDllShareData().common.search.bNotifyNotFound		= (lFlag & 0x08) ? 1 : 0;
			GetDllShareData().common.search.bAutoCloseDlgFind	= (lFlag & 0x10) ? 1 : 0;
			GetDllShareData().common.search.bSearchAll			= (lFlag & 0x20) ? 1 : 0;

			// �R�}���h���s
			editView.GetCommander().HandleCommand(index, true, 0, 0, 0, 0);
			if (bBackupFlag) {
				GetDllShareData().common.search = backupFlags;
				editView.curSearchOption = backupLocalFlags;
				editView.strCurSearchKey = backupStr;
				editView.bCurSearchUpdate = true;
				editView.nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;
				editView.ChangeCurRegexp( backupKeyMark );
				editView.bCurSrchKeyMark = backupKeyMark;
				if (!backupKeyMark) {
					editView.Redraw();
				}
				editView.nCurSearchKeySequence = nBackupSearchKeySequence;
			}
		}
		break;
	case F_DIFF:
		// arguments[0]��Diff�����\���B�I�v�V������arguments[1]�ɁB
		// arguments[1]:
		//		���̐��l�̘a�B
		//		0x0001 -i ignore-case         �啶�����������ꎋ
		//		0x0002 -w ignore-all-space    �󔒖���
		//		0x0004 -b ignore-space-change �󔒕ύX����
		//		0x0008 -B ignore-blank-lines  ��s����
		//		0x0010 -t expand-tabs         TAB-SPACE�ϊ�
		//		0x0020    (�ҏW���̃t�@�C�������t�@�C��)
		//		0x0040    (DIFF�������Ȃ��Ƃ��Ƀ��b�Z�[�W�\��)
		// NO BREAK

	case F_EXECMD:
		// arguments[0]�����s�B�I�v�V������arguments[1]�ɁB
		// arguments[1]:
		//		���̐��l�̘a�B
		//		0x01	�W���o�͂𓾂�
		//		0x02	�W���o�͂��L�����b�g�ʒu��	// 2007.01.02 maru �����̊g��
		//		0x04	�ҏW���t�@�C����W�����͂�	// 2007.01.02 maru �����̊g��
		// arguments[2]:�J�����g�f�B���N�g��
		if (!arguments[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO07)
			);
			return false;
		}
		{
			int nOpt = wtoi_def(arguments[1], 0);
			const wchar_t* pDir = wtow_def(arguments[2], NULL);
			editView.GetCommander().HandleCommand(index, true, (LPARAM)arguments[0], nOpt, (LPARAM)pDir, 0);
		}
		break;

	case F_TRACEOUT:		// 2006.05.01 �}�N���p�A�E�g�v�b�g�E�B���h�E�ɏo��
		// arguments[0]���o�́B�I�v�V������arguments[1]�ɁB
		// arguments[1]:
		//		���̐��l�̘a�B
		//		0x01	ExpandParameter�ɂ�镶����W�J���s��
		//		0x02	�e�L�X�g�����ɉ��s�R�[�h��t�����Ȃ�
		if (!arguments[0]) {
			::MYMESSAGEBOX(
				NULL,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO07)
			);
			return false;
		}
		{
			editView.GetCommander().HandleCommand(index, true, (LPARAM)arguments[0], argLengths[0], (LPARAM)(arguments[1] ? _wtoi(arguments[1]) : 0), 0);
		}
		break;

	// �͂��߂̈����͕�����B�Q�ڂƂR�ڂ͐��l
	case F_PUTFILE:		// 2006.12.10 ��ƒ��t�@�C���̈ꎞ�o��
		// arguments[0]�ɏo�́Barguments[1]�ɕ����R�[�h�B�I�v�V������arguments[2]�ɁB
		// arguments[2]:
		//		���̒l�̘a
		//		0x01	�I��͈͂��o�́i��I����ԂȂ��t�@�C���𐶐��j
		// no break

	case F_INSFILE:		// 2006.12.10 �L�����b�g�ʒu�Ƀt�@�C���}��
		// arguments[0]�ɏo�́Barguments[1]�ɕ����R�[�h�B�I�v�V������arguments[2]�ɁB
		// arguments[2]:
		//		���݂͓��ɂȂ�
		if (!arguments[0]) {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO08));
			return false;
		}
		{
			editView.GetCommander().HandleCommand(
				index,
				true,
				(LPARAM)arguments[0], 
				(LPARAM)(arguments[1] ? _wtoi(arguments[1]) : 0),
				(LPARAM)(arguments[2] ? _wtoi(arguments[2]) : 0),
				0
			);
		}
		break;

	// �͂��߂�2�̈����͕�����B3�ڂ͐��l
	case F_REPLACE:
	case F_REPLACE_ALL:
		// arguments[0]���Aarguments[1]�ɒu���B�I�v�V������arguments[2]�Ɂi�����\��j
		// arguments[2]:
		//		���̐��l�̘a�B
		//		0x001	�P��P�ʂŒT��
		//		0x002	�p�啶���Ə���������ʂ���
		//		0x004	���K�\��
		//		0x008	������Ȃ��Ƃ��Ƀ��b�Z�[�W��\��
		//		0x010	�����_�C�A���O�������I�ɕ���
		//		0x020	�擪�i�����j����Č�������
		//		0x040	�N���b�v�{�[�h����\��t����
		//		******** �ȉ��u�u���͈́v ********
		//		0x000	�t�@�C���S��
		//		0x080	�I��͈�
		//		**********************************
		//		******** �ȉ��u�u���Ώہv ********
		//		0x000	��������������ƒu��
		//		0x100	��������������̑O�ɑ}��
		//		0x200	��������������̌�ɒǉ�
		//		**********************************
		//		0x400	�u���ׂĒu���v�͒u���̌J�Ԃ��iON:�A���u��, OFF:�ꊇ�u���j
		//		0x800	(�}�N����p)�����L�[�𗚗��ɓo�^���Ȃ�
		//		0x1000	(�}�N����p)�����I�v�V���������ɖ߂�
		if (arguments[0] == NULL || arguments[0][0] == L'\0') {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO09));
			return false;
		}
		if (!arguments[1]) {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO10));
			return false;
		}
		{
			DlgReplace& dlgReplace = editView.editWnd.dlgReplace;
			LPARAM lFlag = arguments[2] ? _wtoi(arguments[2]) : 0;
			SearchOption searchOption;
			searchOption.bWordOnly			= ((lFlag & 0x01) != 0);
			searchOption.bLoHiCase			= ((lFlag & 0x02) != 0);
			searchOption.bRegularExp		= ((lFlag & 0x04) != 0);
			bool bAddHistory = ((lFlag & 0x800) == 0);
			bool bBackupFlag = ((lFlag & 0x1000) != 0);
			CommonSetting_Search backupFlags;
			SearchOption backupLocalFlags;
			std::wstring backupStr;
			std::wstring backupStrRep;
			int nBackupSearchKeySequence;
			bool backupKeyMark;
			if (bBackupFlag) {
				backupFlags = GetDllShareData().common.search;
				backupLocalFlags = editView.curSearchOption;
				backupStr = editView.strCurSearchKey;
				backupStrRep = dlgReplace.strText2;
				backupKeyMark = editView.bCurSrchKeyMark;
				nBackupSearchKeySequence = editView.nCurSearchKeySequence;
				bAddHistory = false;
			}
			// ���K�\��
			if (lFlag & 0x04
				&& !CheckRegexpSyntax(arguments[0], NULL, true)
			) {
				break;
			}

			// ����������
			if (wcslen(arguments[0]) < _MAX_PATH && bAddHistory) {
				SearchKeywordManager().AddToSearchKeys(arguments[0]);
				GetDllShareData().common.search.searchOption = searchOption;
			}
			editView.strCurSearchKey = arguments[0];
			editView.curSearchOption = searchOption;
			editView.bCurSearchUpdate = true;
			editView.nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;

			// �u���㕶����
			if (wcslen(arguments[1]) < _MAX_PATH && bAddHistory) {
				SearchKeywordManager().AddToReplaceKeys(arguments[1]);
			}
			dlgReplace.strText2 = arguments[1];

			GetDllShareData().common.search.bNotifyNotFound		= (lFlag & 0x08) ? 1 : 0;
			GetDllShareData().common.search.bAutoCloseDlgFind	= (lFlag & 0x10) ? 1 : 0;
			GetDllShareData().common.search.bSearchAll			= (lFlag & 0x20) ? 1 : 0;
			dlgReplace.bPaste			= (lFlag & 0x40) ? 1 : 0;	// CShareData�ɓ���Ȃ��Ă����́H
			dlgReplace.bConsecutiveAll	= (lFlag & 0x0400) ? 1 : 0;	// 2007.01.16 ryoji
			if (LOWORD(index) == F_REPLACE) {	// 2007.07.08 genta �R�}���h�͉��ʃ��[�h
				// �u�����鎞�͑I�ׂȂ�
				dlgReplace.bSelectedArea = 0;
			}else if (LOWORD(index) == F_REPLACE_ALL) {	// 2007.07.08 genta �R�}���h�͉��ʃ��[�h
				// �S�u���̎��͑I�ׂ�H
				dlgReplace.bSelectedArea	= (lFlag & 0x80) ? 1 : 0;
			}
			dlgReplace.nReplaceTarget	= (lFlag >> 8) & 0x03;	// 8bit�V�t�g�i0x100�Ŋ���Z�j	// 2007.01.16 ryoji ���� 2bit�������o��
			if (bAddHistory) {
				GetDllShareData().common.search.bConsecutiveAll = dlgReplace.bConsecutiveAll;
				GetDllShareData().common.search.bSelectedArea = dlgReplace.bSelectedArea;
			}
			// �R�}���h���s
			editView.GetCommander().HandleCommand(index, true, 0, 0, 0, 0);
			if (bBackupFlag) {
				GetDllShareData().common.search = backupFlags;
				editView.curSearchOption = backupLocalFlags;
				editView.strCurSearchKey = backupStr;
				editView.bCurSearchUpdate = true;
				dlgReplace.strText2 = backupStrRep;
				editView.nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;
				editView.ChangeCurRegexp( backupKeyMark );
				editView.bCurSrchKeyMark = backupKeyMark;
				if (!backupKeyMark) {
					editView.Redraw();
				}
				editView.nCurSearchKeySequence = nBackupSearchKeySequence;
			}
		}
		break;
	case F_GREP_REPLACE:
	case F_GREP:
		// arguments[0]	����������
		// arguments[1]	�����Ώۂɂ���t�@�C����
		// arguments[2]	�����Ώۂɂ���t�H���_��
		// arguments[3]:
		//		���̐��l�̘a�B
		//		0x01	�T�u�t�H���_�������������
		//		0x02	���̕ҏW���̃e�L�X�g���猟������i�������j
		//		0x04	�p�啶���Ɖp����������ʂ���
		//		0x08	���K�\��
		//		0x10	�����R�[�h��������
		//		******** �ȉ��u���ʏo�́v ********
		//		0x00	�Y���s
		//		0x20	�Y������
		//		0x400000	�ۃq�b�g�s	// 2014.09.23
		//		0x400020	(���g�p)	// 2014.09.23
		//		**********************************
		//		******** �ȉ��u�o�͌`���v ********
		//		0x00	�m�[�}��
		//		0x40	�t�@�C����
		//		0x80	���ʂ̂� // 2011.11.24
		//		0xC0	(���g�p) // 2011.11.24
		//		**********************************
		//		0x0100 �` 0xff00	�����R�[�h�Z�b�g�ԍ� * 0x100
		//		0x010000	�P��P�ʂŒT��
		//		0x020000	�t�@�C�����ŏ��̂݌���
		//		0x040000	�x�[�X�t�H���_�\��
		//		0x080000	�t�H���_���ɕ\��
		{
			if (arguments[0] == NULL) {
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO11));
				return false;
			}
			int ArgIndex = 0;
			bool bGrepReplace = LOWORD(index) == F_GREP_REPLACE;
			if (bGrepReplace) {
				if (argLengths[0] == 0) {
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
						LS(STR_ERR_DLGMACRO11));
					return false;
				}
				ArgIndex = 1;
				if (arguments[1] == NULL) {
					::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
						LS(STR_ERR_DLGMACRO17));
					return false;
				}
			}
			if (arguments[ArgIndex+1] == NULL) {
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO12));
				return false;
			}
			if (arguments[ArgIndex+2] == NULL) {
				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO13));
				return false;
			}
			//	��ɊO���E�B���h�E�ɁB
			// ======= Grep�̎��s =============
			// Grep���ʃE�B���h�E�̕\��
			NativeW mWork1;	mWork1.SetString( arguments[0] );	mWork1.Replace( L"\"", L"\"\"" );	//	����������
			NativeW mWork4;
			if (bGrepReplace) {
				mWork4.SetString( arguments[1] );	mWork4.Replace( L"\"", L"\"\"" );	//	�u����
			}
			NativeT mWork2;	mWork2.SetStringW( arguments[ArgIndex+1] );	mWork2.Replace( _T("\""), _T("\"\"") );	//	�t�@�C����
			NativeT mWork3;	mWork3.SetStringW( arguments[ArgIndex+2] );	mWork3.Replace( _T("\""), _T("\"\"") );	//	�t�H���_��

			LPARAM lFlag = wtoi_def(arguments[ArgIndex+3], 5);

			// 2002/09/21 Moca �����R�[�h�Z�b�g
			EncodingType	nCharSet;
			{
				nCharSet = CODE_SJIS;
				if (lFlag & 0x10) {	// �����R�[�h��������(���ʌ݊��p)
					nCharSet = CODE_AUTODETECT;
				}
				int nCode = (lFlag >> 8) & 0xff; // ������ 7-15 �r�b�g��(0�J�n)���g��
				if (IsValidCodeTypeExceptSJIS(nCode) || nCode == CODE_AUTODETECT) {
					nCharSet = (EncodingType)nCode;
				}
				// 2013.06.11 5�Ԗڂ̈������𕶎��R�[�h�ɂ���
				if (ArgIndex + 5 <= argSize) {
					nCharSet = (EncodingType)_wtoi(arguments[ArgIndex + 4]);
				}
			}

			// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
			NativeT cmdLine;
			TCHAR	szTemp[20];
			TCHAR	pOpt[64];
			cmdLine.AppendStringLiteral(_T("-GREPMODE -GKEY=\""));
			cmdLine.AppendStringW(mWork1.GetStringPtr());
			if (bGrepReplace) {
				cmdLine.AppendStringLiteral(_T("\" -GREPR=\""));
				cmdLine.AppendStringW(mWork4.GetStringPtr());
			}
			cmdLine.AppendStringLiteral(_T("\" -GFILE=\""));
			cmdLine.AppendString(mWork2.GetStringPtr());
			cmdLine.AppendStringLiteral(_T("\" -GFOLDER=\""));
			cmdLine.AppendString(mWork3.GetStringPtr());
			cmdLine.AppendStringLiteral(_T("\" -GCODE="));
			auto_sprintf( szTemp, _T("%d"), nCharSet );
			cmdLine.AppendString(szTemp);

			// GOPT�I�v�V����
			pOpt[0] = '\0';
			if (lFlag & 0x01) _tcscat( pOpt, _T("S") );	// �T�u�t�H���_�������������
			if (lFlag & 0x04) _tcscat( pOpt, _T("L") );	// �p�啶���Ɖp����������ʂ���
			if (lFlag & 0x08) _tcscat( pOpt, _T("R") );	// ���K�\��
			if ((lFlag & 0x400020) == 0x20) _tcscat( pOpt, _T("P") );			// �s���o�͂���
			else if ((lFlag & 0x400020) == 0x400000) _tcscat( pOpt, _T("N") );	// �ۃq�b�g�s���o�͂���
			if ((lFlag & 0xC0) == 0x40) _tcscat( pOpt, _T("2") );				// Grep: �o�͌`��
			else if ((lFlag & 0xC0) == 0x80) _tcscat( pOpt, _T("3") );
			else _tcscat( pOpt, _T("1") );
			if (lFlag & 0x10000) _tcscat( pOpt, _T("W") );
			if (lFlag & 0x20000) _tcscat( pOpt, _T("F") );
			if (lFlag & 0x40000) _tcscat( pOpt, _T("B") );
			if (lFlag & 0x80000) _tcscat( pOpt, _T("D") );
			if (bGrepReplace) {
				if (lFlag & 0x100000) _tcscat( pOpt, _T("C") );
				if (lFlag & 0x200000) _tcscat( pOpt, _T("O") );
			}
			if (pOpt[0] != _T('\0')) {
				auto_sprintf( szTemp, _T(" -GOPT=%ts"), pOpt );
				cmdLine.AppendString(szTemp);
			}

			// �V�K�ҏW�E�B���h�E�̒ǉ� ver 0
			LoadInfo loadInfo;
			loadInfo.filePath = _T("");
			loadInfo.eCharCode = CODE_NONE;
			loadInfo.bViewMode = false;
			ControlTray::OpenNewEditor(
				G_AppInstance(),
				editView.GetHwnd(),
				loadInfo,
				cmdLine.GetStringPtr()
			);
			// ======= Grep�̎��s =============
			// Grep���ʃE�B���h�E�̕\��
		}
		break;
	case F_FILEOPEN2:
		// arguments[0]���J���B
		if (!arguments[0]) {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO14));
			return false;
		}
		{
			int  nCharCode = wtoi_def(arguments[1], CODE_AUTODETECT);
			BOOL nViewMode = wtoi_def(arguments[2], FALSE);
			const WCHAR* pDefFileName = wtow_def(arguments[3], L"");
			editView.GetCommander().HandleCommand(index, true, (LPARAM)arguments[0], (LPARAM)nCharCode, (LPARAM)nViewMode, (LPARAM)pDefFileName);
		}
		break;
	case F_FILESAVEAS_DIALOG:
	case F_FILESAVEAS:
		// arguments[0]��ʖ��ŕۑ��B
		if (LOWORD(index) == F_FILESAVEAS && (!arguments[0] ||  L'\0' == arguments[0][0])) {
			// F_FILESAVEAS_DIALOG�̏ꍇ�͋󕶎�������e
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO15));
			return false;
		}
		{
			// �����R�[�h�Z�b�g
			// Sep. 11, 2004 genta �����R�[�h�ݒ�͈̔̓`�F�b�N
			EncodingType nCharCode = CODE_NONE;	// �f�t�H���g�l
			if (arguments[1]) {
				nCharCode = (EncodingType)_wtoi(arguments[1]);
			}
			if (LOWORD(index) == F_FILESAVEAS && IsValidCodeOrCPType(nCharCode) && nCharCode != editView.pEditDoc->GetDocumentEncoding()) {
				// From Here Jul. 26, 2003 ryoji BOM��Ԃ�������
				editView.pEditDoc->SetDocumentEncoding(nCharCode, CodeTypeName(editView.pEditDoc->GetDocumentEncoding()).IsBomDefOn());
				// To Here Jul. 26, 2003 ryoji BOM��Ԃ�������
			}

			// ���s�R�[�h
			int nSaveLineCode = 0;	// �f�t�H���g�l	// Sep. 11, 2004 genta �����l���u�ύX���Ȃ��v��
			if (arguments[2]) {
				nSaveLineCode = _wtoi(arguments[2]);
			}
			EolType eEol;
			switch (nSaveLineCode) {
			case 0:		eEol = EolType::None;	break;
			case 1:		eEol = EolType::CRLF;	break;
			case 2:		eEol = EolType::LF;		break;
			case 3:		eEol = EolType::CR;		break;
			default:	eEol = EolType::None;	break;
			}
			
			editView.GetCommander().HandleCommand(index, true, (LPARAM)arguments[0], (LPARAM)nCharCode, (LPARAM)eEol, 0);
		}
		break;
	// 2�̈�����������
	// Jul. 5, 2002 genta
	case F_EXTHTMLHELP:
	case F_EXECEXTMACRO:				// 2009.06.14 syat
		editView.GetCommander().HandleCommand(index, true, (LPARAM)arguments[0], (LPARAM)arguments[1], 0, 0);
		break;
	// From Here Dec. 4, 2002 genta
	case F_FILE_REOPEN				: // �J������
	case F_FILE_REOPEN_SJIS			: // SJIS�ŊJ������
	case F_FILE_REOPEN_JIS			: // JIS�ŊJ������
	case F_FILE_REOPEN_EUC			: // EUC�ŊJ������
	case F_FILE_REOPEN_UNICODE		: // Unicode�ŊJ������
	case F_FILE_REOPEN_UNICODEBE	: // UnicodeBE�ŊJ������
	case F_FILE_REOPEN_UTF8			: // UTF-8�ŊJ������
	case F_FILE_REOPEN_UTF7			: // UTF-7�ŊJ������
		{
			int noconfirm = 0;
			if (arguments[0]) {
				noconfirm = (_wtoi(arguments[0]) != 0);
			}
			editView.GetCommander().HandleCommand(index, true, noconfirm, 0, 0, 0);
		}
		break;
	// To Here Dec. 4, 2002 genta
	case F_TOPMOST:
		{
			int lparam1;
			if (arguments[0]) {
				lparam1 = _wtoi(arguments[0]);
				editView.GetCommander().HandleCommand(index, true, lparam1, 0, 0, 0);
			}
		}
		break;	// Jan. 29, 2005 genta �����Ă���
	case F_TAGJUMP_KEYWORD:	// @@ 2005.03.31 MIK
		{
			// ������NULL�ł�OK
			editView.GetCommander().HandleCommand(index, true, (LPARAM)arguments[0], 0, 0, 0);
		}
		break;
	case F_NEXTWINDOW:
	case F_PREVWINDOW:
		editView.GetDocument().HandleCommand(index);	// 2009.04.11 ryoji F_NEXTWINDOW/F_PREVWINDOW�����삵�Ȃ������̂��C��
		break;
	case F_MESSAGEBOX:	// ���b�Z�[�W�{�b�N�X�̕\��
	case F_ERRORMSG:	// ���b�Z�[�W�{�b�N�X�i�G���[�j�̕\��
	case F_WARNMSG:		// ���b�Z�[�W�{�b�N�X�i�x���j�̕\��
	case F_INFOMSG:		// ���b�Z�[�W�{�b�N�X�i���j�̕\��
	case F_OKCANCELBOX:	// ���b�Z�[�W�{�b�N�X�i�m�F�FOK�^�L�����Z���j�̕\��
	case F_YESNOBOX:	// ���b�Z�[�W�{�b�N�X�i�m�F�F�͂��^�������j�̕\��
		{
			VARIANT vArg[2];			// HandleFunction�ɓn������
			VARIANT vResult;			// HandleFunction����Ԃ�l
			if (!arguments[0]) {
				break;
			}
			SysString s(arguments[0], wcslen(arguments[0]));
			Wrap(&vArg[0])->Receive(s);
			int nArgSize = 1;
			// 2�ڂ̈��������l�B
			if (LOWORD(index) == F_MESSAGEBOX) {
				vArg[1].vt = VT_I4;
				vArg[1].intVal = (arguments[1] ? _wtoi(arguments[1]) : 0);
				nArgSize = 2;
			}
			return HandleFunction(editView, index, vArg, nArgSize, vResult);
		}
	case F_MOVECURSORLAYOUT:
	case F_MOVECURSOR:
		{
			if (arguments[0] && arguments[1] && arguments[2]) {
				int lparam1 = _wtoi(arguments[0]) - 1;
				int lparam2 = _wtoi(arguments[1]) - 1;
				int lparam3 = _wtoi(arguments[2]);
				editView.GetCommander().HandleCommand(index, true, lparam1, lparam2, lparam3, 0);
			}else {
				::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
				LS(STR_ERR_DLGMACRO16));
				return false;
			}
		}
		break;
	case F_CHGTABWIDTH:		//  �^�u�T�C�Y���擾�A�ݒ肷��i�L�[�}�N���ł͎擾�͖��Ӗ��j
	case F_CHGWRAPCOLUMN:	//  �܂�Ԃ������擾�A�ݒ肷��i�L�[�}�N���ł͎擾�͖��Ӗ��j
	case F_MACROSLEEP:
	case F_SETDRAWSWITCH:	//  �ĕ`��X�C�b�`���擾�A�ݒ肷��
		{
			VARIANT vArg[1];			// HandleFunction�ɓn������
			VARIANT vResult;			// HandleFunction����Ԃ�l
			// ��ڂ̈��������l�B
			vArg[0].vt = VT_I4;
			vArg[0].intVal = (arguments[0] ? _wtoi(arguments[0]) : 0);
			return HandleFunction(editView, index, vArg, 1, vResult);
		}
	case F_SETFONTSIZE:
		{
			int val0 = arguments[0] ? _wtoi(arguments[0]) : 0;
			int val1 = arguments[1] ? _wtoi(arguments[1]) : 0;
			int val2 = arguments[2] ? _wtoi(arguments[2]) : 0;
			editView.GetCommander().HandleCommand(index, true, (LPARAM)val0, (LPARAM)val1, (LPARAM)val2, 0);
		}
		break;
	case F_STATUSMSG:
		{
			if (!arguments[0]) {
				::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, EXEC_ERROR_TITLE,
					LS(STR_ERR_DLGMACRO07));
				return false;
			}
			std::tstring val0 = to_tchar(arguments[0]);
			int val1 = arguments[1] ? _wtoi(arguments[1]) : 0;
			if ((val1 & 0x03) == 0) {
				editView.SendStatusMessage(val0.c_str());
			}else if ((val1 & 0x03) == 1) {
				if (editView.editWnd.statusBar.GetStatusHwnd()) {
					editView.SendStatusMessage(val0.c_str());
				}else {
					InfoMessage(editView.GetHwnd(), _T("%ts"), val0.c_str());
				}
			}else if ((val1 & 0x03) == 2) {
				editView.editWnd.statusBar.SendStatusMessage2(val0.c_str());
			}
		}
		break;
	case F_MSGBEEP:
		{
			int val0 = arguments[0] ? _wtoi(arguments[0]) : 0;
			switch (val0) {
			case -1: break;
			case 0: val0 = MB_OK; break;
			case 1: val0 = MB_ICONERROR; break;
			case 2: val0 = MB_ICONQUESTION; break;
			case 3: val0 = MB_ICONWARNING; break;
			case 4: val0 = MB_ICONINFORMATION; break;
			default: val0 = MB_OK; break;
			}
			::MessageBeep(val0);
		}
		break;
	case F_COMMITUNDOBUFFER:
		{
			OpeBlk* opeBlk = editView.commander.GetOpeBlk();
			if (opeBlk) {
				int nCount = opeBlk->GetRefCount();
				opeBlk->SetRefCount(1); // �����I�Ƀ��Z�b�g���邽��1���w��
				editView.SetUndoBuffer();
				if (!editView.commander.GetOpeBlk() && 0 < nCount) {
					editView.commander.SetOpeBlk(new OpeBlk());
					editView.commander.GetOpeBlk()->SetRefCount(nCount);
				}
			}
		}
		break;
	case F_ADDREFUNDOBUFFER:
		{
			OpeBlk* opeBlk = editView.commander.GetOpeBlk();
			if (!opeBlk) {
				editView.commander.SetOpeBlk(new OpeBlk());
			}
			editView.commander.GetOpeBlk()->AddRef();
		}
		break;
	case F_SETUNDOBUFFER:
		{
			editView.SetUndoBuffer();
		}
		break;
	case F_APPENDUNDOBUFFERCURSOR:
		{
			OpeBlk* opeBlk = editView.commander.GetOpeBlk();
			if (!opeBlk) {
				editView.commander.SetOpeBlk(new OpeBlk());
			}
			opeBlk = editView.commander.GetOpeBlk();
			opeBlk->AddRef();
			opeBlk->AppendOpe(
				new MoveCaretOpe(
					editView.GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
			editView.SetUndoBuffer();
		}
		break;
	case F_CLIPBOARDEMPTY:
		{
			Clipboard clipboard(editView.GetHwnd());
			clipboard.Empty();
		}
		break;
	case F_SETVIEWTOP:
		{
			if (argSize <= 0) {
				return false;
			}
			if (argLengths[0] <= 0) {
				return false;
			}
			if (!WCODE::Is09( arguments[0][0] )) {
				return false;
			}
			int nLineNum = _wtoi(arguments[0]) - 1;
			if (nLineNum < 0) {
				nLineNum = 0;
			}
			editView.SyncScrollV( editView.ScrollAtV( nLineNum ));
		}
		break;
	case F_SETVIEWLEFT:
		{
			if (argSize <= 0) {
				return false;
			}
			if (argLengths[0] <= 0) {
				return false;
			}
			if (!WCODE::Is09( arguments[0][0] )) {
				return false;
			}
			int nColumn = _wtoi(arguments[0]) - 1;
			if (nColumn < 0) {
				nColumn = 0;
			}
			editView.SyncScrollH( editView.ScrollAtH( nColumn ) );
		}
		break;
	default:
		// �����Ȃ��B
		editView.GetCommander().HandleCommand(index, true, 0, 0, 0, 0);	// �W��
		break;
	}
	return true;
}


inline bool VariantToBStr(Variant& varCopy, const VARIANT& arg)
{
	return VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(arg)), 0, VT_BSTR) == S_OK;
}

inline bool VariantToI4(Variant& varCopy, const VARIANT& arg)
{
	return VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(arg)), 0, VT_I4) == S_OK;
}

/**	�l��Ԃ��֐�����������

	@param View      [in] �ΏۂƂȂ�View
	@param ID        [in] ����16bit: �֐��ԍ�
	@param Arguments [in] �����̔z��
	@param argSize   [in] �����̐�(arguments)
	@param Result  [out] ���ʂ̒l��Ԃ��ꏊ�B�߂�l��false�̂Ƃ��͕s��B
	
	@return true: ����, false: ���s

	@author �S
	@date 2003.02.21 �S
	@date 2003.06.01 Moca �֐��ǉ�
	@date 2005.08.05 maru,zenryaku �֐��ǉ�
	@date 2005.11.29 FILE VariantChangeType�Ή�
*/
bool Macro::HandleFunction(
	EditView& view,
	EFunctionCode id,
	const VARIANT* args,
	int numArgs,
	VARIANT& result
	)
{
	Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p

	// 2003-02-21 �S
	switch (LOWORD(id)) {
	case F_GETFILENAME:
		{
			const TCHAR* FileName = view.pEditDoc->docFile.GetFilePath();
			SysString s(FileName, _tcslen(FileName));
			Wrap(&result)->Receive(s);
		}
		return true;
	case F_GETSAVEFILENAME:
		// 2006.09.04 ryoji �ۑ����̃t�@�C���̃p�X
		{
			const TCHAR* FileName = view.pEditDoc->docFile.GetSaveFilePath();
			SysString s(FileName, lstrlen(FileName));
			Wrap(&result)->Receive(s);
		}
		return true;
	case F_GETSELECTED:
		{
			if (view.GetSelectionInfo().IsTextSelected()) {
				NativeW mem;
				if (!view.GetSelectedDataSimple(mem)) return false;
				SysString s(mem.GetStringPtr(), mem.GetStringLength());
				Wrap(&result)->Receive(s);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_EXPANDPARAMETER:
		// 2003.02.24 Moca
		{
			if (numArgs != 1) {
				return false;
			}
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTR�Ƃ��ĉ���
			}
			//void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);
			// pszSource��W�J���āApszBuffer�ɃR�s�[
			wchar_t* source;
			int sourceLength;
			Wrap(&varCopy.data.bstrVal)->GetW(&source, &sourceLength);
			wchar_t buffer[2048];
			SakuraEnvironment::ExpandParameter(source, buffer, 2047);
			delete[] source;
			SysString s(buffer, wcslen(buffer));
			Wrap(&result)->Receive(s);
		}
		return true;
	case F_GETLINESTR:
		// 2003.06.01 Moca �}�N���ǉ�
		{
			if (numArgs != 1) return false;
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			if (-1 < varCopy.data.lVal) {
				const wchar_t* Buffer;
				size_t nLength;
				int nLine;
				if (varCopy.data.lVal == 0) {
					nLine = view.GetCaret().GetCaretLogicPos().y;
				}else {
					nLine = varCopy.data.lVal - 1;
				}
				Buffer = view.pEditDoc->docLineMgr.GetLine(nLine)->GetDocLineStrWithEOL(&nLength);
				if (Buffer) {
					SysString s(Buffer, nLength);
					Wrap(&result)->Receive(s);
				}else {
					result.vt = VT_BSTR;
					result.bstrVal = SysAllocString(L"");
				}
			}else {
				return false;
			}
		}
		return true;
	case F_GETLINECOUNT:
		// 2003.06.01 Moca �}�N���ǉ�
		{
			if (numArgs != 1) return false;
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			if (varCopy.data.lVal == 0) {
				size_t nLineCount = view.pEditDoc->docLineMgr.GetLineCount();
				Wrap(&result)->Receive(nLineCount);
			}else {
				return false;
			}
		}
		return true;
	case F_CHGTABWIDTH:
		// 2004.03.16 zenryaku �}�N���ǉ�
		{
			if (numArgs != 1) return false;
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) return false;	// VT_I4�Ƃ��ĉ���
			size_t nTab = view.pEditDoc->layoutMgr.GetTabSpace();
			Wrap(&result)->Receive(nTab);
			// 2013.04.30 Moca �����ǉ��B�s�v�ȏꍇ��ChangeLayoutParam���Ă΂Ȃ�
			if (0 < varCopy.data.iVal && nTab != varCopy.data.iVal) {
				view.GetDocument().bTabSpaceCurTemp = true;
				view.editWnd.ChangeLayoutParam(
					false, 
					varCopy.data.iVal,
					view.pEditDoc->layoutMgr.GetMaxLineKetas()
				);

				// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I������TAB�����ύX���ꂽ��e�L�X�g�ő啝�̍ĎZ�o���K�v
				if (view.pEditDoc->nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
					// �ő啝�̍ĎZ�o���Ɋe�s�̃��C�A�E�g���̌v�Z���s��
					view.pEditDoc->layoutMgr.CalculateTextWidth();
				}
				view.editWnd.RedrawAllViews(NULL);		// TAB�����ς�����̂ōĕ`�悪�K�v
			}
		}
		return true;
	case F_ISTEXTSELECTED:
		// 2005.07.30 maru �}�N���ǉ�
		{
			if (view.GetSelectionInfo().IsTextSelected()) {
				if (view.GetSelectionInfo().IsBoxSelecting()) {
					Wrap(&result)->Receive(2);	// ��`�I��
				}else {
					Wrap(&result)->Receive(1);	// �I��
				}
			}else {
				Wrap(&result)->Receive(0);		// ��I��
			}
		}
		return true;
	case F_GETSELLINEFROM:
		// 2005.07.30 maru �}�N���ǉ�
		{
			Wrap(&result)->Receive(view.GetSelectionInfo().select.GetFrom().y + 1);
		}
		return true;
	case F_GETSELCOLUMNFROM:
		// 2005.07.30 maru �}�N���ǉ�
		{
			Wrap(&result)->Receive(view.GetSelectionInfo().select.GetFrom().x + 1);
		}
		return true;
	case F_GETSELLINETO:
		// 2005.07.30 maru �}�N���ǉ�
		{
			Wrap(&result)->Receive(view.GetSelectionInfo().select.GetTo().y + 1);
		}
		return true;
	case F_GETSELCOLUMNTO:
		// 2005.07.30 maru �}�N���ǉ�
		{
			Wrap(&result)->Receive(view.GetSelectionInfo().select.GetTo().x + 1);
		}
		return true;
	case F_ISINSMODE:
		// 2005.07.30 maru �}�N���ǉ�
		{
			Wrap(&result)->Receive(view.IsInsMode() /* Oct. 2, 2005 genta */);
		}
		return true;
	case F_GETCHARCODE:
		// 2005.07.31 maru �}�N���ǉ�
		{
			Wrap(&result)->Receive(view.pEditDoc->GetDocumentEncoding());
		}
		return true;
	case F_GETLINECODE:
		// 2005.08.04 maru �}�N���ǉ�
		{
			int n = 0;
			switch (view.pEditDoc->docEditor.GetNewLineCode()) {
			case EolType::CRLF:
				n = 0;
				break;
			case EolType::CR:
				n = 1;
				break;
			case EolType::LF:
				n = 2;
				break;
			case EolType::NEL:
				n = 3;
				break;
			case EolType::LS:
				n = 4;
				break;
			case EolType::PS:
				n = 5;
				break;
			}
			Wrap(&result)->Receive(n);
		}
		return true;
	case F_ISPOSSIBLEUNDO:
		// 2005.08.04 maru �}�N���ǉ�
		{
			Wrap(&result)->Receive(view.pEditDoc->docEditor.IsEnableUndo());
		}
		return true;
	case F_ISPOSSIBLEREDO:
		// 2005.08.04 maru �}�N���ǉ�
		{
			Wrap(&result)->Receive(view.pEditDoc->docEditor.IsEnableRedo());
		}
		return true;
	case F_CHGWRAPCOLUMN:
		// 2008.06.19 ryoji �}�N���ǉ�
		{
			if (numArgs != 1) {
				return false;
			}
			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) {
				return false;	// VT_I4�Ƃ��ĉ���
			}
			Wrap(&result)->Receive(view.pEditDoc->layoutMgr.GetMaxLineKetas());
			if (varCopy.data.iVal < MINLINEKETAS || varCopy.data.iVal > MAXLINEKETAS) {
				return true;
			}
			view.pEditDoc->nTextWrapMethodCur = TextWrappingMethod::SettingWidth;
			view.pEditDoc->bTextWrapMethodCurTemp = !(view.pEditDoc->nTextWrapMethodCur == view.pEditDoc->docType.GetDocumentAttribute().nTextWrapMethod);
			view.editWnd.ChangeLayoutParam(
				false, 
				view.pEditDoc->layoutMgr.GetTabSpace(),
				varCopy.data.iVal
			);
		}
		return true;
	case F_ISCURTYPEEXT:
		// 2006.09.04 ryoji �w�肵���g���q�����݂̃^�C�v�ʐݒ�Ɋ܂܂�Ă��邩�ǂ����𒲂ׂ�
		{
			if (numArgs != 1) {
				return false;
			}

			TCHAR* source;
			int sourceLength;

			int nType1 = view.pEditDoc->docType.GetDocumentType().GetIndex();	// ���݂̃^�C�v

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTR�Ƃ��ĉ���
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			int nType2 = DocTypeManager().GetDocumentTypeOfExt(source).GetIndex();	// �w��g���q�̃^�C�v
			delete[] source;

			Wrap(&result)->Receive((nType1 == nType2)? 1: 0);	// �^�C�v�ʐݒ�̈�v�^�s��v
		}
		return true;
	case F_ISSAMETYPEEXT:
		// 2006.09.04 ryoji �Q�̊g���q�������^�C�v�ʐݒ�Ɋ܂܂�Ă��邩�ǂ����𒲂ׂ�
		{
			if (numArgs != 2) {
				return false;
			}

			TCHAR* source;
			int sourceLength;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTR�Ƃ��ĉ���
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			int nType1 = DocTypeManager().GetDocumentTypeOfExt(source).GetIndex();	// �g���q�P�̃^�C�v
			delete[] source;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTR�Ƃ��ĉ���
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			int nType2 = DocTypeManager().GetDocumentTypeOfExt(source).GetIndex();	// �g���q�Q�̃^�C�v
			delete[] source;

			Wrap(&result)->Receive((nType1 == nType2)? 1: 0);	// �^�C�v�ʐݒ�̈�v�^�s��v
		}
		return true;
	case F_INPUTBOX:
		// 2011.03.18 syat �e�L�X�g���̓_�C�A���O�̕\��
		{
			if (numArgs < 1) {
				return false;
			}
			TCHAR* source;
			int sourceLength;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTR�Ƃ��ĉ���
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			std::tstring sMessage = source;	// �\�����b�Z�[�W
			delete[] source;

			std::tstring sDefaultValue = _T("");
			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTR�Ƃ��ĉ���
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sDefaultValue = source;	// �f�t�H���g�l
				delete[] source;
			}

			int nMaxLen = _MAX_PATH;
			if (numArgs >= 3) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[2])), 0, VT_I4) != S_OK) {
					return false;	// VT_I4�Ƃ��ĉ���
				}
				nMaxLen = varCopy.data.intVal;	// �ő���͒�
				if (nMaxLen <= 0) {
					nMaxLen = _MAX_PATH;
				}
			}

			std::vector<TCHAR> buffer(nMaxLen + 1);
			TCHAR* pBuffer = &buffer[0];
			_tcscpy( pBuffer, sDefaultValue.c_str() );
			DlgInput1 dlgInput1;
			if (dlgInput1.DoModal(G_AppInstance(), view.GetHwnd(), _T("sakura macro"), sMessage.c_str(), nMaxLen, pBuffer)) {
				SysString s(pBuffer, _tcslen(pBuffer));
				Wrap(&result)->Receive(s);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_MESSAGEBOX:	// ���b�Z�[�W�{�b�N�X�̕\��
	case F_ERRORMSG:	// ���b�Z�[�W�{�b�N�X�i�G���[�j�̕\��
	case F_WARNMSG:		// ���b�Z�[�W�{�b�N�X�i�x���j�̕\��
	case F_INFOMSG:		// ���b�Z�[�W�{�b�N�X�i���j�̕\��
	case F_OKCANCELBOX:	// ���b�Z�[�W�{�b�N�X�i�m�F�FOK�^�L�����Z���j�̕\��
	case F_YESNOBOX:	// ���b�Z�[�W�{�b�N�X�i�m�F�F�͂��^�������j�̕\��
		// 2011.03.18 syat ���b�Z�[�W�{�b�N�X�̕\��
		{
			if (numArgs < 1) {
				return false;
			}
			TCHAR* source;
			int sourceLength;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTR�Ƃ��ĉ���
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			std::tstring sMessage = source;	// �\��������
			delete[] source;

			UINT uType = 0;		// ���b�Z�[�W�{�b�N�X���
			switch (LOWORD(id)) {
			case F_MESSAGEBOX:
				if (numArgs >= 2) {
					if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_I4) != S_OK) {
						return false;	// VT_I4�Ƃ��ĉ���
					}
					uType = varCopy.data.uintVal;
				}else {
					uType = MB_OK;
				}
				break;
			case F_ERRORMSG:
				uType |= MB_OK | MB_ICONSTOP;
				break;
			case F_WARNMSG:
				uType |= MB_OK | MB_ICONEXCLAMATION;
				break;
			case F_INFOMSG:
				uType |= MB_OK | MB_ICONINFORMATION;
				break;
			case F_OKCANCELBOX:
				uType |= MB_OKCANCEL | MB_ICONQUESTION;
				break;
			case F_YESNOBOX:
				uType |= MB_YESNO | MB_ICONQUESTION;
				break;
			}
			int ret = ::MessageBox(view.GetHwnd(), sMessage.c_str(), _T("sakura macro"), uType);
			Wrap(&result)->Receive(ret);
		}
		return true;
	case F_COMPAREVERSION:
		// 2011.03.18 syat �o�[�W�����ԍ��̔�r
		{
			if (numArgs != 2) {
				return false;
			}
			TCHAR* source;
			int sourceLength;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTR�Ƃ��ĉ���
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			std::tstring sVerA = source;	// �o�[�W����A
			delete[] source;

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
				return false;	// VT_BSTR�Ƃ��ĉ���
			}
			Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
			std::tstring sVerB = source;	// �o�[�W����B
			delete[] source;

			Wrap(&result)->Receive(CompareVersion(sVerA.c_str(), sVerB.c_str()));
		}
		return true;
	case F_MACROSLEEP:
		// 2011.03.18 syat �w�肵�����ԁi�~���b�j��~����
		{
			if (numArgs != 1) {
				return false;
			}

			if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_UI4) != S_OK) {
				return false;	// VT_UI4�Ƃ��ĉ���
			}
			WaitCursor waitCursor(view.GetHwnd());	// �J�[�\���������v�ɂ���
			::Sleep(varCopy.data.uintVal);
			Wrap(&result)->Receive(0);	// �߂�l�͍��̂Ƃ���0�Œ�
		}
		return true;
	case F_FILEOPENDIALOG:
	case F_FILESAVEDIALOG:
		// 2011.03.18 syat �t�@�C���_�C�A���O�̕\��
		{
			TCHAR* source;
			int sourceLength;
			std::tstring sDefault;
			std::tstring sFilter;

			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTR�Ƃ��ĉ���
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sDefault = source;	// ����̃t�@�C����
				delete[] source;
			}

			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) return false;	// VT_BSTR�Ƃ��ĉ���
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sFilter = source;	// �t�B���^������
				delete[] source;
			}

			DlgOpenFile dlgOpenFile;
			dlgOpenFile.Create(
				G_AppInstance(), view.GetHwnd(),
				sFilter.c_str(),
				sDefault.c_str()
			);
			bool bRet;
			TCHAR szPath[_MAX_PATH];
			_tcscpy( szPath, sDefault.c_str() );
			if (LOWORD(id) == F_FILEOPENDIALOG) {
				bRet = dlgOpenFile.DoModal_GetOpenFileName(szPath);
			}else {
				bRet = dlgOpenFile.DoModal_GetSaveFileName(szPath);
			}
			if (bRet) {
				SysString s(szPath, _tcslen(szPath));
				Wrap(&result)->Receive(s);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_FOLDERDIALOG:
		// 2011.03.18 syat �t�H���_�_�C�A���O�̕\��
		{
			TCHAR* source;
			int sourceLength;
			std::tstring sMessage;
			std::tstring sDefault;

			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTR�Ƃ��ĉ���
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sMessage = source;	// �\�����b�Z�[�W
				delete[] source;
			}

			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTR�Ƃ��ĉ���
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&source, &sourceLength);
				sDefault = source;	// ����̃t�@�C����
				delete[] source;
			}

			TCHAR szPath[_MAX_PATH];
			int nRet = SelectDir(view.GetHwnd(), sMessage.c_str(), sDefault.c_str(), szPath);
			if (nRet == IDOK) {
				SysString s(szPath, _tcslen(szPath));
				Wrap(&result)->Receive(s);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_GETCLIPBOARD:
		// 2011.03.18 syat �N���b�v�{�[�h�̕�������擾
		{
			int nOpt = 0;

			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) {
					return false;	// VT_I4�Ƃ��ĉ���
				}
				nOpt = varCopy.data.intVal;	// �I�v�V����
			}

			NativeW memBuff;
			bool bColumnSelect = false;
			bool bLineSelect = false;
			bool bRet = view.MyGetClipboardData(memBuff, &bColumnSelect, &bLineSelect);
			if (bRet) {
				SysString s(memBuff.GetStringPtr(), memBuff.GetStringLength());
				Wrap(&result)->Receive(s);
			}else {
				result.vt = VT_BSTR;
				result.bstrVal = SysAllocString(L"");
			}
		}
		return true;
	case F_SETCLIPBOARD:
		// 2011.03.18 syat �N���b�v�{�[�h�ɕ������ݒ�
		{
			std::tstring sValue;
			int nOpt = 0;

			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_I4) != S_OK) {
					return false;	// VT_I4�Ƃ��ĉ���
				}
				nOpt = varCopy.data.intVal;	// �I�v�V����
			}

			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
					return false;	// VT_BSTR�Ƃ��ĉ���
				}
				Wrap(&varCopy.data.bstrVal)->GetT(&sValue);
			}

			// 2013.06.12 �I�v�V�����ݒ�
			bool bColumnSelect = ((nOpt & 0x01) == 0x01);
			bool bLineSelect = ((nOpt & 0x02) == 0x02);
			bool bRet = view.MySetClipboardData(sValue.c_str(), sValue.size(), bColumnSelect, bLineSelect);
			Wrap(&result)->Receive(bRet);
		}
		return true;

	case F_LAYOUTTOLOGICLINENUM:
		// ���C�A�E�g�����W�b�N�s
		{
			if (numArgs < 1) {
				return false;
			}
			if (!VariantToI4(varCopy, args[0])) {
				return false;
			}
			int nLineNum = varCopy.data.lVal - 1;
			int ret = 0;
			if (view.pEditDoc->layoutMgr.GetLineCount() == nLineNum) {
				ret = view.pEditDoc->docLineMgr.GetLineCount() + 1;
			}else {
				const Layout* pLayout = view.pEditDoc->layoutMgr.SearchLineByLayoutY(nLineNum);
				if (pLayout) {
					ret = pLayout->GetLogicLineNo() + 1;
				}else {
					return false;
				}
			}
			Wrap(&result)->Receive(ret);
		}
		return true;

	case F_LINECOLUMNTOINDEX:
		// ���C�A�E�g�����W�b�N��
		{
			if (numArgs < 2) {
				return false;
			}
			if (!VariantToI4(varCopy, args[0])) {
				return false;
			}
			int nLineNum = varCopy.data.lVal - 1;
			if (!VariantToI4(varCopy, args[1])) {
				return false;
			}
			int nLineCol = varCopy.data.lVal - 1;
			if (nLineNum < 0) {
				return false;
			}

			Point nLayoutPos(nLineCol, nLineNum);
			Point nLogicPos(0, 0);
			view.pEditDoc->layoutMgr.LayoutToLogic(nLayoutPos, &nLogicPos);
			int ret = nLogicPos.GetX() + 1;
			Wrap(&result)->Receive(ret);
		}
		return true;

	case F_LOGICTOLAYOUTLINENUM:
	case F_LINEINDEXTOCOLUMN:
		// ���W�b�N�����C�A�E�g�s/��
		{
			if (numArgs < 2) {
				return false;
			}
			if (!VariantToI4(varCopy, args[0])) {
				return false;
			}
			int nLineNum = varCopy.data.lVal - 1;
			if (!VariantToI4(varCopy, args[1])) {
				return false;
			}
			int nLineIdx = varCopy.data.lVal - 1;
			if (nLineNum < 0) {
				return false;
			}

			Point nLogicPos(nLineIdx, nLineNum);
			Point nLayoutPos(0, 0);
			view.pEditDoc->layoutMgr.LogicToLayout(nLogicPos, &nLayoutPos);
			int ret = ((LOWORD(id) == F_LOGICTOLAYOUTLINENUM) ? nLayoutPos.y : nLayoutPos.x) + 1;
			Wrap(&result)->Receive(ret);
		}
		return true;

	case F_GETCOOKIE:
		{
			Variant varCopy2;
			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) {
					return false;
				}
				if (VariantChangeType(&varCopy2.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) {
					return false;
				}
				SysString ret = view.GetDocument().cookie.GetCookie(varCopy.data.bstrVal, varCopy2.data.bstrVal);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_GETCOOKIEDEFAULT:
		{
			Variant varCopy2, varCopy3;
			if (numArgs >= 3) {
				if (0
					|| VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK
					|| VariantChangeType(&varCopy2.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK
					|| VariantChangeType(&varCopy3.data, const_cast<VARIANTARG*>(&(args[2])), 0, VT_BSTR) != S_OK
				) {
					return false;
				}
				SysString ret = view.GetDocument().cookie.GetCookieDefault(
					varCopy.data.bstrVal,
					varCopy2.data.bstrVal,
					varCopy3.data.bstrVal,
					SysStringLen(varCopy3.data.bstrVal)
				);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_SETCOOKIE:
		{
			Variant varCopy2, varCopy3;
			if (numArgs >= 3) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) return false;
				if (VariantChangeType(&varCopy2.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) return false;
				if (VariantChangeType(&varCopy3.data, const_cast<VARIANTARG*>(&(args[2])), 0, VT_BSTR) != S_OK) return false;
				int ret = view.GetDocument().cookie.SetCookie(varCopy.data.bstrVal, varCopy2.data.bstrVal,
					varCopy3.data.bstrVal, SysStringLen(varCopy3.data.bstrVal));
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_DELETECOOKIE:
		{
			Variant varCopy2;
			if (numArgs >= 2) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) return false;
				if (VariantChangeType(&varCopy2.data, const_cast<VARIANTARG*>(&(args[1])), 0, VT_BSTR) != S_OK) return false;
				int ret = view.GetDocument().cookie.DeleteCookie(varCopy.data.bstrVal, varCopy2.data.bstrVal);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_GETCOOKIENAMES:
		{
			if (numArgs >= 1) {
				if (VariantChangeType(&varCopy.data, const_cast<VARIANTARG*>(&(args[0])), 0, VT_BSTR) != S_OK) return false;
				SysString ret = view.GetDocument().cookie.GetCookieNames(varCopy.data.bstrVal);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_SETDRAWSWITCH:
		{
			if (1 <= numArgs) {
				if (!VariantToI4(varCopy, args[0])) return false;
				int ret = (view.editWnd.SetDrawSwitchOfAllViews(varCopy.data.iVal != 0) ? 1: 0);
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_GETDRAWSWITCH:
		{
			int ret = (view.GetDrawSwitch() ? 1: 0);
			Wrap(&result)->Receive(ret);
			return true;
		}
	case F_ISSHOWNSTATUS:
		{
			int ret = (view.editWnd.statusBar.GetStatusHwnd() ? 1: 0);
			Wrap(&result)->Receive(ret);
			return true;
		}
	case F_GETSTRWIDTH:
		{
			Variant varCopy2;
			if (1 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) { return false; }
				if (2 <= numArgs) {
					if (!VariantToI4(varCopy2, args[1])) { return false; }
				}else {
					varCopy2.data.lVal = 1;
				}
				const wchar_t* pLine = varCopy.data.bstrVal;
				size_t nLen = ::SysStringLen(varCopy.data.bstrVal);
				if (2 <= nLen) {
					if (pLine[nLen-2] == WCODE::CR && pLine[nLen-1] == WCODE::LF) {
						--nLen;
					}
				}
				const size_t nTabWidth = view.GetDocument().layoutMgr.GetTabSpaceKetas();
				int nPosX = varCopy2.data.lVal - 1;
				for (size_t i=0; i<nLen;) {
					if (pLine[i] == WCODE::TAB) {
						nPosX += nTabWidth - (nPosX % nTabWidth);
					}else {
						nPosX += NativeW::GetKetaOfChar(pLine, nLen, i);
					}
					i += t_max(1, (int)NativeW::GetSizeOfChar(pLine, nLen, i));
				}
				nPosX -= varCopy2.data.lVal - 1;
				Wrap(&result)->Receive(nPosX);
				return true;
			}
			return false;
		}
	case F_GETSTRLAYOUTLENGTH:
		{
			Variant varCopy2;
			if (1 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) { return false; }
				if (2 <= numArgs) {
					if (!VariantToI4(varCopy2, args[1])) { return false; }
				}else {
					varCopy2.data.lVal = 1;
				}
				DocLine tmpDocLine;
				tmpDocLine.SetDocLineString(varCopy.data.bstrVal, ::SysStringLen(varCopy.data.bstrVal));
				const size_t tmpLenWithEol1 = tmpDocLine.GetLengthWithoutEOL() + (0 < tmpDocLine.GetEol().GetLen() ? 1: 0);
				const int offset = varCopy2.data.lVal - 1;
				const Layout tmpLayout(
					&tmpDocLine,
					Point(0, 0),
					tmpLenWithEol1,
					COLORIDX_TEXT,
					offset,
					NULL
				);
				size_t width = view.LineIndexToColumn(&tmpLayout, tmpDocLine.GetLengthWithEOL()) - offset;
				Wrap(&result)->Receive(width);
				return true;
			}
			return false;
		}
	case F_GETDEFAULTCHARLENGTH:
		{
			int width = view.GetTextMetrics().GetLayoutXDefault();
			Wrap(&result)->Receive(width);
			return true;
		}
	case F_ISINCLUDECLIPBOARDFORMAT:
		{
			if (1 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) return false;
				Clipboard clipboard(view.GetHwnd());
				bool bret = clipboard.IsIncludeClipboradFormat(varCopy.data.bstrVal);
				Wrap(&result)->Receive(bret ? 1 : 0);
				return true;
			}
			return false;
		}
	case F_GETCLIPBOARDBYFORMAT:
		{
			Variant varCopy2, varCopy3;
			if (2 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) return false;
				if (!VariantToI4(varCopy2, args[1])) return false;
				if (3 <= numArgs) {
					if (!VariantToI4(varCopy3, args[2])) return false;
				}else {
					varCopy3.data.lVal = -1;
				}
				Clipboard clipboard(view.GetHwnd());
				NativeW mem;
				Eol eol = view.pEditDoc->docEditor.GetNewLineCode();
				clipboard.GetClipboradByFormat(mem, varCopy.data.bstrVal, varCopy2.data.lVal, varCopy3.data.lVal, eol);
				SysString ret(mem.GetStringPtr(), mem.GetStringLength());
				Wrap(&result)->Receive(ret);
				return true;
			}
			return false;
		}
	case F_SETCLIPBOARDBYFORMAT:
		{
			Variant varCopy2, varCopy3, varCopy4;
			if (3 <= numArgs) {
				if (!VariantToBStr(varCopy, args[0])) return false;
				if (!VariantToBStr(varCopy2, args[1])) return false;
				if (!VariantToI4(varCopy3, args[2])) return false;
				if (3 <= numArgs) {
					if (!VariantToI4(varCopy4, args[3])) return false;
				}else {
					varCopy4.data.lVal = -1;
				}
				Clipboard clipboard(view.GetHwnd());
				StringRef cstr(varCopy.data.bstrVal, ::SysStringLen(varCopy.data.bstrVal));
				bool bret = clipboard.SetClipboradByFormat(cstr, varCopy2.data.bstrVal, varCopy3.data.lVal, varCopy4.data.lVal);
				Wrap(&result)->Receive(bret ? 1 : 0);
				return true;
			}
			return false;
		}
	case F_GETLINEATTRIBUTE:
		{
			int nLineNum;
			int nAttType;
			if (numArgs < 2) {
				return false;
			}
			if (!variant_to_int(args[0], nLineNum)) return false;
			if (!variant_to_int(args[1], nAttType)) return false;
			int nLine;
			if (nLineNum == 0) {
				nLine = view.GetCaret().GetCaretLogicPos().y;
			}else if (nLineNum < 0) {
				return false;
			}else {
				nLine = nLineNum - 1; // nLineNum��1�J�n
			}
			const DocLine* pDocLine = view.GetDocument().docLineMgr.GetLine(nLine);
			if (!pDocLine) {
				return false;
			}
			int nRet;
			switch (nAttType) {
			case 0:
				nRet = (ModifyVisitor().IsLineModified(pDocLine, view.GetDocument().docEditor.opeBuf.GetNoModifiedSeq()) ? 1: 0);
				break;
			case 1:
				nRet = pDocLine->mark.modified.GetSeq();
				break;
			case 2:
				nRet = (pDocLine->mark.bookmarked ? 1: 0);
				break;
			case 3:
				nRet = (int)(DiffMark)(pDocLine->mark.diffMarked);
				break;
			case 4:
				nRet = (pDocLine->mark.funcList.GetFuncListMark() ? 1: 0 );
				break;
			default:
				return false;
			}
			Wrap( &result )->Receive( nRet );
			return true;
		}
	case F_ISTEXTSELECTINGLOCK:
		{
			if (view.GetSelectionInfo().bSelectingLock) {
				if (view.GetSelectionInfo().IsBoxSelecting()) {
					Wrap( &result )->Receive( 2 );	//�I�����b�N+��`�I��
				}else {
					Wrap( &result )->Receive( 1 );	//�I�����b�N��
				}
			}else {
				Wrap( &result )->Receive( 0 );		//�񃍃b�N��
			}
		}
		return true;
	case F_GETVIEWLINES:
		{
			int nLines = view.GetTextArea().nViewRowNum;
			Wrap( &result )->Receive( nLines );
			return true;
		}
	case F_GETVIEWCOLUMNS:
		{
			int nColumns = view.GetTextArea().nViewColNum;
			Wrap( &result )->Receive( nColumns );
			return true;
		}
	case F_CREATEMENU:
		{
			Variant varCopy2;
			if (2 <= numArgs) {
				if (!VariantToI4(varCopy, args[0])) {
					return false;
				}
				if (!VariantToBStr(varCopy2, args[1])) {
					return false;
				}
				std::vector<wchar_t> vStrMenu;
				int nLen = (int)auto_strlen(varCopy2.data.bstrVal);
				vStrMenu.assign( nLen + 1, L'\0' );
				auto_strcpy(&vStrMenu[0], varCopy2.data.bstrVal);
				HMENU hMenu = ::CreatePopupMenu();
				std::vector<HMENU> vHmenu;
				vHmenu.push_back( hMenu );
				HMENU hMenuCurrent = hMenu;
				size_t nPos = 0;
				wchar_t* p;
				int i = 1;
				while (p = my_strtok( &vStrMenu[0], nLen, &nPos, L"," )) {
					wchar_t* r = p;
					int nFlags = MF_STRING;
					int nFlagBreak = 0;
					bool bSpecial = false;
					bool bRadio = false;
					bool bSubMenu = false;
					size_t nBreakNum = 0;
					if (p[0] == L'[') {
						++r;
						while (*r != L']' && *r != L'\0') {
							switch (*r) {
							case L'S':
								if (!bSubMenu) {
									HMENU hMenuSub = ::CreatePopupMenu();
									vHmenu.push_back( hMenuSub );
									bSubMenu = true;
								}
								break;
							case L'E':
								++nBreakNum;
								break;
							case L'C':
								nFlags |= MF_CHECKED;
								break;
							case L'D':
								nFlags |= MF_GRAYED;
								break;
							case L'R':
								bRadio = true;
								break;
							case L'B':
								nFlagBreak |= MF_MENUBARBREAK;
								break;
							default:
								break;
							}
							++r;
						}
						if (*r == L']') {
							++r;
						}
					}
					if (p[0] == L'-' && p[1] == L'\0') {
						nFlags |= MF_SEPARATOR;
						++r;
						bSpecial = true;
					}

					if (bSubMenu) {
						nFlags |= nFlagBreak;
						::InsertMenu( hMenuCurrent, -1, nFlags | MF_BYPOSITION | MF_POPUP, (UINT_PTR)vHmenu.back(), to_tchar(r) );
						hMenuCurrent = vHmenu.back();
					}else if (bSpecial) {
						nFlags |= nFlagBreak;
						::InsertMenu( hMenuCurrent, -1, nFlags | MF_BYPOSITION, 0, NULL );
					}else {
						nFlags |= nFlagBreak;
						::InsertMenu( hMenuCurrent, -1, nFlags | MF_BYPOSITION, i, to_tchar(r) );
						if (bRadio) {
							::CheckMenuRadioItem( hMenuCurrent, i, i, i, MF_BYCOMMAND );
						}
						++i;
					}
					for (size_t n=0; n<nBreakNum; ++n) {
						if (1 < vHmenu.size()) {
							vHmenu.resize( vHmenu.size() - 1 );
						}
						hMenuCurrent = vHmenu.back();
					}
				}
				POINT pt;
				int nViewPointType = varCopy.data.lVal;
				if (nViewPointType == 1) {
					// �L�����b�g�ʒu
					pt = view.GetCaret().CalcCaretDrawPos( view.GetCaret().GetCaretLayoutPos() );
					if (view.GetTextArea().GetAreaLeft() <= pt.x
						&& view.GetTextArea().GetAreaTop() <= pt.y
						&& pt.x < view.GetTextArea().GetAreaRight()
						&& pt.y < view.GetTextArea().GetAreaBottom()
					) {
						::ClientToScreen( view.GetHwnd(), &pt );
					}else {
						::GetCursorPos( &pt );
					}
				}else {
					// �}�E�X�ʒu
					::GetCursorPos( &pt );
				}
				RECT rcWork;
				GetMonitorWorkRect( pt, &rcWork );
				int nId = ::TrackPopupMenu(
					hMenu,
					TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
					(pt.x > rcWork.left) ? pt.x: rcWork.left,
					(pt.y < rcWork.bottom) ? pt.y: rcWork.bottom,
					0,
					view.GetHwnd(),
					NULL
				);
				::DestroyMenu( hMenu );
				Wrap( &result )->Receive( nId );
				return true;
			}
			return false;
		}
	default:
		return false;
	}
}

