#pragma once

void GetAppVersionInfo(HINSTANCE, int, DWORD*, DWORD*);	// ���\�[�X���琻�i�o�[�W�����̎擾

HICON GetAppIcon(HINSTANCE hInst, int nResource, const TCHAR* szFile, bool bSmall = false);

DWORD GetDllVersion(LPCTSTR lpszDllName);	// �V�F����R�����R���g���[�� DLL �̃o�[�W�����ԍ����擾

void ChangeCurrentDirectoryToExeDir();

// �J�����g�f�B���N�g���ړ��@�\�tLoadLibrary
HMODULE LoadLibraryExedir(LPCTSTR pszDll);

