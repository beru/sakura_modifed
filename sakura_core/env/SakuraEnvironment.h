#pragma once

#include <string>

class EditWnd;

class SakuraEnvironment {
public:
	static EditWnd& GetMainWindow();
	static void ExpandParameter(const wchar_t* pszSource, wchar_t* pszBuffer, int nBufferLen);
	static std::tstring GetDlgInitialDir(bool bControlProcess = false);

	static void ResolvePath(TCHAR* pszPath); // ショートカットの解決とロングファイル名へ変換を行う。
private:
	static const wchar_t* _ExParam_SkipCond(const wchar_t* pszSource, int part); // ExpandParameter補助関数
	static int _ExParam_Evaluate(const wchar_t* pCond);
};


// ウィンドウ管理
// 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる
bool IsSakuraMainWindow(HWND hWnd);

