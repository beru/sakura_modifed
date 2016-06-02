/*
	Copyright (C) 2002, SUI
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
#pragma once

bool fexist(LPCTSTR pszPath); // �t�@�C���܂��̓f�B���N�g�������݂����true

bool IsFilePath(const wchar_t*, size_t*, int*, bool = true);
bool IsFileExists(const TCHAR* path, bool bFileOnly = false);
bool IsDirectory(LPCTSTR pszPath);	// 2009.08.20 ryoji

// Apr. 30, 2003 genta
// �f�B���N�g���̐[���𒲂ׂ�
int CalcDirectoryDepth(const TCHAR* path);

// 2005.11.26 aroka
bool IsLocalDrive(const TCHAR* pszDrive);

// ���T�N���ˑ�
FILE* _tfopen_absexe(LPCTSTR fname, LPCTSTR mode); // 2003.06.23 Moca
FILE* _tfopen_absini(LPCTSTR fname, LPCTSTR mode, bool bOrExedir = true); // 2007.05.19 ryoji

// �p�X�����񏈗�
void CutLastYenFromDirectoryPath(TCHAR*);						// �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ�
void AddLastYenFromDirectoryPath( CHAR*);						// �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������
void AddLastYenFromDirectoryPath(wchar_t*);						// �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������
void SplitPath_FolderAndFile(const TCHAR*, TCHAR*, TCHAR*);	// �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ���
void Concat_FolderAndFile(const TCHAR*, const TCHAR*, TCHAR*);// �t�H���_�A�t�@�C��������A���������p�X���쐬
BOOL GetLongFileName(const TCHAR*, TCHAR*);					// �����O�t�@�C�������擾����
BOOL CheckEXT(const TCHAR*, const TCHAR*);					// �g���q�𒲂ׂ�
const TCHAR* GetFileTitlePointer(const TCHAR* tszPath);			// �t�@�C���t���p�X���̃t�@�C�������w���|�C���^���擾�B2007.09.20 kobake �쐬
bool _IS_REL_PATH(const TCHAR* path);							// ���΃p�X�����肷��B2003.06.23 Moca

// ���T�N���ˑ�
void GetExedir(LPTSTR pDir, LPCTSTR szFile = NULL);
void GetInidir(LPTSTR pDir, LPCTSTR szFile = NULL); // 2007.05.19 ryoji
void GetInidirOrExedir(LPTSTR pDir, LPCTSTR szFile = NULL, bool bRetExedirIfFileEmpty = false); // 2007.05.22 ryoji

LPCTSTR GetRelPath(LPCTSTR pszPath);

// �t�@�C������
class FileTime {
public:
	FileTime() { ClearFILETIME(); }
	FileTime(const FILETIME& ftime) { SetFILETIME(ftime); }
	// �ݒ�
	void ClearFILETIME() { ftime.dwLowDateTime = ftime.dwHighDateTime = 0; bModified = true; }
	void SetFILETIME(const FILETIME& ftime) { this->ftime = ftime; bModified = true; }
	// �擾
	const FILETIME& GetFILETIME() const { return ftime; }
	const SYSTEMTIME& GetSYSTEMTIME() const {
		// �L���b�V���X�V -> systime, bModified
		if (bModified) {
			bModified = false;
			FILETIME ftimeLocal;
			if (!::FileTimeToLocalFileTime(&ftime, &ftimeLocal) || !::FileTimeToSystemTime(&ftimeLocal, &systime)) {
				memset(&systime, 0, sizeof(systime)); // ���s���[���N���A
			}
		}
		return systime;
	}
	const SYSTEMTIME* operator->() const { return &GetSYSTEMTIME(); }
	// ����
	bool IsZero() const {
		return ftime.dwLowDateTime == 0 && ftime.dwHighDateTime == 0;
	}
protected:
private:
	FILETIME ftime;
	// �L���b�V��
	mutable SYSTEMTIME	systime;
	mutable bool		bModified;
};

bool GetLastWriteTimestamp(const TCHAR* filename, FileTime* pFileTime); //	Oct. 22, 2005 genta

// �����񕪊�
void my_splitpath (const char* comln , char* drv, char* dir, char* fnm, char* ext);
void my_splitpath_w (const wchar_t* comln, wchar_t* drv, wchar_t* dir, wchar_t* fnm, wchar_t* ext);
void my_splitpath_t (const TCHAR* comln, TCHAR* drv, TCHAR* dir, TCHAR* fnm, TCHAR* ext);
#define my_splitpath_t my_splitpath_w

size_t FileMatchScoreSepExt(const TCHAR* file1, const TCHAR* file2);

void GetStrTrancateWidth(TCHAR* dest, size_t nSize, const TCHAR* path, HDC hDC, int nPxWidth);
void GetShortViewPath(TCHAR* dest, size_t nSize, const TCHAR* path, HDC hDC, int nPxWidth, bool bFitMode);

bool ReadFile(const wchar_t* path, std::vector<char>& buff);
bool ReadFileAndUnicodify(const wchar_t* path, std::vector<wchar_t>& buff);

