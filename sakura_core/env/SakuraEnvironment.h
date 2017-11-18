#pragma once

#include <string>

class EditWnd;

class SakuraEnvironment {
public:
	static EditWnd& GetMainWindow();
	static void ExpandParameter(const wchar_t* pszSource, wchar_t* pszBuffer, int nBufferLen);
	static std::tstring GetDlgInitialDir(bool bControlProcess = false);

	static void ResolvePath(TCHAR* pszPath); // �V���[�g�J�b�g�̉����ƃ����O�t�@�C�����֕ϊ����s���B
private:
	static const wchar_t* _ExParam_SkipCond(const wchar_t* pszSource, int part); // ExpandParameter�⏕�֐�
	static int _ExParam_Evaluate(const wchar_t* pCond);
};


// �E�B���h�E�Ǘ�
// �w��E�B���h�E���A�ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ�
bool IsSakuraMainWindow(HWND hWnd);

