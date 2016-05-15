#include "StdAfx.h"
#include "io/File.h"
#include "window/EditWnd.h" // �ύX�\��
#include <io.h>

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

File::File(LPCTSTR pszPath)
	:
	hLockedFile(INVALID_HANDLE_VALUE),
	nFileShareModeOld(FileShareMode::NonExclusive)
{
	if (pszPath) {
		SetFilePath(pszPath);
	}
}

File::~File()
{
	FileUnlock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �e�픻��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool File::IsFileExist() const
{
	return fexist(GetFilePath());
}

bool File::HasWritablePermission() const
{
	return _taccess(GetFilePath(), 2) != -1;
}

bool File::IsFileWritable() const
{
	// �������߂邩����
	// Note. ���̃v���Z�X�������I�ɏ������݋֎~���Ă��邩�ǂ���
	//       �� GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE �Ń`�F�b�N����
	//          ���ۂ̃t�@�C���ۑ�������Ɠ����� _tfopen �� _T("wb") ���g�p���Ă���
	HANDLE hFile = CreateFile(
		this->GetFilePath(),			// �t�@�C����
		GENERIC_WRITE,					// �������[�h
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// �ǂݏ������L
		NULL,							// ����̃Z�L�����e�B�L�q�q
		OPEN_EXISTING,					// �t�@�C�������݂��Ȃ���Ύ��s
		FILE_ATTRIBUTE_NORMAL,			// ���ɑ����͎w�肵�Ȃ�
		NULL							// �e���v���[�g����
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	CloseHandle(hFile);
	return true;
}

bool File::IsFileReadable() const
{
	HANDLE hTest = CreateFile(
		this->GetFilePath(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if (hTest == INVALID_HANDLE_VALUE) {
		// �ǂݍ��݃A�N�Z�X�����Ȃ�
		return false;
	}
	CloseHandle(hTest);
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���b�N                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �t�@�C���̔r�����b�N����
void File::FileUnlock()
{
	// �N���[�Y
	if (hLockedFile != INVALID_HANDLE_VALUE) {
		::CloseHandle(hLockedFile);
		hLockedFile = INVALID_HANDLE_VALUE;
	}
}

// �t�@�C���̔r�����b�N
bool File::FileLock(FileShareMode eShareMode, bool bMsg)
{
	// ���b�N����
	FileUnlock();

	// �t�@�C���̑��݃`�F�b�N
	if (!this->IsFileExist()) {
		return false;
	}

	// ���[�h�ݒ�
	if (eShareMode == FileShareMode::NonExclusive) {
		return true;
	}
	// �t���O
	DWORD dwShareMode = 0;
	switch (eShareMode) {
	case FileShareMode::NonExclusive:	return true;										break; // �r�����䖳��
	case FileShareMode::DenyReadWrite:	dwShareMode = 0;									break; // �ǂݏ����֎~�����L����
	case FileShareMode::DenyWrite:		dwShareMode = FILE_SHARE_READ;						break; // �������݋֎~���ǂݍ��݂̂ݔF�߂�
	default:							dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;	break; // �֎~�����Ȃ����ǂݏ������ɔF�߂�
	}

	// �I�[�v��
	hLockedFile = CreateFile(
		this->GetFilePath(),			// �t�@�C����
		GENERIC_READ,					// �ǂݏ����^�C�v
		dwShareMode,					// ���L���[�h
		NULL,							// ����̃Z�L�����e�B�L�q�q
		OPEN_EXISTING,					// �t�@�C�������݂��Ȃ���Ύ��s
		FILE_ATTRIBUTE_NORMAL,			// ���ɑ����͎w�肵�Ȃ�
		NULL							// �e���v���[�g����
	);

	// ����
	if (hLockedFile == INVALID_HANDLE_VALUE && bMsg) {
		const TCHAR* pszMode;
		switch (eShareMode) {
		case FileShareMode::DenyReadWrite:	pszMode = LS(STR_EXCLU_DENY_READWRITE); break;
		case FileShareMode::DenyWrite:		pszMode = LS(STR_EXCLU_DENY_WRITE); break;
		default:							pszMode = LS(STR_EXCLU_UNDEFINED); break;
		}
		TopWarningMessage(
			EditWnd::getInstance().GetHwnd(),
			LS(STR_FILE_LOCK_ERR),
			GetFilePathClass().IsValidPath() ? GetFilePath() : LS(STR_NO_TITLE1),
			pszMode
		);
		return false;
	}

	return true;
}


