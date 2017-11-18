// インポート、エクスポートマネージャ

#pragma once

#include "DataProfile.h"
#include "env/DllSharedData.h"

using std::wstring;

class ImpExpManager {
public:
	bool ImportUI(HINSTANCE, HWND);
	bool ExportUI(HINSTANCE, HWND);
	virtual bool ImportAscertain(HINSTANCE, HWND, const wstring&, wstring&);
	virtual bool Import(const wstring&, wstring&) = 0;
	virtual bool Export(const wstring&, wstring&) = 0;
	// ファイル名の初期値を設定
	void SetBaseName(const wstring&);
	// フルパス名を取得
	inline wstring GetFullPath() {
		return to_wchar(GetDllShareData().history.szIMPORTFOLDER) + sOriginName;
	}
	// フルパス名を取得
	inline wstring MakeFullPath(wstring sFileName) {
		return to_wchar(GetDllShareData().history.szIMPORTFOLDER) + sFileName;
	}
	// ファイル名を取得
	inline wstring GetFileName() { return sOriginName; }

protected:
	// Import Folderの設定
	inline void SetImportFolder(const TCHAR* szPath) {
		// ファイルのフルパスをフォルダとファイル名に分割
		// [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt]
		::SplitPath_FolderAndFile(szPath, GetDllShareData().history.szIMPORTFOLDER, NULL);
		_tcscat(GetDllShareData().history.szIMPORTFOLDER, _T("\\"));
	}

	// デフォルト拡張子の取得(「*.txt」形式)
	virtual const TCHAR* GetDefaultExtension();
	// デフォルト拡張子の取得(「txt」形式)
	virtual const wchar_t* GetOriginExtension();

protected:
	wstring		sBase;
	wstring		sOriginName;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          タイプ別設定                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpType : public ImpExpManager {
public:
	// Constructor
	ImpExpType(int nIdx, TypeConfig& types, HWND hwndList)
		:
		nIdx(nIdx),
		types(types),
		hwndList(hwndList)
	{
		// 共有データ構造体のアドレスを返す
		pShareData = &GetDllShareData();
	}

public:
	bool ImportAscertain(HINSTANCE, HWND, const wstring&, wstring&);
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }
	bool IsAddType() { return bAddType; }

private:
	// インターフェース用
	int 			nIdx;
	TypeConfig&		types;
	HWND			hwndList;

	// 内部使用
	DllSharedData*	pShareData;
	int				nColorType;
	wstring 		sColorFile;
	bool			bAddType;
	DataProfile		profile;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          カラー                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpColors : public ImpExpManager {
public:
	// Constructor
	ImpExpColors(ColorInfo * psColorInfoArr)
		:
		colorInfoArr(psColorInfoArr)
	{
	}

public:
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.col"); }
	const wchar_t* GetOriginExtension()	{ return L"col"; }

private:
	ColorInfo*		colorInfoArr;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    正規表現キーワード                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpRegex : public ImpExpManager {
public:
	// Constructor
	ImpExpRegex(TypeConfig& types)
		:
		types(types)
	{
	}

public:
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.rkw"); }
	const wchar_t* GetOriginExtension()	{ return L"rkw"; }

private:
	TypeConfig&	types;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キーワードヘルプ                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpKeyHelp : public ImpExpManager {
public:
	// Constructor
	ImpExpKeyHelp(TypeConfig& types)
		:
		types(types)
	{
	}

public:
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.txt"); }
	const wchar_t* GetOriginExtension()	{ return L"txt"; }

private:
	TypeConfig&	types;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     キー割り当て                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpKeybind : public ImpExpManager {
public:
	// Constructor
	ImpExpKeybind(CommonSetting& common)
		:
		common(common)
	{
	}

public:
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.key"); }
	const wchar_t* GetOriginExtension()	{ return L"key"; }

private:
	CommonSetting& common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     カスタムメニュー                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpCustMenu : public ImpExpManager {
public:
	// Constructor
	ImpExpCustMenu(CommonSetting& common)
		:
		common(common)
	{
	}

public:
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.mnu"); }
	const wchar_t* GetOriginExtension()	{ return L"mnu"; }

private:
	CommonSetting& common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     強調キーワード                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpKeyword : public ImpExpManager {
public:
	// Constructor
	ImpExpKeyword(CommonSetting& common, int nKeywordSetIdx, bool& bCase)
		:
		common(common),
		nIdx(nKeywordSetIdx),
		bCase(bCase)
	{
	}

public:
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.kwd"); }
	const wchar_t* GetOriginExtension()	{ return L"kwd"; }

private:
	CommonSetting&	common;
	int 			nIdx;
	bool&			bCase;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     メインメニュー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpMainMenu : public ImpExpManager {
public:
	// Constructor
	ImpExpMainMenu(CommonSetting& common)
		:
		common(common)
	{
	}

public:
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }

private:
	CommonSetting& common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     メインメニュー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpFileTree : public ImpExpManager
{
public:
	// Constructor
	ImpExpFileTree(std::vector<FileTreeItem>& items)
		:
		fileTreeItems(items)
	{
	}

public:
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);
	static void IO_FileTreeIni(DataProfile&, std::vector<FileTreeItem>&);

public:
	// デフォルト拡張子の取得
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }

private:
	std::vector<FileTreeItem>&	fileTreeItems;
};

