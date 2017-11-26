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
	watchUpdateType(WatchUpdateType::Query),
	nPauseCount(0),
	nDelayCount(0)
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnBeforeSave(const SaveInfo& saveInfo)
{
	// �ۑ�����������܂ł̓t�@�C���X�V�̒ʒm��}������
	PauseWatching();
}

void AutoReloadAgent::OnAfterSave(const SaveInfo& saveInfo)
{
	// �t�@�C���X�V�̒ʒm�����ɖ߂�
	ResumeWatching();

	if (!saveInfo.bOverwriteMode) {
		watchUpdateType = WatchUpdateType::Query;	// �u���O��t���ĕۑ��v�őΏۃt�@�C�����ύX���ꂽ�̂ōX�V�Ď����@���f�t�H���g�ɖ߂�
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void AutoReloadAgent::OnAfterLoad(const LoadInfo& loadInfo)
{
	//pDoc->docFile.sFileInfo.cFileTime.SetFILETIME(ftime); //#####���ɐݒ�ς݂̂͂�
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
		|| watchUpdateType == WatchUpdateType::None
		|| setting.nFileShareMode != FileShareMode::NonExclusive	 // �t�@�C���̔r�����䃂�[�h
		|| !hwndActive		// �A�N�e�B�u�H
		|| hwndActive != EditWnd::getInstance().GetHwnd()
		|| !GetListeningDoc()->docFile.GetFilePathClass().IsValidPath()
		|| GetListeningDoc()->docFile.IsFileTimeZero()	// ���ݕҏW���̃t�@�C���̃^�C���X�^���v
		|| GetListeningDoc()->pEditWnd->pPrintPreview	// ���Preview��
	) {
		return false;
	}
	return true;
}

bool AutoReloadAgent::_IsFileUpdatedByOther(FILETIME* pNewFileTime) const
{
	// �t�@�C���X�^���v���`�F�b�N����
	FileTime ftime;
	if (GetLastWriteTimestamp(GetListeningDoc()->docFile.GetFilePath(), &ftime)) {
		if (::CompareFileTime(
				&GetListeningDoc()->docFile.GetFileTime().GetFILETIME(),
				&ftime.GetFILETIME()
			) != 0
		) {	// �^�C���X�^���v���Â��ύX����Ă���ꍇ�����o�ΏۂƂ���
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
	if (watchUpdateType == WatchUpdateType::AutoLoad) {
		if (++nDelayCount < GetDllShareData().common.file.nAutoloadDelay) {
			return;
		}
		nDelayCount = 0;
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
	pDoc->docFile.SetFileTime(ftime); // �^�C���X�^���v�X�V

	switch (watchUpdateType) {
	case WatchUpdateType::Notify:
		{
			// �t�@�C���X�V�̂��m�点 -> �X�e�[�^�X�o�[
			TCHAR szText[40];
			const FileTime& time = pDoc->docFile.GetFileTime();
			auto_sprintf_s(szText, LS(STR_AUTORELOAD_NOFITY), time->wHour, time->wMinute, time->wSecond);
			pDoc->pEditWnd->SendStatusMessage(szText);
		}
		break;
	case WatchUpdateType::AutoLoad:		// �Ȍ㖢�ҏW�ōă��[�h
		if (!pDoc->docEditor.IsModified()) {
			PauseWatching(); // �X�V�Ď��̗}��

			// ����t�@�C���̍ăI�[�v��
			pDoc->docFileOperation.ReloadCurrentFile(pDoc->docFile.GetCodeSet());
			watchUpdateType = WatchUpdateType::AutoLoad;

			ResumeWatching(); // �Ď��ĊJ
			break;
		}
		// through
	default:
		{
			PauseWatching(); // �X�V�Ď��̗}��

			DlgFileUpdateQuery dlg(pDoc->docFile.GetFilePath(), pDoc->docEditor.IsModified());
			INT_PTR result = dlg.DoModal(
				G_AppInstance(),
				EditWnd::getInstance().GetHwnd(),
				IDD_FILEUPDATEQUERY,
				0
			);

			switch (result) {
			case 1:	// �ēǍ�
				// ����t�@�C���̍ăI�[�v��
				pDoc->docFileOperation.ReloadCurrentFile(pDoc->docFile.GetCodeSet());
				watchUpdateType = WatchUpdateType::Query;
				break;
			case 2:	// �Ȍ�ʒm���b�Z�[�W�̂�
				watchUpdateType = WatchUpdateType::Notify;
				break;
			case 3:	// �Ȍ�X�V���Ď����Ȃ�
				watchUpdateType = WatchUpdateType::None;
				break;
			case 4:	// �Ȍ㖢�ҏW�ōă��[�h
				// ����t�@�C���̍ăI�[�v��
				pDoc->docFileOperation.ReloadCurrentFile(pDoc->docFile.GetCodeSet());
				watchUpdateType = WatchUpdateType::AutoLoad;
				nDelayCount = 0;
				break;
			case 0:	// CLOSE
			default:
				watchUpdateType = WatchUpdateType::Query;
				break;
			}

			ResumeWatching(); // �Ď��ĊJ
		}
		break;
	}
}

