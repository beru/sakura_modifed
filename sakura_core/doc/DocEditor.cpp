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

#include "StdAfx.h"
#include "DocEditor.h"
#include "EditDoc.h"
#include "doc/logic/DocLine.h"
#include "doc/logic/DocLineMgr.h"
#include "env/DllSharedData.h"
#include "_main/AppMode.h"
#include "Eol.h"
#include "window/EditWnd.h"
#include "debug/RunningTimer.h"

DocEditor::DocEditor(EditDoc* pDoc)
	:
	m_pDocRef(pDoc),
	m_newLineCode(EolType::CRLF),	// New Line Type
	m_pOpeBlk(NULL),
	m_bInsMode(true),			// Oct. 2, 2005 genta
	m_bIsDocModified(false)	// 変更フラグ // Jan. 22, 2002 genta 型変更
{
	// Oct. 2, 2005 genta 挿入モード
	this->SetInsMode(GetDllShareData().m_common.general.m_bIsINSMode);
}


/*! 変更フラグの設定

	@param flag [in] 設定する値．true: 変更有り / false: 変更無し
	@param redraw [in] true: タイトルの再描画を行う / false: 行わない
	
	@author genta
	@date 2002.01.22 新規作成
*/
void DocEditor::SetModified(bool flag, bool redraw)
{
	if (m_bIsDocModified == flag)	// 変更がなければ何もしない
		return;

	m_bIsDocModified = flag;
	if (redraw)
		m_pDocRef->m_pEditWnd->UpdateCaption();
}

void DocEditor::OnBeforeLoad(LoadInfo* loadInfo)
{
	// ビューのテキスト選択解除
	GetListeningDoc()->m_pEditWnd->Views_DisableSelectArea(true);
}

void DocEditor::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// May 12, 2000 genta
	// 編集用改行コードの設定
	{
		const TypeConfig& type = pDoc->m_docType.GetDocumentAttribute();
		if (pDoc->m_docFile.GetCodeSet() == type.m_encoding.m_eDefaultCodetype) {
			SetNewLineCode(type.m_encoding.m_eDefaultEoltype);	// 2011.01.24 ryoji デフォルトEOL
		}else {
			SetNewLineCode(EolType::CRLF);
		}
		DocLine* pFirstlineinfo = pDoc->m_docLineMgr.GetLine(LogicInt(0));
		if (pFirstlineinfo) {
			EolType t = pFirstlineinfo->GetEol();
			if (t != EolType::None && t != EolType::Unknown) {
				SetNewLineCode(t);
			}
		}
	}

	// Nov. 20, 2000 genta
	// IME状態の設定
	this->SetImeMode(pDoc->m_docType.GetDocumentAttribute().m_nImeState);

	// カレントディレクトリの変更
	::SetCurrentDirectory(pDoc->m_docFile.GetFilePathClass().GetDirPath().c_str());
	AppMode::getInstance()->SetViewMode(loadInfo.bViewMode);		// ビューモード	##ここも、アリかな
}

void DocEditor::OnAfterSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	this->SetModified(false, false);	// Jan. 22, 2002 genta 関数化 更新フラグのクリア

	// 現在位置で無変更な状態になったことを通知
	this->m_opeBuf.SetNoModified();

	// カレントディレクトリの変更
	::SetCurrentDirectory(pDoc->m_docFile.GetFilePathClass().GetDirPath().c_str());
}

// From Here Nov. 20, 2000 genta
/*!	IME状態の設定
	
	@param mode [in] IMEのモード
	
	@date Nov 20, 2000 genta
*/
void DocEditor::SetImeMode(int mode)
{
	HWND hwnd = m_pDocRef->m_pEditWnd->GetActiveView().GetHwnd();
	HIMC hIme = ImmGetContext(hwnd); //######大丈夫？ // 2013.06.04 EditWndからViewに変更

	// 最下位ビットはIME自身のOn/Off制御
	if ((mode & 3) == 2) {
		ImmSetOpenStatus(hIme, FALSE);
	}
	if ((mode >> 2) > 0) {
		DWORD conv, sent;
		ImmGetConversionStatus(hIme, &conv, &sent);

		switch (mode >> 2) {
		case 1:	// FullShape
			conv |= IME_CMODE_FULLSHAPE;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 2:	// FullShape & Hiragana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
			conv &= ~(IME_CMODE_KATAKANA | IME_CMODE_NOCONVERSION);
			break;
		case 3:	// FullShape & Katakana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE | IME_CMODE_KATAKANA;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 4: // Non-Conversion
			conv |= IME_CMODE_NOCONVERSION;
			break;
		}
		ImmSetConversionStatus(hIme, conv, sent);
	}
	if ((mode & 3) == 1) {
		ImmSetOpenStatus(hIme, TRUE);
	}
	ImmReleaseContext(hwnd, hIme); //######大丈夫？
}
// To Here Nov. 20, 2000 genta

/*!
	末尾に行を追加

	@version 1.5

	@param pData    [in] 追加する文字列へのポインタ
	@param nDataLen [in] 文字列の長さ。文字単位。
	@param eol     [in] 行末コード

*/
void DocEditAgent::AddLineStrX(const wchar_t* pData, int nDataLen)
{
	// チェーン適用
	DocLine* pDocLine = m_pDocLineMgr->AddNewLine();

	// インスタンス設定
	pDocLine->SetDocLineString(pData, nDataLen);
}

