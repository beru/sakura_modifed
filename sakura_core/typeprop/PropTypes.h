/*!	@file
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/05/08  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, asa-o
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, MIK, aroka, genta
	Copyright (C) 2006, fon
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#pragma once

#include "types/Type.h" // TypeConfig

class PropTypes;
class KeywordSetMgr;

/*-----------------------------------------------------------------------
�萔
-----------------------------------------------------------------------*/

// 2007.11.29 kobake �ϐ��̈Ӗ��𖾊m�ɂ��邽�߁AnMethos �� �e���v���[�g���B
template <class TYPE>
struct TYPE_NAME {
	TYPE			nMethod;
	const TCHAR*	pszName;
};

template <class TYPE>
struct TYPE_NAME_ID {
	TYPE		nMethod;
	int			nNameId;
};

template <class TYPE>
struct TYPE_NAME_ID2 {
	TYPE			nMethod;
	int				nNameId;
	const TCHAR*	pszName;
};

// �v���p�e�B�V�[�g�ԍ�
enum PropTypeSheetOrder {
	ID_PROPTYPE_PAGENUM_SCREEN = 0,		// �X�N���[��
	ID_PROPTYPE_PAGENUM_COLOR,			// �J���[
	ID_PROPTYPE_PAGENUM_WINDOW,			// �E�B���h�E
	ID_PROPTYPE_PAGENUM_SUPPORT,		// �x��
	ID_PROPTYPE_PAGENUM_REGEX,			// ���K�\���L�[���[�h
	ID_PROPTYPE_PAGENUM_KEYHELP,		// �X�e�[�^�X�o�[
	ID_PROPTYPE_PAGENUM_MAX,
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �^�C�v�ʐݒ�_�C�A���O�{�b�N�X

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
*/
class PropTypes {

public:
	// �����Ɣj��
	PropTypes();
	~PropTypes();
	void Create(HINSTANCE, HWND);		// ������
	INT_PTR DoPropertySheet(int);		// �v���p�e�B�V�[�g�̍쐬

	// �C���^�[�t�F�[�X	
	void SetTypeData(const TypeConfig& t) { m_types = t; }	// �^�C�v�ʐݒ�f�[�^�̐ݒ�  Jan. 23, 2005 genta
	void GetTypeData(TypeConfig& t) const { t = m_types; }	// �^�C�v�ʐݒ�f�[�^�̎擾  Jan. 23, 2005 genta
	HWND GetHwndParent()const { return m_hwndParent; }
	int GetPageNum() { return m_nPageNum; }
	bool GetChangeKeywordSet() const { return m_bChangeKeywordSet; }

protected:
	// �C�x���g
	void OnHelp(HWND , int);	// �w���v

protected:
	// �e��Q��
	HINSTANCE	m_hInstance;	// �A�v���P�[�V�����C���X�^���X�̃n���h��
	HWND		m_hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
	HWND		m_hwndThis;		// ���̃_�C�A���O�̃n���h��

	// �_�C�A���O�f�[�^
	PropTypeSheetOrder	m_nPageNum;
	DllSharedData*		m_pShareData;
	TypeConfig			m_types;

	// �X�N���[���p�f�[�^	2010/5/10 CPropTypes_P1_Screen.cpp����ړ�
	static std::vector<TYPE_NAME_ID2<EOutlineType>> m_OlmArr;			// �A�E�g���C����̓��[���z��
	static std::vector<TYPE_NAME_ID2<SmartIndentType>> m_SIndentArr;	// �X�}�[�g�C���f���g���[���z��

	// �J���[�p�f�[�^
	DWORD			m_dwCustColors[16];						// �t�H���gDialog�J�X�^���p���b�g
	int				m_nSet[ MAX_KEYWORDSET_PER_TYPE ];		// keyword set index  2005.01.13 MIK
	int				m_nCurrentColorType;					// ���ݑI������Ă���F�^�C�v
	KeywordSetMgr*	m_pKeywordSetMgr;						// �������팸�̂��߃|�C���^��  Mar. 31, 2003 genta
	bool			m_bChangeKeywordSet;

	// �t�H���g�\���p�f�[�^
	HFONT			m_hTypeFont;							// �^�C�v�ʃt�H���g�\���n���h��

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      �e�v���p�e�B�y�[�W                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);			// ���b�Z�[�W����
protected:
	void SetData(HWND);											// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);											// �_�C�A���O�f�[�^�̎擾
	bool Import(HWND);											// �C���|�[�g
	bool Export(HWND);											// �G�N�X�|�[�g

	HFONT SetCtrlFont(HWND hwndDlg, int idc_static, const LOGFONT& lf);								// �R���g���[���Ƀt�H���g�ݒ肷��		// 2013/4/24 Uchi
	HFONT SetFontLabel(HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps, bool bUse = true);	// �t�H���g���x���Ƀt�H���g�ƃt�H���g���ݒ肷��	// 2013/4/24 Uchi
};


/*!
	@brief �^�C�v�ʐݒ�v���p�e�B�y�[�W�N���X

	�v���p�e�B�y�[�W���ɒ�`
	�ϐ��̒�`��PropTypes�ōs��
*/
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �X�N���[��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class PropTypesScreen : PropTypes {
public:
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);			// ���b�Z�[�W����
protected:
	void SetData(HWND);											// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);											// �_�C�A���O�f�[�^�̎擾

public:
	static void AddOutlineMethod(int nMethod, const WCHAR* szName);		// �A�E�g���C����̓��[���̒ǉ�
	static void AddSIndentMethod(SmartIndentType nMethod, const WCHAR* szName);		// �X�}�[�g�C���f���g���[���̒ǉ�
	static void RemoveOutlineMethod(int nMethod, const WCHAR* szName);	// �A�E�g���C����̓��[���̒ǉ�
	static void RemoveSIndentMethod(SmartIndentType nMethod, const WCHAR* szName);	// �X�}�[�g�C���f���g���[���̒ǉ�
	void CPropTypes_Screen();											// �X�N���[���^�u�̃R���X�g���N�^
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �E�B���h�E                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class PropTypesWindow : PropTypes {
public:
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);			// ���b�Z�[�W����
protected:
	void SetData(HWND);											// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);											// �_�C�A���O�f�[�^�̎擾

protected:
	void SetCombobox(HWND hwndWork, const int* nIds, int nCount, int select);
	void EnableTypesPropInput(HWND hwndDlg);						// �^�C�v�ʐݒ��ON/OFF
public:
private:
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �J���[                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class PropTypesColor : PropTypes {
public:
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// ���b�Z�[�W����
protected:
	void SetData(HWND);									// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);									// �_�C�A���O�f�[�^�̎擾
	bool Import(HWND);									// �C���|�[�g
	bool Export(HWND);									// �G�N�X�|�[�g

protected:
	void DrawColorListItem(DRAWITEMSTRUCT*);				// �F��ʃ��X�g �I�[�i�[�`��
	void EnableTypesPropInput(HWND hwndDlg);				// �^�C�v�ʐݒ�̃J���[�ݒ��ON/OFF
	void RearrangeKeywordSet(HWND);							// �L�[���[�h�Z�b�g�Ĕz�u  Jan. 23, 2005 genta
	void DrawColorButton(DRAWITEMSTRUCT* , COLORREF);		// �F�{�^���̕`��
public:
	static BOOL SelectColor(HWND , COLORREF*, DWORD*);		// �F�I���_�C�A���O
private:
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �x��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class PropTypesSupport : PropTypes {
public:
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);				// ���b�Z�[�W����
protected:
	void SetData(HWND);												// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);												// �_�C�A���O�f�[�^�̎擾
public:
	static void AddHokanMethod(int nMethod, const WCHAR* szName);		// �⊮��ʂ̒ǉ�
	static void RemoveHokanMethod(int nMethod, const WCHAR* szName);	// �⊮��ʂ̒ǉ�
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���K�\���L�[���[�h                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class PropTypesRegex : PropTypes {
public:
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);			// ���b�Z�[�W����
protected:														  
	void SetData(HWND);											// �_�C�A���O�f�[�^�̐ݒ�
	void SetDataKeywordList(HWND);								// �_�C�A���O�f�[�^�̐ݒ胊�X�g����
	int  GetData(HWND);											// �_�C�A���O�f�[�^�̎擾
	bool Import(HWND);											// �C���|�[�g
	bool Export(HWND);											// �G�N�X�|�[�g
private:
	BOOL RegexKakomiCheck(const wchar_t* s);	//@@@ 2001.11.17 add MIK

	bool CheckKeywordList(HWND, const TCHAR*, int);

};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �L�[���[�h�w���v                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class PropTypesKeyHelp : PropTypes {
public:
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);			// ���b�Z�[�W����
protected:
	void SetData(HWND);											// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);											// �_�C�A���O�f�[�^�̎擾
	bool Import(HWND);											// �C���|�[�g
	bool Export(HWND);											// �G�N�X�|�[�g
};

template <typename T>
void InitTypeNameId2(std::vector<TYPE_NAME_ID2<T>>& vec, TYPE_NAME_ID<T>* arr, size_t size)
{
	for (size_t i=0; i<size; ++i) {
		TYPE_NAME_ID2<T> item = {arr[i].nMethod, arr[i].nNameId, NULL};
		vec.push_back(item);
	}
}

