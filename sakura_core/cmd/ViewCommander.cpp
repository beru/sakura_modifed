#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "macro/SMacroMgr.h"
#include "EditApp.h"
#include "plugin/JackManager.h"

// EditView�N���X�̃R�}���h�����n�֐��Q

ViewCommander::ViewCommander(EditView& editView)
	:
	view(editView)
{
	bPrevCommand = 0;
	pSMacroMgr = EditApp::getInstance().pSMacroMgr;
}


/*!
	�R�}���h�R�[�h�ɂ�鏈���U�蕪��

	@param nCommand �R�}���h�R�[�h
	@param lparam1 parameter1(���e�̓R�}���h�R�[�h�ɂ���ĕς��܂�)
	@param lparam2 parameter2(���e�̓R�}���h�R�[�h�ɂ���ĕς��܂�)
	@param lparam3 parameter3(���e�̓R�}���h�R�[�h�ɂ���ĕς��܂�)
	@param lparam4 parameter4(���e�̓R�}���h�R�[�h�ɂ���ĕς��܂�)
*/
bool ViewCommander::HandleCommand(
	EFunctionCode	nCommand,
	bool			bRedraw,
	LPARAM			lparam1,
	LPARAM			lparam2,
	LPARAM			lparam3,
	LPARAM			lparam4
	)
{
	bool bRet = true;
	bool bRepeat = false;
	int nFuncID;

	// ���16bit�ɑ��M���̎��ʎq������悤�ɕύX�����̂ŉ���16�r�b�g�݂̂����o��
	// �萔�Ɣ�r���邽�߂ɃV�t�g���Ȃ��Ŏg��
	int nCommandFrom = nCommand & ~0xffff;
	nCommand = (EFunctionCode)LOWORD(nCommand);


	if (view.nAutoScrollMode && nCommand != F_AUTOSCROLL) {
		view.AutoScrollExit();
	}
	view.GetCaret().bClearStatus = true;
	view.TranslateCommand_grep(nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4);
	view.TranslateCommand_isearch(nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4);

	// �@�\�����p�\�����ׂ�
	if (!IsFuncEnable(GetDocument(), GetDllShareData(), nCommand)) {
		return true;
	}

	++GetDocument().nCommandExecNum;		// �R�}���h���s��
//	if (nCommand != F_COPY) {
		// ����Tip������
		view.tipWnd.Hide();
		view.dwTipTimer = ::GetTickCount();	// ����Tip�N���^�C�}�[
//	}
	// ���Preview���[�h��
	if (GetEditWindow().pPrintPreview && nCommand != F_PRINT_PREVIEW) {
		ErrorBeep();
		return true;
	}
	// �L�[���s�[�g���
	if (bPrevCommand == nCommand) {
		bRepeat = true;
	}
	bPrevCommand = nCommand;
	if (GetDllShareData().flags.bRecordingKeyMacro &&									// �L�[�{�[�h�}�N���̋L�^��
		GetDllShareData().flags.hwndRecordingKeyMacro == GetMainWindow() &&	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
		(nCommandFrom & FA_NONRECORD) != FA_NONRECORD	// �L�^�}���t���O off
	) {
		// �L�[���s�[�g��Ԃ��Ȃ�����
		bRepeat = false;
		// �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ�
		// F_EXECEXTMACRO�R�}���h�̓t�@�C����I��������Ƀ}�N�������m�肷�邽�ߌʂɋL�^����B
		if (SMacroMgr::CanFuncIsKeyMacro(nCommand) &&
			nCommand != F_EXECEXTMACRO	// F_EXECEXTMACRO�͌ʂŋL�^���܂�
		) {
			// �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
			LPARAM lparams[] = {lparam1, lparam2, lparam3, lparam4};
			pSMacroMgr->Append(STAND_KEYMACRO, nCommand, lparams, view);
		}
	}

	// �}�N�����s���t���O�̐ݒ�
	// �}�N������̃R�}���h���ǂ�����nCommandFrom�ł킩�邪
	// nCommandFrom�������ŐZ��������̂���ςȂ̂ŁC�]���̃t���O�ɂ��l���R�s�[����
	view.bExecutingKeyMacro = (nCommandFrom & FA_FROMMACRO) ? true : false;

	// �L�[�{�[�h�}�N���̎��s��
	if (view.bExecutingKeyMacro) {
		// �L�[���s�[�g��Ԃ��Ȃ�����
		bRepeat = false;
	}

	// �}�N���̎��s�@�\�ǉ�
	if (F_USERMACRO_0 <= nCommand && nCommand < F_USERMACRO_0 + MAX_CUSTMACRO) {
		if (!pSMacroMgr->Exec(nCommand - F_USERMACRO_0, G_AppInstance(), view,
			nCommandFrom & FA_NONRECORD)
		) {
			InfoMessage(
				view.hwndParent,
				LS(STR_ERR_MACRO1),
				nCommand - F_USERMACRO_0,
				pSMacroMgr->GetFile(nCommand - F_USERMACRO_0)
			);
		}
		return true;
	}

	view.PreprocessCommand_hokan(nCommand);
	if (view.ProcessCommand_isearch(nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4))
		return true;

	// -------------------------------------
	// �������O�ł�Undo�o�b�t�@�̏������ł��Ă��Ȃ��̂�
	// �����̑�����s���Ă͂����Ȃ�
	if (!GetOpeBlk()) {	// ����u���b�N
		SetOpeBlk(new OpeBlk);
	}
	GetOpeBlk()->AddRef();	// �Q�ƃJ�E���^����

	// ��������ł�switch�̌���Undo�𐳂����o�^���邽�߁C
	// �r���ŏ����̑ł��؂���s���Ă͂����Ȃ�
	// -------------------------------------

	switch (nCommand) {
	case F_WCHAR:	// ��������
		{
			Command_WCHAR((wchar_t)lparam1);
		}
		break;

	// �t�@�C������n
	case F_FILENEW:				Command_FileNew(); break;			// �V�K�쐬
	case F_FILENEW_NEWWINDOW:	Command_FileNew_NewWindow(); break;
	case F_FILEOPEN:			Command_FileOpen((const wchar_t*)lparam1); break;			// �t�@�C�����J��
	case F_FILEOPEN2:			Command_FileOpen((const wchar_t*)lparam1, (EncodingType)lparam2, lparam3 != 0, (const wchar_t*)lparam4); break;	// �t�@�C�����J��2
	case F_FILEOPEN_DROPDOWN:	Command_FileOpen((const wchar_t*)lparam1); break;			// �t�@�C�����J��(�h���b�v�_�E��)
	case F_FILESAVE:			bRet = Command_FileSave(); break;	// �㏑���ۑ�
	case F_FILESAVEAS_DIALOG:	bRet = Command_FileSaveAs_Dialog((const wchar_t*)lparam1, (EncodingType)lparam2, (EolType)lparam3); break;	// ���O��t���ĕۑ�
	case F_FILESAVEAS:			bRet = Command_FileSaveAs((const wchar_t*)lparam1, (EolType)lparam3); break;	// ���O��t���ĕۑ�
	case F_FILESAVEALL:			bRet = Command_FileSaveAll(); break;	// �S�Ă̕ҏW�E�B���h�E�ŏ㏑���ۑ�
	case F_FILESAVE_QUIET:		bRet = Command_FileSave(false, false); break;	// �Â��ɏ㏑���ۑ�
	case F_FILESAVECLOSE:
		// �ۑ�������
		// �ۑ����s�v�Ȃ�P�ɕ���
		{	// Command_FileSave()�Ƃ͕ʂɕۑ��s�v���`�F�b�N
			if (!GetDllShareData().common.file.bEnableUnmodifiedOverwrite && !GetDocument().docEditor.IsModified()) {
				Command_WinClose();
				break;
			}
		}
		if (Command_FileSave(false, true)) {
			Command_WinClose();
		}
		break;
	case F_FILECLOSE:										// ����(����)
		Command_FileClose();
		break;
	case F_FILECLOSE_OPEN:	// ���ĊJ��
		Command_FileClose_Open();
		break;
	case F_FILE_REOPEN:				Command_File_Reopen(GetDocument().GetDocumentEncoding(), lparam1 != 0); break;
	case F_FILE_REOPEN_SJIS:		Command_File_Reopen(CODE_SJIS, lparam1 != 0); break;		// SJIS�ŊJ������
	case F_FILE_REOPEN_JIS:			Command_File_Reopen(CODE_JIS, lparam1 != 0); break;			// JIS�ŊJ������
	case F_FILE_REOPEN_EUC:			Command_File_Reopen(CODE_EUC, lparam1 != 0); break;			// EUC�ŊJ������
	case F_FILE_REOPEN_LATIN1:		Command_File_Reopen(CODE_LATIN1, lparam1 != 0); break;		// Latin1�ŊJ���Ȃ���
	case F_FILE_REOPEN_UNICODE:		Command_File_Reopen(CODE_UNICODE, lparam1 != 0); break;		// Unicode�ŊJ������
	case F_FILE_REOPEN_UNICODEBE: 	Command_File_Reopen(CODE_UNICODEBE, lparam1 != 0); break;	// UnicodeBE�ŊJ������
	case F_FILE_REOPEN_UTF8:		Command_File_Reopen(CODE_UTF8, lparam1 != 0); break;		// UTF-8�ŊJ������
	case F_FILE_REOPEN_CESU8:		Command_File_Reopen(CODE_CESU8, lparam1 != 0); break;		// CESU-8�ŊJ���Ȃ���
	case F_FILE_REOPEN_UTF7:		Command_File_Reopen(CODE_UTF7, lparam1 != 0); break;		// UTF-7�ŊJ������
	case F_PRINT:					Command_Print(); break;					// ���
	case F_PRINT_PREVIEW:			Command_Print_Preview(); break;			// ���Preview
	case F_PRINT_PAGESETUP:			Command_Print_PageSetUp(); break;		// ����y�[�W�ݒ�
	case F_OPEN_HfromtoC:			bRet = Command_Open_HfromtoC(lparam1 != 0); break;			// ������C/C++�w�b�_(�\�[�X)���J��
//	case F_OPEN_HHPP:				bRet = Command_Open_HHPP((bool)lparam1, true); break;		// ������C/C++�w�b�_�t�@�C�����J��
//	case F_OPEN_CCPP:				bRet = Command_Open_CCPP((bool)lparam1, true); break;		// ������C/C++�\�[�X�t�@�C�����J��
	case F_ACTIVATE_SQLPLUS:		Command_Activate_SQLPlus(); break;		// Oracle SQL*Plus���A�N�e�B�u�\��
	case F_PLSQL_COMPILE_ON_SQLPLUS:									// Oracle SQL*Plus�Ŏ��s
		Command_PLSQL_Compile_On_SQLPlus();
		break;
	case F_BROWSE:				Command_Browse(); break;			// �u���E�Y
	case F_VIEWMODE:			Command_ViewMode(); break;			// �r���[���[�h
	case F_PROPERTY_FILE:		Command_Property_File(); break;		// �t�@�C���̃v���p�e�B
	case F_PROFILEMGR:			Command_ProfileMgr(); break;		// �v���t�@�C���}�l�[�W��
	case F_EXITALLEDITORS:		Command_ExitAllEditors(); break;	// �ҏW�̑S�I��
	case F_EXITALL:				Command_ExitAll(); break;			// �T�N���G�f�B�^�̑S�I��
	case F_PUTFILE:				Command_PutFile((LPCWSTR)lparam1, (EncodingType)lparam2, (int)lparam3); break;	// ��ƒ��t�@�C���̈ꎞ�o��
	case F_INSFILE:				Command_InsFile((LPCWSTR)lparam1, (EncodingType)lparam2, (int)lparam3); break;	// �L�����b�g�ʒu�Ƀt�@�C���}��

	// �ҏW�n
	case F_UNDO:				Command_Undo(); break;				// ���ɖ߂�(Undo)
	case F_REDO:				Command_Redo(); break;				// ��蒼��(Redo)
	case F_DELETE:				Command_Delete(); break;			// �폜
	case F_DELETE_BACK:			Command_Delete_Back(); break;		// �J�[�\���O���폜
	case F_WordDeleteToStart:	Command_WordDeleteToStart(); break;	// �P��̍��[�܂ō폜
	case F_WordDeleteToEnd:		Command_WordDeleteToEnd(); break;	// �P��̉E�[�܂ō폜
	case F_WordDelete:			Command_WordDelete(); break;		// �P��폜
	case F_WordCut:				Command_WordCut(); break;			// �P��؂���
	case F_LineCutToStart:		Command_LineCutToStart(); break;	// �s���܂Ő؂���(���s�P��)
	case F_LineCutToEnd:		Command_LineCutToEnd(); break;		// �s���܂Ő؂���(���s�P��)
	case F_LineDeleteToStart:	Command_LineDeleteToStart(); break;	// �s���܂ō폜(���s�P��)
	case F_LineDeleteToEnd:		Command_LineDeleteToEnd(); break;	// �s���܂ō폜(���s�P��)
	case F_CUT_LINE:			Command_Cut_Line(); break;			// �s�؂���(�܂�Ԃ��P��)
	case F_DELETE_LINE:			Command_Delete_Line(); break;		// �s�폜(�܂�Ԃ��P��)
	case F_DUPLICATELINE:		Command_DuplicateLine(); break;		// �s�̓�d��(�܂�Ԃ��P��)
	case F_INDENT_TAB:			Command_Indent(WCODE::TAB, IndentType::Tab); break;		// TAB�C���f���g
	case F_UNINDENT_TAB:		Command_Unindent(WCODE::TAB); break;				// �tTAB�C���f���g
	case F_INDENT_SPACE:		Command_Indent(WCODE::SPACE, IndentType::Space); break;	// SPACE�C���f���g
	case F_UNINDENT_SPACE:		Command_Unindent(WCODE::SPACE); break;			// �tSPACE�C���f���g
//	case F_WORDSREFERENCE:		Command_WORDSREFERENCE(); break;		// �P�ꃊ�t�@�����X
	case F_LTRIM:				Command_Trim(true); break;
	case F_RTRIM:				Command_Trim(false); break;
	case F_SORT_ASC:			Command_Sort(true); break;
	case F_SORT_DESC:			Command_Sort(false); break;
	case F_MERGE:				Command_Merge(); break;
	case F_RECONVERT:			Command_Reconvert(); break;			// ���j���[����̍ĕϊ��Ή�

	// �J�[�\���ړ��n
	case F_IME_CHAR:			Command_IME_CHAR((WORD)lparam1); break;					// �S�p��������
	case F_MOVECURSOR:			Command_MoveCursor(Point((int)lparam2, (int)lparam1), (int)lparam3); break;
	case F_MOVECURSORLAYOUT:	Command_MoveCursorLayout(Point((int)lparam2, (int)lparam1), (int)lparam3); break;
	case F_UP:					Command_Up(view.GetSelectionInfo().bSelectingLock, bRepeat); break;				// �J�[�\����ړ�
	case F_DOWN:				Command_Down(view.GetSelectionInfo().bSelectingLock, bRepeat); break;			// �J�[�\�����ړ�
	case F_LEFT:				Command_Left(view.GetSelectionInfo().bSelectingLock, bRepeat); break;			// �J�[�\�����ړ�
	case F_RIGHT:				Command_Right(view.GetSelectionInfo().bSelectingLock, false, bRepeat); break;	// �J�[�\���E�ړ�
	case F_UP2:					Command_Up2(view.GetSelectionInfo().bSelectingLock); break;						// �J�[�\����ړ�(�Q�s�Â�)
	case F_DOWN2:				Command_Down2(view.GetSelectionInfo().bSelectingLock); break;					// �J�[�\�����ړ�(�Q�s�Â�)
	case F_WORDLEFT:			Command_WordLeft(view.GetSelectionInfo().bSelectingLock); break;					// �P��̍��[�Ɉړ�
	case F_WORDRIGHT:			Command_WordRight(view.GetSelectionInfo().bSelectingLock); break;				// �P��̉E�[�Ɉړ�
	// �}�N�������@�\�g��
	case F_GOLINETOP:			Command_GoLineTop(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;		// �s���Ɉړ�(�܂�Ԃ��P��/���s�P��)
	case F_GOLINEEND:			Command_GoLineEnd(view.GetSelectionInfo().bSelectingLock, 0, (int)lparam1); break;	// �s���Ɉړ�(�܂�Ԃ��P��)
	case F_HalfPageUp:			Command_HalfPageUp(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;				// ���y�[�W�A�b�v	
	case F_HalfPageDown:		Command_HalfPageDown(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;			// ���y�[�W�_�E��	
	case F_1PageUp:				Command_1PageUp(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;					// �P�y�[�W�A�b�v	
	case F_1PageDown:			Command_1PageDown(view.GetSelectionInfo().bSelectingLock, (int)lparam1); break;				// �P�y�[�W�_�E��	
	case F_GOFILETOP:			Command_GoFileTop(view.GetSelectionInfo().bSelectingLock); break;				// �t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND:			Command_GoFileEnd(view.GetSelectionInfo().bSelectingLock); break;				// �t�@�C���̍Ō�Ɉړ�
	case F_CURLINECENTER:		Command_CurLineCenter(); break;								// �J�[�\���s���E�B���h�E������
	case F_JUMPHIST_PREV:		Command_JumpHist_Prev(); break;								// �ړ�����: �O��
	case F_JUMPHIST_NEXT:		Command_JumpHist_Next(); break;								// �ړ�����: ����
	case F_JUMPHIST_SET:		Command_JumpHist_Set(); break;								// ���݈ʒu���ړ������ɓo�^
	case F_WndScrollDown:		Command_WndScrollDown(); break;								// �e�L�X�g���P�s����Scroll
	case F_WndScrollUp:			Command_WndScrollUp(); break;								// �e�L�X�g���P�s���Scroll
	case F_GONEXTPARAGRAPH:		Command_GoNextParagraph(view.GetSelectionInfo().bSelectingLock); break;			// ���̒i���֐i��
	case F_GOPREVPARAGRAPH:		Command_GoPrevParagraph(view.GetSelectionInfo().bSelectingLock); break;			// �O�̒i���֖߂�
	case F_AUTOSCROLL:			Command_AutoScroll(); break;									// Auto Scroll
	case F_WHEELUP:				Command_WheelUp((int)lparam1); break;
	case F_WHEELDOWN:			Command_WheelDown((int)lparam1); break;
	case F_WHEELLEFT:			Command_WheelLeft((int)lparam1); break;
	case F_WHEELRIGHT:			Command_WheelRight((int)lparam1); break;
	case F_WHEELPAGEUP:			Command_WheelPageUp((int)lparam1); break;
	case F_WHEELPAGEDOWN:		Command_WheelPageDown((int)lparam1); break;
	case F_WHEELPAGELEFT:		Command_WheelPageLeft((int)lparam1); break;
	case F_WHEELPAGERIGHT:		Command_WheelPageRight((int)lparam1); break;
	case F_MODIFYLINE_NEXT:		Command_ModifyLine_Next( view.GetSelectionInfo().bSelectingLock ); break;	// ���̕ύX�s��
	case F_MODIFYLINE_PREV:		Command_ModifyLine_Prev( view.GetSelectionInfo().bSelectingLock ); break;	// �O�̕ύX�s��

	// �I���n
	case F_SELECTWORD:		Command_SelectWord(); break;					// ���݈ʒu�̒P��I��
	case F_SELECTALL:		Command_SelectAll(); break;						// ���ׂđI��
	case F_SELECTLINE:		Command_SelectLine((int)lparam1); break;				// 1�s�I��
	case F_BEGIN_SEL:		Command_Begin_Select(); break;					// �͈͑I���J�n
	case F_UP_SEL:			Command_Up(true, bRepeat, (int)lparam1); break;		// (�͈͑I��)�J�[�\����ړ�
	case F_DOWN_SEL:		Command_Down(true, bRepeat); break;				// (�͈͑I��)�J�[�\�����ړ�
	case F_LEFT_SEL:		Command_Left(true, bRepeat); break;				// (�͈͑I��)�J�[�\�����ړ�
	case F_RIGHT_SEL:		Command_Right(true, false, bRepeat); break;		// (�͈͑I��)�J�[�\���E�ړ�
	case F_UP2_SEL:			Command_Up2(true); break;						// (�͈͑I��)�J�[�\����ړ�(�Q�s����)
	case F_DOWN2_SEL:		Command_Down2(true); break;						// (�͈͑I��)�J�[�\�����ړ�(�Q�s����)
	case F_WORDLEFT_SEL:	Command_WordLeft(true); break;					// (�͈͑I��)�P��̍��[�Ɉړ�
	case F_WORDRIGHT_SEL:	Command_WordRight(true); break;					// (�͈͑I��)�P��̉E�[�Ɉړ�
	case F_GOLINETOP_SEL:	Command_GoLineTop(true, (int)lparam1); break;		// (�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��/���s�P��)
	case F_GOLINEEND_SEL:	Command_GoLineEnd(true, 0, (int)lparam1); break;		// (�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	case F_HalfPageUp_Sel:	Command_HalfPageUp(true, (int)lparam1); break;				//(�͈͑I��)���y�[�W�A�b�v
	case F_HalfPageDown_Sel:Command_HalfPageDown(true, (int)lparam1); break;			//(�͈͑I��)���y�[�W�_�E��
	case F_1PageUp_Sel:		Command_1PageUp(true, (int)lparam1); break;					//(�͈͑I��)�P�y�[�W�A�b�v
	case F_1PageDown_Sel:	Command_1PageDown(true, (int)lparam1); break;				//(�͈͑I��)�P�y�[�W�_�E��
	case F_GOFILETOP_SEL:	Command_GoFileTop(true); break;					// (�͈͑I��)�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND_SEL:	Command_GoFileEnd(true); break;					// (�͈͑I��)�t�@�C���̍Ō�Ɉړ�
	case F_GONEXTPARAGRAPH_SEL:	Command_GoNextParagraph(true); break;		// ���̒i���֐i��
	case F_GOPREVPARAGRAPH_SEL:	Command_GoPrevParagraph(true); break;		// �O�̒i���֖߂�
	case F_MODIFYLINE_NEXT_SEL:	Command_ModifyLine_Next( true ); break;			//(�͈͑I��)���̕ύX�s��
	case F_MODIFYLINE_PREV_SEL:	Command_ModifyLine_Prev( true ); break;			//(�͈͑I��)�O�̕ύX�s��

	// ��`�I���n
	case F_BEGIN_BOX:		Command_Begin_BoxSelect(true); break;	// ��`�͈͑I���J�n
	case F_UP_BOX:			Sub_BoxSelectLock((int)lparam1); this->Command_Up(true, bRepeat); break;		// (��`�I��)�J�[�\����ړ�
	case F_DOWN_BOX:		Sub_BoxSelectLock((int)lparam1); this->Command_Down(true, bRepeat); break;	// (��`�I��)�J�[�\�����ړ�
	case F_LEFT_BOX:		Sub_BoxSelectLock((int)lparam1); this->Command_Left(true, bRepeat); break;	// (��`�I��)�J�[�\�����ړ�
	case F_RIGHT_BOX:		Sub_BoxSelectLock((int)lparam1); this->Command_Right(true, false, bRepeat); break;	// (��`�I��)�J�[�\���E�ړ�
	case F_UP2_BOX:			Sub_BoxSelectLock((int)lparam1); this->Command_Up2(true); break;				// (��`�I��)�J�[�\����ړ�(�Q�s����)
	case F_DOWN2_BOX:		Sub_BoxSelectLock((int)lparam1); this->Command_Down2(true);break;			// (��`�I��)�J�[�\�����ړ�(�Q�s����)
	case F_WORDLEFT_BOX:	Sub_BoxSelectLock((int)lparam1); this->Command_WordLeft(true);break;			// (��`�I��)�P��̍��[�Ɉړ�
	case F_WORDRIGHT_BOX:	Sub_BoxSelectLock((int)lparam1); this->Command_WordRight(true);break;		// (��`�I��)�P��̉E�[�Ɉړ�
	case F_GOLOGICALLINETOP_BOX:Sub_BoxSelectLock((int)lparam2); this->Command_GoLineTop(true, 8 | (int)lparam1);break;	// (��`�I��)�s���Ɉړ�(���s�P��)
	case F_GOLINETOP_BOX:	Sub_BoxSelectLock((int)lparam2); this->Command_GoLineTop(true, (int)lparam1);break;		// (��`�I��)�s���Ɉړ�(�܂�Ԃ��P��/���s�P��)
	case F_GOLINEEND_BOX:	Sub_BoxSelectLock((int)lparam2); this->Command_GoLineEnd(true, 0, (int)lparam1);break;	// (��`�I��)�s���Ɉړ�(�܂�Ԃ��P��/���s�P��)
	case F_HalfPageUp_BOX:	Sub_BoxSelectLock((int)lparam2); this->Command_HalfPageUp(true, (int)lparam1); break;		// (��`�I��)���y�[�W�A�b�v
	case F_HalfPageDown_BOX:Sub_BoxSelectLock((int)lparam2); this->Command_HalfPageDown(true, (int)lparam1); break;		// (��`�I��)���y�[�W�_�E��
	case F_1PageUp_BOX:		Sub_BoxSelectLock((int)lparam2); this->Command_1PageUp(true, (int)lparam1); break;			// (��`�I��)�P�y�[�W�A�b�v
	case F_1PageDown_BOX:	Sub_BoxSelectLock((int)lparam2); this->Command_1PageDown(true, (int)lparam1); break;			// (��`�I��)�P�y�[�W�_�E��
	case F_GOFILETOP_BOX:	Sub_BoxSelectLock((int)lparam1); this->Command_GoFileTop(true);break;			// (��`�I��)�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND_BOX:	Sub_BoxSelectLock((int)lparam1); this->Command_GoFileEnd(true);break;			// (��`�I��)�t�@�C���̍Ō�Ɉړ�

	// �N���b�v�{�[�h�n
	case F_CUT:						Command_Cut(); break;					// �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	case F_COPY:					Command_Copy(false, GetDllShareData().common.edit.bAddCRLFWhenCopy); break;			// �R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_ADDCRLF:			Command_Copy(false, true); break;		// �܂�Ԃ��ʒu�ɉ��s�����ăR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_CRLF:				Command_Copy(false, GetDllShareData().common.edit.bAddCRLFWhenCopy, EolType::CRLF); break;	// CRLF���s�ŃR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_PASTE:					Command_Paste((int)lparam1); break;					// �\��t��(�N���b�v�{�[�h����\��t��)
	case F_PASTEBOX:				Command_PasteBox((int)lparam1); break;				// ��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	case F_INSBOXTEXT:				Command_InsBoxText((const wchar_t*)lparam1, (int)lparam2); break;				// ��`�e�L�X�g�}��
	case F_INSTEXT_W:				Command_InsText(bRedraw, (const wchar_t*)lparam1, (size_t)lparam2, lparam3 != FALSE); break; // �e�L�X�g��\��t��
	case F_ADDTAIL_W:				Command_AddTail((const wchar_t*)lparam1, (int)lparam2); break;	// �Ō�Ƀe�L�X�g��ǉ�
	case F_COPYFNAME:				Command_CopyFileName(); break;						// ���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[
	case F_COPYPATH:				Command_CopyPath(); break;							// ���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	case F_COPYTAG:					Command_CopyTag(); break;							// ���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[
	case F_COPYLINES:				Command_CopyLines(); break;							// �I��͈͓��S�s�R�s�[
	case F_COPYLINESASPASSAGE:		Command_CopyLinesAsPassage(); break;				// �I��͈͓��S�s���p���t���R�s�[
	case F_COPYLINESWITHLINENUMBER:	Command_CopyLinesWithLineNumber(); break;			// �I��͈͓��S�s�s�ԍ��t���R�s�[
	case F_COPY_COLOR_HTML:				Command_Copy_Color_HTML(); break;				// �I��͈͓��F�t��HTML�R�s�[
	case F_COPY_COLOR_HTML_LINENUMBER:	Command_Copy_Color_HTML_LineNumber(); break;	// �I��͈͓��s�ԍ��F�t��HTML�R�s�[

	case F_CREATEKEYBINDLIST:		Command_CreateKeyBindList(); break;		// �L�[���蓖�Ĉꗗ���R�s�[

	// �}���n
	case F_INS_DATE:				Command_Ins_Date(); break;				// ���t�}��
	case F_INS_TIME:				Command_Ins_Time(); break;				// �����}��
    case F_CTRL_CODE_DIALOG:		Command_CtrlCode_Dialog(); break;		// �R���g���[���R�[�h�̓���(�_�C�A���O)
    case F_CTRL_CODE:				Command_WCHAR((wchar_t)lparam1, false); break;

	// �ϊ�
	case F_TOLOWER:					Command_ToLower(); break;				// ������
	case F_TOUPPER:					Command_ToUpper(); break;				// �啶��
	case F_TOHANKAKU:				Command_ToHankaku(); break;				// �S�p�����p
	case F_TOHANKATA:				Command_ToHankata(); break;				// �S�p�J�^�J�i�����p�J�^�J�i
	case F_TOZENEI:					Command_ToZenEi(); break;				// �S�p�����p
	case F_TOHANEI:					Command_ToHanEi(); break;				// ���p���S�p
	case F_TOZENKAKUKATA:			Command_ToZenkakuKata(); break;			// ���p�{�S�Ђ灨�S�p�E�J�^�J�i
	case F_TOZENKAKUHIRA:			Command_ToZenkakuHira(); break;			// ���p�{�S�J�^���S�p�E�Ђ炪��
	case F_HANKATATOZENKATA:		Command_HanKataToZenkakuKata(); break;	// ���p�J�^�J�i���S�p�J�^�J�i
	case F_HANKATATOZENHIRA:		Command_HanKataToZenKakuHira(); break;	// ���p�J�^�J�i���S�p�Ђ炪��
	case F_TABTOSPACE:				Command_TabToSpace(); break;			// TAB����
	case F_SPACETOTAB:				Command_SpaceToTab(); break;			// �󔒁�TAB
	case F_CODECNV_AUTO2SJIS:		Command_CodeCnv_Auto2SJIS(); break;		// �������ʁ�SJIS�R�[�h�ϊ�
	case F_CODECNV_EMAIL:			Command_CodeCnv_EMail(); break;			// E-Mail(JIS��SJIS)�R�[�h�ϊ�
	case F_CODECNV_EUC2SJIS:		Command_CodeCnv_EUC2SJIS(); break;		// EUC��SJIS�R�[�h�ϊ�
	case F_CODECNV_UNICODE2SJIS:	Command_CodeCnv_Unicode2SJIS(); break;	// Unicode��SJIS�R�[�h�ϊ�
	case F_CODECNV_UNICODEBE2SJIS:	Command_CodeCnv_UnicodeBE2SJIS(); break;	// UnicodeBE��SJIS�R�[�h�ϊ�
	case F_CODECNV_UTF82SJIS:		Command_CodeCnv_UTF82SJIS(); break;		// UTF-8��SJIS�R�[�h�ϊ�
	case F_CODECNV_UTF72SJIS:		Command_CodeCnv_UTF72SJIS(); break;		// UTF-7��SJIS�R�[�h�ϊ�
	case F_CODECNV_SJIS2JIS:		Command_CodeCnv_SJIS2JIS(); break;		// SJIS��JIS�R�[�h�ϊ�
	case F_CODECNV_SJIS2EUC:		Command_CodeCnv_SJIS2EUC(); break;		// SJIS��EUC�R�[�h�ϊ�
	case F_CODECNV_SJIS2UTF8:		Command_CodeCnv_SJIS2UTF8(); break;		// SJIS��UTF-8�R�[�h�ϊ�
	case F_CODECNV_SJIS2UTF7:		Command_CodeCnv_SJIS2UTF7(); break;		// SJIS��UTF-7�R�[�h�ϊ�
	case F_BASE64DECODE:			Command_Base64Decode(); break;			// Base64�f�R�[�h���ĕۑ�
	case F_UUDECODE:				Command_UUDecode(); break;				// uudecode���ĕۑ�

	// �����n
	case F_SEARCH_DIALOG:		Command_Search_Dialog(); break;				// ����(�P�ꌟ���_�C�A���O)
	case F_SEARCH_BOX:			Command_Search_Box(); break;		// ����(�{�b�N�X)
	case F_SEARCH_NEXT:			Command_Search_Next(true, bRedraw, false, (HWND)lparam1, (const wchar_t*)lparam2); break;	// ��������
	case F_SEARCH_PREV:			Command_Search_Prev(bRedraw, (HWND)lparam1); break;						// �O������
	case F_REPLACE_DIALOG:	// �u��(�u���_�C�A���O)
		Command_Replace_Dialog();	// �_�C�A���O�Ăяo���ƁA���s�𕪗�
		break;
	case F_REPLACE:				Command_Replace((HWND)lparam1); break;			// �u�����s
	case F_REPLACE_ALL:			Command_Replace_All(); break;		// ���ׂĒu�����s(�ʏ�)
	case F_SEARCH_CLEARMARK:	Command_Search_ClearMark(); break;	// �����}�[�N�̃N���A
	case F_GREP_DIALOG:	// Grep�_�C�A���O�̕\��
		// �ċA�����΍�
		view.SetUndoBuffer(true);
		Command_Grep_Dialog();
		return bRet;
	case F_GREP:			Command_Grep(); break;							// Grep
	case F_GREP_REPLACE_DLG:	// Grep�u���_�C�A���O�̕\��
		// �ċA�����΍�
		view.SetUndoBuffer( true );
		Command_Grep_Replace_Dlg();
		return bRet;
	case F_GREP_REPLACE:	Command_Grep_Replace();break;							//Grep�u��
	case F_JUMP_DIALOG:		Command_Jump_Dialog(); break;					// �w��s�w�W�����v�_�C�A���O�̕\��
	case F_JUMP:			Command_Jump(); break;							// �w��s�w�W�����v
	case F_OUTLINE:			bRet = Command_FuncList((ShowDialogType)lparam1, OutlineType::Default); break;	// �A�E�g���C�����
	case F_OUTLINE_TOGGLE:	bRet = Command_FuncList(ShowDialogType::Toggle, OutlineType::Default); break;	// �A�E�g���C�����(toggle)
	case F_FILETREE:		bRet = Command_FuncList((ShowDialogType)lparam1, OutlineType::FileTree); break;	//�t�@�C���c���[
	case F_TAGJUMP:			Command_TagJump(lparam1 != 0); break;			// �^�O�W�����v�@�\
	case F_TAGJUMP_CLOSE:	Command_TagJump(true); break;					// �^�O�W�����v(���E�B���h�EClose)
	case F_TAGJUMPBACK:		Command_TagJumpBack(); break;					// �^�O�W�����v�o�b�N�@�\
	case F_TAGS_MAKE:		Command_TagsMake(); break;						// �^�O�t�@�C���̍쐬
	case F_DIRECT_TAGJUMP:	Command_TagJumpByTagsFileMsg(true); break;		// �_�C���N�g�^�O�W�����v�@�\
	case F_TAGJUMP_KEYWORD:	Command_TagJumpByTagsFileKeyword((const wchar_t*)lparam1); break;	// �L�[���[�h���w�肵�ă_�C���N�g�^�O�W�����v�@�\
	case F_COMPARE:			Command_Compare(); break;						// �t�@�C�����e��r
	case F_DIFF_DIALOG:		Command_Diff_Dialog(); break;					// DIFF�����\��(�_�C�A���O)
	case F_DIFF:			Command_Diff((const wchar_t*)lparam1, (int)lparam2); break;		// DIFF�����\��
	case F_DIFF_NEXT:		Command_Diff_Next(); break;						// DIFF�����\��(����)
	case F_DIFF_PREV:		Command_Diff_Prev(); break;						// DIFF�����\��(�O��)
	case F_DIFF_RESET:		Command_Diff_Reset(); break;					// DIFF�����\��(�S����)
	case F_BRACKETPAIR:		Command_BracketPair(); break;					// �Ί��ʂ̌���
	case F_BOOKMARK_SET:	Command_Bookmark_Set(); break;					// �u�b�N�}�[�N�ݒ�E����
	case F_BOOKMARK_NEXT:	Command_Bookmark_Next(); break;					// ���̃u�b�N�}�[�N��
	case F_BOOKMARK_PREV:	Command_Bookmark_Prev(); break;					// �O�̃u�b�N�}�[�N��
	case F_BOOKMARK_RESET:	Command_Bookmark_Reset(); break;				// �u�b�N�}�[�N�̑S����
	case F_BOOKMARK_VIEW:	bRet = Command_FuncList((ShowDialogType)lparam1 , OutlineType::BookMark); break;	// �A�E�g���C�����
	case F_BOOKMARK_PATTERN:Command_Bookmark_Pattern(); break;				// �w��p�^�[���Ɉ�v����s���}�[�N
	case F_JUMP_SRCHSTARTPOS:	Command_Jump_SrchStartPos(); break;			// �����J�n�ʒu�֖߂�
	case F_FUNCLIST_NEXT:	Command_FuncList_Next();break;					// ���̊֐����X�g�}�[�N
	case F_FUNCLIST_PREV:	Command_FuncList_Prev();break;					// �O�̊֐����X�g�}�[�N

	// ���[�h�؂�ւ��n
	case F_CHGMOD_INS:		Command_ChgMod_Ins(); break;		// �}���^�㏑�����[�h�؂�ւ�
	case F_CHG_CHARSET:		Command_Chg_Charset((EncodingType)lparam1, lparam2 != 0); break;	// �����R�[�h�Z�b�g�w��
	// F_CHGMOD_EOL_xxx �̓}�N���ɋL�^����Ȃ����AF_CHGMOD_EOL�̓}�N���ɋL�^�����̂ŁA�}�N���֐��𓝍��ł���Ƃ�����͂�
	case F_CHGMOD_EOL_CRLF:	HandleCommand(F_CHGMOD_EOL, bRedraw, (int)EolType::CRLF, 0, 0, 0); break;	// ���͂�����s�R�[�h��CRLF�ɐݒ�
	case F_CHGMOD_EOL_LF:	HandleCommand(F_CHGMOD_EOL, bRedraw, (int)EolType::LF, 0, 0, 0); break;	// ���͂�����s�R�[�h��LF�ɐݒ�
	case F_CHGMOD_EOL_CR:	HandleCommand(F_CHGMOD_EOL, bRedraw, (int)EolType::CR, 0, 0, 0); break;	// ���͂�����s�R�[�h��CR�ɐݒ�
	case F_CHGMOD_EOL:		Command_ChgMod_EOL((EolType)lparam1); break;	// ���͂�����s�R�[�h��ݒ�
	case F_CANCEL_MODE:		Command_Cancel_Mode(); break;	// �e�탂�[�h�̎�����

	// �ݒ�n
	case F_SHOWTOOLBAR:		Command_ShowToolBar(); break;	// �c�[���o�[�̕\��/��\��
	case F_SHOWFUNCKEY:		Command_ShowFuncKey(); break;	// �t�@���N�V�����L�[�̕\��/��\��
	case F_SHOWTAB:			Command_ShowTab(); break;		// �^�u�̕\��/��\��
	case F_SHOWSTATUSBAR:	Command_ShowStatusBar(); break;	// �X�e�[�^�X�o�[�̕\��/��\��
	case F_SHOWMINIMAP:		Command_ShowMiniMap(); break;	// �~�j�}�b�v�̕\��/��\��
	case F_TYPE_LIST:		Command_Type_List(); break;		// �^�C�v�ʐݒ�ꗗ
	case F_CHANGETYPE:		Command_ChangeType((int)lparam1); break;		// �^�C�v�ʐݒ�ꎞ�K�p
	case F_OPTION_TYPE:		Command_Option_Type(); break;	// �^�C�v�ʐݒ�
	case F_OPTION:			Command_Option(); break;		// ���ʐݒ�
	case F_FONT:			Command_Font(); break;			// �t�H���g�ݒ�
	case F_SETFONTSIZE:		Command_SetFontSize((int)lparam1, (int)lparam2, (int)lparam3); break;	// �t�H���g�T�C�Y�ݒ�
	case F_SETFONTSIZEUP:	HandleCommand(F_SETFONTSIZE, bRedraw, 0, 1, 2, 0); break;	// �t�H���g�T�C�Y�g��
	case F_SETFONTSIZEDOWN:	HandleCommand(F_SETFONTSIZE, bRedraw, 0, -1, 2, 0); break;	// �t�H���g�T�C�Y�k��
	case F_WRAPWINDOWWIDTH:	Command_WrapWindowWidth(); break;// ���݂̃E�B���h�E���Ő܂�Ԃ�
	case F_FAVORITE:		Command_Favorite(); break;		// �����̊Ǘ�
	// ���p���̐ݒ�
	case F_SET_QUOTESTRING:	Command_Set_QuoteString((const wchar_t*)lparam1);	break;
	case F_TMPWRAPNOWRAP:	HandleCommand(F_TEXTWRAPMETHOD, bRedraw, (LPARAM)TextWrappingMethod::NoWrapping, 0, 0, 0); break;	// �܂�Ԃ��Ȃ��i�ꎞ�ݒ�j
	case F_TMPWRAPSETTING:	HandleCommand(F_TEXTWRAPMETHOD, bRedraw, (LPARAM)TextWrappingMethod::SettingWidth, 0, 0, 0); break;	// �w�茅�Ő܂�Ԃ��i�ꎞ�ݒ�j
	case F_TMPWRAPWINDOW:	HandleCommand(F_TEXTWRAPMETHOD, bRedraw, (LPARAM)TextWrappingMethod::WindowWidth, 0, 0, 0); break;	// �E�[�Ő܂�Ԃ��i�ꎞ�ݒ�j
	case F_TEXTWRAPMETHOD:	Command_TextWrapMethod((TextWrappingMethod)lparam1); break;			// �e�L�X�g�̐܂�Ԃ����@
	case F_SELECT_COUNT_MODE:	Command_Select_Count_Mode((int)lparam1); break;		// �����J�E���g�̕��@

	// �}�N���n
	case F_RECKEYMACRO:		Command_RecKeyMacro(); break;	// �L�[�}�N���̋L�^�J�n�^�I��
	case F_SAVEKEYMACRO:	Command_SaveKeyMacro(); break;	// �L�[�}�N���̕ۑ�
	case F_LOADKEYMACRO:	Command_LoadKeyMacro(); break;	// �L�[�}�N���̓ǂݍ���
	case F_EXECKEYMACRO:									// �L�[�}�N���̎��s
		// �ċA�����΍�
		view.SetUndoBuffer(true);
		Command_ExecKeyMacro(); return bRet;
	case F_EXECEXTMACRO:
		// �ċA�����΍�
		view.SetUndoBuffer(true);
		// ���O���w�肵�ă}�N�����s
		Command_ExecExtMacro((const wchar_t*)lparam1, (const wchar_t*)lparam2);
		return bRet;
	//	case F_EXECCMMAND:		Command_ExecCmmand(); break;	// �O���R�}���h���s
	case F_EXECMD_DIALOG:
		//Command_ExecCommand_Dialog((const char*)lparam1);	// �O���R�}���h���s
		Command_ExecCommand_Dialog();	// �O���R�}���h���s	// ���������ĂȂ��݂����Ȃ̂�
		break;
	case F_EXECMD:
		//Command_ExecCommand((const char*)lparam1);
		Command_ExecCommand((LPCWSTR)lparam1, (int)lparam2, (LPCWSTR)lparam3);
		break;

	// �J�X�^�����j���[
	case F_MENU_RBUTTON:	// �E�N���b�N���j���[
		// �ċA�����΍�
		view.SetUndoBuffer(true);
		Command_Menu_RButton();
		return bRet;
	case F_CUSTMENU_1:  // �J�X�^�����j���[1
	case F_CUSTMENU_2:  // �J�X�^�����j���[2
	case F_CUSTMENU_3:  // �J�X�^�����j���[3
	case F_CUSTMENU_4:  // �J�X�^�����j���[4
	case F_CUSTMENU_5:  // �J�X�^�����j���[5
	case F_CUSTMENU_6:  // �J�X�^�����j���[6
	case F_CUSTMENU_7:  // �J�X�^�����j���[7
	case F_CUSTMENU_8:  // �J�X�^�����j���[8
	case F_CUSTMENU_9:  // �J�X�^�����j���[9
	case F_CUSTMENU_10: // �J�X�^�����j���[10
	case F_CUSTMENU_11: // �J�X�^�����j���[11
	case F_CUSTMENU_12: // �J�X�^�����j���[12
	case F_CUSTMENU_13: // �J�X�^�����j���[13
	case F_CUSTMENU_14: // �J�X�^�����j���[14
	case F_CUSTMENU_15: // �J�X�^�����j���[15
	case F_CUSTMENU_16: // �J�X�^�����j���[16
	case F_CUSTMENU_17: // �J�X�^�����j���[17
	case F_CUSTMENU_18: // �J�X�^�����j���[18
	case F_CUSTMENU_19: // �J�X�^�����j���[19
	case F_CUSTMENU_20: // �J�X�^�����j���[20
	case F_CUSTMENU_21: // �J�X�^�����j���[21
	case F_CUSTMENU_22: // �J�X�^�����j���[22
	case F_CUSTMENU_23: // �J�X�^�����j���[23
	case F_CUSTMENU_24: // �J�X�^�����j���[24
		// �ċA�����΍�
		view.SetUndoBuffer(true);
		nFuncID = Command_CustMenu(nCommand - F_CUSTMENU_1 + 1);
		if (nFuncID != 0) {
			// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
//			HandleCommand(nFuncID, true, 0, 0, 0, 0);
			::PostMessage(GetMainWindow(), WM_COMMAND, MAKELONG(nFuncID, 0), (LPARAM)NULL);
		}
		return bRet;

	// �E�B���h�E�n
	case F_SPLIT_V:			Command_Split_V(); break;	// �㉺�ɕ���
	case F_SPLIT_H:			Command_Split_H(); break;	// ���E�ɕ���
	case F_SPLIT_VH:		Command_Split_VH(); break;	// �c���ɕ���
	case F_WINCLOSE:		Command_WinClose(); break;	// �E�B���h�E�����
	case F_WIN_CLOSEALL:	// ���ׂẴE�B���h�E�����
		Command_FileCloseAll();
		break;
	case F_BIND_WINDOW:		Command_Bind_Window(); break;	// �������ĕ\��
	case F_CASCADE:			Command_Cascade(); break;		// �d�˂ĕ\��
	case F_TILE_V:			Command_Tile_V(); break;		// �㉺�ɕ��ׂĕ\��
	case F_TILE_H:			Command_Tile_H(); break;		// ���E�ɕ��ׂĕ\��
	case F_MAXIMIZE_V:		Command_Maximize_V(); break;	// �c�����ɍő剻
	case F_MAXIMIZE_H:		Command_Maximize_H(); break;	// �������ɍő剻
	case F_MINIMIZE_ALL:	Command_Minimize_All(); break;	// ���ׂčŏ���
	case F_REDRAW:			Command_Redraw(); break;		// �ĕ`��
	case F_WIN_OUTPUT:		Command_Win_Output(); break;	// �A�E�g�v�b�g�E�B���h�E�\��
	case F_TRACEOUT:		Command_TraceOut((const wchar_t*)lparam1, (int)lparam2, (int)lparam3); break;		// �}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\��
	case F_TOPMOST:			Command_WinTopMost(lparam1); break;		// ��Ɏ�O�ɕ\��
	case F_WINLIST:			Command_WinList(nCommandFrom); break;	// �E�B���h�E�ꗗ�|�b�v�A�b�v�\������
	case F_GROUPCLOSE:		Command_GroupClose(); break;	// �O���[�v�����
	case F_NEXTGROUP:		Command_NextGroup(); break;		// ���̃O���[�v
	case F_PREVGROUP:		Command_PrevGroup(); break;		// �O�̃O���[�v
	case F_TAB_MOVERIGHT:	Command_Tab_MoveRight(); break;	// �^�u���E�Ɉړ�
	case F_TAB_MOVELEFT:	Command_Tab_MoveLeft(); break;	// �^�u�����Ɉړ�
	case F_TAB_SEPARATE:	Command_Tab_Separate(); break;	// �V�K�O���[�v
	case F_TAB_JOINTNEXT:	Command_Tab_JointNext(); break;	// ���̃O���[�v�Ɉړ�
	case F_TAB_JOINTPREV:	Command_Tab_JointPrev(); break;	// �O�̃O���[�v�Ɉړ�
	case F_TAB_CLOSEOTHER:	Command_Tab_CloseOther(); break;	// ���̃^�u�ȊO�����
	case F_TAB_CLOSELEFT:	Command_Tab_CloseLeft(); break;		// �������ׂĕ���
	case F_TAB_CLOSERIGHT:	Command_Tab_CloseRight(); break;	// �E�����ׂĕ���

	// ����O���[�v���̍�����n�Ԗڂ̃^�u�Ƀt�H�[�J�X�ؑ�
	case F_TAB_1:
	case F_TAB_2:
	case F_TAB_3:
	case F_TAB_4:
	case F_TAB_5:
	case F_TAB_6:
	case F_TAB_7:
	case F_TAB_8:
	case F_TAB_9:
		// TODO: refactoring...
		{
			auto sd = &GetDllShareData();
			HWND hActiveWnd = GetActiveWindow();
			int activeWndGroup = -1;
			for (size_t i=0; i<sd->nodes.nEditArrNum; ++i) {
				auto& editNode = sd->nodes.pEditArr[i];
				if (editNode.GetHwnd() == hActiveWnd) {
					activeWndGroup = editNode.nGroup;
					break;
				}
			}
			if (activeWndGroup != -1) {
				std::vector<int> indices;
				for (size_t i=0; i<sd->nodes.nEditArrNum; ++i) {
					auto& editNode = sd->nodes.pEditArr[i];
					if (editNode.nGroup == activeWndGroup) {
						indices.push_back(editNode.nIndex);
					}
				}
				std::sort(indices.begin(), indices.end());
				int idx = nCommand - F_TAB_1;
				if (idx < (int)indices.size()) {
					int nodeIdx = indices[idx];
					for (size_t i=0; i<sd->nodes.nEditArrNum; ++i) {
						auto& editNode = sd->nodes.pEditArr[i];
						if (editNode.nIndex == nodeIdx) {
							ActivateFrameWindow(editNode.hWnd);
							break;
						}
					}
				}
			}
		}
		break;

	// �x��
	case F_HOKAN:			Command_Hokan(); break;			// ���͕⊮
	case F_HELP_CONTENTS:	Command_Help_Contents(); break;	// �w���v�ڎ�
	case F_HELP_SEARCH:		Command_Help_Search(); break;	// �w���v�g�L�[���[�h����
	case F_TOGGLE_KEY_SEARCH:	Command_ToggleKeySearch((int)lparam1); break;	// �L�����b�g�ʒu�̒P���������������@�\ON-OFF
	case F_MENU_ALLFUNC:									// �R�}���h�ꗗ
		// �ċA�����΍�
		view.SetUndoBuffer(true);
		Command_Menu_AllFunc();
		return bRet;
	case F_EXTHELP1:	Command_ExtHelp1(); break;		// �O���w���v�P
	case F_EXTHTMLHELP:	// �O��HTML�w���v
		Command_ExtHTMLHelp((const wchar_t*)lparam1, (const wchar_t*)lparam2);
		break;
	case F_ABOUT:	Command_About(); break;				// �o�[�W�������

	// ���̑�

	case F_0: break; // F_0�Ńv���O�C�������s�����o�O�΍�	// �� rev1886 �̖��͌Ăь��ő΍􂵂������S�قƂ��Ďc��

	default:
		// �v���O�C���R�}���h�����s����
		{
			view.SetUndoBuffer(true); // �ċA�Ή�

			Plug::Array plugs;
			JackManager::getInstance().GetUsablePlug(PP_COMMAND, nCommand, &plugs);
			if (plugs.size() > 0) {
				assert_warning(plugs.size() == 1);
				// �C���^�t�F�[�X�I�u�W�F�N�g����
				WSHIfObj::List params;
				// �v���O�C���Ăяo��
				(*plugs.begin())->Invoke(view, params);

				return bRet;
			}
		}
	}

	// �A���h�D�o�b�t�@�̏���
	view.SetUndoBuffer(true);

	return bRet;
}


void ViewCommander::Sub_BoxSelectLock( int flags )
{
	bool bSelLock;
	if (flags == 0x00) {
		bSelLock = GetDllShareData().common.edit.bBoxSelectLock;
	}else if (flags == 0x01) {
		bSelLock = true;
	}else if (flags == 0x02) {
		bSelLock = false;
	}
	if (!this->view.GetSelectionInfo().IsBoxSelecting()) {
		this->Command_Begin_BoxSelect( bSelLock );
	}
}


size_t ViewCommander::ConvertEol(
	const wchar_t* pszText,
	size_t nTextLen,
	wchar_t* pszConvertedText
	)
{
	size_t nConvertedTextLen;
	Eol eol = GetDocument().docEditor.GetNewLineCode();

	nConvertedTextLen = 0;
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
	if (!pszConvertedText) {
		for (size_t i=0; i<nTextLen; ++i) {
			if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
				if (pszText[i] == WCODE::CR) {
					if (i + 1 < nTextLen && pszText[i + 1] == WCODE::LF) {
						++i;
					}
				}
				nConvertedTextLen += eol.GetLen();
			}else {
				++nConvertedTextLen;
			}
		}
	}else {
		for (size_t i=0; i<nTextLen; ++i) {
			if (WCODE::IsLineDelimiter(pszText[i], bExtEol)) {
				if (pszText[i] == WCODE::CR) {
					if (i + 1 < nTextLen && pszText[i + 1] == WCODE::LF) {
						++i;
					}
				}
				wmemcpy(&pszConvertedText[nConvertedTextLen], eol.GetValue2(), eol.GetLen());
				nConvertedTextLen += eol.GetLen();
			}else {
				pszConvertedText[nConvertedTextLen++] = pszText[i];
			}
		}
	}
	return nConvertedTextLen;
}


/*!
	@brief �����Ō�����Ȃ��Ƃ��̌x���i���b�Z�[�W�{�b�N�X�^�T�E���h�j
*/
void ViewCommander::AlertNotFound(
	HWND hwnd,
	bool bReplaceAll,
	LPCTSTR format,
	...
	)
{
	if (GetDllShareData().common.search.bNotifyNotFound
		&& !bReplaceAll
	) {
		if (!hwnd) {
			hwnd = view.GetHwnd();
		}
		//InfoMessage(hwnd, format, __VA_ARGS__);
		va_list p;
		va_start(p, format);
		VMessageBoxF(hwnd, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME, format, p);
		va_end(p);
	}else {
		DefaultBeep();
	}
}

