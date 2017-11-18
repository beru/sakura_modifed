#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "_main/ControlTray.h"
#include "EditApp.h"
#include "GrepAgent.h"
#include "plugin/Plugin.h"
#include "plugin/JackManager.h"

// ViewCommander�N���X�̃R�}���h(Grep)�֐��Q

/*! GREP�_�C�A���O�̕\��

	@date 2005.01.10 genta CEditView_Command���ړ�
	@author Yazaki
*/
void ViewCommander::Command_Grep_Dialog(void)
{
	NativeW memCurText;
	auto& dlgGrep = GetEditWindow().dlgGrep;
	// 2014.07.01 ����Grep�E�B���h�E���g�������Ă���ꍇ�Ȃǂɉe�����Ȃ��悤�ɁA���ݒ�̂Ƃ�����History������
	bool bGetHistory = (dlgGrep.bSetText == false);

	// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	bool bSet = view.GetCurrentTextForSearchDlg(memCurText, bGetHistory);	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	if (bSet) {
		dlgGrep.strText = memCurText.GetStringPtr();
		dlgGrep.bSetText = true;
	}

	// Grep�_�C�A���O�̕\��
	INT_PTR nRet = dlgGrep.DoModal(G_AppInstance(), view.GetHwnd(), GetDocument().docFile.GetFilePath());
//	MYTRACE(_T("nRet=%d\n"), nRet);
	if (!nRet) {
		return;
	}
	HandleCommand(F_GREP, true, 0, 0, 0, 0);	// GREP�R�}���h�̔��s
}

/*! GREP���s

	@date 2005.01.10 genta CEditView_Command���ړ�
*/
void ViewCommander::Command_Grep(void)
{
	NativeW mWork1;
	NativeT mWork2;
	NativeT mWork3;
	NativeW	mWork4;

	auto& dlgGrep = GetEditWindow().dlgGrep;
	mWork1.SetString(dlgGrep.strText.c_str());
	mWork2.SetString(dlgGrep.szFile);
	mWork3.SetString(dlgGrep.szFolder);

	auto& grepAgent = *EditApp::getInstance().pGrepAgent;
	auto& doc = GetDocument();
	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��A�܂��͖��ҏW�Ŗ��肩�A�E�g�v�b�g�łȂ��ꍇ�B
		���E�B���h�E��Grep���s�����A(�ُ�I������̂�)�ʃE�B���h�E�ɂ���
	*/
	if (
		(grepAgent.bGrepMode && !grepAgent.bGrepRunning)
		|| (
			!doc.docEditor.IsModified()
			&& !doc.docFile.GetFilePathClass().IsValidPath()		// ���ݕҏW���̃t�@�C���̃p�X
			&& !AppMode::getInstance().IsDebugMode()
		)
	) {
		// 2011.01.23 Grep�^�C�v�ʓK�p
		if (!doc.docEditor.IsModified() && doc.docLineMgr.GetLineCount() == 0) {
			TypeConfigNum typeGrep = DocTypeManager().GetDocumentTypeOfExt(_T("grepout"));
			const TypeConfigMini* pConfig;
			DocTypeManager().GetTypeConfigMini(typeGrep, &pConfig);
			doc.docType.SetDocumentTypeIdx(pConfig->id);
			doc.docType.LockDocumentType();
			doc.OnChangeType();
		}
		
		grepAgent.DoGrep(
			view,
			false,
			&mWork1,
			&mWork4,
			&mWork2,
			&mWork3,
			false,
			dlgGrep.bSubFolder,
			false,
			true, // Header
			dlgGrep.searchOption,
			dlgGrep.nGrepCharSet,
			dlgGrep.nGrepOutputLineType,
			dlgGrep.nGrepOutputStyle,
			dlgGrep.bGrepOutputFileOnly,
			dlgGrep.bGrepOutputBaseFolder,
			dlgGrep.bGrepSeparateFolder,
			false,
			false
		);

		// �v���O�C���FDocumentOpen�C�x���g���s
		Plug::Array plugs;
		WSHIfObj::List params;
		JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(GetEditWindow().GetActiveView(), params);
		}
	}else {
		// �ҏW�E�B���h�E�̏���`�F�b�N
		if (GetDllShareData().nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// �ő�l�C��	//@@@ 2003.05.31 MIK
			OkMessage(view.GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
			return;
		}

		//======= Grep�̎��s =============
		// Grep���ʃE�B���h�E�̕\��
		ControlTray::DoGrepCreateWindow(G_AppInstance(), view.GetHwnd(), dlgGrep);
	}
	return;
}


/*! GREP�u���_�C�A���O�̕\��
*/
void ViewCommander::Command_Grep_Replace_Dlg( void )
{
	NativeW memCurText;
	DlgGrepReplace& dlgGrepRep = GetEditWindow().dlgGrepReplace;

	// ����Grep�E�B���h�E���g�������Ă���ꍇ�Ȃǂɉe�����Ȃ��悤�ɁA���ݒ�̂Ƃ�����History������
	bool bGetHistory = dlgGrepRep.bSetText == false;

	view.GetCurrentTextForSearchDlg( memCurText, bGetHistory );

	if (0 < memCurText.GetStringLength()) {
		dlgGrepRep.strText = memCurText.GetStringPtr();
		dlgGrepRep.bSetText = true;
	}
	if (0 < GetDllShareData().searchKeywords.replaceKeys.size()) {
		if (dlgGrepRep.nReplaceKeySequence < GetDllShareData().common.search.nReplaceKeySequence) {
			dlgGrepRep.strText2 = GetDllShareData().searchKeywords.replaceKeys[0];
		}
	}

	INT_PTR nRet = dlgGrepRep.DoModal( G_AppInstance(), view.GetHwnd(), GetDocument().docFile.GetFilePath(), (LPARAM)&view );
	if (!nRet) {
		return;
	}
	HandleCommand(F_GREP_REPLACE, true, 0, 0, 0, 0);	//	GREP�R�}���h�̔��s
}

/*! GREP�u�����s
*/
void ViewCommander::Command_Grep_Replace(void)
{
	NativeW cmWork1;
	NativeT cmWork2;
	NativeT cmWork3;
	NativeW cmWork4;

	DlgGrepReplace& dlgGrepRep = GetEditWindow().dlgGrepReplace;
	cmWork1.SetString( dlgGrepRep.strText.c_str() );
	cmWork2.SetString( dlgGrepRep.szFile );
	cmWork3.SetString( dlgGrepRep.szFolder );
	cmWork4.SetString( dlgGrepRep.strText2.c_str() );

	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��A�܂��͖��ҏW�Ŗ��肩�A�E�g�v�b�g�łȂ��ꍇ�B
		���E�B���h�E��Grep���s�����A(�ُ�I������̂�)�ʃE�B���h�E�ɂ���
	*/
	auto& grepAgent = *EditApp::getInstance().pGrepAgent;
	if ((grepAgent.bGrepMode &&
		  !grepAgent.bGrepRunning ) ||
		( !GetDocument().docEditor.IsModified() &&
		  !GetDocument().docFile.GetFilePathClass().IsValidPath() &&		// ���ݕҏW���̃t�@�C���̃p�X
		  !AppMode::getInstance().IsDebugMode()
		)
	) {
		grepAgent.DoGrep(
			view,
			true,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			dlgGrepRep.bSubFolder,
			false, // Stdout
			true, // Header
			dlgGrepRep.searchOption,
			dlgGrepRep.nGrepCharSet,
			dlgGrepRep.nGrepOutputLineType,
			dlgGrepRep.nGrepOutputStyle,
			dlgGrepRep.bGrepOutputFileOnly,
			dlgGrepRep.bGrepOutputBaseFolder,
			dlgGrepRep.bGrepSeparateFolder,
			dlgGrepRep.bPaste,
			dlgGrepRep.bBackup
		);
	}else {
		// �ҏW�E�B���h�E�̏���`�F�b�N
		if (GetDllShareData().nodes.nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
			OkMessage( view.GetHwnd(), _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
			return;
		}
		// ======= Grep�̎��s =============
		// Grep���ʃE�B���h�E�̕\��
		cmWork1.Replace( L"\"", L"\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );
		cmWork4.Replace( L"\"", L"\"\"" );

		// -GREPMODE -GKEY="1" -GREPR="2" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		NativeT cmdLine;
		TCHAR szTemp[20];
		cmdLine.AppendStringLiteral(_T("-GREPMODE -GKEY=\""));
		cmdLine.AppendStringW(cmWork1.GetStringPtr());
		cmdLine.AppendStringLiteral(_T("\" -GREPR=\""));
		cmdLine.AppendStringW(cmWork4.GetStringPtr());
		cmdLine.AppendStringLiteral(_T("\" -GFILE=\""));
		cmdLine.AppendString(cmWork2.GetStringPtr());
		cmdLine.AppendStringLiteral(_T("\" -GFOLDER=\""));
		cmdLine.AppendString(cmWork3.GetStringPtr());
		cmdLine.AppendStringLiteral(_T("\" -GCODE="));
		auto_sprintf( szTemp, _T("%d"), dlgGrepRep.nGrepCharSet );
		cmdLine.AppendString(szTemp);

		//GOPT�I�v�V����
		TCHAR	pOpt[64];
		pOpt[0] = _T('\0');
		if (dlgGrepRep.bSubFolder				) _tcscat( pOpt, _T("S") );	// �T�u�t�H���_�������������
		if (dlgGrepRep.searchOption.bWordOnly	) _tcscat( pOpt, _T("W") );	// �P��P�ʂŒT��
		if (dlgGrepRep.searchOption.bLoHiCase	) _tcscat( pOpt, _T("L") );	// �p�啶���Ɖp����������ʂ���
		if (dlgGrepRep.searchOption.bRegularExp	) _tcscat( pOpt, _T("R") );	// ���K�\��
		if (dlgGrepRep.nGrepOutputLineType == 1	) _tcscat( pOpt, _T("P") );	// �s���o�͂���
		// if (dlgGrepRep.nGrepOutputLineType == 2) _tcscat( pOpt, _T("N") );	// �ۃq�b�g�s���o�͂��� 2014.09.23
		if (dlgGrepRep.nGrepOutputStyle == 1	) _tcscat( pOpt, _T("1") );	// Grep: �o�͌`��
		if (dlgGrepRep.nGrepOutputStyle == 2	) _tcscat( pOpt, _T("2") );	// Grep: �o�͌`��
		if (dlgGrepRep.nGrepOutputStyle == 3	) _tcscat( pOpt, _T("3") );
		if (dlgGrepRep.bGrepOutputFileOnly		) _tcscat( pOpt, _T("F") );
		if (dlgGrepRep.bGrepOutputBaseFolder	) _tcscat( pOpt, _T("B") );
		if (dlgGrepRep.bGrepSeparateFolder		) _tcscat( pOpt, _T("D") );
		if (dlgGrepRep.bPaste					) _tcscat( pOpt, _T("C") );	// �N���b�v�{�[�h����\��t��
		if (dlgGrepRep.bBackup				) _tcscat( pOpt, _T("O") );	// �o�b�N�A�b�v�쐬
		if (0 < _tcslen( pOpt )) {
			cmdLine.AppendStringLiteral( _T(" -GOPT=") );
			cmdLine.AppendString( pOpt );
		}

		LoadInfo loadInfo;
		loadInfo.filePath = _T("");
		loadInfo.eCharCode = CODE_NONE;
		loadInfo.bViewMode = false;
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			view.GetHwnd(),
			loadInfo,
			cmdLine.GetStringPtr(),
			false,
			NULL,
			GetDllShareData().common.tabBar.bNewWindow
		);
	}
	return;
}

