#pragma once

bool fexist(LPCTSTR pszPath); // �t�@�C���܂��̓f�B���N�g�������݂����true

bool IsFilePath(const wchar_t*, size_t*, int*, bool = true);
bool IsFileExists(const TCHAR* path, bool bFileOnly = false);
bool IsDirectory(LPCTSTR pszPath);

// �f�B���N�g���̐[���𒲂ׂ�
int CalcDirectoryDepth(const TCHAR* path);

bool IsLocalDrive(const TCHAR* pszDrive);

// ���T�N���ˑ�
FILE* _tfopen_absexe(LPCTSTR fname, LPCTSTR mode);
FILE* _tfopen_absini(LPCTSTR fname, LPCTSTR mode, bool bOrExedir = true);

// �p�X�����񏈗�
void CutLastYenFromDirectoryPath(TCHAR*);						// �t�H���_�̍Ōオ���p����'\\'�̏ꍇ�́A��菜�� "c:\\"���̃��[�g�͎�菜���Ȃ�
void AddLastYenFromDirectoryPath( CHAR*);						// �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������
void AddLastYenFromDirectoryPath(wchar_t*);						// �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������
void SplitPath_FolderAndFile(const TCHAR*, TCHAR*, TCHAR*);	// �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ���
void Concat_FolderAndFile(const TCHAR*, const TCHAR*, TCHAR*);// �t�H���_�A�t�@�C��������A���������p�X���쐬
BOOL GetLongFileName(const TCHAR*, TCHAR*);					// �����O�t�@�C�������擾����
BOOL CheckEXT(const TCHAR*, const TCHAR*);					// �g���q�𒲂ׂ�
const TCHAR* GetFileTitlePointer(const TCHAR* tszPath);			// �t�@�C���t���p�X���̃t�@�C�������w���|�C���^���擾
bool _IS_REL_PATH(const TCHAR* path);							// ���΃p�X�����肷��

// ���T�N���ˑ�
void GetExedir(LPTSTR pDir, LPCTSTR szFile = NULL);
void GetInidir(LPTSTR pDir, LPCTSTR szFile = NULL);
void GetInidirOrExedir(LPTSTR pDir, LPCTSTR szFile = NULL, bool bRetExedirIfFileEmpty = false);

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

bool GetLastWriteTimestamp(const TCHAR* filename, FileTime* pFileTime);

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

