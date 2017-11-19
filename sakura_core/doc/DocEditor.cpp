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

DocEditor::DocEditor(EditDoc& doc)
	:
	doc(doc),
	newLineCode(EolType::CRLF),	// New Line Type
	pOpeBlk(nullptr),
	bInsMode(true),
	bIsDocModified(false)	// 変更フラグ
{
	// 挿入モード
	this->SetInsMode(GetDllShareData().common.general.bIsINSMode);
}


/*! 変更フラグの設定

	@param flag [in] 設定する値．true: 変更有り / false: 変更無し
	@param redraw [in] true: タイトルの再描画を行う / false: 行わない
*/
void DocEditor::SetModified(bool flag, bool redraw)
{
	if (bIsDocModified == flag)	{ // 変更がなければ何もしない
		return;
    }

	bIsDocModified = flag;
	if (redraw) {
		doc.pEditWnd->UpdateCaption();
	}
}

void DocEditor::OnBeforeLoad(LoadInfo* loadInfo)
{
	// ビューのテキスト選択解除
	GetListeningDoc()->pEditWnd->Views_DisableSelectArea(true);
}

void DocEditor::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// 編集用改行コードの設定
	{
		const TypeConfig& type = pDoc->docType.GetDocumentAttribute();
		if (pDoc->docFile.GetCodeSet() == type.encoding.eDefaultCodetype) {
			SetNewLineCode(type.encoding.eDefaultEoltype);
		}else {
			SetNewLineCode(EolType::CRLF);
		}
		DocLine* pFirstlineinfo = pDoc->docLineMgr.GetLine(0);
		if (pFirstlineinfo) {
			EolType t = pFirstlineinfo->GetEol();
			if (t != EolType::None && t != EolType::Unknown) {
				SetNewLineCode(t);
			}
		}
	}

	// IME状態の設定
	this->SetImeMode(pDoc->docType.GetDocumentAttribute().nImeState);

	// カレントディレクトリの変更
	::SetCurrentDirectory(pDoc->docFile.GetFilePathClass().GetDirPath().c_str());
	AppMode::getInstance().SetViewMode(loadInfo.bViewMode);		// ビューモード	##ここも、アリかな
}

void DocEditor::OnAfterSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	this->SetModified(false, false);	// 関数化 更新フラグのクリア

	// 現在位置で無変更な状態になったことを通知
	this->opeBuf.SetNoModified();

	// カレントディレクトリの変更
	::SetCurrentDirectory(pDoc->docFile.GetFilePathClass().GetDirPath().c_str());
}

/*!	IME状態の設定
	
	@param mode [in] IMEのモード
*/
void DocEditor::SetImeMode(int mode)
{
	HWND hwnd = doc.pEditWnd->GetActiveView().GetHwnd();
	HIMC hIme = ImmGetContext(hwnd); //######大丈夫？

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

/*!
	末尾に行を追加

	@param pData    [in] 追加する文字列へのポインタ
	@param nDataLen [in] 文字列の長さ。文字単位。
	@param eol     [in] 行末コード
*/
void DocEditAgent::AddLineStrX(const wchar_t* pData, int nDataLen)
{
	// チェーン適用
	DocLine* pDocLine = docLineMgr.AddNewLine();

	// インスタンス設定
	pDocLine->SetDocLineString(pData, nDataLen);
}

