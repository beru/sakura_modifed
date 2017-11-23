#include "StdAfx.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"
#include "window/EditWnd.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/TagJumpManager.h"
#include "util/fileUtil.h"
#include "util/module.h"
#include "util/window.h"
#include "_main/ControlTray.h"
#include "charset/charcode.h"
#include "recent/Recent.h"

/*
	指定ファイルの指定位置にタグジャンプする。
*/
bool EditView::TagJumpSub(
	const TCHAR*	pszFileName,
	Point			ptJumpTo,		// ジャンプ位置(1開始)
	bool			bClose,			// [in] true: 元ウィンドウを閉じる / false: 元ウィンドウを閉じない
	bool			bRelFromIni,
	bool*			pbJumpToSelf	// [out] オプションnullptr可。自分にジャンプしたか
	)
{
	// 2004/06/21 novice タグジャンプ機能追加
	TagJump	tagJump;

	if (pbJumpToSelf) {
		*pbJumpToSelf = false;
	}

	// 参照元ウィンドウ保存
	tagJump.hwndReferer = EditWnd::getInstance().GetHwnd();

	//	Feb. 17, 2007 genta 実行ファイルからの相対指定の場合は
	//	予め絶対パスに変換する．(キーワードヘルプジャンプで用いる)
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	TCHAR szJumpToFile[1024];
	if (bRelFromIni && _IS_REL_PATH(pszFileName)) {
		GetInidirOrExedir(szJumpToFile, pszFileName);
	}else {
		_tcscpy_s(szJumpToFile, pszFileName);
	}

	// ロングファイル名を取得する
	TCHAR szWork[1024];
	if (::GetLongFileName(szJumpToFile, szWork)) {
		_tcscpy(szJumpToFile, szWork);
	}

// 2004/06/21 novice タグジャンプ機能追加
// 2004/07/05 みちばな
// 同一ファイルだとSendMesssageで GetCaret().GetCaretLayoutPos().GetX2(),GetCaret().GetCaretLayoutPos().yが更新されてしまい、
// ジャンプ先の場所がジャンプ元として保存されてしまっているので、
// その前で保存するように変更。

	// カーソル位置変換
	EditDoc& doc = GetDocument();
	Caret& caret = GetCaret();
	tagJump.point = doc.layoutMgr.LayoutToLogic(caret.GetCaretLayoutPos());

	// タグジャンプ情報の保存
	TagJumpManager().PushTagJump(&tagJump);

	// 指定ファイルが開かれているか調べる
	// 開かれている場合は開いているウィンドウのハンドルも返す
	// ファイルを開いているか
	HWND hwndOwner;
	if (ShareData::getInstance().IsPathOpened(szJumpToFile, &hwndOwner)) {
		// 2004.05.13 Moca マイナス値は無効
		if (0 < ptJumpTo.y) {
			POINT poCaret;
			// カーソルを移動させる
			poCaret.y = ptJumpTo.y - 1;
			if (0 < ptJumpTo.x) {
				poCaret.x = ptJumpTo.x - 1;
			}else {
				poCaret.x = 0;
			}
			GetDllShareData().workBuffer.logicPoint.Set(poCaret.x, poCaret.y);
			::SendMessage(hwndOwner, MYWM_SETCARETPOS, 0, 0);
		}
		// アクティブにする
		ActivateFrameWindow(hwndOwner);
		if (tagJump.hwndReferer == hwndOwner) {
			if (pbJumpToSelf) {
				*pbJumpToSelf = true;
			}
		}
	}else {
		// 新しく開く
		EditInfo inf;
		_tcscpy(inf.szPath, szJumpToFile);
		inf.ptCursor.Set(ptJumpTo.x - 1, ptJumpTo.y - 1);
		inf.nViewLeftCol = -1;
		inf.nViewTopLine = -1;
		inf.nCharCode    = CODE_AUTODETECT;

		bool bSuccess = ControlTray::OpenNewEditor2(
			G_AppInstance(),
			GetHwnd(),
			inf,
			false,	// ビューモードか
			true	// 同期モードで開く
		);

		if (!bSuccess)	// ファイルが開けなかった
			return false;

		// Apr. 23, 2001 genta
		// hwndOwnerに値が入らなくなってしまったために
		// Tag Jump Backが動作しなくなっていたのを修正
		if (!ShareData::getInstance().IsPathOpened(szJumpToFile, &hwndOwner))
			return false;
	}

	// 2006.12.30 ryoji 閉じる処理は最後に（処理位置移動）
	// Apr. 2003 genta 閉じるかどうかは引数による
	// grep結果からEnterでジャンプするところにCtrl判定移動
	if (bClose) {
		ViewCommander& commander = GetCommander();
		commander.Command_WinClose();	// 挑戦するだけ。
	}

	return true;
}



/*! 指定拡張子のファイルに対応するファイルを開く補助関数 */
bool EditView::OPEN_ExtFromtoExt(
	bool			bCheckOnly,		// [in] true: チェックのみ行ってファイルは開かない
	bool			bBeepWhenMiss,	// [in] true: ファイルを開けなかった場合に警告音を出す
	const TCHAR*	file_ext[],		// [in] 処理対象とする拡張子
	const TCHAR*	open_ext[],		// [in] 開く対象とする拡張子
	int				file_extno,		// [in] 処理対象拡張子リストの要素数
	int				open_extno,		// [in] 開く対象拡張子リストの要素数
	const TCHAR*	errmes			// [in] ファイルを開けなかった場合に表示するエラーメッセージ
	)
{
	// 編集中ファイルの拡張子を調べる
	for (int i=0; i<file_extno; ++i) {
		if (CheckEXT(GetDocument().docFile.GetFilePath(), file_ext[i])) {
			goto open_c;
		}
	}
	if (bBeepWhenMiss) {
		ErrorBeep();
	}
	return false;

open_c:;

	TCHAR	szPath[_MAX_PATH];
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	HWND	hwndOwner;

	_tsplitpath(GetDocument().docFile.GetFilePath(), szDrive, szDir, szFname, szExt);

	for (int i=0; i<open_extno; ++i) {
		_tmakepath(szPath, szDrive, szDir, szFname, open_ext[i]);
		if (!fexist(szPath)) {
			if (i < open_extno - 1)
				continue;
			if (bBeepWhenMiss) {
				ErrorBeep();
			}
			return false;
		}
		break;
	}
	if (bCheckOnly) {
		return true;
	}

	// 指定ファイルが開かれているか調べる
	// 開かれている場合は開いているウィンドウのハンドルも返す
	// ファイルを開いているか
	if (ShareData::getInstance().IsPathOpened(szPath, &hwndOwner)) {
	}else {
		// 文字コードはこのファイルに合わせる
		LoadInfo loadInfo;
		loadInfo.filePath = szPath;
		loadInfo.eCharCode = GetDocument().GetDocumentEncoding();
		loadInfo.bViewMode = false;
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			this->GetHwnd(),
			loadInfo,
			NULL,
			true
		);
		// ファイルを開いているか
		if (ShareData::getInstance().IsPathOpened(szPath, &hwndOwner)) {
		}else {
			// 2011.01.12 ryoji エラーは表示しないでおく
			// ファイルサイズが大きすぎて読むかどうか問い合わせているような場合でもエラー表示になるのは変
			// OpenNewEditor()または起動された側のメッセージ表示で十分と思われる

			//ErrorMessage(this->GetHwnd(), _T("%ts\n\n%ts\n\n"), errmes, szPath);
			return false;
		}
	}
	// アクティブにする
	ActivateFrameWindow(hwndOwner);

// 2004/06/21 novice タグジャンプ機能追加
// 2004/07/09 genta/Moca タグジャンプバックの登録が取り除かれていたが、
//            こちらでも従来どおり登録する
	TagJump	tagJump;
	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	tagJump.point = GetDocument().layoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos());
	tagJump.hwndReferer = EditWnd::getInstance().GetHwnd();
	// タグジャンプ情報の保存
	TagJumpManager().PushTagJump(&tagJump);
	return true;
}


/*!	@brief 折り返しの動作を決定

	トグルコマンド「現在のウィンドウ幅で折り返し」を行った場合の動作を決定する
	
	@retval TGWRAP_NONE No action
	@retval TGWRAP_FULL 最大値
	@retval TGWRAP_WINDOW ウィンドウ幅
	@retval TGWRAP_PROP 設定値
*/
EditView::TOGGLE_WRAP_ACTION EditView::GetWrapMode(int* _newKetas)
{
	int& newKetas = *_newKetas;
	// ウィンドウ幅==文書タイプ||最大値==文書タイプ の場合があるため判定順序に注意する。
	/*
		じゅうじさんの要望により判定方法を再考．現在の幅に合わせるのを最優先に．
	
		基本動作： 設定値→ウィンドウ幅
			→(ウィンドウ幅と合っていなければ)→ウィンドウ幅→上へ戻る
			→(ウィンドウ幅と合っていたら)→最大値→設定値
			ただし，最大値==設定値の場合には最大値→設定値の遷移が省略されて上に戻る
			
			ウィンドウ幅が極端に狭い場合にはウィンドウ幅に合わせることは出来ないが，
			設定値と最大値のトグルは可能．

		0)現在のテキストの折り返し方法 != 指定桁で折り返す：変更不能
		1)現在の折り返し幅 == ウィンドウ幅 : 最大値
		2)現在の折り返し幅 != ウィンドウ幅
		3)→ウィンドウ幅が極端に狭い場合
		4)　└→折り返し幅!=最大値 : 最大値
		5)　└→折り返し幅==最大値
		6)　　　└→最大値==設定値 : 変更不能
		7)　　　└→最大値!=設定値 : 設定値
		8)→ウィンドウ幅が十分にある
		9)　└→折り返し幅==最大値
		a)　　　└→最大値!=設定値 : 設定値
	 	b)　　　└→最大値==設定値 : ウィンドウ幅
		c)　└→ウィンドウ幅
	*/
	
	auto& layoutMgr = GetDocument().layoutMgr;
	if (layoutMgr.GetMaxLineKetas() == ViewColNumToWrapColNum(GetTextArea().nViewColNum)) {
		// a)
		newKetas = MAXLINEKETAS;
		return TGWRAP_FULL;
	}else if (MINLINEKETAS > GetTextArea().nViewColNum - GetWrapOverhang()) { // 2)
		// 3)
		if (layoutMgr.GetMaxLineKetas() != MAXLINEKETAS) {
			// 4)
			newKetas = MAXLINEKETAS;
			return TGWRAP_FULL;
		}else if (pTypeData->nMaxLineKetas == MAXLINEKETAS) { // 5)
			// 6)
			return TGWRAP_NONE;
		}else { // 7)
			newKetas = pTypeData->nMaxLineKetas;
			return TGWRAP_PROP;
		}
	}else { // 8)
		if (1
			&& layoutMgr.GetMaxLineKetas() == MAXLINEKETAS // 9)
			&& pTypeData->nMaxLineKetas != MAXLINEKETAS
		) {
			// a)
			newKetas = pTypeData->nMaxLineKetas;
			return TGWRAP_PROP;
			
		}else {	// b) c)
			//	現在のウィンドウ幅
			newKetas = ViewColNumToWrapColNum(GetTextArea().nViewColNum);
			return TGWRAP_WINDOW;
		}
	}
}


void EditView::AddToCmdArr(const TCHAR* szCmd)
{
	RecentCmd recentCmd;
	recentCmd.AppendItem(szCmd);
	recentCmd.Terminate();
}

/*! 正規表現の検索パターンを必要に応じて更新する(ライブラリが使用できないときは false を返す) */
bool EditView::ChangeCurRegexp(bool bRedrawIfChanged)
{
	bool bChangeState = false;

	if (GetDllShareData().common.search.bInheritKeyOtherView
			&& nCurSearchKeySequence < GetDllShareData().common.search.nSearchKeySequence
		|| strCurSearchKey.size() == 0
	) {
		// 履歴の検索キーに更新
		strCurSearchKey = GetDllShareData().searchKeywords.searchKeys[0];		// 検索文字列
		curSearchOption = GetDllShareData().common.search.searchOption;// 検索／置換  オプション
		nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;
		bChangeState = true;
	}else if (bCurSearchUpdate) {
		bChangeState = true;
	}
	bCurSearchUpdate = false;
	if (bChangeState) {
		if (!searchPattern.SetPattern(this->GetHwnd(), strCurSearchKey.c_str(), strCurSearchKey.size(),
			curSearchOption, &curRegexp)
		) {
			bCurSrchKeyMark = false;
			return false;
		}
		bCurSrchKeyMark = true;
		if (bRedrawIfChanged) {
			Redraw();
		}
		editWnd.toolbar.AcceptSharedSearchKey();
		return true;
	}
	if (!bCurSrchKeyMark) {
		bCurSrchKeyMark = true;
		// 検索文字列のマークだけ設定
		if (bRedrawIfChanged) {
			Redraw(); // 自View再描画
		}
	}

	return true;
}


/*!
	カーソル行をクリップボードにコピーする
*/
void EditView::CopyCurLine(
	bool	bAddCRLFWhenCopy,		// [in] 折り返し位置に改行コードを挿入するか？
	EolType	neweol,					// [in] コピーするときのEOL。
	bool	bEnableLineModePaste	// [in] ラインモード貼り付けを可能にする
	)
{
	if (GetSelectionInfo().IsTextSelected()) {
		return;
	}

	const Layout* pLayout = pEditDoc->layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().y);
	if (!pLayout) {
		return;
	}

	// クリップボードに入れるべきテキストデータを、memBufに格納する
	NativeW memBuf;
	memBuf.SetString(pLayout->GetPtr(), pLayout->GetLengthWithoutEOL());
	if (pLayout->GetLayoutEol().GetLen() != 0) {
		memBuf.AppendString(
			(neweol == EolType::Unknown) ?
				pLayout->GetLayoutEol().GetValue2() : Eol(neweol).GetValue2()
		);
	}else if (bAddCRLFWhenCopy) {	// 2007.10.08 ryoji bAddCRLFWhenCopy対応処理追加
		memBuf.AppendString(
			(neweol == EolType::Unknown) ?
				WCODE::CRLF : Eol(neweol).GetValue2()
		);
	}

	// クリップボードにデータmemBufの内容を設定
	BOOL bSetResult = MySetClipboardData(
		memBuf.GetStringPtr(),
		memBuf.GetStringLength(),
		false,
		bEnableLineModePaste
	);
	if (!bSetResult) {
		ErrorBeep();
	}
}

void EditView::DrawBracketCursorLine(bool bDraw)
{
	if (bDraw) {
		GetCaret().underLine.CaretUnderLineON(true, true);
		DrawBracketPair(false);
		SetBracketPairPos(true);
		DrawBracketPair(true);
	}
}

HWND EditView::StartProgress()
{
	HWND hwndProgress = editWnd.statusBar.GetProgressHwnd();
	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
		Progress_SetRange(hwndProgress, 0, 101);
		Progress_SetPos(hwndProgress, 0);
	}
	return hwndProgress;
}

