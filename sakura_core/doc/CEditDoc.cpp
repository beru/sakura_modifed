/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, YAZAKI, jepro, novice, asa-o, MIK,
	Copyright (C) 2002, YAZAKI, hor, genta, aroka, frozen, Moca, MIK
	Copyright (C) 2003, MIK, genta, ryoji, Moca, zenryaku, naoh, wmlhq
	Copyright (C) 2004, genta, novice, Moca, MIK, zenryaku
	Copyright (C) 2005, genta, naoh, FILE, Moca, ryoji, D.S.Koba, aroka
	Copyright (C) 2006, genta, ryoji, aroka
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2011, ryoji
	Copyright (C) 2013, Uchi

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

#include "StdAfx.h"
#include <stdlib.h>
#include <string.h>	// Apr. 03, 2003 genta
#include <OleCtl.h>
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h" /// 2002/2/3 aroka
#include "doc/layout/CLayout.h"	// 2007.08.22 ryoji �ǉ�
#include "docplus/CModifyManager.h"
#include "_main/global.h"
#include "_main/CAppMode.h"
#include "_main/CControlTray.h"
#include "_main/CNormalProcess.h"
#include "window/CEditWnd.h"
#include "_os/CClipboard.h"
#include "CCodeChecker.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "print/CPrintPreview.h"
#include "uiparts/CVisualProgress.h"
#include "charset/CCodeMediator.h"
#include "charset/charcode.h"
#include "debug/CRunningTimer.h"
#include "env/CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "func/Funccode.h"
#include "mem/CMemoryIterator.h"	// 2007.08.22 ryoji �ǉ�
#include "outline/CFuncInfoArr.h" /// 2002/2/3 aroka
#include "macro/CSMacroMgr.h"
#include "recent/CMRUFolder.h"
#include "util/file.h"
#include "util/format.h"
#include "util/module.h"
#include "util/other_util.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "sakura_rc.h"

#define IDT_ROLLMOUSE	1

//! �ҏW�֎~�R�}���h
static const EFunctionCode EIsModificationForbidden[] = {
	F_WCHAR,
	F_IME_CHAR,
	F_UNDO,		// 2007.10.12 genta
	F_REDO,		// 2007.10.12 genta
	F_DELETE,
	F_DELETE_BACK,
	F_WordDeleteToStart,
	F_WordDeleteToEnd,
	F_WordCut,
	F_WordDelete,
	F_LineCutToStart,
	F_LineCutToEnd,
	F_LineDeleteToStart,
	F_LineDeleteToEnd,
	F_CUT_LINE,
	F_DELETE_LINE,
	F_DUPLICATELINE,
	F_INDENT_TAB,
	F_UNINDENT_TAB,
	F_INDENT_SPACE,
	F_UNINDENT_SPACE,
	F_LTRIM,		// 2001.12.03 hor
	F_RTRIM,		// 2001.12.03 hor
	F_SORT_ASC,	// 2001.12.11 hor
	F_SORT_DESC,	// 2001.12.11 hor
	F_MERGE,		// 2001.12.11 hor
	F_CUT,
	F_PASTE,
	F_PASTEBOX,
	F_INSTEXT_W,
	F_ADDTAIL_W,
	F_INS_DATE,
	F_INS_TIME,
	F_CTRL_CODE_DIALOG,	//@@@ 2002.06.02 MIK
	F_TOLOWER,
	F_TOUPPER,
	F_TOHANKAKU,
	F_TOZENKAKUKATA,
	F_TOZENKAKUHIRA,
	F_HANKATATOZENKATA,
	F_HANKATATOZENHIRA,
	F_TOZENEI,					// 2001/07/30 Misaka
	F_TOHANEI,
	F_TOHANKATA,				// 2002/08/29 ai
	F_TABTOSPACE,
	F_SPACETOTAB,  //---- Stonee, 2001/05/27
	F_CODECNV_AUTO2SJIS,
	F_CODECNV_EMAIL,
	F_CODECNV_EUC2SJIS,
	F_CODECNV_UNICODE2SJIS,
	F_CODECNV_UTF82SJIS,
	F_CODECNV_UTF72SJIS,
	F_CODECNV_UNICODEBE2SJIS,
	F_CODECNV_SJIS2JIS,
	F_CODECNV_SJIS2EUC,
	F_CODECNV_SJIS2UTF8,
	F_CODECNV_SJIS2UTF7,
	F_REPLACE_DIALOG,
	F_REPLACE,
	F_REPLACE_ALL,
	F_CHGMOD_INS,
	F_HOKAN,
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
	@note
		m_pcEditWnd �̓R���X�g���N�^���ł͎g�p���Ȃ����ƁD

	@date 2000.05.12 genta ���������@�ύX
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	@date 2004.06.21 novice �^�O�W�����v�@�\�ǉ�
*/
EditDoc::EditDoc(EditApp* pcApp)
	:
	m_cDocFile(this),					// warning C4355: 'this' : �x�[�X �����o�[�������q���X�g�Ŏg�p����܂����B
	m_cDocFileOperation(this),			// warning C4355: 'this' : �x�[�X �����o�[�������q���X�g�Ŏg�p����܂����B
	m_cDocEditor(this),					// warning C4355: 'this' : �x�[�X �����o�[�������q���X�g�Ŏg�p����܂����B
	m_cDocType(this),					// warning C4355: 'this' : �x�[�X �����o�[�������q���X�g�Ŏg�p����܂����B
	m_cDocOutline(this),				// warning C4355: 'this' : �x�[�X �����o�[�������q���X�g�Ŏg�p����܂����B
	m_nCommandExecNum(0),				// �R�}���h���s��
	m_hBackImg(NULL)
{
	MY_RUNNINGTIMER(cRunningTimer, "EditDoc::EditDoc");
	
	// ���C�A�E�g�Ǘ����̏�����
	m_cLayoutMgr.Create(this, &m_cDocLineMgr);
	
	// ���C�A�E�g���̕ύX
	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINEKETAS�ŏ���������
	// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
	const TypeConfig& ref = m_cDocType.GetDocumentAttribute();
	LayoutInt nMaxLineKetas = ref.m_nMaxLineKetas;
	if (ref.m_nTextWrapMethod != (int)TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}
	m_cLayoutMgr.SetLayoutInfo(true, ref, ref.m_nTabSpace, nMaxLineKetas);

	// �����ۑ��̐ݒ�	// Aug, 21, 2000 genta
	m_cAutoSaveAgent.ReloadAutoSaveParam();

	//$$ ModifyManager �C���X�^���X�𐶐�
	ModifyManager::getInstance();

	//$$ CodeChecker �C���X�^���X�𐶐�
	CodeChecker::getInstance();

	// 2008.06.07 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	m_nTextWrapMethodCur = m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	m_bTextWrapMethodCurTemp = false;									// �ꎞ�ݒ�K�p��������
	m_blfCurTemp = false;
	m_nPointSizeCur = -1;
	m_nPointSizeOrg = -1;
	m_bTabSpaceCurTemp = false;

	// �����R�[�h��ʂ�������
	m_cDocFile.SetCodeSet(ref.m_encoding.m_eDefaultCodetype, ref.m_encoding.m_bDefaultBom);
	m_cDocEditor.m_cNewLineCode = ref.m_encoding.m_eDefaultEoltype;

	// �r������I�v�V������������
	m_cDocFile.SetShareMode(GetDllShareData().m_common.m_sFile.m_nFileShareMode);

#ifdef _DEBUG
	{
		// �ҏW�֎~�R�}���h�̕��т��`�F�b�N
		for (int i=0; i<_countof(EIsModificationForbidden)-1; ++i){
			assert( EIsModificationForbidden[i] <  EIsModificationForbidden[i+1] );
		}
	}
#endif
}


EditDoc::~EditDoc()
{
	if (m_hBackImg) {
		::DeleteObject(m_hBackImg);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditDoc::Clear()
{
	// �t�@�C���̔r�����b�N����
	m_cDocFileOperation.DoFileUnlock();

	// �����݋֎~�̃N���A
	m_cDocLocker.Clear();

	// �A���h�D�E���h�D�o�b�t�@�̃N���A
	m_cDocEditor.m_cOpeBuf.ClearAll();

	// �e�L�X�g�f�[�^�̃N���A
	m_cDocLineMgr.DeleteAllLine();

	// �t�@�C���p�X�ƃA�C�R���̃N���A
	SetFilePathAndIcon(_T(""));

	// �t�@�C���̃^�C���X�^���v�̃N���A
	m_cDocFile.ClearFileTime();

	// �u��{�v�̃^�C�v�ʐݒ��K�p
	m_cDocType.SetDocumentType(DocTypeManager().GetDocumentTypeOfPath(m_cDocFile.GetFilePath()), true);
	m_blfCurTemp = false;
	m_pcEditWnd->m_pcViewFontMiniMap->UpdateFont(&m_pcEditWnd->GetLogfont());
	InitCharWidthCache( m_pcEditWnd->m_pcViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache(CharWidthFontMode::Edit, m_pcEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(m_pcEditWnd->GetLogfont());
	m_pcEditWnd->m_pcViewFont->UpdateFont(&m_pcEditWnd->GetLogfont());

	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	const TypeConfig& ref = m_cDocType.GetDocumentAttribute();
	LayoutInt nMaxLineKetas = ref.m_nMaxLineKetas;
	if (ref.m_nTextWrapMethod != (int)TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}
	m_cLayoutMgr.SetLayoutInfo(true, ref, ref.m_nTabSpace, nMaxLineKetas);
	m_pcEditWnd->ClearViewCaretPosInfo();
}

// �����f�[�^�̃N���A
void EditDoc::InitDoc()
{
	AppMode::getInstance()->SetViewMode(false);	// �r���[���[�h $$ ����OnClearDoc��p�ӂ�����
	AppMode::getInstance()->m_szGrepKey[0] = 0;	//$$

	EditApp::getInstance()->m_pcGrepAgent->m_bGrepMode = false;	// Grep���[�h	//$$����
	m_cAutoReloadAgent.m_eWatchUpdate = WU_QUERY; // Dec. 4, 2002 genta �X�V�Ď����@ $$

	// 2005.06.24 Moca �o�O�C��
	// �A�E�g�v�b�g�E�B���h�E�Łu����(����)�v���s���Ă��A�E�g�v�b�g�E�B���h�E�̂܂�
	if (AppMode::getInstance()->IsDebugMode()) {
		AppMode::getInstance()->SetDebugModeOFF();
	}

	// Sep. 10, 2002 genta
	// �A�C�R���ݒ�̓t�@�C�����ݒ�ƈ�̉��̂��߂�������͍폜

	Clear();

	// �ύX�t���O
	m_cDocEditor.SetModified(false, false);	// Jan. 22, 2002 genta

	// �����R�[�h���
	const TypeConfig& ref = m_cDocType.GetDocumentAttribute();
	m_cDocFile.SetCodeSet(ref.m_encoding.m_eDefaultCodetype, ref.m_encoding.m_bDefaultBom);
	m_cDocEditor.m_cNewLineCode = ref.m_encoding.m_eDefaultEoltype;

	// Oct. 2, 2005 genta �}�����[�h
	m_cDocEditor.SetInsMode(GetDllShareData().m_common.m_sGeneral.m_bIsINSMode);

	m_cCookie.DeleteAll(L"document");
}

void EditDoc::SetBackgroundImage()
{
	FilePath path = m_cDocType.GetDocumentAttribute().m_szBackImgPath.c_str();
	if (m_hBackImg) {
		::DeleteObject(m_hBackImg);
		m_hBackImg = NULL;
	}
	if (path[0] == 0) {
		return;
	}
	if (_IS_REL_PATH(path.c_str())) {
		FilePath fullPath;
		GetInidirOrExedir(&fullPath[0], &path[0]);
		path = fullPath;
	}
	const TCHAR* ext = path.GetExt();
	if (auto_stricmp(ext, _T(".bmp")) != 0) {
		HANDLE hFile = ::CreateFile(path.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE) {
			return;
		}
		DWORD fileSize  = ::GetFileSize(hFile, NULL);
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, fileSize);
		if (!hGlobal) {
			::CloseHandle(hFile);
			return;
		}
		DWORD nRead;
		BOOL bRead = ::ReadFile(hFile, GlobalLock(hGlobal), fileSize, &nRead, NULL);
		::CloseHandle(hFile);
		hFile = NULL;
		if (!bRead) {
			::GlobalFree(hGlobal);
			return;
		}
		::GlobalUnlock(hGlobal);
		{
			IPicture* iPicture = NULL;
			IStream* iStream = NULL;
			// hGlobal�̊Ǘ����ڏ�
			if (::CreateStreamOnHGlobal(hGlobal, TRUE, &iStream) != S_OK) {
				GlobalFree(hGlobal);
			}else {
				if (::OleLoadPicture(iStream, fileSize, FALSE, IID_IPicture, (void**)&iPicture) != S_OK) {
				}else {
					HBITMAP hBitmap = NULL;
					short imgType = PICTYPE_NONE;
					if (iPicture->get_Type(&imgType) == S_OK
						&& imgType == PICTYPE_BITMAP
						&& iPicture->get_Handle((OLE_HANDLE*)&hBitmap) == S_OK
					) {
						m_nBackImgWidth = m_nBackImgHeight = 1;
						m_hBackImg = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);
					}
				}
			}
			if (iStream)  iStream->Release();
			if (iPicture) iPicture->Release();
		}
	}else {
		m_hBackImg = (HBITMAP)::LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}
	if (m_hBackImg) {
		BITMAP bmp;
		GetObject(m_hBackImg, sizeof(BITMAP), &bmp);
		m_nBackImgWidth  = bmp.bmWidth;
		m_nBackImgHeight = bmp.bmHeight;
		if (m_nBackImgWidth == 0 || m_nBackImgHeight == 0) {
			::DeleteObject(m_hBackImg);
			m_hBackImg = NULL;
		}
	}
}

// �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������
void EditDoc::InitAllView(void)
{
	m_nCommandExecNum = 0;	// �R�}���h���s��
	
	// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	m_nTextWrapMethodCur = m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	m_bTextWrapMethodCurTemp = false;											// �ꎞ�ݒ�K�p��������
	m_blfCurTemp = false;
	m_bTabSpaceCurTemp = false;
	
	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if (m_nTextWrapMethodCur == (int)TextWrappingMethod::NoWrapping) {
		m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	}else {
		m_cLayoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
	// EditWnd�Ɉ��z��
	m_pcEditWnd->InitAllViews();
	
	return;
}

/*! �E�B���h�E�̍쐬��

	@date 2001.09.29 genta �}�N���N���X��n���悤��
	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
*/
BOOL EditDoc::Create(EditWnd* pcEditWnd)
{
	MY_RUNNINGTIMER(cRunningTimer, "EditDoc::Create");

	m_pcEditWnd = pcEditWnd;

	// Oct. 2, 2001 genta
	m_cFuncLookup.Init(GetDllShareData().m_common.m_sMacro.m_MacroTable, &GetDllShareData().m_common);

	SetBackgroundImage();

	MY_TRACETIME(cRunningTimer, "End: PropSheet");

	return TRUE;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ݒ�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�t�@�C�����̐ݒ�
	
	�t�@�C������ݒ肷��Ɠ����ɁC�E�B���h�E�A�C�R����K�؂ɐݒ肷��D
	
	@param szFile [in] �t�@�C���̃p�X��
	
	@author genta
	@date 2002.09.09
*/
void EditDoc::SetFilePathAndIcon(const TCHAR* szFile)
{
	TCHAR szWork[MAX_PATH];
	if (::GetLongFileName(szFile, szWork)) {
		szFile = szWork;
	}
	m_cDocFile.SetFilePath(szFile);
	m_cDocType.SetDocumentIcon();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �h�L�������g�̕����R�[�h���擾
ECodeType EditDoc::GetDocumentEncoding() const
{
	return m_cDocFile.GetCodeSet();
}

// �h�L�������g��BOM�t�����擾
bool EditDoc::GetDocumentBomExist() const
{
	return m_cDocFile.IsBomExist();
}

// �h�L�������g�̕����R�[�h��ݒ�
void EditDoc::SetDocumentEncoding(ECodeType eCharCode, bool bBom)
{
	if (!IsValidCodeType(eCharCode)) {
		return; // �����Ȕ͈͂��󂯕t���Ȃ�
	}

	m_cDocFile.SetCodeSet(eCharCode, bBom);
}


void EditDoc::GetSaveInfo(SaveInfo* pSaveInfo) const
{
	pSaveInfo->cFilePath   = m_cDocFile.GetFilePath();
	pSaveInfo->eCharCode   = m_cDocFile.GetCodeSet();
	pSaveInfo->bBomExist   = m_cDocFile.IsBomExist();
	pSaveInfo->bChgCodeSet = m_cDocFile.IsChgCodeSet();
	pSaveInfo->cEol        = m_cDocEditor.m_cNewLineCode; // �ҏW�����s�R�[�h��ۑ������s�R�[�h�Ƃ��Đݒ�
}


// �ҏW�t�@�C�������i�[
void EditDoc::GetEditInfo(
	EditInfo* pfi	// [out]
) const
{
	// �t�@�C���p�X
	_tcscpy(pfi->m_szPath, m_cDocFile.GetFilePath());

	// �\����
	pfi->m_nViewTopLine = m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	// �\����̈�ԏ�̍s(0�J�n)
	pfi->m_nViewLeftCol = m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	// �\����̈�ԍ��̌�(0�J�n)

	// �L�����b�g�ʒu
	pfi->m_ptCursor.Set(m_pcEditWnd->GetActiveView().GetCaret().GetCaretLogicPos());

	// �e����
	pfi->m_bIsModified = m_cDocEditor.IsModified();			// �ύX�t���O
	pfi->m_nCharCode = m_cDocFile.GetCodeSet();				// �����R�[�h���
	pfi->m_bBom = GetDocumentBomExist();
	pfi->m_nTypeId = m_cDocType.GetDocumentAttribute().m_id;

	// GREP���[�h
	pfi->m_bIsGrep = EditApp::getInstance()->m_pcGrepAgent->m_bGrepMode;
	wcscpy(pfi->m_szGrepKey, AppMode::getInstance()->m_szGrepKey);

	// �f�o�b�O���j�^ (�A�E�g�v�b�g�E�B���h�E) ���[�h
	pfi->m_bIsDebug = AppMode::getInstance()->IsDebugMode();
}


/*! @brief �w��R�}���h�ɂ�鏑���������֎~����Ă��邩�ǂ���

	@retval true  �֎~
	@retval false ����

	@date 2000.08.14 genta �V�K�쐬
	@date 2014.07.27 novice �ҏW�֎~�̏ꍇ�̌������@�ύX
*/
bool EditDoc::IsModificationForbidden(EFunctionCode nCommand) const
{
	// �ҏW�\�̏ꍇ
	if (IsEditable()) {
		return false; // ��ɏ�����������
	}
	
	//	�ҏW�֎~�̏ꍇ(�o�C�i���T�[�`)
	{
		int lbound = 0;
		int ubound = _countof(EIsModificationForbidden) - 1;

		while (lbound <= ubound) {
			int mid = (lbound + ubound) / 2;
			if (nCommand < EIsModificationForbidden[mid]) {
				ubound = mid - 1;
			}else if (nCommand > EIsModificationForbidden[mid]) {
				lbound = mid + 1;
			}else {
				return true;
			}
		}
	}

	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! @brief ���̃E�B���h�E�ŐV�����t�@�C�����J���邩

	�V�����E�B���h�E���J�����Ɍ��݂̃E�B���h�E���ė��p�ł��邩�ǂ����̃e�X�g���s���D
	�ύX�ς݁C�t�@�C�����J���Ă���CGrep�E�B���h�E�C�A�E�g�v�b�g�E�B���h�E�̏ꍇ�ɂ�
	�ė��p�s�D

	@author Moca
	@date 2005.06.24 Moca
*/
bool EditDoc::IsAcceptLoad() const
{
	if (
		m_cDocEditor.IsModified()
		|| m_cDocFile.GetFilePathClass().IsValidPath()
		|| EditApp::getInstance()->m_pcGrepAgent->m_bGrepMode
		|| AppMode::getInstance()->IsDebugMode()
	) {
		return false;
	}
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �R�}���h�R�[�h�ɂ�鏈���U�蕪��

	@param[in] nCommand MAKELONG(�R�}���h�R�[�h�C���M�����ʎq)

	@date 2006.05.19 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX
	@date 2007.06.20 ryoji �O���[�v���ŏ��񂷂�悤�ɕύX
*/
bool EditDoc::HandleCommand(EFunctionCode nCommand)
{
	// May. 19, 2006 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX�����̂�
	// ����16�r�b�g�݂̂����o��
	switch (LOWORD(nCommand)) {
	case F_PREVWINDOW:	// �O�̃E�B���h�E
		{
			int nPane = m_pcEditWnd->m_cSplitterWnd.GetPrevPane();
			if (nPane != -1) {
				m_pcEditWnd->SetActivePane(nPane);
			}else {
				ControlTray::ActiveNextWindow(m_pcEditWnd->GetHwnd());
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	// ���̃E�B���h�E
		{
			int nPane = m_pcEditWnd->m_cSplitterWnd.GetNextPane();
			if (nPane != -1) {
				m_pcEditWnd->SetActivePane(nPane);
			}else {
				ControlTray::ActivePrevWindow(m_pcEditWnd->GetHwnd());
			}
		}
		return TRUE;
	case F_CHG_CHARSET:
		return m_pcEditWnd->GetActiveView().GetCommander().HandleCommand(nCommand, true, (LPARAM)CODE_NONE, 0, 0, 0);

	default:
		return m_pcEditWnd->GetActiveView().GetCommander().HandleCommand(nCommand, true, 0, 0, 0, 0);
	}
}

/*!	�^�C�v�ʐݒ�̓K�p��ύX
	@date 2011.12.15 CViewCommander::Command_TYPE_LIST����ړ�
*/
void EditDoc::OnChangeType()
{
	// �ݒ�ύX�𔽉f������
	m_bTextWrapMethodCurTemp = false;	// �܂�Ԃ����@�̈ꎞ�ݒ�K�p��������	// 2008.06.08 ryoji
	m_blfCurTemp = false;
	m_bTabSpaceCurTemp = false;
	OnChangeSetting();

	// �V�K�Ŗ��ύX�Ȃ�f�t�H���g�����R�[�h��K�p����	// 2011.01.24 ryoji
	if (!m_cDocFile.GetFilePathClass().IsValidPath()) {
		if (!m_cDocEditor.IsModified() && m_cDocLineMgr.GetLineCount() == 0) {
			const TypeConfig& types = m_cDocType.GetDocumentAttribute();
			m_cDocFile.SetCodeSet(types.m_encoding.m_eDefaultCodetype, types.m_encoding.m_bDefaultBom);
			m_cDocEditor.m_cNewLineCode = types.m_encoding.m_eDefaultEoltype;
			m_pcEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();
		}
	}

	// 2006.09.01 ryoji �^�C�v�ύX�㎩�����s�}�N�������s����
	RunAutoMacro(GetDllShareData().m_common.m_sMacro.m_nMacroOnTypeChanged);
}

/*! �r���[�ɐݒ�ύX�𔽉f������
	@param [in] bDoLayout ���C�A�E�g���̍č쐬

	@date 2004.06.09 Moca ���C�A�E�g�č\�z����Progress Bar��\������D
	@date 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@�̕ύX������ǉ�
	@date 2013.04.22 novice ���C�A�E�g���̍č쐬��ݒ�ł���悤�ɂ���
*/
void EditDoc::OnChangeSetting(
	bool bDoLayout
)
{
	HWND hwndProgress = NULL;
	EditWnd* pCEditWnd = m_pcEditWnd;	// Sep. 10, 2002 genta

	if (pCEditWnd) {
		hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
		// Status Bar���\������Ă��Ȃ��Ƃ���m_hwndProgressBar == NULL
	}

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
	}

	// �t�@�C���̔r�����[�h�ύX
	if (m_cDocFile.GetShareMode() != GetDllShareData().m_common.m_sFile.m_nFileShareMode) {
		m_cDocFile.SetShareMode(GetDllShareData().m_common.m_sFile.m_nFileShareMode);

		// �t�@�C���̔r�����b�N����
		m_cDocFileOperation.DoFileUnlock();

		// �t�@�C�������\�̃`�F�b�N����
		bool bOld = m_cDocLocker.IsDocWritable();
		m_cDocLocker.CheckWritable(bOld && !AppMode::getInstance()->IsViewMode());	// ��������s�ɑJ�ڂ����Ƃ��������b�Z�[�W���o���i�o�߂���ƟT��������ˁH�j
		if (bOld != m_cDocLocker.IsDocWritable()) {
			pCEditWnd->UpdateCaption();
		}

		// �t�@�C���̔r�����b�N
		if (m_cDocLocker.IsDocWritable()) {
			m_cDocFileOperation.DoFileLock();
		}
	}

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	FileNameManager::getInstance()->TransformFileName_MakeCache();

	LogicPointEx* posSaveAry = NULL;
	if (m_pcEditWnd->m_posSaveAry) {
		if (bDoLayout) {
			posSaveAry = m_pcEditWnd->m_posSaveAry;
			m_pcEditWnd->m_posSaveAry = NULL;
		}
	}else {
		if (m_pcEditWnd->m_pPrintPreview) {
			// �ꎞ�I�ɐݒ��߂�
			SelectCharWidthCache(CharWidthFontMode::Edit, CWM_CACHE_NEUTRAL);
		}
		if (bDoLayout) {
			posSaveAry = m_pcEditWnd->SavePhysPosOfAllView();
		}
	}

	// �����̕ێ�
	const int nTypeId = m_cDocType.GetDocumentAttribute().m_id;
	const bool bFontTypeOld = m_cDocType.GetDocumentAttribute().m_bUseTypeFont;
	int nFontPointSizeOld = m_nPointSizeOrg;
	if (bFontTypeOld) {
		nFontPointSizeOld = m_cDocType.GetDocumentAttribute().m_nPointSize;
	}
	const KetaXInt nTabSpaceOld = m_cDocType.GetDocumentAttribute().m_nTabSpace;

	// �������
	m_cDocType.SetDocumentType(DocTypeManager().GetDocumentTypeOfPath(m_cDocFile.GetFilePath()), false);
	const TypeConfig& ref = m_cDocType.GetDocumentAttribute();

	// �^�C�v�ʐݒ�̎�ނ��ύX���ꂽ��A�ꎞ�K�p�����ɖ߂�
	if (nTypeId != ref.m_id) {
		m_blfCurTemp = false;
		if (bDoLayout) {
			m_bTextWrapMethodCurTemp = false;
			m_bTabSpaceCurTemp = false;
		}
	}

	// �t�H���g�T�C�Y�̈ꎞ�ݒ�
	if (m_blfCurTemp) {
		if (bFontTypeOld != ref.m_bUseTypeFont) {
			m_blfCurTemp = false;
		}else if (nFontPointSizeOld != pCEditWnd->GetFontPointSize(false)) {
			m_blfCurTemp = false; // �t�H���g�ݒ肪�ύX���ꂽ�B���ɖ߂�
		}else {
			// �t�H���g�̎�ނ̕ύX�ɒǐ�����
			int lfHeight = m_lfCur.lfHeight;
			m_lfCur = pCEditWnd->GetLogfont(false);
			m_lfCur.lfHeight = lfHeight;
		}
	}

	// �t�H���g�X�V
	m_pcEditWnd->m_pcViewFont->UpdateFont(&m_pcEditWnd->GetLogfont());
	m_pcEditWnd->m_pcViewFontMiniMap->UpdateFont(&m_pcEditWnd->GetLogfont());

	InitCharWidthCache( m_pcEditWnd->m_pcViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache(CharWidthFontMode::Edit, m_pcEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(m_pcEditWnd->GetLogfont());

	LayoutInt nMaxLineKetas = ref.m_nMaxLineKetas;
	LayoutInt nTabSpace = ref.m_nTabSpace;
	if (bDoLayout) {
		// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
		// �܂�Ԃ����@�̈ꎞ�ݒ�ƃ^�C�v�ʐݒ肪��v������ꎞ�ݒ�K�p���͉���
		if (m_nTextWrapMethodCur == ref.m_nTextWrapMethod) {
			if (m_nTextWrapMethodCur == (int)TextWrappingMethod::SettingWidth
				&& m_cLayoutMgr.GetMaxLineKetas() != ref.m_nMaxLineKetas
			) {
				// 2013.05.29 �܂�Ԃ������Ⴄ�̂ł��̂܂܂ɂ���
			}else if (bDoLayout) {
				m_bTextWrapMethodCurTemp = false;		// �ꎞ�ݒ�K�p��������
			}
		}
		// �ꎞ�ݒ�K�p���łȂ���ΐ܂�Ԃ����@�ύX
		if (!m_bTextWrapMethodCurTemp) {
			m_nTextWrapMethodCur = ref.m_nTextWrapMethod;	// �܂�Ԃ����@
		}
		// �w�茅�Ő܂�Ԃ��F�^�C�v�ʐݒ���g�p
		// �E�[�Ő܂�Ԃ��F���Ɍ��݂̐܂�Ԃ������g�p
		// ��L�ȊO�FMAXLINEKETAS���g�p
		switch (m_nTextWrapMethodCur) {
		case TextWrappingMethod::NoWrapping:
			nMaxLineKetas = MAXLINEKETAS;
			break;
		case TextWrappingMethod::SettingWidth:
			if (m_bTextWrapMethodCurTemp) {
				// 2013.05.29 ���݂̈ꎞ�K�p�̐܂�Ԃ������g���悤��
				nMaxLineKetas = m_cLayoutMgr.GetMaxLineKetas();
			}
			break;
		case TextWrappingMethod::WindowWidth:
			nMaxLineKetas = m_cLayoutMgr.GetMaxLineKetas();	// ���݂̐܂�Ԃ���
			break;
		}

		if (m_bTabSpaceCurTemp) {
			if (nTabSpaceOld != ref.m_nTabSpace) {
				// �^�C�v�ʐݒ肪�ύX���ꂽ�̂ňꎞ�K�p����
				m_bTabSpaceCurTemp = false;
			}else {
				// �ꎞ�K�p�p��
				nTabSpace = m_cLayoutMgr.GetTabSpace();
			}
		}
	}else {
		// ���C�A�E�g���č\�z���Ȃ��̂Ō��̐ݒ���ێ�
		nMaxLineKetas = m_cLayoutMgr.GetMaxLineKetas();	// ���݂̐܂�Ԃ���
		nTabSpace = m_cLayoutMgr.GetTabSpace();	// ���݂̃^�u��
	}
	ProgressSubject* pOld = EditApp::getInstance()->m_pcVisualProgress->ProgressListener::Listen(&m_cLayoutMgr);
	m_cLayoutMgr.SetLayoutInfo(bDoLayout, ref, nTabSpace, nMaxLineKetas);
	EditApp::getInstance()->m_pcVisualProgress->ProgressListener::Listen(pOld);
	m_pcEditWnd->ClearViewCaretPosInfo();
	
	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if (m_nTextWrapMethodCur == (int)TextWrappingMethod::NoWrapping) {
		m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	}else {
		m_cLayoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
	// �r���[�ɐݒ�ύX�𔽉f������
	int viewCount = m_pcEditWnd->GetAllViewCount();
	for (int i=0; i<viewCount; ++i) {
		m_pcEditWnd->GetView(i).OnChangeSetting();
	}
	if (posSaveAry) {
		m_pcEditWnd->RestorePhysPosOfAllView(posSaveAry);
	}
	for (int i=0; i<viewCount; ++i) {
		m_pcEditWnd->GetView(i).AdjustScrollBars();	// 2008.06.18 ryoji
	}
	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_HIDE);
	}
	if (m_pcEditWnd->m_pPrintPreview) {
		// �ݒ��߂�
		SelectCharWidthCache(CharWidthFontMode::Print, CWM_CACHE_LOCAL);
	}
}

/*! �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
BOOL EditDoc::OnFileClose(bool bGrepNoConfirm)
{
	// �N���[�Y���O����
	CallbackResultType eBeforeCloseResult = NotifyBeforeClose();
	if (eBeforeCloseResult == CallbackResultType::Interrupt) {
		return FALSE;
	}
	
	// �f�o�b�O���j�^���[�h�̂Ƃ��͕ۑ��m�F���Ȃ�
	if (AppMode::getInstance()->IsDebugMode()) {
		return TRUE;
	}

	// GREP���[�h�ŁA���A�uGREP���[�h�ŕۑ��m�F���邩�v��OFF��������A�ۑ��m�F���Ȃ�
	// 2011.11.13 Grep���[�h��Grep�����"���ҏW"��ԂɂȂ��Ă��邪�ۑ��m�F���K�v
	if (EditApp::getInstance()->m_pcGrepAgent->m_bGrepMode) {
		if (bGrepNoConfirm) { // Grep�ŕۑ��m�F���Ȃ����[�h
			return TRUE;
		}
		if (!GetDllShareData().m_common.m_sSearch.m_bGrepExitConfirm) {
			return TRUE;
		}
	}else {
		// �e�L�X�g,�����R�[�h�Z�b�g���ύX����Ă��Ȃ��ꍇ�͕ۑ��m�F���Ȃ�
		if (!m_cDocEditor.IsModified() && !m_cDocFile.IsChgCodeSet()) {
			return TRUE;
		}
	}

	// -- -- �ۑ��m�F -- -- //
	TCHAR szGrepTitle[90];
	LPCTSTR pszTitle = m_cDocFile.GetFilePathClass().IsValidPath() ? m_cDocFile.GetFilePath() : NULL;
	if (EditApp::getInstance()->m_pcGrepAgent->m_bGrepMode) {
		LPCWSTR		pszGrepKey = AppMode::getInstance()->m_szGrepKey;
		int			nLen = (int)wcslen(pszGrepKey);
		NativeW	cmemDes;
		LimitStringLengthW(pszGrepKey , nLen, 64, cmemDes);
		auto_sprintf(szGrepTitle, LS(STR_TITLE_GREP),
			cmemDes.GetStringPtr(),
			(nLen > cmemDes.GetStringLength()) ? _T("...") : _T("")
		);
		pszTitle = szGrepTitle;
	}
	if (!pszTitle) {
		const EditNode* node = AppNodeManager::getInstance()->GetEditNode(EditWnd::getInstance()->GetHwnd());
		auto_sprintf(szGrepTitle, _T("%s%d"), LS(STR_NO_TITLE1), node->m_nId);	// (����)
		pszTitle = szGrepTitle;
	}
	// �E�B���h�E���A�N�e�B�u�ɂ���
	HWND hwndMainFrame = EditWnd::getInstance()->GetHwnd();
	ActivateFrameWindow(hwndMainFrame);
	int nBool;
	if (AppMode::getInstance()->IsViewMode()) {	// �r���[���[�h
		ConfirmBeep();
		int nRet = ::MYMESSAGEBOX(
			hwndMainFrame,
			MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			LS(STR_ERR_DLGEDITDOC30),
			pszTitle
		);
		switch (nRet) {
		case IDYES:
			nBool = m_cDocFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			return FALSE;
		}
	}else {
		ConfirmBeep();
		int nRet;
		if (m_cDocFile.IsChgCodeSet()) {
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				LS(STR_CHANGE_CHARSET),
				pszTitle);
		}else {
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				LS(STR_ERR_DLGEDITDOC31),
				pszTitle
			);
		}
		switch (nRet) {
		case IDYES:
			if (m_cDocFile.GetFilePathClass().IsValidPath()) {
				nBool = m_cDocFileOperation.FileSave();	// 2006.12.30 ryoji
			}else {
				nBool = m_cDocFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			}
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			if (m_cDocFile.IsChgCodeSet()) {
				m_cDocFile.CancelChgCodeSet();	// �����R�[�h�Z�b�g�̕ύX���L�����Z������
				this->m_pcEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();	// �X�e�[�^�X�\��
			}
			return FALSE;
		}
	}
}

/*!	@brief �}�N���������s

	@param type [in] �������s�}�N���ԍ�
	@return

	@author ryoji
	@date 2006.09.01 ryoji �쐬
	@date 2007.07.20 genta HandleCommand�ɒǉ�����n���D
		�������s�}�N���Ŕ��s�����R�}���h�̓L�[�}�N���ɕۑ����Ȃ�
*/
void EditDoc::RunAutoMacro(int idx, LPCTSTR pszSaveFilePath)
{
	// �J�t�@�C���^�^�C�v�ύX���̓A�E�g���C�����ĉ�͂���
	if (!pszSaveFilePath) {
		m_pcEditWnd->m_cDlgFuncList.Refresh();
	}

	static bool bRunning = false;
	if (bRunning) {
		return;	// �ē�����s�͂��Ȃ�
	}
	bRunning = true;
	if (EditApp::getInstance()->m_pcSMacroMgr->IsEnabled(idx)) {
		if (!(::GetAsyncKeyState(VK_SHIFT) & 0x8000)) {	// Shift �L�[��������Ă��Ȃ���Ύ��s
			if (pszSaveFilePath)
				m_cDocFile.SetSaveFilePath(pszSaveFilePath);
			// 2007.07.20 genta �������s�}�N���Ŕ��s�����R�}���h�̓L�[�}�N���ɕۑ����Ȃ�
			HandleCommand((EFunctionCode)((F_USERMACRO_0 + idx) | FA_NONRECORD));
			m_cDocFile.SetSaveFilePath(_T(""));
		}
	}
	bRunning = false;
}

// (����)�̎��̃J�����g�f�B���N�g����ݒ肷��
void EditDoc::SetCurDirNotitle()
{
	if (m_cDocFile.GetFilePathClass().IsValidPath()) {
		return; // �t�@�C��������Ƃ��͉������Ȃ�
	}
	EOpenDialogDir eOpenDialogDir = GetDllShareData().m_common.m_sEdit.m_eOpenDialogDir;
	TCHAR szSelDir[_MAX_PATH];
	const TCHAR* pszDir = NULL;
	if (eOpenDialogDir == OPENDIALOGDIR_MRU) {
		const MRUFolder cMRU;
		std::vector<LPCTSTR> vMRU = cMRU.GetPathList();
		int nCount = cMRU.Length();
		for (int i=0; i<nCount ; ++i) {
			DWORD attr = ::GetFileAttributes( vMRU[i] );
			if ((attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				pszDir = vMRU[i];
				break;
			}
		}
	}else if (eOpenDialogDir == OPENDIALOGDIR_SEL) {
		FileNameManager::ExpandMetaToFolder( GetDllShareData().m_common.m_sEdit.m_OpenDialogSelDir, szSelDir, _countof(szSelDir) );
		pszDir = szSelDir;
	}
	if (pszDir) {
		::SetCurrentDirectory( pszDir );
	}
}
