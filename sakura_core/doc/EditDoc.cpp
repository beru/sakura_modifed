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
#include <memory>

#include "doc/EditDoc.h"
#include "doc/logic/DocLine.h" /// 2002/2/3 aroka
#include "doc/layout/Layout.h"	// 2007.08.22 ryoji �ǉ�
#include "docplus/ModifyManager.h"
#include "_main/global.h"
#include "_main/AppMode.h"
#include "_main/ControlTray.h"
#include "_main/NormalProcess.h"
#include "window/EditWnd.h"
#include "_os/Clipboard.h"
#include "CodeChecker.h"
#include "EditApp.h"
#include "GrepAgent.h"
#include "print/PrintPreview.h"
#include "uiparts/VisualProgress.h"
#include "charset/CodeMediator.h"
#include "charset/charcode.h"
#include "debug/RunningTimer.h"
#include "env/SakuraEnvironment.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "func/Funccode.h"
#include "mem/MemoryIterator.h"	// 2007.08.22 ryoji �ǉ�
#include "outline/FuncInfoArr.h" /// 2002/2/3 aroka
#include "macro/SMacroMgr.h"
#include "recent/MRUFolder.h"
#include "util/fileUtil.h"
#include "util/format.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "sakura_rc.h"

#define IDT_ROLLMOUSE	1

// �ҏW�֎~�R�}���h
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
		pEditWnd �̓R���X�g���N�^���ł͎g�p���Ȃ����ƁD

	@date 2000.05.12 genta ���������@�ύX
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
	@date 2002.01.14 YAZAKI ���Preview��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	@date 2004.06.21 novice �^�O�W�����v�@�\�ǉ�
*/
EditDoc::EditDoc(EditApp& app)
	:
	docFile(*this),
	docFileOperation(*this),
	docEditor(*this),
	docType(*this),
	docOutline(*this),
	nCommandExecNum(0),				// �R�}���h���s��
	hBackImg(NULL)
{
	MY_RUNNINGTIMER(runningTimer, "EditDoc::EditDoc");
	
	// ���C�A�E�g�Ǘ����̏�����
	layoutMgr.Create(this, &docLineMgr);
	
	// ���C�A�E�g���̕ύX
	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINEKETAS�ŏ���������
	// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
	const TypeConfig& ref = docType.GetDocumentAttribute();
	size_t nMaxLineKetas = ref.nMaxLineKetas;
	if (ref.nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}
	layoutMgr.SetLayoutInfo(true, ref, ref.nTabSpace, nMaxLineKetas);

	// �����ۑ��̐ݒ�	// Aug, 21, 2000 genta
	autoSaveAgent.ReloadAutoSaveParam();

	//$$ ModifyManager �C���X�^���X�𐶐�
	ModifyManager::getInstance();

	//$$ CodeChecker �C���X�^���X�𐶐�
	CodeChecker::getInstance();

	// 2008.06.07 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	nTextWrapMethodCur = docType.GetDocumentAttribute().nTextWrapMethod;	// �܂�Ԃ����@
	bTextWrapMethodCurTemp = false;									// �ꎞ�ݒ�K�p��������
	blfCurTemp = false;
	nPointSizeCur = -1;
	nPointSizeOrg = -1;
	bTabSpaceCurTemp = false;

	// �����R�[�h��ʂ�������
	docFile.SetCodeSet(ref.encoding.eDefaultCodetype, ref.encoding.bDefaultBom);
	docEditor.newLineCode = ref.encoding.eDefaultEoltype;

	// �r������I�v�V������������
	docFile.SetShareMode(GetDllShareData().common.file.nFileShareMode);

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
	if (hBackImg) {
		::DeleteObject(hBackImg);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditDoc::Clear()
{
	// �t�@�C���̔r�����b�N����
	docFileOperation.DoFileUnlock();

	// �����݋֎~�̃N���A
	docLocker.Clear();

	// Undo, Redo�o�b�t�@�̃N���A
	docEditor.opeBuf.ClearAll();

	// �e�L�X�g�f�[�^�̃N���A
	docLineMgr.DeleteAllLine();

	// �t�@�C���p�X�ƃA�C�R���̃N���A
	SetFilePathAndIcon(_T(""));

	// �t�@�C���̃^�C���X�^���v�̃N���A
	docFile.ClearFileTime();

	// �u��{�v�̃^�C�v�ʐݒ��K�p
	docType.SetDocumentType(DocTypeManager().GetDocumentTypeOfPath(docFile.GetFilePath()), true);
	blfCurTemp = false;
	pEditWnd->pViewFontMiniMap->UpdateFont(&pEditWnd->GetLogfont());
	InitCharWidthCache( pEditWnd->pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache(CharWidthFontMode::Edit, pEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(pEditWnd->GetLogfont());
	pEditWnd->pViewFont->UpdateFont(&pEditWnd->GetLogfont());

	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	const TypeConfig& ref = docType.GetDocumentAttribute();
	size_t nMaxLineKetas = ref.nMaxLineKetas;
	if (ref.nTextWrapMethod != TextWrappingMethod::SettingWidth) {
		nMaxLineKetas = MAXLINEKETAS;
	}
	layoutMgr.SetLayoutInfo(true, ref, ref.nTabSpace, nMaxLineKetas);
	pEditWnd->ClearViewCaretPosInfo();
}

// �����f�[�^�̃N���A
void EditDoc::InitDoc()
{
	AppMode::getInstance().SetViewMode(false);	// �r���[���[�h $$ ����OnClearDoc��p�ӂ�����
	AppMode::getInstance().szGrepKey[0] = 0;	//$$

	EditApp::getInstance().pGrepAgent->bGrepMode = false;	// Grep���[�h	//$$����
	autoReloadAgent.watchUpdateType = WatchUpdateType::Query; // Dec. 4, 2002 genta �X�V�Ď����@ $$

	// 2005.06.24 Moca �o�O�C��
	// �A�E�g�v�b�g�E�B���h�E�Łu����(����)�v���s���Ă��A�E�g�v�b�g�E�B���h�E�̂܂�
	if (AppMode::getInstance().IsDebugMode()) {
		AppMode::getInstance().SetDebugModeOFF();
	}

	// Sep. 10, 2002 genta
	// �A�C�R���ݒ�̓t�@�C�����ݒ�ƈ�̉��̂��߂�������͍폜

	Clear();

	// �ύX�t���O
	docEditor.SetModified(false, false);	// Jan. 22, 2002 genta

	// �����R�[�h���
	const TypeConfig& ref = docType.GetDocumentAttribute();
	docFile.SetCodeSet(ref.encoding.eDefaultCodetype, ref.encoding.bDefaultBom);
	docEditor.newLineCode = ref.encoding.eDefaultEoltype;

	// Oct. 2, 2005 genta �}�����[�h
	docEditor.SetInsMode(GetDllShareData().common.general.bIsINSMode);

	cookie.DeleteAll(L"document");
}

void EditDoc::SetBackgroundImage()
{
	FilePath path = docType.GetDocumentAttribute().szBackImgPath.c_str();
	if (hBackImg) {
		::DeleteObject(hBackImg);
		hBackImg = NULL;
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
			IPicture* iPicture = nullptr;
			IStream* iStream = nullptr;
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
						nBackImgWidth = nBackImgHeight = 1;
						hBackImg = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);
					}
				}
			}
			if (iStream)  iStream->Release();
			if (iPicture) iPicture->Release();
		}
	}else {
		hBackImg = (HBITMAP)::LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}
	if (hBackImg) {
		BITMAP bmp;
		GetObject(hBackImg, sizeof(BITMAP), &bmp);
		nBackImgWidth  = bmp.bmWidth;
		nBackImgHeight = bmp.bmHeight;
		if (nBackImgWidth == 0 || nBackImgHeight == 0) {
			::DeleteObject(hBackImg);
			hBackImg = NULL;
		}
	}
}

// �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������
void EditDoc::InitAllView(void)
{
	nCommandExecNum = 0;	// �R�}���h���s��
	
	// 2008.05.30 nasukoji	�e�L�X�g�̐܂�Ԃ����@��������
	nTextWrapMethodCur = docType.GetDocumentAttribute().nTextWrapMethod;	// �܂�Ԃ����@
	bTextWrapMethodCurTemp = false;											// �ꎞ�ݒ�K�p��������
	blfCurTemp = false;
	bTabSpaceCurTemp = false;
	
	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if (nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		layoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	}else {
		layoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
	// EditWnd�Ɉ��z��
	pEditWnd->InitAllViews();
	
	return;
}

/*! �E�B���h�E�̍쐬��

	@date 2001.09.29 genta �}�N���N���X��n���悤��
	@date 2002.01.03 YAZAKI tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
*/
BOOL EditDoc::Create(EditWnd* pEditWnd)
{
	MY_RUNNINGTIMER(runningTimer, "EditDoc::Create");

	this->pEditWnd = pEditWnd;

	// Oct. 2, 2001 genta
	funcLookup.Init(GetDllShareData().common.macro.macroTable, &GetDllShareData().common);

	SetBackgroundImage();

	MY_TRACETIME(runningTimer, "End: PropSheet");

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
	docFile.SetFilePath(szFile);
	docType.SetDocumentIcon();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �h�L�������g�̕����R�[�h���擾
EncodingType EditDoc::GetDocumentEncoding() const
{
	return docFile.GetCodeSet();
}

// �h�L�������g��BOM�t�����擾
bool EditDoc::GetDocumentBomExist() const
{
	return docFile.IsBomExist();
}

// �h�L�������g�̕����R�[�h��ݒ�
void EditDoc::SetDocumentEncoding(EncodingType eCharCode, bool bBom)
{
	if (!IsValidCodeType(eCharCode)) {
		return; // �����Ȕ͈͂��󂯕t���Ȃ�
	}

	docFile.SetCodeSet(eCharCode, bBom);
}


void EditDoc::GetSaveInfo(SaveInfo* pSaveInfo) const
{
	pSaveInfo->filePath		= docFile.GetFilePath();
	pSaveInfo->eCharCode	= docFile.GetCodeSet();
	pSaveInfo->bBomExist	= docFile.IsBomExist();
	pSaveInfo->bChgCodeSet	= docFile.IsChgCodeSet();
	pSaveInfo->eol			= docEditor.newLineCode; // �ҏW�����s�R�[�h��ۑ������s�R�[�h�Ƃ��Đݒ�
}


// �ҏW�t�@�C�������i�[
void EditDoc::GetEditInfo(
	EditInfo* pfi	// [out]
	) const
{
	// �t�@�C���p�X
	_tcscpy(pfi->szPath, docFile.GetFilePath());

	// �\����
	pfi->nViewTopLine = pEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	// �\����̈�ԏ�̍s(0�J�n)
	pfi->nViewLeftCol = pEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	// �\����̈�ԍ��̌�(0�J�n)

	// �L�����b�g�ʒu
	pfi->ptCursor.Set(pEditWnd->GetActiveView().GetCaret().GetCaretLogicPos());

	// �e����
	pfi->bIsModified = docEditor.IsModified();			// �ύX�t���O
	pfi->nCharCode = docFile.GetCodeSet();				// �����R�[�h���
	pfi->bBom = GetDocumentBomExist();
	pfi->nTypeId = docType.GetDocumentAttribute().id;

	// GREP���[�h
	pfi->bIsGrep = EditApp::getInstance().pGrepAgent->bGrepMode;
	wcscpy(pfi->szGrepKey, AppMode::getInstance().szGrepKey);

	// �f�o�b�O���j�^ (�A�E�g�v�b�g�E�B���h�E) ���[�h
	pfi->bIsDebug = AppMode::getInstance().IsDebugMode();
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
		docEditor.IsModified()
		|| docFile.GetFilePathClass().IsValidPath()
		|| EditApp::getInstance().pGrepAgent->bGrepMode
		|| AppMode::getInstance().IsDebugMode()
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
			int nPane = pEditWnd->splitterWnd.GetPrevPane();
			if (nPane != -1) {
				pEditWnd->SetActivePane(nPane);
			}else {
				ControlTray::ActiveNextWindow(pEditWnd->GetHwnd());
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	// ���̃E�B���h�E
		{
			int nPane = pEditWnd->splitterWnd.GetNextPane();
			if (nPane != -1) {
				pEditWnd->SetActivePane(nPane);
			}else {
				ControlTray::ActivePrevWindow(pEditWnd->GetHwnd());
			}
		}
		return TRUE;
	case F_CHG_CHARSET:
		return pEditWnd->GetActiveView().GetCommander().HandleCommand(nCommand, true, (LPARAM)CODE_NONE, 0, 0, 0);

	default:
		return pEditWnd->GetActiveView().GetCommander().HandleCommand(nCommand, true, 0, 0, 0, 0);
	}
}

/*!	�^�C�v�ʐݒ�̓K�p��ύX
	@date 2011.12.15 ViewCommander::Command_Type_List����ړ�
*/
void EditDoc::OnChangeType()
{
	// �ݒ�ύX�𔽉f������
	bTextWrapMethodCurTemp = false;	// �܂�Ԃ����@�̈ꎞ�ݒ�K�p��������	// 2008.06.08 ryoji
	blfCurTemp = false;
	bTabSpaceCurTemp = false;
	OnChangeSetting();

	// �V�K�Ŗ��ύX�Ȃ�f�t�H���g�����R�[�h��K�p����	// 2011.01.24 ryoji
	if (!docFile.GetFilePathClass().IsValidPath()) {
		if (!docEditor.IsModified() && docLineMgr.GetLineCount() == 0) {
			const TypeConfig& types = docType.GetDocumentAttribute();
			docFile.SetCodeSet(types.encoding.eDefaultCodetype, types.encoding.bDefaultBom);
			docEditor.newLineCode = types.encoding.eDefaultEoltype;
			pEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();
		}
	}

	// 2006.09.01 ryoji �^�C�v�ύX�㎩�����s�}�N�������s����
	RunAutoMacro(GetDllShareData().common.macro.nMacroOnTypeChanged);
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
	if (pEditWnd) {
		hwndProgress = pEditWnd->statusBar.GetProgressHwnd();
		// Status Bar���\������Ă��Ȃ��Ƃ���hwndProgressBar == NULL
	}

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
	}

	// �t�@�C���̔r�����[�h�ύX
	if (docFile.GetShareMode() != GetDllShareData().common.file.nFileShareMode) {
		docFile.SetShareMode(GetDllShareData().common.file.nFileShareMode);

		// �t�@�C���̔r�����b�N����
		docFileOperation.DoFileUnlock();

		// �t�@�C�������\�̃`�F�b�N����
		bool bOld = docLocker.IsDocWritable();
		docLocker.CheckWritable(bOld && !AppMode::getInstance().IsViewMode());	// ��������s�ɑJ�ڂ����Ƃ��������b�Z�[�W���o���i�o�߂���ƟT��������ˁH�j
		if (bOld != docLocker.IsDocWritable()) {
			pEditWnd->UpdateCaption();
		}

		// �t�@�C���̔r�����b�N
		if (docLocker.IsDocWritable()) {
			docFileOperation.DoFileLock();
		}
	}

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	FileNameManager::getInstance().TransformFileName_MakeCache();

	PointEx* posSaveAry = nullptr;
	if (pEditWnd->posSaveAry) {
		if (bDoLayout) {
			posSaveAry = pEditWnd->posSaveAry;
			pEditWnd->posSaveAry = nullptr;
		}
	}else {
		if (pEditWnd->pPrintPreview) {
			// �ꎞ�I�ɐݒ��߂�
			SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Neutral);
		}
		if (bDoLayout) {
			posSaveAry = pEditWnd->SavePhysPosOfAllView();
		}
	}

	// �����̕ێ�
	const int nTypeId = docType.GetDocumentAttribute().id;
	const bool bFontTypeOld = docType.GetDocumentAttribute().bUseTypeFont;
	int nFontPointSizeOld = nPointSizeOrg;
	if (bFontTypeOld) {
		nFontPointSizeOld = docType.GetDocumentAttribute().nPointSize;
	}
	const auto nTabSpaceOld = docType.GetDocumentAttribute().nTabSpace;

	// �������
	docType.SetDocumentType(DocTypeManager().GetDocumentTypeOfPath(docFile.GetFilePath()), false);
	const TypeConfig& ref = docType.GetDocumentAttribute();

	// �^�C�v�ʐݒ�̎�ނ��ύX���ꂽ��A�ꎞ�K�p�����ɖ߂�
	if (nTypeId != ref.id) {
		blfCurTemp = false;
		if (bDoLayout) {
			bTextWrapMethodCurTemp = false;
			bTabSpaceCurTemp = false;
		}
	}

	// �t�H���g�T�C�Y�̈ꎞ�ݒ�
	if (blfCurTemp) {
		if (bFontTypeOld != ref.bUseTypeFont) {
			blfCurTemp = false;
		}else if (nFontPointSizeOld != pEditWnd->GetFontPointSize(false)) {
			blfCurTemp = false; // �t�H���g�ݒ肪�ύX���ꂽ�B���ɖ߂�
		}else {
			// �t�H���g�̎�ނ̕ύX�ɒǐ�����
			int lfHeight = lfCur.lfHeight;
			lfCur = pEditWnd->GetLogfont(false);
			lfCur.lfHeight = lfHeight;
		}
	}

	// �t�H���g�X�V
	pEditWnd->pViewFont->UpdateFont(&pEditWnd->GetLogfont());
	pEditWnd->pViewFontMiniMap->UpdateFont(&pEditWnd->GetLogfont());

	InitCharWidthCache( pEditWnd->pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap );
	SelectCharWidthCache(CharWidthFontMode::Edit, pEditWnd->GetLogfontCacheMode());
	InitCharWidthCache(pEditWnd->GetLogfont());

	size_t nMaxLineKetas = ref.nMaxLineKetas;
	size_t nTabSpace = ref.nTabSpace;
	if (bDoLayout) {
		// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
		// �܂�Ԃ����@�̈ꎞ�ݒ�ƃ^�C�v�ʐݒ肪��v������ꎞ�ݒ�K�p���͉���
		if (nTextWrapMethodCur == ref.nTextWrapMethod) {
			if (nTextWrapMethodCur == TextWrappingMethod::SettingWidth
				&& layoutMgr.GetMaxLineKetas() != ref.nMaxLineKetas
			) {
				// 2013.05.29 �܂�Ԃ������Ⴄ�̂ł��̂܂܂ɂ���
			}else if (bDoLayout) {
				bTextWrapMethodCurTemp = false;		// �ꎞ�ݒ�K�p��������
			}
		}
		// �ꎞ�ݒ�K�p���łȂ���ΐ܂�Ԃ����@�ύX
		if (!bTextWrapMethodCurTemp) {
			nTextWrapMethodCur = ref.nTextWrapMethod;	// �܂�Ԃ����@
		}
		// �w�茅�Ő܂�Ԃ��F�^�C�v�ʐݒ���g�p
		// �E�[�Ő܂�Ԃ��F���Ɍ��݂̐܂�Ԃ������g�p
		// ��L�ȊO�FMAXLINEKETAS���g�p
		switch (nTextWrapMethodCur) {
		case TextWrappingMethod::NoWrapping:
			nMaxLineKetas = MAXLINEKETAS;
			break;
		case TextWrappingMethod::SettingWidth:
			if (bTextWrapMethodCurTemp) {
				// 2013.05.29 ���݂̈ꎞ�K�p�̐܂�Ԃ������g���悤��
				nMaxLineKetas = layoutMgr.GetMaxLineKetas();
			}
			break;
		case TextWrappingMethod::WindowWidth:
			nMaxLineKetas = layoutMgr.GetMaxLineKetas();	// ���݂̐܂�Ԃ���
			break;
		}

		if (bTabSpaceCurTemp) {
			if (nTabSpaceOld != ref.nTabSpace) {
				// �^�C�v�ʐݒ肪�ύX���ꂽ�̂ňꎞ�K�p����
				bTabSpaceCurTemp = false;
			}else {
				// �ꎞ�K�p�p��
				nTabSpace = layoutMgr.GetTabSpace();
			}
		}
	}else {
		// ���C�A�E�g���č\�z���Ȃ��̂Ō��̐ݒ���ێ�
		nMaxLineKetas = layoutMgr.GetMaxLineKetas();	// ���݂̐܂�Ԃ���
		nTabSpace = layoutMgr.GetTabSpace();	// ���݂̃^�u��
	}
	ProgressSubject* pOld = EditApp::getInstance().pVisualProgress->ProgressListener::Listen(&layoutMgr);
	layoutMgr.SetLayoutInfo(bDoLayout, ref, nTabSpace, nMaxLineKetas);
	EditApp::getInstance().pVisualProgress->ProgressListener::Listen(pOld);
	pEditWnd->ClearViewCaretPosInfo();
	
	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if (nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
		layoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	}else {
		layoutMgr.ClearLayoutLineWidth();	// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
	// �r���[�ɐݒ�ύX�𔽉f������
	int viewCount = pEditWnd->GetAllViewCount();
	for (int i=0; i<viewCount; ++i) {
		pEditWnd->GetView(i).OnChangeSetting();
	}
	if (posSaveAry) {
		pEditWnd->RestorePhysPosOfAllView(posSaveAry);
	}
	for (int i=0; i<viewCount; ++i) {
		pEditWnd->GetView(i).AdjustScrollBars();	// 2008.06.18 ryoji
	}
	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_HIDE);
	}
	if (pEditWnd->pPrintPreview) {
		// �ݒ��߂�
		SelectCharWidthCache(CharWidthFontMode::Print, CharWidthCacheMode::Local);
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
	if (AppMode::getInstance().IsDebugMode()) {
		return TRUE;
	}

	// GREP���[�h�ŁA���A�uGREP���[�h�ŕۑ��m�F���邩�v��OFF��������A�ۑ��m�F���Ȃ�
	// 2011.11.13 Grep���[�h��Grep�����"���ҏW"��ԂɂȂ��Ă��邪�ۑ��m�F���K�v
	if (EditApp::getInstance().pGrepAgent->bGrepMode) {
		if (bGrepNoConfirm) { // Grep�ŕۑ��m�F���Ȃ����[�h
			return TRUE;
		}
		if (!GetDllShareData().common.search.bGrepExitConfirm) {
			return TRUE;
		}
	}else {
		// �e�L�X�g,�����R�[�h�Z�b�g���ύX����Ă��Ȃ��ꍇ�͕ۑ��m�F���Ȃ�
		if (!docEditor.IsModified() && !docFile.IsChgCodeSet()) {
			return TRUE;
		}
	}

	// -- -- �ۑ��m�F -- -- //
	TCHAR szGrepTitle[90];
	LPCTSTR pszTitle = docFile.GetFilePathClass().IsValidPath() ? docFile.GetFilePath() : NULL;
	if (EditApp::getInstance().pGrepAgent->bGrepMode) {
		LPCWSTR		pszGrepKey = AppMode::getInstance().szGrepKey;
		int			nLen = (int)wcslen(pszGrepKey);
		NativeW	memDes;
		LimitStringLengthW(pszGrepKey , nLen, 64, memDes);
		auto_sprintf(szGrepTitle, LS(STR_TITLE_GREP),
			memDes.GetStringPtr(),
			(nLen > memDes.GetStringLength()) ? _T("...") : _T("")
		);
		pszTitle = szGrepTitle;
	}
	if (!pszTitle) {
		const EditNode* node = AppNodeManager::getInstance().GetEditNode(EditWnd::getInstance().GetHwnd());
		auto_sprintf(szGrepTitle, _T("%s%d"), LS(STR_NO_TITLE1), node->nId);	// (����)
		pszTitle = szGrepTitle;
	}
	// �E�B���h�E���A�N�e�B�u�ɂ���
	HWND hwndMainFrame = EditWnd::getInstance().GetHwnd();
	ActivateFrameWindow(hwndMainFrame);
	int nBool;
	if (AppMode::getInstance().IsViewMode()) {	// �r���[���[�h
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
			nBool = docFileOperation.FileSaveAs();	// 2006.12.30 ryoji
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
		if (docFile.IsChgCodeSet()) {
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
			if (docFile.GetFilePathClass().IsValidPath()) {
				nBool = docFileOperation.FileSave();	// 2006.12.30 ryoji
			}else {
				nBool = docFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			}
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			if (docFile.IsChgCodeSet()) {
				docFile.CancelChgCodeSet();	// �����R�[�h�Z�b�g�̕ύX���L�����Z������
				this->pEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();	// �X�e�[�^�X�\��
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
		pEditWnd->dlgFuncList.Refresh();
	}

	static bool bRunning = false;
	if (bRunning) {
		return;	// �ē�����s�͂��Ȃ�
	}
	bRunning = true;
	if (EditApp::getInstance().pSMacroMgr->IsEnabled(idx)) {
		if (!(::GetAsyncKeyState(VK_SHIFT) & 0x8000)) {	// Shift �L�[��������Ă��Ȃ���Ύ��s
			if (pszSaveFilePath)
				docFile.SetSaveFilePath(pszSaveFilePath);
			// 2007.07.20 genta �������s�}�N���Ŕ��s�����R�}���h�̓L�[�}�N���ɕۑ����Ȃ�
			HandleCommand((EFunctionCode)((F_USERMACRO_0 + idx) | FA_NONRECORD));
			docFile.SetSaveFilePath(_T(""));
		}
	}
	bRunning = false;
}

// (����)�̎��̃J�����g�f�B���N�g����ݒ肷��
void EditDoc::SetCurDirNotitle()
{
	if (docFile.GetFilePathClass().IsValidPath()) {
		return; // �t�@�C��������Ƃ��͉������Ȃ�
	}
	EOpenDialogDir eOpenDialogDir = GetDllShareData().common.edit.eOpenDialogDir;
	TCHAR szSelDir[_MAX_PATH];
	const TCHAR* pszDir = NULL;
	if (eOpenDialogDir == OPENDIALOGDIR_MRU) {
		const MruFolder mru;
		std::vector<LPCTSTR> vMRU = mru.GetPathList();
		int nCount = mru.Length();
		for (int i=0; i<nCount ; ++i) {
			DWORD attr = ::GetFileAttributes( vMRU[i] );
			if ((attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				pszDir = vMRU[i];
				break;
			}
		}
	}else if (eOpenDialogDir == OPENDIALOGDIR_SEL) {
		FileNameManager::ExpandMetaToFolder( GetDllShareData().common.edit.openDialogSelDir, szSelDir, _countof(szSelDir) );
		pszDir = szSelDir;
	}
	if (pszDir) {
		::SetCurrentDirectory( pszDir );
	}
}
