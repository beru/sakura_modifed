#pragma once

class EditView;
enum EFunctionCode;
class EditDoc;
struct DllSharedData;
class OpeBlk;
class Caret;
class EditWnd;
class ColorStrategy;
class ColorStrategyPool;
class SMacroMgr;
#include "Eol.h"

class ViewCommander {
public:
	ViewCommander(EditView& editView);

public:
	// �O���ˑ�
	EditDoc& GetDocument();
	EditWnd& GetEditWindow();
	HWND GetMainWindow();
	OpeBlk* GetOpeBlk();
	void SetOpeBlk(OpeBlk* p);
	Range& GetSelect();
	Caret& GetCaret();

private:
	EditView&	view;
	SMacroMgr*	pSMacroMgr;

public:
	// �L�[���s�[�g���
	int bPrevCommand;

private:
	enum class IndentType {
		None,
		Tab,
		Space
	};

	// -- -- -- -- �ȉ��A�R�}���h�����֐��Q -- -- -- -- //
public:
	bool HandleCommand(
		EFunctionCode	nCommand,
		bool			bRedraw,
		LPARAM			lparam1,
		LPARAM			lparam2,
		LPARAM			lparam3,
		LPARAM			lparam4
	);

	// �t�@�C������n
	void Command_FileNew(void);				// �V�K�쐬
	void Command_FileNew_NewWindow(void);	// �V�K�쐬�i�^�u�ŊJ���Łj
	// �t�@�C�����J��
	void Command_FileOpen(
		const wchar_t*	filename	= NULL,
		EncodingType	nCharCode	= CODE_AUTODETECT,
		bool			bViewMode	= false,
		const wchar_t*	defaultName	= NULL
	);

	// �㏑���ۑ�
	bool Command_FileSave(bool warnbeep = true, bool askname = true);
	bool Command_FileSaveAs_Dialog(const wchar_t*, EncodingType, EolType);		// ���O��t���ĕۑ�
	bool Command_FileSaveAs(const wchar_t* filename, EolType eEolType);		// ���O��t���ĕۑ�
	bool Command_FileSaveAll(void);				// �S�ď㏑���ۑ�
	void Command_FileClose(void);				// �J����(����)
	// ���ĊJ��
	void Command_FileClose_Open(LPCWSTR filename = NULL,
		EncodingType nCharCode = CODE_AUTODETECT, bool bViewMode = false);

	void Command_File_Reopen(EncodingType nCharCode, bool bNoConfirm);		// �ăI�[�v��

	void Command_Print(void);					// ���
	void Command_Print_Preview(void);			// ���Preview
	void Command_Print_PageSetUp(void);			// ����y�[�W�ݒ�
	bool Command_Open_HfromtoC(bool);			// ������C/C++�w�b�_(�\�[�X)���J��
	bool Command_Open_HHPP(bool bCheckOnly, bool bBeepWhenMiss);				// ������C/C++�w�b�_�t�@�C�����J��
	bool Command_Open_CCPP(bool bCheckOnly, bool bBeepWhenMiss);				// ������C/C++�\�[�X�t�@�C�����J��
	void Command_Activate_SQLPlus(void);			// Oracle SQL*Plus���A�N�e�B�u�\��
	void Command_PLSQL_Compile_On_SQLPlus(void);	// Oracle SQL*Plus�Ŏ��s
	void Command_Browse(void);					// �u���E�Y
	void Command_ViewMode(void);				// �r���[���[�h
	void Command_Property_File(void);			// �t�@�C���̃v���p�e�B
	void Command_ProfileMgr(void);				// �v���t�@�C���}�l�[�W��
	void Command_ExitAllEditors(void);			// �ҏW�̑S�I��
	void Command_ExitAll(void);					// �T�N���G�f�B�^�̑S�I��
	bool Command_PutFile(LPCWSTR, EncodingType, int);	// ��ƒ��t�@�C���̈ꎞ�o��
	bool Command_InsFile(LPCWSTR, EncodingType, int);	// �L�����b�g�ʒu�Ƀt�@�C���}��

	// �ҏW�n
	void Command_WCHAR(wchar_t, bool bConvertEOL = true);			// ��������
	void Command_IME_CHAR(WORD);			// �S�p��������
	void Command_Undo(void);				// ���ɖ߂�(Undo)
	void Command_Redo(void);				// ��蒼��(Redo)
	void Command_Delete(void);				// �J�[�\���ʒu�܂��͑I���G���A���폜
	void Command_Delete_Back(void);			// �J�[�\���O���폜
	void Command_WordDeleteToStart(void);	// �P��̍��[�܂ō폜
	void Command_WordDeleteToEnd(void);		// �P��̉E�[�܂ō폜
	void Command_WordCut(void);				// �P��؂���
	void Command_WordDelete(void);			// �P��폜
	void Command_LineCutToStart(void);		// �s���܂Ő؂���(���s�P��)
	void Command_LineCutToEnd(void);		// �s���܂Ő؂���(���s�P��)
	void Command_LineDeleteToStart(void);	// �s���܂ō폜(���s�P��)
	void Command_LineDeleteToEnd(void);  	// �s���܂ō폜(���s�P��)
	void Command_Cut_Line(void);			// �s�؂���(�܂�Ԃ��P��)
	void Command_Delete_Line(void);			// �s�폜(�܂�Ԃ��P��)
	void Command_DuplicateLine(void);		// �s�̓�d��(�܂�Ԃ��P��)
	void Command_Indent(wchar_t cChar, IndentType = IndentType::None); // �C���f���g ver 1
	void Command_Indent(const wchar_t*, size_t, IndentType = IndentType::None);// �C���f���g ver0
	void Command_Unindent(wchar_t wcChar);// �t�C���f���g
//	void Command_WORDSREFERENCE(void);		// �P�ꃊ�t�@�����X
	void Command_Trim(bool);
	void Command_Sort(bool);
	void Command_Merge(void);
	void Command_Reconvert(void);			// ���j���[����̍ĕϊ��Ή�
	void Command_CtrlCode_Dialog(void);		// �R���g���[���R�[�h�̓���(�_�C�A���O)

	// �J�[�\���ړ��n
	void Command_MoveCursor(Point pos, int option);
	void Command_MoveCursorLayout(Point pos, int option);
	int Command_Up(bool bSelect, bool bRepeat, int line = 0);			// �J�[�\����ړ�
	int Command_Down(bool bSelect, bool bRepeat);	// �J�[�\�����ړ�
	int Command_Left(bool, bool);					// �J�[�\�����ړ�
	void Command_Right(bool bSelect, bool bIgnoreCurrentSelection, bool bRepeat);	// �J�[�\���E�ړ�
	void Command_Up2(bool bSelect);					// �J�[�\����ړ��i�Q�s�Âj
	void Command_Down2(bool bSelect);				// �J�[�\�����ړ��i�Q�s�Âj
	void Command_WordLeft(bool bSelect);			// �P��̍��[�Ɉړ�
	void Command_WordRight(bool bSelect);			// �P��̉E�[�Ɉړ�
	void Command_GoLineTop(bool bSelect, int lparam);	// �s���Ɉړ��i�܂�Ԃ��P�ʁj
	void Command_GoLineEnd(bool bSelect, int, int);	// �s���Ɉړ��i�܂�Ԃ��P�ʁj
	void Command_HalfPageUp(bool bSelect, int);		// ���y�[�W�A�b�v
	void Command_HalfPageDown(bool bSelect, int);	// ���y�[�W�_�E��
	void Command_1PageUp(bool bSelect, int);		// �P�y�[�W�A�b�v	
	void Command_1PageDown(bool bSelect, int);		// �P�y�[�W�_�E��
	void Command_GoFileTop(bool bSelect);			// �t�@�C���̐擪�Ɉړ�
	void Command_GoFileEnd(bool bSelect);			// �t�@�C���̍Ō�Ɉړ�
	void Command_CurLineCenter(void);				// �J�[�\���s���E�B���h�E������
	void Command_JumpHist_Prev(void);				// �ړ�����: �O��
	void Command_JumpHist_Next(void);				// �ړ�����: ����
	void Command_JumpHist_Set(void);				// ���݈ʒu���ړ������ɓo�^
	void Command_WndScrollDown(void);				// �e�L�X�g���P�s����Scroll
	void Command_WndScrollUp(void);					// �e�L�X�g���P�s���Scroll
	void Command_GoNextParagraph(bool bSelect);		// ���̒i���֐i��
	void Command_GoPrevParagraph(bool bSelect);		// �O�̒i���֖߂�
	void Command_AutoScroll();						// Auto Scroll
	void Command_WheelUp(int);
	void Command_WheelDown(int);
	void Command_WheelLeft(int);
	void Command_WheelRight(int);
	void Command_WheelPageUp(int);
	void Command_WheelPageDown(int);
	void Command_WheelPageLeft(int);
	void Command_WheelPageRight(int);
	void Command_ModifyLine_Next(bool bSelect);	// ���̕ύX�s��
	void Command_ModifyLine_Prev(bool bSelect);	// �O�̕ύX�s��

	// �I���n
	bool Command_SelectWord(const Point* pptCaretPos = nullptr);		// ���݈ʒu�̒P��I��
	void Command_SelectAll(void);			// ���ׂđI��
	void Command_SelectLine(int lparam);	// 1�s�I��
	void Command_Begin_Select(void);		// �͈͑I���J�n

	// ��`�I���n
	void Command_Begin_BoxSelect(bool bSelectingLock = false);	// ��`�͈͑I���J�n

	// �N���b�v�{�[�h�n
	void Command_Cut(void);						// �؂���i�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜
	void Command_Copy(bool, bool bAddCRLFWhenCopy, EolType neweol = EolType::Unknown);// �R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	void Command_Paste(int option);				// �\��t���i�N���b�v�{�[�h����\��t��
	void Command_PasteBox(int option);			// ��`�\��t���i�N���b�v�{�[�h�����`�\��t��
	// ��`�\��t���i�����n���ł̒���t���j
	void Command_PasteBox(const wchar_t* szPaste, size_t nPasteSize);
	void Command_InsBoxText(const wchar_t*, int); // ��`�\��t��
	void Command_InsText(bool bRedraw, const wchar_t*, size_t, bool bNoWaitCursor,
		                   bool bLinePaste = false,
                       bool bFastMode = false,
                       const Range* psDelRangeLogicFast = nullptr);
	void Command_AddTail(const wchar_t* pszData, size_t nDataLen);	// �Ō�Ƀe�L�X�g��ǉ�
	void Command_CopyFileName(void);				// ���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[
	void Command_CopyPath(void);					// ���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	void Command_CopyTag(void);						// ���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[
	void Command_CopyLines(void);					// �I��͈͓��S�s�R�s�[
	void Command_CopyLinesAsPassage(void);			// �I��͈͓��S�s���p���t���R�s�[
	void Command_CopyLinesWithLineNumber(void);		// �I��͈͓��S�s�s�ԍ��t���R�s�[
	void Command_Copy_Color_HTML(bool bLineNumber = false);	// �I��͈͓��S�s�s�ԍ��t���R�s�[
	void Command_Copy_Color_HTML_LineNumber(void);		// �I��͈͓��F�t��HTML�R�s�[
	ColorStrategy* GetColorStrategyHTML(const StringRef&, int, const ColorStrategyPool*, ColorStrategy**, ColorStrategy**, bool& bChange);
	void Command_CreateKeyBindList(void);				// �L�[���蓖�Ĉꗗ���R�s�[

	// �}���n
	void Command_Ins_Date(void);	// ���t�}��
	void Command_Ins_Time(void);	// �����}��

	// �ϊ��n
	void Command_ToLower(void);					// ������
	void Command_ToUpper(void);					// �啶��
	void Command_ToZenkakuKata(void);			// ���p�{�S�Ђ灨�S�p�E�J�^�J�i
	void Command_ToZenkakuHira(void);			// ���p�{�S�J�^���S�p�E�Ђ炪��
	void Command_ToHankaku(void);				// �S�p�����p
	void Command_ToHankata(void);				// �S�p�J�^�J�i�����p�J�^�J�i
	void Command_ToZenEi(void);					// ���p�p�����S�p�p��
	void Command_ToHanEi(void);					// �S�p�p�������p�p��
	void Command_HanKataToZenkakuKata(void);	// ���p�J�^�J�i���S�p�J�^�J�i
	void Command_HanKataToZenKakuHira(void);	// ���p�J�^�J�i���S�p�Ђ炪��
	void Command_TabToSpace(void);				// TAB����
	void Command_SpaceToTab(void);				// �󔒁�TAB
	void Command_CodeCnv_Auto2SJIS(void);		// �������ʁ�SJIS�R�[�h�ϊ�
	void Command_CodeCnv_EMail(void);			// E-Mail(JIS��SJIS)�R�[�h�ϊ�
	void Command_CodeCnv_EUC2SJIS(void);		// EUC��SJIS�R�[�h�ϊ�
	void Command_CodeCnv_Unicode2SJIS(void);	// Unicode��SJIS�R�[�h�ϊ�
	void Command_CodeCnv_UnicodeBE2SJIS(void);	// UnicodeBE��SJIS�R�[�h�ϊ�
	void Command_CodeCnv_UTF82SJIS(void);		// UTF-8��SJIS�R�[�h�ϊ�
	void Command_CodeCnv_UTF72SJIS(void);		// UTF-7��SJIS�R�[�h�ϊ�
	void Command_CodeCnv_SJIS2JIS(void);		// SJIS��JIS�R�[�h�ϊ�
	void Command_CodeCnv_SJIS2EUC(void);		// SJIS��EUC�R�[�h�ϊ�
	void Command_CodeCnv_SJIS2UTF8(void);		// SJIS��UTF-8�R�[�h�ϊ�
	void Command_CodeCnv_SJIS2UTF7(void);		// SJIS��UTF-7�R�[�h�ϊ�
	void Command_Base64Decode(void);			// Base64�f�R�[�h���ĕۑ�
	void Command_UUDecode(void);				// uudecode���ĕۑ�

	// �����n
	void Command_Search_Box(void);						// ����(�{�b�N�X)
	void Command_Search_Dialog(void);					// ����(�P�ꌟ���_�C�A���O)
	void Command_Search_Next(bool, bool, bool, HWND, const wchar_t*, Range* = nullptr);// ��������
	void Command_Search_Prev(bool bReDraw, HWND);		// �O������
	void Command_Replace_Dialog(void);					// �u��(�u���_�C�A���O)
	void Command_Replace(HWND hwndParent);				// �u��(���s)
	void Command_Replace_All();							// ���ׂĒu��(���s)
	void Command_Search_ClearMark(void);				// �����}�[�N�̃N���A
	void Command_Jump_SrchStartPos(void);				// �����J�n�ʒu�֖߂�


	void Command_Grep_Dialog(void);						// Grep�_�C�A���O�̕\��
	void Command_Grep(void);							// Grep
	void Command_Grep_Replace_Dlg( void );				// Grep�u���_�C�A���O�̕\��
	void Command_Grep_Replace( void );					// Grep�u��
	void Command_Jump_Dialog(void);						// �w��s�w�W�����v�_�C�A���O�̕\��
	void Command_Jump(void);							// �w��s�w�W�����v
	bool Command_FuncList(ShowDialogType nAction, OutlineType outlineType);	// �A�E�g���C�����
	bool Command_TagJump(bool bClose = false);			// �^�O�W�����v�@�\
	void Command_TagJumpBack(void);						// �^�O�W�����v�o�b�N�@�\
	bool Command_TagJumpByTagsFileMsg(bool);			// �_�C���N�g�^�O�W�����v(�ʒm��)
	bool Command_TagJumpByTagsFile(bool);				// �_�C���N�g�^�O�W�����v

	bool Command_TagsMake(void);						// �^�O�t�@�C���̍쐬
	bool Command_TagJumpByTagsFileKeyword(const wchar_t* keyword);
	void Command_Compare(void);							// �t�@�C�����e��r
	void Command_Diff_Dialog(void);						// DIFF�����\���_�C�A���O
	void Command_Diff(const wchar_t* szTmpFile2, int nFlgOpt);	// DIFF�����\��
	void Command_Diff_Next(void);						// ���̍�����
	void Command_Diff_Prev(void);						// �O�̍�����
	void Command_Diff_Reset(void);						// �����̑S����
	void Command_BracketPair(void);						// �Ί��ʂ̌���
	void Command_Bookmark_Set(void);					// �u�b�N�}�[�N�ݒ�E����
	void Command_Bookmark_Next(void);					// ���̃u�b�N�}�[�N��
	void Command_Bookmark_Prev(void);					// �O�̃u�b�N�}�[�N��
	void Command_Bookmark_Reset(void);					// �u�b�N�}�[�N�̑S����
	void Command_Bookmark_Pattern(void);				// �w��p�^�[���Ɉ�v����s���}�[�N
	void Command_FuncList_Next( void );					// ���̊֐����X�g�}�[�N
	void Command_FuncList_Prev( void );					// �O�̊֐����X�g�}�[�N

	// ���[�h�؂�ւ��n
	void Command_ChgMod_Ins(void);						// �}���^�㏑�����[�h�؂�ւ�
	void Command_Chg_Charset(EncodingType, bool);		// �����R�[�h�Z�b�g�w��
	void Command_ChgMod_EOL(EolType);					// ���͂�����s�R�[�h��ݒ�
	void Command_Cancel_Mode(int whereCursorIs = 0);	// �e�탂�[�h�̎�����

	// �ݒ�n
	void Command_ShowToolBar(void);					// �c�[���o�[�̕\��/��\��
	void Command_ShowFuncKey(void);					// �t�@���N�V�����L�[�̕\��/��\��
	void Command_ShowTab(void);						// �^�u�̕\��/��\��
	void Command_ShowStatusBar(void);				// �X�e�[�^�X�o�[�̕\��/��\��
	void Command_ShowMiniMap(void);					// �~�j�}�b�v�̕\��/��\��
	void Command_Type_List(void);					// �^�C�v�ʐݒ�ꗗ
	void Command_ChangeType(int nTypePlusOne);		// �^�C�v�ʐݒ�ꎞ�K�p
	void Command_Option_Type(void);					// �^�C�v�ʐݒ�
	void Command_Option(void);						// ���ʐݒ�
	void Command_Font(void);						// �t�H���g�ݒ�
	void Command_SetFontSize(int, int, int);		// �t�H���g�T�C�Y�ݒ�
	void Command_WrapWindowWidth(void);				// ���݂̃E�B���h�E���Ő܂�Ԃ�
	void Command_Favorite(void);					// �����̊Ǘ�
	void Command_Set_QuoteString(const wchar_t*);	// ���p���̐ݒ�
	void Command_TextWrapMethod(TextWrappingMethod);// �e�L�X�g�̐܂�Ԃ����@��ύX����
	void Command_Select_Count_Mode(int nMode);		// �����J�E���g���@

	// �}�N���n
	void Command_RecKeyMacro(void);		// �L�[�}�N���̋L�^�J�n�^�I��
	void Command_SaveKeyMacro(void);	// �L�[�}�N���̕ۑ�
	void Command_LoadKeyMacro(void);	// �L�[�}�N���̓ǂݍ���
	void Command_ExecKeyMacro(void);	// �L�[�}�N���̎��s
	void Command_ExecExtMacro(const wchar_t* path, const wchar_t* type);	// ���O���w�肵�ă}�N�����s
	void Command_ExecCommand_Dialog(void);		// �O���R�}���h���s�_�C�A���O�\��
	void Command_ExecCommand(LPCWSTR cmd, const int nFlgOpt, LPCWSTR);	// �O���R�}���h���s

	// �J�X�^�����j���[
	void Command_Menu_RButton(void);	// �E�N���b�N���j���[
	int Command_CustMenu(int);			// �J�X�^�����j���[�\��

	// �E�B���h�E�n
	void Command_Split_V(void);			// �㉺�ɕ���
	void Command_Split_H(void);			// ���E�ɕ���
	void Command_Split_VH(void);		// �c���ɕ���
	void Command_WinClose(void);		// �E�B���h�E�����
	void Command_FileCloseAll(void);	// ���ׂẴE�B���h�E�����
	void Command_Bind_Window(void);		// �������ĕ\��
	void Command_Cascade(void);			// �d�˂ĕ\��
	void Command_Tile_V(void);			// �㉺�ɕ��ׂĕ\��
	void Command_Tile_H(void);			// ���E�ɕ��ׂĕ\��
	void Command_Maximize_V(void);		// �c�����ɍő剻
	void Command_Maximize_H(void);		// �������ɍő剻
	void Command_Minimize_All(void);	// ���ׂčŏ���
	void Command_Redraw(void);			// �ĕ`��
	void Command_Win_Output(void);		// �A�E�g�v�b�g�E�B���h�E�\��
	void Command_TraceOut(const wchar_t* outputstr , int, int);	// �}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\��
	void Command_WinTopMost(LPARAM);		// ��Ɏ�O�ɕ\��
	void Command_WinList(int nCommandFrom);	// �E�B���h�E�ꗗ�|�b�v�A�b�v�\������
	void Command_GroupClose(void);		// �O���[�v�����
	void Command_NextGroup(void);		// ���̃O���[�v
	void Command_PrevGroup(void);		// �O�̃O���[�v
	void Command_Tab_MoveRight(void);	// �^�u���E�Ɉړ�	
	void Command_Tab_MoveLeft(void);	// �^�u�����Ɉړ�	
	void Command_Tab_Separate(void);	// �V�K�O���[�v
	void Command_Tab_JointNext(void);	// ���̃O���[�v�Ɉړ�
	void Command_Tab_JointPrev(void);	// �O�̃O���[�v�Ɉړ�
	void Command_Tab_CloseOther(void);	// ���̃^�u�ȊO�����
	void Command_Tab_CloseLeft(void);	// �������ׂĕ���
	void Command_Tab_CloseRight(void);	// �E�����ׂĕ���


	void Command_ToggleKeySearch(int);	// �L�����b�g�ʒu�̒P���������������@�\ON-OFF

	void Command_Hokan(void);			// ���͕⊮
	void Command_Help_Contents(void);	// �w���v�ڎ�
	void Command_Help_Search(void);		// �w���v�L�[���[�h����
	void Command_Menu_AllFunc(void);	// �R�}���h�ꗗ
	void Command_ExtHelp1(void);		// �O���w���v�P
	void Command_ExtHTMLHelp(const wchar_t* helpfile = NULL, const wchar_t* kwd = NULL);	// �O��HTML�w���v
	void Command_About(void);			// �o�[�W�������

	// ���̑�

private:
	void AlertNotFound(HWND hwnd, bool, LPCTSTR format, ...);
	void DelCharForOverwrite(const wchar_t* pszInput, size_t nLen);	// �㏑���p�̈ꕶ���폜
	bool Sub_PreProcTagJumpByTagsFile( TCHAR* szCurrentPath, size_t count ); // �^�O�W�����v�̑O����
public:
	size_t ConvertEol(const wchar_t* pszText, size_t nTextLen, wchar_t* pszConvertedText);
	void Sub_BoxSelectLock(int flags);

};

