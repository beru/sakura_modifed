// �C���|�[�g�A�G�N�X�|�[�g�}�l�[�W��

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
	// �t�@�C�����̏����l��ݒ�
	void SetBaseName(const wstring&);
	// �t���p�X�����擾
	inline wstring GetFullPath() {
		return to_wchar(GetDllShareData().history.szIMPORTFOLDER) + sOriginName;
	}
	// �t���p�X�����擾
	inline wstring MakeFullPath(wstring sFileName) {
		return to_wchar(GetDllShareData().history.szIMPORTFOLDER) + sFileName;
	}
	// �t�@�C�������擾
	inline wstring GetFileName() { return sOriginName; }

protected:
	// Import Folder�̐ݒ�
	inline void SetImportFolder(const TCHAR* szPath) {
		// �t�@�C���̃t���p�X���t�H���_�ƃt�@�C�����ɕ���
		// [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt]
		::SplitPath_FolderAndFile(szPath, GetDllShareData().history.szIMPORTFOLDER, NULL);
		_tcscat(GetDllShareData().history.szIMPORTFOLDER, _T("\\"));
	}

	// �f�t�H���g�g���q�̎擾(�u*.txt�v�`��)
	virtual const TCHAR* GetDefaultExtension();
	// �f�t�H���g�g���q�̎擾(�utxt�v�`��)
	virtual const wchar_t* GetOriginExtension();

protected:
	wstring		sBase;
	wstring		sOriginName;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �^�C�v�ʐݒ�                       //
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
		// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
		pShareData = &GetDllShareData();
	}

public:
	bool ImportAscertain(HINSTANCE, HWND, const wstring&, wstring&);
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }
	bool IsAddType() { return bAddType; }

private:
	// �C���^�[�t�F�[�X�p
	int 			nIdx;
	TypeConfig&		types;
	HWND			hwndList;

	// �����g�p
	DllSharedData*	pShareData;
	int				nColorType;
	wstring 		sColorFile;
	bool			bAddType;
	DataProfile		profile;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �J���[                             //
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
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.col"); }
	const wchar_t* GetOriginExtension()	{ return L"col"; }

private:
	ColorInfo*		colorInfoArr;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���K�\���L�[���[�h                       //
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
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.rkw"); }
	const wchar_t* GetOriginExtension()	{ return L"rkw"; }

private:
	TypeConfig&	types;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�w���v                        //
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
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.txt"); }
	const wchar_t* GetOriginExtension()	{ return L"txt"; }

private:
	TypeConfig&	types;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���蓖��                            //
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
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.key"); }
	const wchar_t* GetOriginExtension()	{ return L"key"; }

private:
	CommonSetting& common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �J�X�^�����j���[                        //
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
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.mnu"); }
	const wchar_t* GetOriginExtension()	{ return L"mnu"; }

private:
	CommonSetting& common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �����L�[���[�h                          //
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
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.kwd"); }
	const wchar_t* GetOriginExtension()	{ return L"kwd"; }

private:
	CommonSetting&	common;
	int 			nIdx;
	bool&			bCase;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ���C�����j���[                          //
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
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }

private:
	CommonSetting& common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ���C�����j���[                          //
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
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }

private:
	std::vector<FileTreeItem>&	fileTreeItems;
};

