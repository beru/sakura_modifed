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
#include "doc/DocListener.h" // �e�N���X
#include "SaveAgent.h"
#include "doc/EditDoc.h"
#include "doc/DocVisitor.h"
#include "window/EditWnd.h"
#include "uiparts/VisualProgress.h"
#include "uiparts/WaitCursor.h"
#include "WriteManager.h"
#include "io/BinaryStream.h"
#include "EditApp.h"
#include "_main/AppMode.h"
#include "env/ShareData.h"

SaveAgent::SaveAgent()
{
}


CallbackResultType SaveAgent::OnCheckSave(SaveInfo* pSaveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	//	Jun.  5, 2004 genta
	//	�r���[���[�h�̃`�F�b�N��EditDoc����㏑���ۑ������Ɉړ�
	//	�����ŏ㏑�������̂�h��
	if (AppMode::getInstance().IsViewMode()
		&& pSaveInfo->IsSamePath(pDoc->docFile.GetFilePath())
	) {
		ErrorBeep();
		TopErrorMessage(EditWnd::getInstance().GetHwnd(), LS(STR_SAVEAGENT_VIEW_FILE));
		return CallbackResultType::Interrupt;
	}

	// ���E�B���h�E�ŊJ���Ă��邩�m�F����	// 2009.04.07 ryoji
	if (!pSaveInfo->IsSamePath(pDoc->docFile.GetFilePath())) {
		HWND hwndOwner;
		if (ShareData::getInstance().IsPathOpened(pSaveInfo->filePath, &hwndOwner)) {
			ErrorMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_SAVEAGENT_OTHER),
				(LPCTSTR)pSaveInfo->filePath
			);
			return CallbackResultType::Interrupt;
		}
	}

	// �����\�`�F�b�N ######### �X�}�[�g����Ȃ��B�z���g�͏������ݎ��G���[�`�F�b�N���o�@�\��p�ӂ�����
	{
		// ���b�N�͈ꎞ�I�ɉ������ă`�F�b�N����i�`�F�b�N�����Ɍ�߂�ł��Ȃ��Ƃ���܂Ői�߂�����S�j
		// �� ���b�N���Ă��Ă��t�@�C��������A�N�Z�X���̕ύX�ɂ���ď������߂Ȃ��Ȃ��Ă��邱�Ƃ�����
		bool bLock = (pSaveInfo->IsSamePath(pDoc->docFile.GetFilePath()) && pDoc->docFile.IsFileLocking());
		if (bLock) {
			pDoc->docFileOperation.DoFileUnlock();
		}
		try {
			bool bExist = fexist(pSaveInfo->filePath);
			Stream out(pSaveInfo->filePath, _T("ab"), true);	// ���ۂ̕ۑ��� "wb" ���������� "ab"�i�t�@�C�����e�͔j�����Ȃ��j�Ń`�F�b�N����	// 2009.08.21 ryoji
			out.Close();
			if (!bExist) {
				::DeleteFile(pSaveInfo->filePath);
			}
		}catch (Error_FileOpen) {
			// �� ���Ƃ��㏑���ۑ��̏ꍇ�ł������ł̎��s�ł͏����݋֎~�ւ͑J�ڂ��Ȃ�
			if (bLock) {
				pDoc->docFileOperation.DoFileLock(false);
			}
			ErrorMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_SAVEAGENT_OTHER_APP),
				pSaveInfo->filePath.c_str()
			);
			return CallbackResultType::Interrupt;
		}
		if (bLock) {
			pDoc->docFileOperation.DoFileLock(false);
		}
	}
	return CallbackResultType::Continue;
}

void SaveAgent::OnBeforeSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// ���s�R�[�h����
	DocVisitor(*pDoc).SetAllEol(saveInfo.eol);
}

void SaveAgent::OnSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// �J�L�R
	WriteManager writer;
	EditApp::getInstance().pVisualProgress->ProgressListener::Listen(&writer);
	writer.WriteFile_From_CDocLineMgr(
		pDoc->docLineMgr,
		saveInfo
	);

	// �Z�[�u���̊m��
	pDoc->SetFilePathAndIcon(saveInfo.filePath);
	pDoc->docFile.SetCodeSet(saveInfo.eCharCode, saveInfo.bBomExist);
	if (saveInfo.eol.IsValid()) {
		pDoc->docEditor.SetNewLineCode(saveInfo.eol);
	}
}

void SaveAgent::OnAfterSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	/* �X�V��̃t�@�C�������̎擾
	 * CloseHandle�O�ł�FlushFileBuffers���Ă�ł��^�C���X�^���v���X�V
	 * ����Ȃ����Ƃ�����B
	 */
	GetLastWriteTimestamp(pDoc->docFile.GetFilePath(), &pDoc->docFile.GetFileTime());

	// �^�C�v�ʐݒ�̕ύX���w���B
	// �㏑���i�����I�ȏ㏑���⎩���ۑ��j�ł͕ύX���Ȃ�
	// ---> �㏑���̏ꍇ�͈ꎞ�I�Ȑ܂�Ԃ����ύX��^�u���ύX���ێ������܂܂ɂ���
	if (!saveInfo.bOverwriteMode) {
		pDoc->OnChangeSetting();
	}
}

void SaveAgent::OnFinalSave(SaveResultType eSaveResult)
{
}

