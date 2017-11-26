#include "StdAfx.h"
#include "MruListener.h"
#include "recent/MRUFile.h"
#include "doc/EditDoc.h"
#include "window/EditWnd.h"
#include "view/EditView.h"
#include "charset/CodePage.h"
#include "charset/CodeMediator.h"
#include "util/fileUtil.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void MruListener::OnAfterSave(const SaveInfo& saveInfo)
{
	_HoldBookmarks_And_AddToMRU();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void MruListener::OnBeforeLoad(LoadInfo* pLoadInfo)
{
	// �ă��[�h�p�Ɍ��݃t�@�C����MRU�o�^���Ă���
	_HoldBookmarks_And_AddToMRU();	// �� �V�K�I�[�v���i�t�@�C�������ݒ�j�ł͉������Ȃ�

	// �����R�[�h�w��͖����I�ł��邩
	bool bSpecified = IsValidCodeOrCPType(pLoadInfo->eCharCode);

	// �O��̃R�[�h -> ePrevCode
	EditInfo	fi;
	EncodingType ePrevCode = CODE_NONE;
	int nPrevTypeId = -1;
	if (MruFile().GetEditInfo(pLoadInfo->filePath, &fi)) {
		ePrevCode = fi.nCharCode;
		nPrevTypeId = fi.nTypeId;
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
	if (pLoadInfo->eCharCode == CODE_AUTODETECT) {
		if (fexist(pLoadInfo->filePath)) {
			// �f�t�H���g�����R�[�h�F���̂��߂Ɉꎞ�I�ɓǂݍ��ݑΏۃt�@�C���̃t�@�C���^�C�v��K�p����
			const TypeConfigMini* type;
			DocTypeManager().GetTypeConfigMini(pLoadInfo->nType, &type);
			CodeMediator mediator(type->encoding);
			pLoadInfo->eCharCode = mediator.CheckKanjiCodeOfFile(pLoadInfo->filePath);
		}else {
			pLoadInfo->eCharCode = ePrevCode;
		}
	}else if (pLoadInfo->eCharCode == CODE_NONE) {
		pLoadInfo->eCharCode = ePrevCode;
	}
	if (pLoadInfo->eCharCode == CODE_NONE) {
		const TypeConfigMini* type;
		if (DocTypeManager().GetTypeConfigMini(pLoadInfo->nType, &type)) {
			pLoadInfo->eCharCode = type->encoding.eDefaultCodetype;	// �����l�̉��
		}else {
			pLoadInfo->eCharCode = GetDllShareData().typeBasis.encoding.eDefaultCodetype;
		}
	}

	// �H���Ⴄ�ꍇ
	if (IsValidCodeOrCPType(ePrevCode) && pLoadInfo->eCharCode != ePrevCode) {
		// �I�v�V�����F�O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s��
		if (GetDllShareData().common.file.bQueryIfCodeChange && !pLoadInfo->bRequestReload) {
			TCHAR szCpNameNew[260];
			TCHAR szCpNameOld[260];
			CodePage::GetNameLong(szCpNameOld, ePrevCode);
			CodePage::GetNameLong(szCpNameNew, pLoadInfo->eCharCode);
			ConfirmBeep();
			int nRet = MYMESSAGEBOX(
				EditWnd::getInstance().GetHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				LS(STR_ERR_DLGEDITDOC5),
				LS(STR_ERR_DLGEDITDOC6),
				pLoadInfo->filePath.c_str(),
				szCpNameNew,
				szCpNameOld,
				szCpNameOld,
				szCpNameNew
			);
			if (nRet == IDYES) {
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


void MruListener::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	MruFile mru;

	EditInfo eiOld;
	bool bIsExistInMRU = mru.GetEditInfo(pDoc->docFile.GetFilePath(), &eiOld);

	// �L�����b�g�ʒu�̕���
	if (bIsExistInMRU && GetDllShareData().common.file.GetRestoreCurPosition()) {
		// �L�����b�g�ʒu�擾
		Point ptCaretPos = pDoc->layoutMgr.LogicToLayout(eiOld.ptCursor);

		// �r���[�擾
		EditView& view = pDoc->pEditWnd->GetActiveView();

		if (ptCaretPos.y >= (int)pDoc->layoutMgr.GetLineCount()) {
			// �t�@�C���̍Ō�Ɉړ�
			view.GetCommander().HandleCommand(F_GOFILEEND, false, 0, 0, 0, 0);
		}else {
			view.GetTextArea().SetViewTopLine(eiOld.nViewTopLine);
			view.GetTextArea().SetViewLeftCol(eiOld.nViewLeftCol);
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
			const DocLine *pTmpDocLine = pDoc->docLineMgr.GetLine(eiOld.ptCursor.y);
			if (pTmpDocLine) {
				if ((int)pTmpDocLine->GetLengthWithoutEOL() < eiOld.ptCursor.x) {
					ptCaretPos.x--;
				}
			}
			view.GetCaret().MoveCursor(ptCaretPos, true);
			view.GetCaret().nCaretPosX_Prev = view.GetCaret().GetCaretLayoutPos().x;
		}
	}

	// �u�b�N�}�[�N����
	if (bIsExistInMRU) {
		if (GetDllShareData().common.file.GetRestoreBookmarks()) {
			BookmarkManager(pDoc->docLineMgr).SetBookMarks(eiOld.szMarkLines);
		}
	}else {
		eiOld.szMarkLines[0] = 0;
	}

	// MRU���X�g�ւ̓o�^
	EditInfo	eiNew;
	pDoc->GetEditInfo(&eiNew);
	// �u�b�N�}�[�N�̕ێ�(�G�f�B�^���������Ƃ��u�b�N�}�[�N�������邽��)
	if (bIsExistInMRU) {
		if (GetDllShareData().common.file.GetRestoreBookmarks()) {
			// SetBookMarks�Ńf�[�^��NUL��؂�ɏ���������Ă���̂ōĎ擾
			mru.GetEditInfo(pDoc->docFile.GetFilePath(), &eiOld);
			auto_strcpy(eiNew.szMarkLines, eiOld.szMarkLines);
		}
	}
	mru.Add(&eiNew);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �N���[�Y�O��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CallbackResultType MruListener::OnBeforeClose()
{
	_HoldBookmarks_And_AddToMRU();

	return CallbackResultType::Continue;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �w���p                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�J�����g�t�@�C����MRU�ɓo�^����B
	�u�b�N�}�[�N���ꏏ�ɓo�^����B
*/
void MruListener::_HoldBookmarks_And_AddToMRU()
{
	// EditInfo�擾
	EditDoc* pDoc = GetListeningDoc();
	EditInfo fi;
	pDoc->GetEditInfo(&fi);

	// �u�b�N�}�[�N���̕ۑ�
	wcscpy_s(fi.szMarkLines, BookmarkManager(pDoc->docLineMgr).GetBookMarks());

	// MRU���X�g�ɓo�^
	MruFile mru;
	mru.Add(&fi);
}

