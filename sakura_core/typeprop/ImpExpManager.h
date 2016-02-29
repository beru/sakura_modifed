/*!	@file
	@brief �C���|�[�g�A�G�N�X�|�[�g�}�l�[�W��

	@author Uchi
	@date 2010/4/22 �V�K�쐬
*/
/*
	Copyright (C) 2010, Uchi, Moca

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
		return to_wchar(GetDllShareData().history.m_szIMPORTFOLDER) + m_sOriginName;
	}
	// �t���p�X�����擾
	inline wstring MakeFullPath(wstring sFileName) {
		return to_wchar(GetDllShareData().history.m_szIMPORTFOLDER) + sFileName;
	}
	// �t�@�C�������擾
	inline wstring GetFileName()	{ return m_sOriginName; }

protected:
	// Import Folder�̐ݒ�
	inline void SetImportFolder(const TCHAR* szPath) {
		// �t�@�C���̃t���p�X���t�H���_�ƃt�@�C�����ɕ���
		// [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt]
		::SplitPath_FolderAndFile(szPath, GetDllShareData().history.m_szIMPORTFOLDER, NULL);
		_tcscat(GetDllShareData().history.m_szIMPORTFOLDER, _T("\\"));
	}

	// �f�t�H���g�g���q�̎擾(�u*.txt�v�`��)
	virtual const TCHAR* GetDefaultExtension();
	// �f�t�H���g�g���q�̎擾(�utxt�v�`��)
	virtual const wchar_t* GetOriginExtension();

protected:
	wstring		m_sBase;
	wstring		m_sOriginName;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �^�C�v�ʐݒ�                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpType : public ImpExpManager {
public:
	// Constructor
	ImpExpType(int nIdx, TypeConfig& types, HWND hwndList)
		: m_nIdx(nIdx)
		, m_types(types)
		, m_hwndList(hwndList)
	{
		// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
		m_pShareData = &GetDllShareData();
	}

public:
	bool ImportAscertain(HINSTANCE, HWND, const wstring&, wstring&);
	bool Import(const wstring&, wstring&);
	bool Export(const wstring&, wstring&);

public:
	// �f�t�H���g�g���q�̎擾
	const TCHAR* GetDefaultExtension()	{ return _T("*.ini"); }
	const wchar_t* GetOriginExtension()	{ return L"ini"; }
	bool IsAddType() { return m_bAddType; }

private:
	// �C���^�[�t�F�[�X�p
	int 			m_nIdx;
	TypeConfig&		m_types;
	HWND			m_hwndList;

	// �����g�p
	DllSharedData*	m_pShareData;
	int				m_nColorType;
	wstring 		m_sColorFile;
	bool			m_bAddType;
	DataProfile		m_profile;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �J���[                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpColors : public ImpExpManager {
public:
	// Constructor
	ImpExpColors(ColorInfo * psColorInfoArr)
		:
		m_colorInfoArr(psColorInfoArr)
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
	ColorInfo*		m_colorInfoArr;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���K�\���L�[���[�h                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpRegex : public ImpExpManager {
public:
	// Constructor
	ImpExpRegex(TypeConfig& types)
		:
		m_types(types)
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
	TypeConfig&	m_types;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�w���v                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpKeyHelp : public ImpExpManager {
public:
	// Constructor
	ImpExpKeyHelp(TypeConfig& types)
		:
		m_types(types)
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
	TypeConfig&	m_types;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���蓖��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpKeybind : public ImpExpManager {
public:
	// Constructor
	ImpExpKeybind(CommonSetting& common)
		:
		m_common(common)
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
	CommonSetting& m_common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �J�X�^�����j���[                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpCustMenu : public ImpExpManager {
public:
	// Constructor
	ImpExpCustMenu(CommonSetting& common)
		:
		m_common(common)
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
	CommonSetting& m_common;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �����L�[���[�h                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpKeyword : public ImpExpManager {
public:
	// Constructor
	ImpExpKeyword(CommonSetting& common, int nKeywordSetIdx, bool& bCase)
		:
		m_common(common),
		m_nIdx(nKeywordSetIdx),
		m_bCase(bCase)
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
	CommonSetting&	m_common;
	int 			m_nIdx;
	bool&			m_bCase;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ���C�����j���[                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class ImpExpMainMenu : public ImpExpManager {
public:
	// Constructor
	ImpExpMainMenu(CommonSetting& common)
		:
		m_common(common)
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
	CommonSetting& m_common;
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
		m_aFileTreeItems(items)
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
	std::vector<FileTreeItem>&	m_aFileTreeItems;
};

