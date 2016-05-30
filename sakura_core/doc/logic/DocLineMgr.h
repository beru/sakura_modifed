/*!	@file
	@brief 行データの管理

	@author Norio Nakatani
	@date 1998/3/5  新規作成
	@date 2001/06/23 N.Nakatani WhereCurrentWord_2()追加 staticメンバ
	@date 2001/12/03 hor しおり(bookmark)機能追加に伴う関数追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, aroka, MIK, hor
	Copyright (C) 2003, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "_main/global.h" // 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "util/design_template.h"
#include "Ope.h"

class DocLine; // 2002/2/10 aroka
class Bregexp; // 2002/2/10 aroka

struct DocLineReplaceArg {
	Range			delRange;			// [in] 削除範囲。ロジック単位。
	OpeLineData*	pMemDeleted;		// [out] 削除されたデータを保存
	OpeLineData*	pInsData;			// [in/out] 挿入するデータ(中身が移動する)
	size_t			nDeletedLineNum;	// [out] 削除した行の総数
	size_t			nInsLineNum;		// [out] 挿入によって増えた行の数
	Point			ptNewPos;			// [out] 挿入された部分の次の位置
	int				nDelSeq;			// [in] 削除行のOpeシーケンス
	int				nInsSeq;			// [out] 挿入行の元のシーケンス
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
// 2007.09.30 kobake WhereCurrentWord_2 を CWordParse に移動
class DocLineMgr {
public:
	// コンストラクタ・デストラクタ
	DocLineMgr();
	~DocLineMgr();
	
	// 状態
	size_t GetLineCount() const { return nLines; }	// 全行数を返す
	
	// 行データへのアクセス
	const DocLine* GetLine(size_t nLine) const;		// 指定行を取得
	DocLine* GetLine(size_t nLine) {
		return const_cast<DocLine*>(const_cast<DocLine*>(static_cast<const DocLineMgr*>(this)->GetLine( nLine )));
	}
	const DocLine* GetDocLineTop() const { return pDocLineTop; }		// 先頭行を取得
	DocLine* GetDocLineTop() { return pDocLineTop; }					// 先頭行を取得
	const DocLine* GetDocLineBottom() const { return pDocLineBot; }	// 最終行を取得
	DocLine* GetDocLineBottom() { return pDocLineBot; }				// 最終行を取得
	
	// 行データの管理
	DocLine* InsertNewLine(DocLine* pPos);		// pPosの直前に新しい行を挿入
	DocLine* AddNewLine();						// 最下部に新しい行を挿入
	void DeleteAllLine();						// 全ての行を削除する
	void DeleteLine(DocLine*);					// 行の削除
	
	// デバッグ
	void DUMP();
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         実装補助                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	void _Init();
	// -- -- チェーン関数 -- -- // 2007.10.11 kobake 作成
	void _PushBottom(DocLine* pDocLineNew);						// 最下部に挿入
	void _InsertBeforePos(DocLine* pDocLineNew, DocLine* pPos);	// pPosの直前に挿入
	void _InsertAfterPos(DocLine* pDocLineNew, DocLine* pPos);	// pPosの直後に挿入
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	DocLine*	pDocLineTop;	// 最初の行
	DocLine*	pDocLineBot;	// 最後の行(※1行しかない場合はpDocLineTopと等しくなる)
	size_t nLines;			// 全行数
	
public:
	//$$ kobake注: 以下、絶対に切り離したい（最低切り離せなくても、変数の意味をコメントで明確に記すべき）変数群
	mutable DocLine*	pDocLineCurrent;	// 順アクセス時の現在位置
	mutable int			nPrevReferLine;
	mutable DocLine*	pCodePrevRefer;

private:
	DISALLOW_COPY_AND_ASSIGN(DocLineMgr);
};

