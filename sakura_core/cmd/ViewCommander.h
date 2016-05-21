/*
	Copyright (C) 2008, kobake

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
	LayoutRange& GetSelect();
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
	// Oct. 2, 2001 genta �}�N���p�ɋ@�\�g��
	// Mar. 30, 2003 genta �����ǉ�
	void Command_FileOpen(
		const WCHAR*	filename	= NULL,
		EncodingType	nCharCode	= CODE_AUTODETECT,
		bool			bViewMode	= false,
		const WCHAR*	defaultName	= NULL
	);

	// �㏑���ۑ� // Feb. 28, 2004 genta �����ǉ�, Jan. 24, 2005 genta �����ǉ�
	bool Command_FileSave(bool warnbeep = true, bool askname = true);
	bool Command_FileSaveAs_Dialog(const WCHAR*, EncodingType, EolType);		// ���O��t���ĕۑ�
	bool Command_FileSaveAs(const WCHAR* filename, EolType eEolType);		// ���O��t���ĕۑ�
	bool Command_FileSaveAll(void);				// �S�ď㏑���ۑ� // Jan. 23, 2005 genta
	void Command_FileClose(void);				// �J����(����)	// Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	// ���ĊJ��
	// Mar. 30, 2003 genta �����ǉ�
	void Command_FileClose_Open(LPCWSTR filename = NULL,
		EncodingType nCharCode = CODE_AUTODETECT, bool bViewMode = false);

	void Command_File_Reopen(EncodingType nCharCode, bool bNoConfirm);		// �ăI�[�v��	// Dec. 4, 2002 genta �����ǉ�

	void Command_Print(void);					// ���
	void Command_Print_Preview(void);			// ���Preview
	void Command_Print_PageSetUp(void);			// ����y�[�W�ݒ�	// Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	bool Command_Open_HfromtoC(bool);			// ������C/C++�w�b�_(�\�[�X)���J��	// Feb. 7, 2001 JEPRO �ǉ�
	bool Command_Open_HHPP(bool bCheckOnly, bool bBeepWhenMiss);				// ������C/C++�w�b�_�t�@�C�����J��	// Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	bool Command_Open_CCPP(bool bCheckOnly, bool bBeepWhenMiss);				// ������C/C++�\�[�X�t�@�C�����J��	// Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	void Command_Activate_SQLPlus(void);			// Oracle SQL*Plus���A�N�e�B�u�\��
	void Command_PLSQL_Compile_On_SQLPlus(void);	// Oracle SQL*Plus�Ŏ��s
	void Command_Browse(void);					// �u���E�Y
	void Command_ViewMode(void);				// �r���[���[�h
	void Command_Property_File(void);			// �t�@�C���̃v���p�e�B
	void Command_ProfileMgr( void );			// �v���t�@�C���}�l�[�W��
	void Command_ExitAllEditors(void);			// �ҏW�̑S�I��	// 2007.02.13 ryoji �ǉ�
	void Command_ExitAll(void);					// �T�N���G�f�B�^�̑S�I��	// Dec. 27, 2000 JEPRO �ǉ�
	bool Command_PutFile(LPCWSTR, EncodingType, int);	// ��ƒ��t�@�C���̈ꎞ�o�� maru 2006.12.10
	bool Command_InsFile(LPCWSTR, EncodingType, int);	// �L�����b�g�ʒu�Ƀt�@�C���}�� maru 2006.12.10

	// �ҏW�n
	void Command_WCHAR(wchar_t, bool bConvertEOL = true);			// �������� // 2007.09.02 kobake Command_CHAR(char)��Command_WCHAR(wchar_t)�ɕύX
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
// From Here 2001.12.03 hor
//	void Command_Indent(const char*, int);// �C���f���g ver0
	void Command_Indent(const wchar_t*, LogicInt , IndentType = IndentType::None);// �C���f���g ver0
// To Here 2001.12.03 hor
	void Command_Unindent(wchar_t wcChar);// �t�C���f���g
//	void Command_WORDSREFERENCE(void);		// �P�ꃊ�t�@�����X
	void Command_Trim(bool);				// 2001.12.03 hor
	void Command_Sort(bool);				// 2001.12.06 hor
	void Command_Merge(void);				// 2001.12.06 hor
	void Command_Reconvert(void);			// ���j���[����̍ĕϊ��Ή� minfu 2002.04.09
	void Command_CtrlCode_Dialog(void);		// �R���g���[���R�[�h�̓���(�_�C�A���O)	//@@@ 2002.06.02 MIK


	// �J�[�\���ړ��n
	// Oct. 24, 2001 genta �@�\�g���̂��߈����ǉ�
	void Command_MoveCursor(LogicPoint pos, int option);
	void Command_MoveCursorLayout(LayoutPoint pos, int option);
	int Command_Up(bool bSelect, bool bRepeat, int line = 0);			// �J�[�\����ړ�
	int Command_Down(bool bSelect, bool bRepeat);	// �J�[�\�����ړ�
	int Command_Left(bool, bool);					// �J�[�\�����ړ�
	void Command_Right(bool bSelect, bool bIgnoreCurrentSelection, bool bRepeat);	// �J�[�\���E�ړ�
	void Command_Up2(bool bSelect);					// �J�[�\����ړ��i�Q�s�Âj
	void Command_Down2(bool bSelect);				// �J�[�\�����ړ��i�Q�s�Âj
	void Command_WordLeft(bool bSelect);			// �P��̍��[�Ɉړ�
	void Command_WordRight(bool bSelect);			// �P��̉E�[�Ɉړ�
	// Oct. 29, 2001 genta �}�N�������@�\�g��
	void Command_GoLineTop(bool bSelect, int lparam);	// �s���Ɉړ��i�܂�Ԃ��P�ʁj
	void Command_GoLineEnd(bool bSelect, int , int);	// �s���Ɉړ��i�܂�Ԃ��P�ʁj
//	void Command_ROLLDOWN(int);						// Scroll Down
//	void Command_ROLLUP(int);						// Scroll Up
	void Command_HalfPageUp( bool bSelect, LayoutYInt );		// ���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	void Command_HalfPageDown( bool bSelect, LayoutYInt );		// ���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	void Command_1PageUp( bool bSelect, LayoutYInt );			// �P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	void Command_1PageDown( bool bSelect, LayoutYInt );			// �P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	void Command_GoFileTop(bool bSelect);			// �t�@�C���̐擪�Ɉړ�
	void Command_GoFileEnd(bool bSelect);			// �t�@�C���̍Ō�Ɉړ�
	void Command_CurLineCenter(void);				// �J�[�\���s���E�B���h�E������
	void Command_JumpHist_Prev(void);				// �ړ�����: �O��
	void Command_JumpHist_Next(void);				// �ړ�����: ����
	void Command_JumpHist_Set(void);				// ���݈ʒu���ړ������ɓo�^
	void Command_WndScrollDown(void);				// �e�L�X�g���P�s����Scroll	// 2001/06/20 asa-o
	void Command_WndScrollUp(void);					// �e�L�X�g���P�s���Scroll	// 2001/06/20 asa-o
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
	void Command_ModifyLine_Next( bool bSelect );	// ���̕ύX�s��
	void Command_ModifyLine_Prev( bool bSelect );	// �O�̕ύX�s��

	// �I���n
	bool Command_SelectWord(const LayoutPoint* pptCaretPos = nullptr);		// ���݈ʒu�̒P��I��
	void Command_SelectAll(void);			// ���ׂđI��
	void Command_SelectLine(int lparam);	// 1�s�I��	// 2007.10.13 nasukoji
	void Command_Begin_Select(void);		// �͈͑I���J�n

	// ��`�I���n
//	void Command_BOXSELECTALL(void);		// ��`�ł��ׂđI��
	void Command_Begin_BoxSelect(bool bSelectingLock = false);	// ��`�͈͑I���J�n
//	int Command_UP_BOX(BOOL);				// (��`�I��)�J�[�\����ړ�

	// �N���b�v�{�[�h�n
	void Command_Cut(void);						// �؂���i�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜
	void Command_Copy(bool, bool bAddCRLFWhenCopy, EolType neweol = EolType::Unknown);// �R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	void Command_Paste(int option);				// �\��t���i�N���b�v�{�[�h����\��t��
	void Command_PasteBox(int option);			// ��`�\��t���i�N���b�v�{�[�h�����`�\��t��
	//<< 2002/03/29 Azumaiya
	// ��`�\��t���i�����n���ł̒���t���j
	void Command_PasteBox(const wchar_t* szPaste, int nPasteSize);
	//>> 2002/03/29 Azumaiya
	void Command_InsBoxText(const wchar_t*, int); // ��`�\��t��
	void Command_InsText(bool bRedraw, const wchar_t*, LogicInt, bool bNoWaitCursor,
		bool bLinePaste = false, bool bFastMode = false, const LogicRange* psDelRangeLogicFast = nullptr); // 2004.05.14 Moca �e�L�X�g��\��t�� '\0'�Ή�
	void Command_AddTail(const wchar_t* pszData, int nDataLen);	// �Ō�Ƀe�L�X�g��ǉ�
	void Command_CopyFileName(void);				// ���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ // 2002/2/3 aroka
	void Command_CopyPath(void);					// ���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	void Command_CopyTag(void);						// ���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[
	void Command_CopyLines(void);					// �I��͈͓��S�s�R�s�[
	void Command_CopyLinesAsPassage(void);			// �I��͈͓��S�s���p���t���R�s�[
	void Command_CopyLinesWithLineNumber(void);		// �I��͈͓��S�s�s�ԍ��t���R�s�[
	void Command_Copy_Color_HTML(bool bLineNumber = false);	// �I��͈͓��S�s�s�ԍ��t���R�s�[
	void Command_Copy_Color_HTML_LineNumber(void);		// �I��͈͓��F�t��HTML�R�s�[
	ColorStrategy* GetColorStrategyHTML(const StringRef&, int, const ColorStrategyPool*, ColorStrategy**, ColorStrategy**, bool& bChange);
	void Command_CreateKeyBindList(void);				// �L�[���蓖�Ĉꗗ���R�s�[ // Sept. 15, 2000 JEPRO	Command_�̍������킩��Ȃ��̂ŎE���Ă���

	// �}���n
	void Command_Ins_Date(void);	// ���t�}��
	void Command_Ins_Time(void);	// �����}��

	// �ϊ��n
	void Command_ToLower(void);					// ������
	void Command_ToUpper(void);					// �啶��
	void Command_ToZenkakuKata(void);			// ���p�{�S�Ђ灨�S�p�E�J�^�J�i	// Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	void Command_ToZenkakuHira(void);			// ���p�{�S�J�^���S�p�E�Ђ炪��	// Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	void Command_ToHankaku(void);				// �S�p�����p
	void Command_ToHankata(void);				// �S�p�J�^�J�i�����p�J�^�J�i	// Aug. 29, 2002 ai
	void Command_ToZenEi(void);					// ���p�p�����S�p�p�� // July. 30, 2001 Misaka
	void Command_ToHanEi(void);					// �S�p�p�������p�p�� //@@@ 2002.2.11 YAZAKI
	void Command_HanKataToZenkakuKata(void);	// ���p�J�^�J�i���S�p�J�^�J�i
	void Command_HanKataToZenKakuHira(void);	// ���p�J�^�J�i���S�p�Ђ炪��
	void Command_TabToSpace(void);				// TAB����
	void Command_SpaceToTab(void);				// �󔒁�TAB  //---- Stonee, 2001/05/27
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
	void Command_UUDecode(void);				// uudecode���ĕۑ�	// Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX

	// �����n
	void Command_Search_Box(void);						// ����(�{�b�N�X)	// 2006.06.04 yukihane
	void Command_Search_Dialog(void);					// ����(�P�ꌟ���_�C�A���O)
	void Command_Search_Next(bool, bool, bool, HWND, const WCHAR*, LogicRange* = nullptr);// ��������
	void Command_Search_Prev(bool bReDraw, HWND);		// �O������
	void Command_Replace_Dialog(void);					// �u��(�u���_�C�A���O)
	void Command_Replace(HWND hwndParent);				// �u��(���s) 2002/04/08 YAZAKI �e�E�B���h�E���w�肷��悤�ɕύX
	void Command_Replace_All();							// ���ׂĒu��(���s)
	void Command_Search_ClearMark(void);				// �����}�[�N�̃N���A
	void Command_Jump_SrchStartPos(void);				// �����J�n�ʒu�֖߂�	// 02/06/26 ai


	void Command_Grep_Dialog(void);						// Grep�_�C�A���O�̕\��
	void Command_Grep(void);							// Grep
	void Command_Grep_Replace_Dlg( void );				// Grep�u���_�C�A���O�̕\��
	void Command_Grep_Replace( void );					// Grep�u��
	void Command_Jump_Dialog(void);						// �w��s�w�W�����v�_�C�A���O�̕\��
	void Command_Jump(void);							// �w��s�w�W�����v
// From Here 2001.12.03 hor
	bool Command_FuncList(ShowDialogType nAction, OutlineType outlineType);	// �A�E�g���C����� // 20060201 aroka
// To Here 2001.12.03 hor
	// Apr. 03, 2003 genta �����ǉ�
	bool Command_TagJump(bool bClose = false);			// �^�O�W�����v�@�\
	void Command_TagJumpBack(void);						// �^�O�W�����v�o�b�N�@�\
	bool Command_TagJumpByTagsFileMsg(bool);			// �_�C���N�g�^�O�W�����v(�ʒm��)
	bool Command_TagJumpByTagsFile(bool);				// �_�C���N�g�^�O�W�����v	//@@@ 2003.04.13 MIK

	bool Command_TagsMake(void);						// �^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	bool Command_TagJumpByTagsFileKeyword(const wchar_t* keyword);	//	@@ 2005.03.31 MIK
	void Command_Compare(void);							// �t�@�C�����e��r
	void Command_Diff_Dialog(void);						// DIFF�����\���_�C�A���O	//@@@ 2002.05.25 MIK
	void Command_Diff(const WCHAR* szTmpFile2, int nFlgOpt);	// DIFF�����\��	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	void Command_Diff_Next(void);						// ���̍�����	//@@@ 2002.05.25 MIK
	void Command_Diff_Prev(void);						// �O�̍�����	//@@@ 2002.05.25 MIK
	void Command_Diff_Reset(void);						// �����̑S����	//@@@ 2002.05.25 MIK
	void Command_BracketPair(void);						// �Ί��ʂ̌���
// From Here 2001.12.03 hor
	void Command_Bookmark_Set(void);					// �u�b�N�}�[�N�ݒ�E����
	void Command_Bookmark_Next(void);					// ���̃u�b�N�}�[�N��
	void Command_Bookmark_Prev(void);					// �O�̃u�b�N�}�[�N��
	void Command_Bookmark_Reset(void);					// �u�b�N�}�[�N�̑S����
// To Here 2001.12.03 hor
	void Command_Bookmark_Pattern(void);				// 2002.01.16 hor �w��p�^�[���Ɉ�v����s���}�[�N
	void Command_FuncList_Next( void );					// ���̊֐����X�g�}�[�N	2014.01.05
	void Command_FuncList_Prev( void );					// �O�̊֐����X�g�}�[�N	2014.01.05

	// ���[�h�؂�ւ��n
	void Command_ChgMod_Ins(void);						// �}���^�㏑�����[�h�؂�ւ�
	void Command_Chg_Charset(EncodingType, bool);		// �����R�[�h�Z�b�g�w��	// 2010/6/15 Uchi
	void Command_ChgMod_EOL(EolType);					// ���͂�����s�R�[�h��ݒ� 2003.06.23 moca
	void Command_Cancel_Mode(int whereCursorIs = 0);	// �e�탂�[�h�̎�����

	// �ݒ�n
	void Command_ShowToolBar(void);					// �c�[���o�[�̕\��/��\��
	void Command_ShowFuncKey(void);					// �t�@���N�V�����L�[�̕\��/��\��
	void Command_ShowTab(void);						// �^�u�̕\��/��\��	//@@@ 2003.06.10 MIK
	void Command_ShowStatusBar(void);				// �X�e�[�^�X�o�[�̕\��/��\��
	void Command_ShowMiniMap(void);					// �~�j�}�b�v�̕\��/��\��
	void Command_Type_List(void);					// �^�C�v�ʐݒ�ꗗ
	void Command_ChangeType(int nTypePlusOne);		// �^�C�v�ʐݒ�ꎞ�K�p
	void Command_Option_Type(void);					// �^�C�v�ʐݒ�
	void Command_Option(void);						// ���ʐݒ�
	void Command_Font(void);						// �t�H���g�ݒ�
	void Command_SetFontSize(int, int, int);		// �t�H���g�T�C�Y�ݒ�
	void Command_WrapWindowWidth(void);				// ���݂̃E�B���h�E���Ő܂�Ԃ�	// Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
	void Command_Favorite(void);					// �����̊Ǘ�	//@@@ 2003.04.08 MIK
	void Command_Set_QuoteString(const wchar_t*);	// Jan. 29, 2005 genta ���p���̐ݒ�
	void Command_TextWrapMethod(TextWrappingMethod);// �e�L�X�g�̐܂�Ԃ����@��ύX����		// 2008.05.30 nasukoji
	void Command_Select_Count_Mode(int nMode);		// �����J�E���g���@	// 2009.07.06 syat

	// �}�N���n
	void Command_RecKeyMacro(void);		// �L�[�}�N���̋L�^�J�n�^�I��
	void Command_SaveKeyMacro(void);	// �L�[�}�N���̕ۑ�
	void Command_LoadKeyMacro(void);	// �L�[�}�N���̓ǂݍ���
	void Command_ExecKeyMacro(void);	// �L�[�}�N���̎��s
	void Command_ExecExtMacro(const WCHAR* path, const WCHAR* type);	// ���O���w�肵�ă}�N�����s
// From Here 2006.12.03 maru �����̊g���D
// From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	void Command_ExecCmmand(void);	// �O���R�}���h���s
	// Oct. 9, 2001 genta �}�N���Ή��̂��ߋ@�\�g��
//	void Command_ExecCommand_Dialog(const WCHAR* cmd);	// �O���R�}���h���s�_�C�A���O�\��
//	void Command_ExecCommand(const WCHAR* cmd);	// �O���R�}���h���s
	void Command_ExecCommand_Dialog(void);		// �O���R�}���h���s�_�C�A���O�\��	// �����g���ĂȂ��݂����Ȃ̂�
	// �}�N������̌Ăяo���ł̓I�v�V������ۑ������Ȃ����߁ACommand_ExecCommand_Dialog���ŏ������Ă����D
	void Command_ExecCommand(LPCWSTR cmd, const int nFlgOpt, LPCWSTR);	// �O���R�}���h���s
// To Here Sept. 20, 2000
// To Here 2006.12.03 maru �����̊g��

	// �J�X�^�����j���[
	void Command_Menu_RButton(void);	// �E�N���b�N���j���[
	int Command_CustMenu(int);			// �J�X�^�����j���[�\��

	// �E�B���h�E�n
	void Command_Split_V(void);			// �㉺�ɕ���	// Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	void Command_Split_H(void);			// ���E�ɕ���	// Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	void Command_Split_VH(void);		// �c���ɕ���	// Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	void Command_WinClose(void);		// �E�B���h�E�����
	void Command_FileCloseAll(void);	// ���ׂẴE�B���h�E�����	// Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	void Command_Bind_Window(void);		// �������ĕ\��	// 2004.07.14 Kazika �V�K�ǉ�
	void Command_Cascade(void);			// �d�˂ĕ\��
	void Command_Tile_V(void);			// �㉺�ɕ��ׂĕ\��
	void Command_Tile_H(void);			// ���E�ɕ��ׂĕ\��
	void Command_Maximize_V(void);		// �c�����ɍő剻
	void Command_Maximize_H(void);		// �������ɍő剻  // 2001.02.10 by MIK
	void Command_Minimize_All(void);	// ���ׂčŏ���
	void Command_Redraw(void);			// �ĕ`��
	void Command_Win_Output(void);		// �A�E�g�v�b�g�E�B���h�E�\��
	void Command_TraceOut(const wchar_t* outputstr , int, int);	// �}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\�� maru 2006.04.26
	void Command_WinTopMost(LPARAM);		// ��Ɏ�O�ɕ\�� 2004.09.21 Moca
	void Command_WinList(int nCommandFrom);	// �E�B���h�E�ꗗ�|�b�v�A�b�v�\������	// 2006.03.23 fon // 2006.05.19 genta �����ǉ�
	void Command_GroupClose(void);		// �O���[�v�����		// 2007.06.20 ryoji
	void Command_NextGroup(void);		// ���̃O���[�v			// 2007.06.20 ryoji
	void Command_PrevGroup(void);		// �O�̃O���[�v			// 2007.06.20 ryoji
	void Command_Tab_MoveRight(void);	// �^�u���E�Ɉړ�		// 2007.06.20 ryoji
	void Command_Tab_MoveLeft(void);	// �^�u�����Ɉړ�		// 2007.06.20 ryoji
	void Command_Tab_Separate(void);	// �V�K�O���[�v			// 2007.06.20 ryoji
	void Command_Tab_JointNext(void);	// ���̃O���[�v�Ɉړ�	// 2007.06.20 ryoji
	void Command_Tab_JointPrev(void);	// �O�̃O���[�v�Ɉړ�	// 2007.06.20 ryoji
	void Command_Tab_CloseOther(void);	// ���̃^�u�ȊO�����	// 2008.11.22 syat
	void Command_Tab_CloseLeft(void);	// �������ׂĕ���		// 2008.11.22 syat
	void Command_Tab_CloseRight(void);	// �E�����ׂĕ���		// 2008.11.22 syat


	void Command_ToggleKeySearch(int);	// �L�����b�g�ʒu�̒P���������������@�\ON-OFF	// 2006.03.24 fon

	void Command_Hokan(void);			// ���͕⊮
	void Command_Help_Contents(void);	// �w���v�ڎ�			// Nov. 25, 2000 JEPRO added
	void Command_Help_Search(void);		// �w���v�L�[���[�h����	// Nov. 25, 2000 JEPRO added
	void Command_Menu_AllFunc(void);	// �R�}���h�ꗗ
	void Command_ExtHelp1(void);		// �O���w���v�P
	// Jul. 5, 2002 genta
	void Command_ExtHTMLHelp(const WCHAR* helpfile = NULL, const WCHAR* kwd = NULL);	// �O��HTML�w���v
	void Command_About(void);			// �o�[�W�������	// Dec. 24, 2000 JEPRO �ǉ�

	// ���̑�

private:
	void AlertNotFound(HWND hwnd, bool, LPCTSTR format, ...);
	void DelCharForOverwrite(const wchar_t* pszInput, int nLen);	// �㏑���p�̈ꕶ���폜	// 2009.04.11 ryoji
	bool Sub_PreProcTagJumpByTagsFile( TCHAR* szCurrentPath, int count ); // �^�O�W�����v�̑O����
public:
	LogicInt ConvertEol(const wchar_t* pszText, LogicInt nTextLen, wchar_t* pszConvertedText);
	void Sub_BoxSelectLock(int flags);

};

