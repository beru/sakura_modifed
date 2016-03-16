/*!	@file
	@brief ファイルプロパティダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee
	Copyright (C) 2002, Moca, MIK, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji

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
#include "dlg/DlgProperty.h"
#include "doc/EditDoc.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "_main/global.h"		// Moca, 2002/05/26
#include "_main/AppMode.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "charset/charcode.h"	// rastiv, 2006/06/28
#include "charset/CodePage.h"
#include "charset/ESI.h"
#include "io/BinaryStream.h"
#include "util/shell.h"
#include "sakura_rc.h"

// プロパティ CDlgProperty.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12600
	IDOK,					HIDOK_PROP,
//	IDCANCEL,				HIDCANCEL_PROP,			// 未使用 del 2008/7/4 Uchi
	IDC_BUTTON_HELP,		HIDC_PROP_BUTTON_HELP,
	IDC_EDIT_PROPERTY,		HIDC_PROP_EDIT1,		// IDC_EDIT1->IDC_EDIT_PROPERTY	2008/7/3 Uchi
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

// モーダルダイアログの表示
int DlgProperty::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam
	)
{
	return (int)Dialog::DoModal(hInstance, hwndParent, IDD_PROPERTY_FILE, lParam);
}

BOOL DlgProperty::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//「ファイルのプロパティ」のヘルプ
		// Stonee, 2001/03/12 第四引数を、機能番号からヘルプトピック番号を調べるようにした
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE));	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	case IDOK:			// 下検索
		// ダイアログデータの取得
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
//	case IDCANCEL:							// 未使用 del 2008/7/4 Uchi
//		::EndDialog(GetHwnd(), FALSE);
//		return TRUE;
	}
	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}


/*! ダイアログデータの設定

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
void DlgProperty::SetData(void)
{
	EditDoc* pEditDoc = (EditDoc*)m_lParam;
	NativeT memProp;
	TCHAR szWork[500];

	HANDLE nFind;
	WIN32_FIND_DATA	wfd;
	// Aug. 16, 2000 genta	全角化
	memProp.AppendString(LS(STR_DLGFLPROP_FILENAME));
	memProp.AppendString(pEditDoc->m_docFile.GetFilePath());
	memProp.AppendString(_T("\r\n"));

	memProp.AppendString(LS(STR_DLGFLPROP_FILETYPE));
	memProp.AppendString(pEditDoc->m_docType.GetDocumentAttribute().szTypeName);
	memProp.AppendString(_T("\r\n"));

	memProp.AppendString(LS(STR_DLGFLPROP_ENCODING));
	{
		TCHAR szCpName[100];
		CodePage::GetNameNormal(szCpName, pEditDoc->GetDocumentEncoding());
		memProp.AppendString( szCpName );
	}
	// From Here  2008/4/27 Uchi
	if (pEditDoc->GetDocumentBomExist()) {
		memProp.AppendString(LS(STR_DLGFLPROP_WITH_BOM));
	}
	// To Here  2008/4/27 Uchi
	memProp.AppendString(_T("\r\n"));

	auto_sprintf(szWork, LS(STR_DLGFLPROP_LINE_COUNT), pEditDoc->m_docLineMgr.GetLineCount());
	memProp.AppendString(szWork);

	auto_sprintf(szWork, LS(STR_DLGFLPROP_LAYOUT_LINE), pEditDoc->m_layoutMgr.GetLineCount());
	memProp.AppendString(szWork);

	if (AppMode::getInstance().IsViewMode()) {
		memProp.AppendString(LS(STR_DLGFLPROP_VIEW_MODE));	// 2009.04.11 ryoji 「上書き禁止モード」→「ビューモード」
	}
	if (pEditDoc->m_docEditor.IsModified()) {
		memProp.AppendString(LS(STR_DLGFLPROP_MODIFIED));
	}else {
		memProp.AppendString(LS(STR_DLGFLPROP_NOT_MODIFIED));
	}

	auto_sprintf(szWork, LS(STR_DLGFLPROP_CMD_COUNT), pEditDoc->m_nCommandExecNum);
	memProp.AppendString(szWork);

	auto_sprintf(szWork, LS(STR_DLGFLPROP_FILE_INFO), pEditDoc->m_docLineMgr.GetLineCount());
	memProp.AppendString(szWork);

	if ((nFind = ::FindFirstFile(pEditDoc->m_docFile.GetFilePath(), &wfd)) != INVALID_HANDLE_VALUE) {
		if (pEditDoc->m_docFile.IsFileLocking()) {
			if (m_pShareData->common.file.nFileShareMode == FileShareMode::DenyWrite) {
				auto_sprintf(szWork, LS(STR_DLGFLPROP_W_LOCK));
			}else if (m_pShareData->common.file.nFileShareMode == FileShareMode::DenyReadWrite) {
				auto_sprintf(szWork, LS(STR_DLGFLPROP_RW_LOCK));
			}else {
				auto_sprintf(szWork, LS(STR_DLGFLPROP_LOCK));
			}
			memProp.AppendString(szWork);
		}else {
			auto_sprintf(szWork, LS(STR_DLGFLPROP_NOT_LOCK));
			memProp.AppendString(szWork);
		}

		auto_sprintf(szWork, LS(STR_DLGFLPROP_ATTRIBUTES), pEditDoc->m_docLineMgr.GetLineCount());
		memProp.AppendString(szWork);
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			memProp.AppendString(LS(STR_DLGFLPROP_AT_ARCHIVE));
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) {
			memProp.AppendString(LS(STR_DLGFLPROP_AT_COMPRESS));
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			memProp.AppendString(LS(STR_DLGFLPROP_AT_FOLDER));
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
			memProp.AppendString(LS(STR_DLGFLPROP_AT_HIDDEN));
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
			memProp.AppendString(LS(STR_DLGFLPROP_AT_NORMAL));
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) {
			memProp.AppendString(LS(STR_DLGFLPROP_AT_OFFLINE));
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			memProp.AppendString(LS(STR_DLGFLPROP_AT_READONLY));
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			memProp.AppendString(LS(STR_DLGFLPROP_AT_SYSTEM));
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) {
			memProp.AppendString(LS(STR_DLGFLPROP_AT_TEMP));
		}
		memProp.AppendString(_T("\r\n"));

		memProp.AppendString(LS(STR_DLGFLPROP_CREATE_DT));
		FileTime timeCreation = wfd.ftCreationTime;
		auto_sprintf(szWork, LS(STR_DLGFLPROP_YMDHMS),
			timeCreation->wYear,
			timeCreation->wMonth,
			timeCreation->wDay,
			timeCreation->wHour,
			timeCreation->wMinute,
			timeCreation->wSecond
		);
		memProp.AppendString(szWork);
		memProp.AppendString(_T("\r\n"));

		memProp.AppendString(LS(STR_DLGFLPROP_UPDATE_DT));
		FileTime timeLastWrite = wfd.ftLastWriteTime;
		auto_sprintf(szWork, LS(STR_DLGFLPROP_YMDHMS),
			timeLastWrite->wYear,
			timeLastWrite->wMonth,
			timeLastWrite->wDay,
			timeLastWrite->wHour,
			timeLastWrite->wMinute,
			timeLastWrite->wSecond
		);
		memProp.AppendString(szWork);
		memProp.AppendString(_T("\r\n"));

		memProp.AppendString(LS(STR_DLGFLPROP_ACCESS_DT));
		FileTime timeLastAccess = wfd.ftLastAccessTime;
		auto_sprintf(szWork, LS(STR_DLGFLPROP_YMDHMS),
			timeLastAccess->wYear,
			timeLastAccess->wMonth,
			timeLastAccess->wDay,
			timeLastAccess->wHour,
			timeLastAccess->wMinute,
			timeLastAccess->wSecond
		);
		memProp.AppendString(szWork);
		memProp.AppendString(_T("\r\n"));

		auto_sprintf(szWork, LS(STR_DLGFLPROP_DOS_NAME), wfd.cAlternateFileName);
		memProp.AppendString(szWork);

		auto_sprintf( szWork, LS(STR_DLGFLPROP_FILE_SIZE), wfd.nFileSizeLow );
		memProp.AppendString(szWork);

		::FindClose(nFind);
	}


#ifdef _DEBUG/////////////////////////////////////////////////////
	// メモリ確保 & ファイル読み込み
	NativeT text;
	BinaryInputStream in(pEditDoc->m_docFile.GetFilePath());
	if (!in) {
		goto end_of_CodeTest;
	}
	int nBufLen = in.GetLength();
	if (nBufLen > CheckKanjiCode_MAXREADLENGTH) {
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}
	HGLOBAL hgData = ::GlobalAlloc(GHND, nBufLen + 1);
	if (!hgData) {
		in.Close();
		goto end_of_CodeTest;
	}
	char* pBuf = GlobalLockChar(hgData);
	in.Read(pBuf, nBufLen);
	in.Close();

	// ESIのデバッグ情報
	ESI::GetDebugInfo(pBuf, nBufLen, &text);
	memProp.AppendNativeData(text);

	if (hgData) {
		::GlobalUnlock(hgData);
		::GlobalFree(hgData);
		hgData = NULL;
	}
end_of_CodeTest:;
#endif //ifdef _DEBUG/////////////////////////////////////////////////////
	SetItemText(IDC_EDIT_PROPERTY, memProp.GetStringPtr());

	return;
}

//@@@ 2002.01.18 add start
LPVOID DlgProperty::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

