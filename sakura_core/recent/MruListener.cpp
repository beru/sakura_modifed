#include "StdAfx.h"
#include "MruListener.h"
#include "recent/MRUFile.h"
#include "doc/EditDoc.h"
#include "window/EditWnd.h"
#include "view/EditView.h"
#include "charset/CodePage.h"
#include "charset/CodeMediator.h"
#include "util/fileUtil.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void MruListener::OnAfterSave(const SaveInfo& saveInfo)
{
	_HoldBookmarks_And_AddToMRU();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void MruListener::OnBeforeLoad(LoadInfo* pLoadInfo)
{
	// 再ロード用に現在ファイルをMRU登録しておく
	_HoldBookmarks_And_AddToMRU();	// ← 新規オープン（ファイル名未設定）では何もしない

	// 文字コード指定は明示的であるか
	bool bSpecified = IsValidCodeOrCPType(pLoadInfo->eCharCode);

	// 前回のコード -> ePrevCode
	EditInfo	fi;
	EncodingType ePrevCode = CODE_NONE;
	int nPrevTypeId = -1;
	if (MruFile().GetEditInfo(pLoadInfo->filePath, &fi)) {
		ePrevCode = fi.nCharCode;
		nPrevTypeId = fi.nTypeId;
	}

	// タイプ別設定
	if (!pLoadInfo->nType.IsValidType()) {
		if (0 <= nPrevTypeId) {
			pLoadInfo->nType = DocTypeManager().GetDocumentTypeOfId(nPrevTypeId);
		}
		if (!pLoadInfo->nType.IsValidType()) {
			pLoadInfo->nType = DocTypeManager().GetDocumentTypeOfPath(pLoadInfo->filePath);
		}
	}

	// 指定のコード -> pLoadInfo->eCharCode
	if (pLoadInfo->eCharCode == CODE_AUTODETECT) {
		if (fexist(pLoadInfo->filePath)) {
			// デフォルト文字コード認識のために一時的に読み込み対象ファイルのファイルタイプを適用する
			const TypeConfigMini* type;
			DocTypeManager().GetTypeConfigMini(pLoadInfo->nType, &type);
			CodeMediator mediator(type->encoding);
			pLoadInfo->eCharCode = mediator.CheckKanjiCodeOfFile(pLoadInfo->filePath);
		}else {
			pLoadInfo->eCharCode = ePrevCode;
		}
	}else if (pLoadInfo->eCharCode == CODE_NONE) {
		pLoadInfo->eCharCode = ePrevCode;
	}
	if (pLoadInfo->eCharCode == CODE_NONE) {
		const TypeConfigMini* type;
		if (DocTypeManager().GetTypeConfigMini(pLoadInfo->nType, &type)) {
			pLoadInfo->eCharCode = type->encoding.eDefaultCodetype;	// 無効値の回避
		}else {
			pLoadInfo->eCharCode = GetDllShareData().typeBasis.encoding.eDefaultCodetype;
		}
	}

	// 食い違う場合
	if (IsValidCodeOrCPType(ePrevCode) && pLoadInfo->eCharCode != ePrevCode) {
		// オプション：前回と文字コードが異なるときに問い合わせを行う
		if (GetDllShareData().common.file.bQueryIfCodeChange && !pLoadInfo->bRequestReload) {
			TCHAR szCpNameNew[260];
			TCHAR szCpNameOld[260];
			CodePage::GetNameLong(szCpNameOld, ePrevCode);
			CodePage::GetNameLong(szCpNameNew, pLoadInfo->eCharCode);
			ConfirmBeep();
			int nRet = MYMESSAGEBOX(
				EditWnd::getInstance().GetHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				LS(STR_ERR_DLGEDITDOC5),
				LS(STR_ERR_DLGEDITDOC6),
				pLoadInfo->filePath.c_str(),
				szCpNameNew,
				szCpNameOld,
				szCpNameOld,
				szCpNameNew
			);
			if (nRet == IDYES) {
				// 前回の文字コードを採用する
				pLoadInfo->eCharCode = ePrevCode;
			}else {
				// 元々使おうとしていた文字コードを採用する
				pLoadInfo->eCharCode = pLoadInfo->eCharCode;
			}
		// 食い違っても問い合わせを行わない場合
		}else {
			// デフォルトの回答
			//  自動判別の場合：前回の文字コードを採用
			//  明示指定の場合：明示指定の文字コードを採用
			if (!bSpecified) { // 自動判別
				pLoadInfo->eCharCode = ePrevCode;
			}else { // 明示指定
				pLoadInfo->eCharCode = pLoadInfo->eCharCode;
			}
		}
	}
}


void MruListener::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	MruFile mru;

	EditInfo eiOld;
	bool bIsExistInMRU = mru.GetEditInfo(pDoc->docFile.GetFilePath(), &eiOld);

	// キャレット位置の復元
	if (bIsExistInMRU && GetDllShareData().common.file.GetRestoreCurPosition()) {
		// キャレット位置取得
		Point ptCaretPos = pDoc->layoutMgr.LogicToLayout(eiOld.ptCursor);

		// ビュー取得
		EditView& view = pDoc->pEditWnd->GetActiveView();

		if (ptCaretPos.y >= (int)pDoc->layoutMgr.GetLineCount()) {
			// ファイルの最後に移動
			view.GetCommander().HandleCommand(F_GOFILEEND, false, 0, 0, 0, 0);
		}else {
			view.GetTextArea().SetViewTopLine(eiOld.nViewTopLine);
			view.GetTextArea().SetViewLeftCol(eiOld.nViewLeftCol);
			// 改行の真ん中にカーソルが来ないように。
			const DocLine *pTmpDocLine = pDoc->docLineMgr.GetLine(eiOld.ptCursor.y);
			if (pTmpDocLine) {
				if ((int)pTmpDocLine->GetLengthWithoutEOL() < eiOld.ptCursor.x) {
					ptCaretPos.x--;
				}
			}
			view.GetCaret().MoveCursor(ptCaretPos, true);
			view.GetCaret().nCaretPosX_Prev = view.GetCaret().GetCaretLayoutPos().x;
		}
	}

	// ブックマーク復元
	if (bIsExistInMRU) {
		if (GetDllShareData().common.file.GetRestoreBookmarks()) {
			BookmarkManager(pDoc->docLineMgr).SetBookMarks(eiOld.szMarkLines);
		}
	}else {
		eiOld.szMarkLines[0] = 0;
	}

	// MRUリストへの登録
	EditInfo	eiNew;
	pDoc->GetEditInfo(&eiNew);
	// ブックマークの保持(エディタが落ちたときブックマークが消えるため)
	if (bIsExistInMRU) {
		if (GetDllShareData().common.file.GetRestoreBookmarks()) {
			// SetBookMarksでデータがNUL区切りに書き換わっているので再取得
			mru.GetEditInfo(pDoc->docFile.GetFilePath(), &eiOld);
			auto_strcpy(eiNew.szMarkLines, eiOld.szMarkLines);
		}
	}
	mru.Add(&eiNew);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       クローズ前後                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CallbackResultType MruListener::OnBeforeClose()
{
	_HoldBookmarks_And_AddToMRU();

	return CallbackResultType::Continue;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ヘルパ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	カレントファイルをMRUに登録する。
	ブックマークも一緒に登録する。
*/
void MruListener::_HoldBookmarks_And_AddToMRU()
{
	// EditInfo取得
	EditDoc* pDoc = GetListeningDoc();
	EditInfo fi;
	pDoc->GetEditInfo(&fi);

	// ブックマーク情報の保存
	wcscpy_s(fi.szMarkLines, BookmarkManager(pDoc->docLineMgr).GetBookMarks());

	// MRUリストに登録
	MruFile mru;
	mru.Add(&fi);
}

