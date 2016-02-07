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
#include "CAutoReloadAgent.h"
// #include "doc/CEditDoc.h"	//  in under CEditWnd.h
#include "window/CEditWnd.h"
#include "dlg/CDlgFileUpdateQuery.h"
#include "sakura_rc.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CAutoReloadAgent::CAutoReloadAgent()
	:
	m_eWatchUpdate(WU_QUERY),
	m_nPauseCount(0)
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CAutoReloadAgent::OnBeforeSave(const SaveInfo& sSaveInfo)
{
	//	Sep. 7, 2003 genta
	//	�ۑ�����������܂ł̓t�@�C���X�V�̒ʒm��}������
	PauseWatching();
}

void CAutoReloadAgent::OnAfterSave(const SaveInfo& sSaveInfo)
{
	//	Sep. 7, 2003 genta
	//	�t�@�C���X�V�̒ʒm�����ɖ߂�
	ResumeWatching();

	// ���O��t���ĕۑ�����ă��[�h���������ꂽ���̕s��������ǉ��iANSI�łƂ̍��فj	// 2009.08.12 ryoji
	if (!sSaveInfo.bOverwriteMode) {
		m_eWatchUpdate = WU_QUERY;	// �u���O��t���ĕۑ��v�őΏۃt�@�C�����ύX���ꂽ�̂ōX�V�Ď����@���f�t�H���g�ɖ߂�
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CAutoReloadAgent::OnAfterLoad(const LoadInfo& sLoadInfo)
{
	//pcDoc->m_cDocFile.m_sFileInfo.cFileTime.SetFILETIME(ftime); //#####���ɐݒ�ς݂̂͂�
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �e�픻��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool CAutoReloadAgent::_ToDoChecking() const
{
	const CommonSetting_File& setting = GetDllShareData().m_common.m_sFile;
	HWND hwndActive = ::GetActiveWindow();
	if (0
		|| IsPausing()
		|| !setting.m_bCheckFileTimeStamp	// �X�V�̊Ď��ݒ�
		|| m_eWatchUpdate == WU_NONE
		|| setting.m_nFileShareMode != SHAREMODE_NOT_EXCLUSIVE	 // �t�@�C���̔r�����䃂�[�h
		|| !hwndActive		// �A�N�e�B�u�H
		|| hwndActive != CEditWnd::getInstance()->GetHwnd()
		|| !GetListeningDoc()->m_cDocFile.GetFilePathClass().IsValidPath()
		|| GetListeningDoc()->m_cDocFile.IsFileTimeZero()	// ���ݕҏW���̃t�@�C���̃^�C���X�^���v
		|| GetListeningDoc()->m_pcEditWnd->m_pPrintPreview	// ����v���r���[��	2013/5/8 Uchi
	) {
		return false;
	}
	return true;
}

bool CAutoReloadAgent::_IsFileUpdatedByOther(FILETIME* pNewFileTime) const
{
	// �t�@�C���X�^���v���`�F�b�N����
	// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX�i�t�@�C�������b�N����Ă��Ă��^�C���X�^���v�擾�\�j
	CFileTime ftime;
	if (GetLastWriteTimestamp(GetListeningDoc()->m_cDocFile.GetFilePath(), &ftime)) {
		if (::CompareFileTime(
				&GetListeningDoc()->m_cDocFile.GetFileTime().GetFILETIME(),
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
void CAutoReloadAgent::CheckFileTimeStamp()
{
	// ���ҏW�ōă��[�h���̒x��
	if (m_eWatchUpdate == WU_AUTOLOAD) {
		if (++m_nDelayCount < GetDllShareData().m_common.m_sFile.m_nAutoloadDelay) {
			return;
		}
		m_nDelayCount = 0;
	}

	if (!_ToDoChecking()) {
		return;
	}

	CEditDoc* pcDoc = GetListeningDoc();

	// �^�C���X�^���v�Ď�
	FILETIME ftime;
	if (!_IsFileUpdatedByOther(&ftime)) {
		return;
	}
	pcDoc->m_cDocFile.SetFileTime(ftime); // �^�C���X�^���v�X�V

	//	From Here Dec. 4, 2002 genta
	switch (m_eWatchUpdate) {
	case WU_NOTIFY:
		{
			// �t�@�C���X�V�̂��m�点 -> �X�e�[�^�X�o�[
			TCHAR szText[40];
			const CFileTime& ctime = pcDoc->m_cDocFile.GetFileTime();
			auto_sprintf_s(szText, LS(STR_AUTORELOAD_NOFITY), ctime->wHour, ctime->wMinute, ctime->wSecond);
			pcDoc->m_pcEditWnd->SendStatusMessage(szText);
		}
		break;
	case WU_AUTOLOAD:		// �Ȍ㖢�ҏW�ōă��[�h
		if (!pcDoc->m_cDocEditor.IsModified()) {
			PauseWatching(); // �X�V�Ď��̗}��

			// ����t�@�C���̍ăI�[�v��
			pcDoc->m_cDocFileOperation.ReloadCurrentFile(pcDoc->m_cDocFile.GetCodeSet());
			m_eWatchUpdate = WU_AUTOLOAD;

			ResumeWatching(); // �Ď��ĊJ
			break;
		}
		// through
	default:
		{
			PauseWatching(); // �X�V�Ď��̗}��

			CDlgFileUpdateQuery dlg(pcDoc->m_cDocFile.GetFilePath(), pcDoc->m_cDocEditor.IsModified());
			int result = dlg.DoModal(
				G_AppInstance(),
				CEditWnd::getInstance()->GetHwnd(),
				IDD_FILEUPDATEQUERY,
				0
			);

			switch (result) {
			case 1:	// �ēǍ�
				// ����t�@�C���̍ăI�[�v��
				pcDoc->m_cDocFileOperation.ReloadCurrentFile(pcDoc->m_cDocFile.GetCodeSet());
				m_eWatchUpdate = WU_QUERY;
				break;
			case 2:	// �Ȍ�ʒm���b�Z�[�W�̂�
				m_eWatchUpdate = WU_NOTIFY;
				break;
			case 3:	// �Ȍ�X�V���Ď����Ȃ�
				m_eWatchUpdate = WU_NONE;
				break;
			case 4:	// �Ȍ㖢�ҏW�ōă��[�h
				// ����t�@�C���̍ăI�[�v��
				pcDoc->m_cDocFileOperation.ReloadCurrentFile(pcDoc->m_cDocFile.GetCodeSet());
				m_eWatchUpdate = WU_AUTOLOAD;
				m_nDelayCount = 0;
				break;
			case 0:	// CLOSE
			default:
				m_eWatchUpdate = WU_QUERY;
				break;
			}

			ResumeWatching(); // �Ď��ĊJ
		}
		break;
	}
	//	To Here Dec. 4, 2002 genta
}

