/*
	Copyright (C) 2008, kobake

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
#include "CMruListener.h"
#include "recent/CMRUFile.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "view/CEditView.h"
#include "charset/CCodePage.h"
#include "charset/CCodeMediator.h"
#include "util/file.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void MruListener::OnAfterSave(const SaveInfo& sSaveInfo)
{
	_HoldBookmarks_And_AddToMRU();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
void MruListener::OnBeforeLoad(LoadInfo* pLoadInfo)
{
	// �ă��[�h�p�Ɍ��݃t�@�C����MRU�o�^���Ă���
	// Mar. 30, 2003 genta �u�b�N�}�[�N�ۑ��̂���MRU�֓o�^
	_HoldBookmarks_And_AddToMRU();	// �� �V�K�I�[�v���i�t�@�C�������ݒ�j�ł͉������Ȃ�

	// �����R�[�h�w��͖����I�ł��邩
	bool bSpecified = IsValidCodeOrCPType(pLoadInfo->eCharCode);

	// �O��̃R�[�h -> ePrevCode
	EditInfo	fi;
	ECodeType ePrevCode = CODE_NONE;
	int nPrevTypeId = -1;
	if (MRUFile().GetEditInfo(pLoadInfo->filePath, &fi)) {
		ePrevCode = fi.m_nCharCode;
		nPrevTypeId = fi.m_nTypeId;
	}

	// �^�C�v�ʐݒ�
	if (!pLoadInfo->nType.IsValidType()) {
		if (0 <= nPrevTypeId) {
			pLoadInfo->nType = DocTypeManager().GetDocumentTypeOfId(nPrevTypeId);
		}
		if (!pLoadInfo->nType.IsValidType()) {
			pLoadInfo->nType = DocTypeManager().GetDocumentTypeOfPath(pLoadInfo->filePath);
		}
	}

	// �w��̃R�[�h -> pLoadInfo->eCharCode
	if (CODE_AUTODETECT == pLoadInfo->eCharCode) {
		if (fexist(pLoadInfo->filePath)) {
			// �f�t�H���g�����R�[�h�F���̂��߂Ɉꎞ�I�ɓǂݍ��ݑΏۃt�@�C���̃t�@�C���^�C�v��K�p����
			const TypeConfigMini* type;
			DocTypeManager().GetTypeConfigMini(pLoadInfo->nType, &type);
			CodeMediator cmediator(type->m_encoding);
			pLoadInfo->eCharCode = cmediator.CheckKanjiCodeOfFile(pLoadInfo->filePath);
		}else {
			pLoadInfo->eCharCode = ePrevCode;
		}
	}else if (CODE_NONE == pLoadInfo->eCharCode) {
		pLoadInfo->eCharCode = ePrevCode;
	}
	if (CODE_NONE == pLoadInfo->eCharCode) {
		const TypeConfigMini* type;
		if (DocTypeManager().GetTypeConfigMini(pLoadInfo->nType, &type)) {
			pLoadInfo->eCharCode = type->m_encoding.m_eDefaultCodetype;	// �����l�̉��	// 2011.01.24 ryoji CODE_DEFAULT -> m_eDefaultCodetype
		}else {
			pLoadInfo->eCharCode = GetDllShareData().m_TypeBasis.m_encoding.m_eDefaultCodetype;
		}
	}

	// �H���Ⴄ�ꍇ
	if (IsValidCodeOrCPType(ePrevCode) && pLoadInfo->eCharCode != ePrevCode) {
		// �I�v�V�����F�O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s��
		if (GetDllShareData().m_common.m_file.m_bQueryIfCodeChange && !pLoadInfo->bRequestReload) {
			TCHAR szCpNameNew[260];
			TCHAR szCpNameOld[260];
			CodePage::GetNameLong(szCpNameOld, ePrevCode);
			CodePage::GetNameLong(szCpNameNew, pLoadInfo->eCharCode);
			ConfirmBeep();
			int nRet = MYMESSAGEBOX(
				EditWnd::getInstance()->GetHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				LS(STR_ERR_DLGEDITDOC5),
				LS(STR_ERR_DLGEDITDOC6),
				pLoadInfo->filePath.c_str(),
				szCpNameNew,
				szCpNameOld,
				szCpNameOld,
				szCpNameNew
			);
			if (IDYES == nRet) {
				// �O��̕����R�[�h���̗p����
				pLoadInfo->eCharCode = ePrevCode;
			}else {
				// ���X�g�����Ƃ��Ă��������R�[�h���̗p����
				pLoadInfo->eCharCode = pLoadInfo->eCharCode;
			}
		// �H������Ă��₢���킹���s��Ȃ��ꍇ
		}else {
			// �f�t�H���g�̉�
			//  �������ʂ̏ꍇ�F�O��̕����R�[�h���̗p
			//  �����w��̏ꍇ�F�����w��̕����R�[�h���̗p
			if (!bSpecified) { // ��������
				pLoadInfo->eCharCode = ePrevCode;
			}else { // �����w��
				pLoadInfo->eCharCode = pLoadInfo->eCharCode;
			}
		}
	}
}


void MruListener::OnAfterLoad(const LoadInfo& sLoadInfo)
{
	EditDoc* pcDoc = GetListeningDoc();

	MRUFile cMRU;

	EditInfo eiOld;
	bool bIsExistInMRU = cMRU.GetEditInfo(pcDoc->m_docFile.GetFilePath(), &eiOld);

	// �L�����b�g�ʒu�̕���
	if (bIsExistInMRU && GetDllShareData().m_common.m_file.GetRestoreCurPosition()) {
		// �L�����b�g�ʒu�擾
		LayoutPoint ptCaretPos;
		pcDoc->m_layoutMgr.LogicToLayout(eiOld.m_ptCursor, &ptCaretPos);

		// �r���[�擾
		EditView& cView = pcDoc->m_pEditWnd->GetActiveView();

		if (ptCaretPos.GetY2() >= pcDoc->m_layoutMgr.GetLineCount()) {
			// �t�@�C���̍Ō�Ɉړ�
			cView.GetCommander().HandleCommand(F_GOFILEEND, false, 0, 0, 0, 0);
		}else {
			cView.GetTextArea().SetViewTopLine(eiOld.m_nViewTopLine); // 2001/10/20 novice
			cView.GetTextArea().SetViewLeftCol(eiOld.m_nViewLeftCol); // 2001/10/20 novice
			// From Here Mar. 28, 2003 MIK
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
			const DocLine *pTmpDocLine = pcDoc->m_docLineMgr.GetLine(eiOld.m_ptCursor.GetY2());	// 2008.08.22 ryoji ���s�P�ʂ̍s�ԍ���n���悤�ɏC��
			if (pTmpDocLine) {
				if (pTmpDocLine->GetLengthWithoutEOL() < eiOld.m_ptCursor.x) {
					ptCaretPos.x--;
				}
			}
			// To Here Mar. 28, 2003 MIK
			cView.GetCaret().MoveCursor(ptCaretPos, true);
			cView.GetCaret().m_nCaretPosX_Prev = cView.GetCaret().GetCaretLayoutPos().GetX2();
		}
	}

	// �u�b�N�}�[�N����  // 2002.01.16 hor
	if (bIsExistInMRU) {
		if (GetDllShareData().m_common.m_file.GetRestoreBookmarks()) {
			BookmarkManager(&pcDoc->m_docLineMgr).SetBookMarks(eiOld.m_szMarkLines);
		}
	}else {
		eiOld.m_szMarkLines[0] = 0;
	}

	// MRU���X�g�ւ̓o�^
	EditInfo	eiNew;
	pcDoc->GetEditInfo(&eiNew);
	// 2014.07.04 �u�b�N�}�[�N�̕ێ�(�G�f�B�^���������Ƃ��u�b�N�}�[�N�������邽��)
	if (bIsExistInMRU) {
		if (GetDllShareData().m_common.m_file.GetRestoreBookmarks()) {
			// SetBookMarks�Ńf�[�^��NUL��؂�ɏ���������Ă���̂ōĎ擾
			cMRU.GetEditInfo(pcDoc->m_docFile.GetFilePath(), &eiOld);
			auto_strcpy(eiNew.m_szMarkLines, eiOld.m_szMarkLines);
		}
	}
	cMRU.Add(&eiNew);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �N���[�Y�O��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CallbackResultType MruListener::OnBeforeClose()
{
	// Mar. 30, 2003 genta �T�u���[�`���ɂ܂Ƃ߂�
	_HoldBookmarks_And_AddToMRU();

	return CallbackResultType::Continue;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �w���p                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�J�����g�t�@�C����MRU�ɓo�^����B
	�u�b�N�}�[�N���ꏏ�ɓo�^����B

	@date 2003.03.30 genta �쐬

*/
void MruListener::_HoldBookmarks_And_AddToMRU()
{
	// EditInfo�擾
	EditDoc* pcDoc = GetListeningDoc();
	EditInfo fi;
	pcDoc->GetEditInfo(&fi);

	// �u�b�N�}�[�N���̕ۑ�
	wcscpy_s(fi.m_szMarkLines, BookmarkManager(&pcDoc->m_docLineMgr).GetBookMarks());

	// MRU���X�g�ɓo�^
	MRUFile cMRU;
	cMRU.Add(&fi);
}

