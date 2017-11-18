#pragma once

#include "doc/DocListener.h"
#include "_os/Clipboard.h"
#include "OpeBuf.h"

class EditDoc;
class DocLineMgr;

class DocEditor : public DocListenerEx {
public:
	DocEditor(EditDoc& doc);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         イベント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// ロード前後
	void OnBeforeLoad(LoadInfo* pLoadInfo);
	void OnAfterLoad(const LoadInfo& loadInfo);

	// セーブ前後
	void OnAfterSave(const SaveInfo& saveInfo);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           状態                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// Jan. 22, 2002 genta Modified Flagの設定
	void SetModified(bool flag, bool redraw);
	// ファイルが修正中かどうか
	bool IsModified() const { return bIsDocModified; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// Nov. 20, 2000 genta
	void SetImeMode(int mode);	// IME状態の設定

	// May 15, 2000 genta
	Eol  GetNewLineCode() const { return newLineCode; }
	void  SetNewLineCode(const Eol& t) { newLineCode = t; }

	// Oct. 2, 2005 genta 挿入モードの設定
	bool IsInsMode() const { return bInsMode; }
	void SetInsMode(bool mode) { bInsMode = mode; }

	// Undo(元に戻す)可能な状態か？
	bool IsEnableUndo(void) const {
		return opeBuf.IsEnableUndo();
	}

	// Redo(やり直し)可能な状態か？
	bool IsEnableRedo(void) const {
		return opeBuf.IsEnableRedo();
	}

	// クリップボードから貼り付け可能か？
	bool IsEnablePaste(void) const {
		return Clipboard::HasValidData();
	}

public:
	EditDoc&	doc;
	Eol 		newLineCode;			// Enter押下時に挿入する改行コード種別
	OpeBuf		opeBuf;					// アンドゥバッファ
	OpeBlk*		pOpeBlk;				// 操作ブロック
	int			nOpeBlkRedawCount;		// OpeBlkの再描画非対象数
	bool		bInsMode;				// 挿入・上書きモード Oct. 2, 2005 genta
	bool		bIsDocModified;
};


class DocEditAgent {
public:
	DocEditAgent(DocLineMgr& docLineMgr) : docLineMgr(docLineMgr) { }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           操作                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// May 15, 2000 genta
	void AddLineStrX(const wchar_t*, int);	// 末尾に行を追加 Ver1.5

private:
	DocLineMgr& docLineMgr;
};

