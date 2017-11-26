/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�̏���
*/
#pragma once

#include "func/FuncLookup.h"
#include "env/CommonSetting.h"

struct DllSharedData;
class ImageListMgr;
class SMacroMgr;
class MenuDrawer;

/*! �v���p�e�B�V�[�g�ԍ� */
enum PropComSheetOrder {
	ID_PROPCOM_PAGENUM_GENERAL = 0,		// �S��
	ID_PROPCOM_PAGENUM_WIN,				// �E�B���h�E
	ID_PROPCOM_PAGENUM_MAINMENU,		// ���C�����j���[
	ID_PROPCOM_PAGENUM_TOOLBAR,			// �c�[���o�[
	ID_PROPCOM_PAGENUM_TAB,				// �^�u�o�[
	ID_PROPCOM_PAGENUM_STATUSBAR,		// �X�e�[�^�X�o�[
	ID_PROPCOM_PAGENUM_EDIT,			// �ҏW
	ID_PROPCOM_PAGENUM_FILE,			// �t�@�C��
	ID_PROPCOM_PAGENUM_FILENAME,		// �t�@�C�����\��
	ID_PROPCOM_PAGENUM_BACKUP,			// �o�b�N�A�b�v
	ID_PROPCOM_PAGENUM_FORMAT,			// ����
	ID_PROPCOM_PAGENUM_GREP,			// ����
	ID_PROPCOM_PAGENUM_KEYBOARD,		// �L�[���蓖��
	ID_PROPCOM_PAGENUM_CUSTMENU,		// �J�X�^�����j���[
	ID_PROPCOM_PAGENUM_KEYWORD,			// �����L�[���[�h
	ID_PROPCOM_PAGENUM_HELPER,			// �x��
	ID_PROPCOM_PAGENUM_MACRO,			// �}�N��
	ID_PROPCOM_PAGENUM_PLUGIN,			// �v���O�C��
	ID_PROPCOM_PAGENUM_MAX,
};
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�N���X

	1�̃_�C�A���O�{�b�N�X�ɕ����̃v���p�e�B�y�[�W���������\����
	�Ȃ��Ă���ADialog procedure��Event Dispatcher���y�[�W���Ƃɂ���D
*/
class PropCommon {
public:
	/*
	||  Constructors
	*/
	PropCommon();
	~PropCommon();
//@@@ tbMyButton�Ȃǂ�CShareData����MenuDrawer�ֈړ��������Ƃɂ��C���B
	void Create(HWND, ImageListMgr*, MenuDrawer*);	// ������

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoPropertySheet(int, bool);	// �v���p�e�B�V�[�g�̍쐬

	void InitData(void);		// DllSharedData����ꎞ�f�[�^�̈�ɐݒ�𕡐�����
	void ApplyData(void);		// �ꎞ�f�[�^�̈悩���DllSharedData�ݒ���R�s�[����
	int GetPageNum() { return nPageNum; }

	//
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	HWND				hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
	HWND				hwndThis;		// ���̃_�C�A���O�̃n���h��
	PropComSheetOrder	nPageNum;
	DllSharedData*		pShareData;
	int					nKeywordSet1;
	ImageListMgr*	pIcons;	//	Image List
	
	FuncLookup			lookup;

	MenuDrawer*		pMenuDrawer;
	/*
	|| �_�C�A���O�f�[�^
	*/
	CommonSetting	common;

	struct KeywordSetIndex {
		int typeId;
		int index[MAX_KEYWORDSET_PER_TYPE];
	};
	std::vector<KeywordSetIndex> types_nKeywordSetIdx;
	bool	bTrayProc;
	HFONT	hKeywordHelpFont;		// �L�[���[�h�w���v �t�H���g �n���h��
	HFONT	hTabFont;				// �^�u �t�H���g �n���h��

protected:
	/*
	||  �����w���p�֐�
	*/
	void OnHelp(HWND, int);	// �w���v
	int	SearchIntArr(int , int* , int);

	// �ėp�_�C�A���O�v���V�[�W��
	static INT_PTR DlgProc(
		INT_PTR (PropCommon::*DispatchPage)(HWND, UINT, WPARAM, LPARAM),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR DlgProc2( // �Ɨ��E�B���h�E�p
		INT_PTR (PropCommon::*DispatchPage)(HWND, UINT, WPARAM, LPARAM),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	typedef	INT_PTR (PropCommon::*pDispatchPage)(HWND, UINT, WPARAM, LPARAM);

	int nLastPos_Macro;			// �O��t�H�[�J�X�̂������ꏊ
	int nLastPos_FILENAME;	// �O��t�H�[�J�X�̂������ꏊ �t�@�C�����^�u�p

	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾
	void Import(HWND);	// �C���|�[�g����
	void Export(HWND);	// �G�N�X�|�[�g����

	HFONT SetCtrlFont(HWND hwndDlg, int idc_static, const LOGFONT& lf);				// �R���g���[���Ƀt�H���g�ݒ肷��
	HFONT SetFontLabel(HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps);	// �t�H���g���x���Ƀt�H���g�ƃt�H���g���ݒ肷��
};


/*!
	@brief ���ʐݒ�v���p�e�B�y�[�W�N���X

	1�̃v���p�e�B�y�[�W���ɒ�`
	Dialog procedure��Event Dispatcher���y�[�W���Ƃɂ���D
	�ϐ��̒�`��PropCommon�ōs��
*/
//==============================================================
// �S�ʃy�[�W
class PropGeneral : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾
};

//==============================================================
// �t�@�C���y�[�W
class PropFile : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void EnableFilePropInput(HWND hwndDlg);	//	�t�@�C���ݒ��ON/OFF
};

//==============================================================
// �L�[���蓖�ăy�[�W
class PropKeybind : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

	void Import(HWND);	// �C���|�[�g����
	void Export(HWND);	// �G�N�X�|�[�g����

private:
	void ChangeKeyList(HWND); // �L�[���X�g���`�F�b�N�{�b�N�X�̏�Ԃɍ��킹�čX�V����
};

//==============================================================
// �c�[���o�[�y�[�W
class PropToolbar : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void DrawToolBarItemList(DRAWITEMSTRUCT*);	// �c�[���o�[�{�^�����X�g�̃A�C�e���`��
};

//==============================================================
// �L�[���[�h�y�[�W
class PropKeyword : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR CALLBACK DlgProc_dialog(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void SetKeywordSet(HWND, size_t);	// �w��L�[���[�h�Z�b�g�̐ݒ�
	void ClearKeywordSet(HWND);
	void DispKeywordCount(HWND hwndDlg);

	void Edit_List_Keyword(HWND, HWND);		// ���X�g���őI������Ă���L�[���[�h��ҏW����
	void Delete_List_Keyword(HWND, HWND);	// ���X�g���őI������Ă���L�[���[�h���폜����
	void Import_List_Keyword(HWND, HWND);	// ���X�g���̃L�[���[�h���C���|�[�g����
	void Export_List_Keyword(HWND, HWND);	// ���X�g���̃L�[���[�h���G�N�X�|�[�g����
	void Clean_List_Keyword(HWND, HWND);	// ���X�g���̃L�[���[�h�𐮗�����
};

//==============================================================
// �J�X�^�����j���[�y�[�W
class PropCustmenu : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	void SetDataMenuList(HWND, int);
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾
	void Import(HWND);	// �J�X�^�����j���[�ݒ���C���|�[�g����
	void Export(HWND);	// �J�X�^�����j���[�ݒ���G�N�X�|�[�g����
};

//==============================================================
// �����y�[�W
class PropFormat : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void ChangeDateExample(HWND hwndDlg);
	void ChangeTimeExample(HWND hwndDlg);

	void EnableFormatPropInput(HWND hwndDlg);	//	�����ݒ��ON/OFF
};

//==============================================================
// �x���y�[�W
class PropHelper : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾
};

//==============================================================
// �o�b�N�A�b�v�y�[�W
class PropBackup : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void EnableBackupInput(HWND hwndDlg);	//	�o�b�N�A�b�v�ݒ��ON/OFF
	void UpdateBackupFile(HWND hwndDlg);	//	�o�b�N�A�b�v�t�@�C���̏ڍאݒ�
};

//==============================================================
// �E�B���h�E�y�[�W
class PropWin : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void EnableWinPropInput(HWND hwndDlg) ;	//	�E�B���h�E�ݒ��ON/OFF
};

//==============================================================
// �^�u����y�[�W
class PropTab : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void EnableTabPropInput(HWND hwndDlg);
};

//==============================================================
// �ҏW�y�[�W
class PropEdit : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void EnableEditPropInput(HWND hwndDlg);
};

//==============================================================
// �����y�[�W
class PropGrep : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void SetRegexpVersion(HWND);
};

//==============================================================
// �}�N���y�[�W
class PropMacro : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void InitDialog(HWND hwndDlg);// Macro�y�[�W�̏�����
	void SetMacro2List_Macro(HWND hwndDlg);// Macro�f�[�^�̐ݒ�
	void SelectBaseDir_Macro(HWND hwndDlg);// Macro�f�B���N�g���̑I��
	void OnFileDropdown_Macro(HWND hwndDlg);// �t�@�C���h���b�v�_�E�����J�����Ƃ�
	void CheckListPosition_Macro(HWND hwndDlg);// ���X�g�r���[��Focus�ʒu�m�F
	static int CALLBACK DirCallback_Macro(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
};

//==============================================================
// �t�@�C�����\���y�[�W
class PropFileName : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	static int SetListViewItem_FILENAME(HWND hListView, int, LPTSTR, LPTSTR, bool); // ListView�̃A�C�e����ݒ�
	static void GetListViewItem_FILENAME(HWND hListView, int, LPTSTR, LPTSTR); // ListView�̃A�C�e�����擾
	static int MoveListViewItem_FILENAME(HWND hListView, int, int); // ListView�̃A�C�e�����ړ�����
};

//==============================================================
// �X�e�[�^�X�o�[�y�[�W
class PropStatusbar : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾
};

//==============================================================
// �v���O�C���y�[�W
class PropPlugin : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	std::tstring GetReadMeFile(const std::tstring& sName);	//	Readme �t�@�C���̎擾
	bool BrowseReadMe(const std::tstring& sReadMeName);		//	Readme �t�@�C���̕\��
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾

private:
	void SetData_LIST(HWND);
	void InitDialog(HWND hwndDlg);	// Plugin�y�[�W�̏�����
	void EnablePluginPropInput(HWND hwndDlg);
};

//==============================================================
// ���C�����j���[�y�[�W
class PropMainMenu : PropCommon {
public:
	// Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	// Message Handler
	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);
	void SetData(HWND);	// �_�C�A���O�f�[�^�̐ݒ�
	int  GetData(HWND);	// �_�C�A���O�f�[�^�̎擾
	void Import(HWND);	// ���j���[�ݒ���C���|�[�g����
	void Export(HWND);	// ���j���[�ݒ���G�N�X�|�[�g����

private:
	bool GetDataTree(HWND, HTREEITEM, int);

	bool Check_MainMenu(HWND, std::wstring&);						// ���j���[�̌���
	bool Check_MainMenu_Sub(HWND, HTREEITEM, int, std::wstring&);	// ���j���[�̌���
};

