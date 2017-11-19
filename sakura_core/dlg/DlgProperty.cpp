/*!	@file
	@brief ファイルプロパティダイアログ
*/
#include "StdAfx.h"
#include "dlg/DlgProperty.h"
#include "doc/EditDoc.h"
#include "func/Funccode.h"
#include "_main/global.h"
#include "_main/AppMode.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "charset/charcode.h"
#include "charset/CodePage.h"
#include "charset/ESI.h"
#include "io/BinaryStream.h"
#include "util/shell.h"
#include "sakura_rc.h"

#include "sakura.hh"
const DWORD p_helpids[] = {	//12600
	IDOK,					HIDOK_PROP,
	IDC_BUTTON_HELP,		HIDC_PROP_BUTTON_HELP,
	IDC_EDIT_PROPERTY,		HIDC_PROP_EDIT1,
//	IDC_STATIC,				-1,
	0, 0
};

// モーダルダイアログの表示
INT_PTR DlgProperty::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam
	)
{
	return Dialog::DoModal(hInstance, hwndParent, IDD_PROPERTY_FILE, lParam);
}

BOOL DlgProperty::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//「ファイルのプロパティ」のヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PROPERTY_FILE));
		return TRUE;
	case IDOK:			// 下検索
		// ダイアログデータの取得
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}
	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}


/*! ダイアログデータの設定 */
void DlgProperty::SetData(void)
{
	EditDoc* pEditDoc = (EditDoc*)lParam;
	NativeT memProp;
	TCHAR szWork[500];

	HANDLE nFind;
	WIN32_FIND_DATA	wfd;
	// Aug. 16, 2000 genta	全角化
	memProp.AppendString(LS(STR_DLGFLPROP_FILENAME));
	memProp.AppendString(pEditDoc->docFile.GetFilePath());
	memProp.AppendStringLiteral(_T("\r\n"));

	memProp.AppendString(LS(STR_DLGFLPROP_FILETYPE));
	memProp.AppendString(pEditDoc->docType.GetDocumentAttribute().szTypeName);
	memProp.AppendStringLiteral(_T("\r\n"));

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
	memProp.AppendStringLiteral(_T("\r\n"));

	auto_sprintf(szWork, LS(STR_DLGFLPROP_LINE_COUNT), pEditDoc->docLineMgr.GetLineCount());
	memProp.AppendString(szWork);

	auto_sprintf(szWork, LS(STR_DLGFLPROP_LAYOUT_LINE), pEditDoc->layoutMgr.GetLineCount());
	memProp.AppendString(szWork);

	if (AppMode::getInstance().IsViewMode()) {
		memProp.AppendString(LS(STR_DLGFLPROP_VIEW_MODE));
	}
	if (pEditDoc->docEditor.IsModified()) {
		memProp.AppendString(LS(STR_DLGFLPROP_MODIFIED));
	}else {
		memProp.AppendString(LS(STR_DLGFLPROP_NOT_MODIFIED));
	}

	auto_sprintf(szWork, LS(STR_DLGFLPROP_CMD_COUNT), pEditDoc->nCommandExecNum);
	memProp.AppendString(szWork);

	auto_sprintf(szWork, LS(STR_DLGFLPROP_FILE_INFO), pEditDoc->docLineMgr.GetLineCount());
	memProp.AppendString(szWork);

	if ((nFind = ::FindFirstFile(pEditDoc->docFile.GetFilePath(), &wfd)) != INVALID_HANDLE_VALUE) {
		if (pEditDoc->docFile.IsFileLocking()) {
			if (pShareData->common.file.nFileShareMode == FileShareMode::DenyWrite) {
				auto_sprintf(szWork, LS(STR_DLGFLPROP_W_LOCK));
			}else if (pShareData->common.file.nFileShareMode == FileShareMode::DenyReadWrite) {
				auto_sprintf(szWork, LS(STR_DLGFLPROP_RW_LOCK));
			}else {
				auto_sprintf(szWork, LS(STR_DLGFLPROP_LOCK));
			}
			memProp.AppendString(szWork);
		}else {
			auto_sprintf(szWork, LS(STR_DLGFLPROP_NOT_LOCK));
			memProp.AppendString(szWork);
		}

		auto_sprintf(szWork, LS(STR_DLGFLPROP_ATTRIBUTES), pEditDoc->docLineMgr.GetLineCount());
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
		memProp.AppendStringLiteral(_T("\r\n"));

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
		memProp.AppendStringLiteral(_T("\r\n"));

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
		memProp.AppendStringLiteral(_T("\r\n"));

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
		memProp.AppendStringLiteral(_T("\r\n"));

		auto_sprintf(szWork, LS(STR_DLGFLPROP_DOS_NAME), wfd.cAlternateFileName);
		memProp.AppendString(szWork);

		auto_sprintf( szWork, LS(STR_DLGFLPROP_FILE_SIZE), wfd.nFileSizeLow );
		memProp.AppendString(szWork);

		::FindClose(nFind);
	}


#ifdef _DEBUG/////////////////////////////////////////////////////
	// メモリ確保 & ファイル読み込み
	NativeT text;
	BinaryInputStream in(pEditDoc->docFile.GetFilePath());
	if (!in) {
		goto end_of_CodeTest;
	}
	size_t nBufLen = in.GetLength();
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

LPVOID DlgProperty::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

