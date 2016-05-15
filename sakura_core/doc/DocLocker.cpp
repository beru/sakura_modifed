#include "StdAfx.h"
#include "DocLocker.h"
#include "DocFile.h"
#include "window/EditWnd.h"



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

DocLocker::DocLocker()
	:
	bIsDocWritable(true)
{
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void DocLocker::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// �������߂邩����
	CheckWritable(!loadInfo.bViewMode && !loadInfo.bWritableNoMsg);
	if (!bIsDocWritable) {
		return;
	}

	// �t�@�C���̔r�����b�N
	pDoc->docFileOperation.DoFileLock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void DocLocker::OnBeforeSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// �t�@�C���̔r�����b�N����
	pDoc->docFileOperation.DoFileUnlock();
}

void DocLocker::OnAfterSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// �������߂邩����
	bIsDocWritable = true;

	// �t�@�C���̔r�����b�N
	pDoc->docFileOperation.DoFileLock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�F�b�N                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �������߂邩����
void DocLocker::CheckWritable(bool bMsg)
{
	EditDoc* pDoc = GetListeningDoc();

	// �t�@�C�������݂��Ȃ��ꍇ (�u�J���v�ŐV�����t�@�C�����쐬��������) �́A�ȉ��̏����͍s��Ȃ�
	if (!fexist(pDoc->docFile.GetFilePath())) {
		bIsDocWritable = true;
		return;
	}

	// �ǂݎ���p�t�@�C���̏ꍇ�́A�ȉ��̏����͍s��Ȃ�
	if (!pDoc->docFile.HasWritablePermission()) {
		bIsDocWritable = false;
		return;
	}

	// �������߂邩����
	DocFile& docFile = pDoc->docFile;
	bIsDocWritable = docFile.IsFileWritable();
	if (!bIsDocWritable && bMsg) {
		// �r������Ă���ꍇ�������b�Z�[�W���o��
		// ���̑��̌����i�t�@�C���V�X�e���̃Z�L�����e�B�ݒ�Ȃǁj�ł͓ǂݎ���p�Ɠ��l�Ƀ��b�Z�[�W���o���Ȃ�
		if (::GetLastError() == ERROR_SHARING_VIOLATION) {
			TopWarningMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_ERR_DLGEDITDOC21),	// "%ts\n�͌��ݑ��̃v���Z�X�ɂ���ď����݂��֎~����Ă��܂��B"
				docFile.GetFilePathClass().IsValidPath() ? docFile.GetFilePath() : LS(STR_NO_TITLE1)	// "(����)"
			);
		}
	}
}

