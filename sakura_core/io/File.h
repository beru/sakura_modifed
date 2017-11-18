#pragma once

#include "basis/MyString.h" //FilePath
#include "util/fileUtil.h"

// �t�@�C���̔r�����䃂�[�h
enum class FileShareMode {
	NonExclusive,	// �r�����䂵�Ȃ�
	DenyWrite,		// ���v���Z�X����̏㏑�����֎~
	DenyReadWrite,	// ���v���Z�X����̓ǂݏ������֎~
};

class File {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	File(LPCTSTR pszPath = NULL);
	virtual ~File();
	// �p�X
	const FilePath& GetFilePathClass() const { return szFilePath; }
	LPCTSTR GetFilePath() const { return szFilePath; }
	// �ݒ�
	void SetFilePath(LPCTSTR pszPath) { szFilePath.Assign(pszPath); }
	// �e�픻��
	bool IsFileExist() const;
	bool HasWritablePermission() const;
	bool IsFileWritable() const;
	bool IsFileReadable() const;
	// ���b�N
	bool FileLock(FileShareMode eShareMode, bool bMsg);	// �t�@�C���̔r�����b�N
	void FileUnlock();						// �t�@�C���̔r�����b�N����
	bool IsFileLocking() const { return hLockedFile != INVALID_HANDLE_VALUE; }
	FileShareMode GetShareMode() const { return nFileShareModeOld; }
	void SetShareMode(FileShareMode eShareMode) { nFileShareModeOld = eShareMode; }
private:
	FilePath	szFilePath;					// �t�@�C���p�X
	HANDLE		hLockedFile;				// ���b�N���Ă���t�@�C���̃n���h��
	FileShareMode	nFileShareModeOld;		// �t�@�C���̔r�����䃂�[�h
};


// �ꎞ�t�@�C��
class TmpFile {
public:
	TmpFile() { fp = tmpfile(); }
	~TmpFile() { fclose(fp); }
	FILE* GetFilePointer() const { return fp; }
private:
	FILE* fp;
};

