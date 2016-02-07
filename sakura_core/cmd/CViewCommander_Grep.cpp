/*!	@file
@brief ViewCommander�N���X�̃R�}���h(Grep)�֐��Q

*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "plugin/CPlugin.h"
#include "plugin/CJackManager.h"

/*! GREP�_�C�A���O�̕\��

	@date 2005.01.10 genta CEditView_Command���ړ�
	@author Yazaki
*/
void ViewCommander::Command_GREP_DIALOG(void)
{
	CNativeW cmemCurText;
	// 2014.07.01 ����Grep�E�B���h�E���g�������Ă���ꍇ�Ȃǂɉe�����Ȃ��悤�ɁA���ݒ�̂Ƃ�����History������
	bool bGetHistory = GetEditWindow()->m_cDlgGrep.m_bSetText == false;

	// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
	bool bSet = m_pCommanderView->GetCurrentTextForSearchDlg(cmemCurText, bGetHistory);	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	if (bSet) {
		GetEditWindow()->m_cDlgGrep.m_strText = cmemCurText.GetStringPtr();
		GetEditWindow()->m_cDlgGrep.m_bSetText = true;
	}

	// Grep�_�C�A���O�̕\��
	int nRet = GetEditWindow()->m_cDlgGrep.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath());
//	MYTRACE(_T("nRet=%d\n"), nRet);
	if (!nRet) {
		return;
	}
	HandleCommand(F_GREP, true, 0, 0, 0, 0);	// GREP�R�}���h�̔��s
}

/*! GREP���s

	@date 2005.01.10 genta CEditView_Command���ړ�
*/
void ViewCommander::Command_GREP(void)
{
	CNativeW cmWork1;
	CNativeT cmWork2;
	CNativeT cmWork3;
	CNativeW		cmWork4;

	auto& dlgGrep = GetEditWindow()->m_cDlgGrep;
	cmWork1.SetString(dlgGrep.m_strText.c_str());
	cmWork2.SetString(dlgGrep.m_szFile);
	cmWork3.SetString(dlgGrep.m_szFolder);

	auto& grepAgent = *CEditApp::getInstance()->m_pcGrepAgent;
	auto& doc = *GetDocument();
	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��A�܂��͖��ҏW�Ŗ��肩�A�E�g�v�b�g�łȂ��ꍇ�B
		���E�B���h�E��Grep���s�����A(�ُ�I������̂�)�ʃE�B���h�E�ɂ���
	*/
	if (
		(
			grepAgent.m_bGrepMode
			&& !grepAgent.m_bGrepRunning
		)
		|| (
			!doc.m_cDocEditor.IsModified()
			&& !doc.m_cDocFile.GetFilePathClass().IsValidPath()		// ���ݕҏW���̃t�@�C���̃p�X
			&& !AppMode::getInstance()->IsDebugMode()
		)
	) {
		// 2011.01.23 Grep�^�C�v�ʓK�p
		if (!doc.m_cDocEditor.IsModified() && doc.m_cDocLineMgr.GetLineCount() == 0) {
			CTypeConfig cTypeGrep = CDocTypeManager().GetDocumentTypeOfExt(_T("grepout"));
			const TypeConfigMini* pConfig;
			CDocTypeManager().GetTypeConfigMini(cTypeGrep, &pConfig);
			doc.m_cDocType.SetDocumentTypeIdx(pConfig->m_id);
			doc.m_cDocType.LockDocumentType();
			doc.OnChangeType();
		}
		
		grepAgent.DoGrep(
			m_pCommanderView,
			false,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			dlgGrep.m_bSubFolder,
			false,
			true, // Header
			dlgGrep.m_searchOption,
			dlgGrep.m_nGrepCharSet,
			dlgGrep.m_nGrepOutputLineType,
			dlgGrep.m_nGrepOutputStyle,
			dlgGrep.m_bGrepOutputFileOnly,
			dlgGrep.m_bGrepOutputBaseFolder,
			dlgGrep.m_bGrepSeparateFolder,
			false,
			false
		);

		// �v���O�C���FDocumentOpen�C�x���g���s
		CPlug::Array plugs;
		CWSHIfObj::List params;
		CJackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
		for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
			(*it)->Invoke(&GetEditWindow()->GetActiveView(), params);
		}
	}else {
		// �ҏW�E�B���h�E�̏���`�F�b�N
		if (GetDllShareData().m_nodes.m_nEditArrNum >= MAX_EDITWINDOWS) {	// �ő�l�C��	//@@@ 2003.05.31 MIK
			OkMessage(m_pCommanderView->GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
			return;
		}

		//======= Grep�̎��s =============
		// Grep���ʃE�B���h�E�̕\��
		ControlTray::DoGrepCreateWindow(G_AppInstance(), m_pCommanderView->GetHwnd(), dlgGrep);
	}
	return;
}


/*! GREP�u���_�C�A���O�̕\��
*/
void ViewCommander::Command_GREP_REPLACE_DLG( void )
{
	CNativeW cmemCurText;
	CDlgGrepReplace& cDlgGrepRep = GetEditWindow()->m_cDlgGrepReplace;

	// ����Grep�E�B���h�E���g�������Ă���ꍇ�Ȃǂɉe�����Ȃ��悤�ɁA���ݒ�̂Ƃ�����History������
	bool bGetHistory = cDlgGrepRep.m_bSetText == false;

	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText, bGetHistory );

	if (0 < cmemCurText.GetStringLength()) {
		cDlgGrepRep.m_strText = cmemCurText.GetStringPtr();
		cDlgGrepRep.m_bSetText = true;
	}
	if (0 < GetDllShareData().m_searchKeywords.m_aReplaceKeys.size()) {
		if (cDlgGrepRep.m_nReplaceKeySequence < GetDllShareData().m_common.m_sSearch.m_nReplaceKeySequence) {
			cDlgGrepRep.m_strText2 = GetDllShareData().m_searchKeywords.m_aReplaceKeys[0];
		}
	}

	int nRet = cDlgGrepRep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath(), (LPARAM)m_pCommanderView );
	if (!nRet) {
		return;
	}
	HandleCommand(F_GREP_REPLACE, TRUE, 0, 0, 0, 0);	//	GREP�R�}���h�̔��s
}

/*! GREP�u�����s
*/
void ViewCommander::Command_GREP_REPLACE(void)
{
	CNativeW cmWork1;
	CNativeT cmWork2;
	CNativeT cmWork3;
	CNativeW cmWork4;

	CDlgGrepReplace& cDlgGrepRep = GetEditWindow()->m_cDlgGrepReplace;
	cmWork1.SetString( cDlgGrepRep.m_strText.c_str() );
	cmWork2.SetString( cDlgGrepRep.m_szFile );
	cmWork3.SetString( cDlgGrepRep.m_szFolder );
	cmWork4.SetString( cDlgGrepRep.m_strText2.c_str() );

	/*	����EditView��Grep���ʂ�\������B
		Grep���[�h�̂Ƃ��A�܂��͖��ҏW�Ŗ��肩�A�E�g�v�b�g�łȂ��ꍇ�B
		���E�B���h�E��Grep���s�����A(�ُ�I������̂�)�ʃE�B���h�E�ɂ���
	*/
	if (( CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode &&
		  !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() &&		// ���ݕҏW���̃t�@�C���̃p�X
		  !AppMode::getInstance()->IsDebugMode()
		)
	) {
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			true,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			cDlgGrepRep.m_bSubFolder,
			false, // Stdout
			true, // Header
			cDlgGrepRep.m_searchOption,
			cDlgGrepRep.m_nGrepCharSet,
			cDlgGrepRep.m_nGrepOutputLineType,
			cDlgGrepRep.m_nGrepOutputStyle,
			cDlgGrepRep.m_bGrepOutputFileOnly,
			cDlgGrepRep.m_bGrepOutputBaseFolder,
			cDlgGrepRep.m_bGrepSeparateFolder,
			cDlgGrepRep.m_bPaste,
			cDlgGrepRep.m_bBackup
		);
	}else {
		// �ҏW�E�B���h�E�̏���`�F�b�N
		if (GetDllShareData().m_nodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//�ő�l�C��	//@@@ 2003.05.31 MIK
			OkMessage( m_pCommanderView->GetHwnd(), _T("�ҏW�E�B���h�E���̏����%d�ł��B\n����ȏ�͓����ɊJ���܂���B"), MAX_EDITWINDOWS );
			return;
		}
		// ======= Grep�̎��s =============
		// Grep���ʃE�B���h�E�̕\��
		cmWork1.Replace( L"\"", L"\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );
		cmWork4.Replace( L"\"", L"\"\"" );

		// -GREPMODE -GKEY="1" -GREPR="2" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		CNativeT cCmdLine;
		TCHAR szTemp[20];
		cCmdLine.AppendString(_T("-GREPMODE -GKEY=\""));
		cCmdLine.AppendStringW(cmWork1.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GREPR=\""));
		cCmdLine.AppendStringW(cmWork4.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GFILE=\""));
		cCmdLine.AppendString(cmWork2.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GFOLDER=\""));
		cCmdLine.AppendString(cmWork3.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GCODE="));
		auto_sprintf( szTemp, _T("%d"), cDlgGrepRep.m_nGrepCharSet );
		cCmdLine.AppendString(szTemp);

		//GOPT�I�v�V����
		TCHAR	pOpt[64];
		pOpt[0] = _T('\0');
		if (cDlgGrepRep.m_bSubFolder				) _tcscat( pOpt, _T("S") );	// �T�u�t�H���_�������������
		if (cDlgGrepRep.m_searchOption.bWordOnly	) _tcscat( pOpt, _T("W") );	// �P��P�ʂŒT��
		if (cDlgGrepRep.m_searchOption.bLoHiCase	) _tcscat( pOpt, _T("L") );	// �p�啶���Ɖp����������ʂ���
		if (cDlgGrepRep.m_searchOption.bRegularExp	) _tcscat( pOpt, _T("R") );	// ���K�\��
		if (cDlgGrepRep.m_nGrepOutputLineType == 1	) _tcscat( pOpt, _T("P") );	// �s���o�͂���
		// if (cDlgGrepRep.m_nGrepOutputLineType == 2) _tcscat( pOpt, _T("N") );	// �ۃq�b�g�s���o�͂��� 2014.09.23
		if (cDlgGrepRep.m_nGrepOutputStyle == 1		) _tcscat( pOpt, _T("1") );	// Grep: �o�͌`��
		if (cDlgGrepRep.m_nGrepOutputStyle == 2		) _tcscat( pOpt, _T("2") );	// Grep: �o�͌`��
		if (cDlgGrepRep.m_nGrepOutputStyle == 3		) _tcscat( pOpt, _T("3") );
		if (cDlgGrepRep.m_bGrepOutputFileOnly		) _tcscat( pOpt, _T("F") );
		if (cDlgGrepRep.m_bGrepOutputBaseFolder		) _tcscat( pOpt, _T("B") );
		if (cDlgGrepRep.m_bGrepSeparateFolder		) _tcscat( pOpt, _T("D") );
		if (cDlgGrepRep.m_bPaste					) _tcscat( pOpt, _T("C") );	// �N���b�v�{�[�h����\��t��
		if (cDlgGrepRep.m_bBackup					) _tcscat( pOpt, _T("O") );	// �o�b�N�A�b�v�쐬
		if (0 < _tcslen( pOpt )) {
			cCmdLine.AppendString( _T(" -GOPT=") );
			cCmdLine.AppendString( pOpt );
		}

		LoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		sLoadInfo.eCharCode = CODE_NONE;
		sLoadInfo.bViewMode = false;
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			sLoadInfo,
			cCmdLine.GetStringPtr(),
			false,
			NULL,
			GetDllShareData().m_common.m_sTabBar.m_bNewWindow
		);
	}
	return;
}

