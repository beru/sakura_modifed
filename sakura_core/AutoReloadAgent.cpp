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
#include "AutoReloadAgent.h"
// #include "doc/EditDoc.h"	//  in under EditWnd.h
#include "window/EditWnd.h"
#include "dlg/DlgFileUpdateQuery.h"
#include "sakura_rc.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

AutoReloadAgent::AutoReloadAgent()
	:
	m_watchUpdateType(WatchUpdateType::Query),
	m_nPauseCount(0)
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnBeforeSave(const SaveInfo& saveInfo)
{
	//	Sep. 7, 2003 genta
	//	�ۑ�����������܂ł̓t�@�C���X�V�̒ʒm��}������
	PauseWatching();
}

void AutoReloadAgent::OnAfterSave(const SaveInfo& saveInfo)
{
	//	Sep. 7, 2003 genta
	//	�t�@�C���X�V�̒ʒm�����ɖ߂�
	ResumeWatching();

	// ���O��t���ĕۑ�����ă��[�h���������ꂽ���̕s��������ǉ��iANSI�łƂ̍��فj	// 2009.08.12 ryoji
	if (!saveInfo.bOverwriteMode) {
		m_watchUpdateType = WatchUpdateType::Query;	// �u���O��t���ĕۑ��v�őΏۃt�@�C�����ύX���ꂽ�̂ōX�V�Ď����@���f�t�H���g�ɖ߂�
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnAfterLoad(const LoadInfo& loadInfo)
{
	//pDoc->m_docFile.m_sFileInfo.cFileTime.SetFILETIME(ftime); //#####���ɐݒ�ς݂̂͂�
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �e�픻��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool AutoReloadAgent::_ToDoChecking() const
{
	const CommonSetting_File& setting = GetDllShareData().common.file;
	HWND hwndActive = ::GetActiveWindow();
	if (0
		|| IsPausing()
		|| !setting.bCheckFileTimeStamp	// �X�V�̊Ď��ݒ�
		|| m_watchUpdateType == WatchUpdateType::None
		|| setting.nFileShareMode != FileShareMode::NonExclusive	 // �t�@�C���̔r�����䃂�[�h
		|| !hwndActive		// �A�N�e�B�u�H
		|| hwndActive != EditWnd::getInstance()->GetHwnd()
		|| !GetListeningDoc()->m_docFile.GetFilePathClass().IsValidPath()
		|| GetListeningDoc()->m_docFile.IsFileTimeZero()	// ���ݕҏW���̃t�@�C���̃^�C���X�^���v
		|| GetListeningDoc()->m_pEditWnd->m_pPrintPreview	// ���Preview��	2013/5/8 Uchi
	) {
		return false;
	}
	return true;
}

bool AutoReloadAgent::_IsFileUpdatedByOther(FILETIME* pNewFileTime) const
{
	// �t�@�C���X�^���v���`�F�b�N����
	// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX�i�t�@�C�������b�N����Ă��Ă��^�C���X�^���v�擾�\�j
	FileTime ftime;
	if (GetLastWriteTimestamp(GetListeningDoc()->m_docFile.GetFilePath(), &ftime)) {
		if (::CompareFileTime(
				&GetListeningDoc()->m_docFile.GetFileTime().GetFILETIME(),
				&ftime.GetFILETIME()
			) != 0
		) {	//	Aug. 13, 2003 wmlhq �^�C���X�^���v���Â��ύX����Ă���ꍇ�����o�ΏۂƂ���
			*pNewFileTime = ftime.GetFILETIME();
			return true;
		}
	}
	return false;
}

// �t�@�C���̃^�C���X�^���v�̃`�F�b�N����
void AutoReloadAgent::CheckFileTimeStamp()
{
	// ���ҏW�ōă��[�h���̒x��
	if (m_watchUpdateType == WatchUpdateType::AutoLoad) {
		if (++m_nDelayCount < GetDllShareData().common.file.nAutoloadDelay) {
			return;
		}
		m_nDelayCount = 0;
	}

	if (!_ToDoChecking()) {
		return;
	}

	EditDoc* pDoc = GetListeningDoc();

	// �^�C���X�^���v�Ď�
	FILETIME ftime;
	if (!_IsFileUpdatedByOther(&ftime)) {
		return;
	}
	pDoc->m_docFile.SetFileTime(ftime); // �^�C���X�^���v�X�V

	//	From Here Dec. 4, 2002 genta
	switch (m_watchUpdateType) {
	case WatchUpdateType::Notify:
		{
			// �t�@�C���X�V�̂��m�点 -> �X�e�[�^�X�o�[
			TCHAR szText[40];
			const FileTime& time = pDoc->m_docFile.GetFileTime();
			auto_sprintf_s(szText, LS(STR_AUTORELOAD_NOFITY), time->wHour, time->wMinute, time->wSecond);
			pDoc->m_pEditWnd->SendStatusMessage(szText);
		}
		break;
	case WatchUpdateType::AutoLoad:		// �Ȍ㖢�ҏW�ōă��[�h
		if (!pDoc->m_docEditor.IsModified()) {
			PauseWatching(); // �X�V�Ď��̗}��

			// ����t�@�C���̍ăI�[�v��
			pDoc->m_docFileOperation.ReloadCurrentFile(pDoc->m_docFile.GetCodeSet());
			m_watchUpdateType = WatchUpdateType::AutoLoad;

			ResumeWatching(); // �Ď��ĊJ
			break;
		}
		// through
	default:
		{
			PauseWatching(); // �X�V�Ď��̗}��

			DlgFileUpdateQuery dlg(pDoc->m_docFile.GetFilePath(), pDoc->m_docEditor.IsModified());
			int result = dlg.DoModal(
				G_AppInstance(),
				EditWnd::getInstance()->GetHwnd(),
				IDD_FILEUPDATEQUERY,
				0
			);

			switch (result) {
			case 1:	// �ēǍ�
				// ����t�@�C���̍ăI�[�v��
				pDoc->m_docFileOperation.ReloadCurrentFile(pDoc->m_docFile.GetCodeSet());
				m_watchUpdateType = WatchUpdateType::Query;
				break;
			case 2:	// �Ȍ�ʒm���b�Z�[�W�̂�
				m_watchUpdateType = WatchUpdateType::Notify;
				break;
			case 3:	// �Ȍ�X�V���Ď����Ȃ�
				m_watchUpdateType = WatchUpdateType::None;
				break;
			case 4:	// �Ȍ㖢�ҏW�ōă��[�h
				// ����t�@�C���̍ăI�[�v��
				pDoc->m_docFileOperation.ReloadCurrentFile(pDoc->m_docFile.GetCodeSet());
				m_watchUpdateType = WatchUpdateType::AutoLoad;
				m_nDelayCount = 0;
				break;
			case 0:	// CLOSE
			default:
				m_watchUpdateType = WatchUpdateType::Query;
				break;
			}

			ResumeWatching(); // �Ď��ĊJ
		}
		break;
	}
	//	To Here Dec. 4, 2002 genta
}

