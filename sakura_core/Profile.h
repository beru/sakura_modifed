#pragma once

// INIファイル入出力

#include <Windows.h>
#include <string>
#include <vector>
#include <map>

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief INIファイル入出力
*/
class Profile {
	// 文字列型
	typedef std::wstring wstring;
	typedef std::string string;

	typedef std::pair<wstring, wstring> pair_str_str;
	typedef std::map<wstring, wstring> map_str_str;
	struct Section {
		Section(const wstring& name)
			:
			strSectionName(name)
		{
		}
		wstring     strSectionName;
		map_str_str mapEntries;
	};

public:
	Profile() {}
	~Profile() {}
	void Init(void);
	bool IsReadingMode(void) { return bRead; }
	void SetReadingMode(void) { bRead = true; }
	void SetWritingMode(void) { bRead = false; }
	bool ReadProfile(const TCHAR*);
	bool ReadProfileRes(const TCHAR*, const TCHAR*, std::vector<std::wstring>* = nullptr);
	bool WriteProfile(const TCHAR*, const wchar_t* pszComment);

	void Dump(void);

protected:
	void ReadOneline(const wstring& line);
	void ReadOneline(const wchar_t* line, size_t length);
	bool _WriteFile(const tstring& strFilename, const std::vector<wstring>& vecLine);

	bool GetProfileDataImp(const wstring& strSectionName, const wstring& strEntryKey, wstring& strEntryValue);
	bool SetProfileDataImp(const wstring& strSectionName, const wstring& strEntryKey, const wstring& strEntryValue);

protected:
	// メンバ変数
	tstring					strProfileName;	// 最後に読み書きしたファイル名
	std::vector<Section>	profileData;
	bool					bRead;			// モード(true=読み込み/false=書き出し)
};

#define _INI_T LTEXT



