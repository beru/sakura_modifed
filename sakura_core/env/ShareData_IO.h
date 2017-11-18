#pragma once

class DataProfile;
class MenuDrawer;
struct CommonSetting_CustomMenu;	// defined CommonSetting.h
struct CommonSetting_MainMenu;		// defined CommonSetting.h
struct CommonSetting_KeyBind;		// defined CommonSetting.h
struct ColorInfo; // defined doc/DocTypeSetting.h
struct FileTree;

class ShareData_IO {
public:
	// �Z�[�u�E���[�h
	static bool LoadShareData();	// ���L�f�[�^�̃��[�h
	static void SaveShareData();	// ���L�f�[�^�̕ۑ�

protected:
	static bool ShareData_IO_2(bool);	// ���L�f�[�^�̕ۑ�

	static void ShareData_IO_Mru(DataProfile&);
	static void ShareData_IO_Keys(DataProfile&);
	static void ShareData_IO_Grep(DataProfile&);
	static void ShareData_IO_Folders(DataProfile&);
	static void ShareData_IO_Cmd(DataProfile&);
	static void ShareData_IO_Nickname(DataProfile&);
	static void ShareData_IO_Common(DataProfile&);
	static void ShareData_IO_Toolbar(DataProfile&, MenuDrawer*);
	static void ShareData_IO_CustMenu(DataProfile&);
	static void ShareData_IO_Font(DataProfile&);
	static void ShareData_IO_KeyBind(DataProfile&);
	static void ShareData_IO_Print(DataProfile&);
	static void ShareData_IO_Types(DataProfile&);
	static void ShareData_IO_Keywords(DataProfile&);
	static void ShareData_IO_Macro(DataProfile&);
	static void ShareData_IO_Statusbar(DataProfile&);
	static void ShareData_IO_Plugin(DataProfile&, MenuDrawer*);
	static void ShareData_IO_MainMenu(DataProfile&);
	static void ShareData_IO_Other(DataProfile&);

public:
	static void ShareData_IO_FileTree(DataProfile&, FileTree&, const wchar_t*);
	static void ShareData_IO_FileTreeItem(DataProfile&, FileTreeItem&, const wchar_t*, int i);
	static void ShareData_IO_Type_One(DataProfile&, TypeConfig& , const wchar_t*);

public:
	static void IO_CustMenu(DataProfile&, CommonSetting_CustomMenu&, bool);
	static void IO_KeyBind(DataProfile&, CommonSetting_KeyBind&, bool);
	static void IO_MainMenu(DataProfile& c, CommonSetting_MainMenu& s, bool b){
		IO_MainMenu(c, NULL, s, b);
	}
	static void IO_MainMenu(DataProfile& profile,
		std::vector<std::wstring>* pData,
		CommonSetting_MainMenu& mainmenu,
		bool bOutCmdName);
	static void IO_ColorSet(DataProfile*, const wchar_t*, ColorInfo*);		// �F�ݒ� I/O
};

