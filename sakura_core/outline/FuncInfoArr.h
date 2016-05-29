/*!	@file
	@brief アウトライン解析 データ配列

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

class FuncInfo;
#include <string>
#include <map>
#include "util/design_template.h"

// 標準的な付加情報定数
#define FL_OBJ_DEFINITION	0	// 親クラスの定義位置
#define FL_OBJ_DECLARE		1	// 関数プロトタイプ宣言
#define FL_OBJ_FUNCTION		2	// 関数
#define FL_OBJ_CLASS		3	// クラス
#define FL_OBJ_STRUCT		4	// 構造体
#define FL_OBJ_ENUM			5	// 列挙体
#define FL_OBJ_UNION		6	// 共用体
#define FL_OBJ_NAMESPACE	7	// 名前空間
#define FL_OBJ_INTERFACE	8	// インタフェース
#define FL_OBJ_GLOBAL		9	// グローバル（組み込み解析では使用しない）
#define FL_OBJ_ELEMENT_MAX	30	// プラグインで追加可能な定数の上限

// アウトライン解析 データ配列
class FuncInfoArr {
public:
	FuncInfoArr();		// FuncInfoArrクラス構築
	~FuncInfoArr();	// FuncInfoArrクラス消滅
	FuncInfo* GetAt(size_t);		// 0<=の指定番号のデータを返す
	void AppendData(FuncInfo*);	// 配列の最後にデータを追加する
	void AppendData(size_t, size_t, const TCHAR*, int, size_t nDepth = 0);		// 配列の最後にデータを追加する 2002.04.01 YAZAKI 深さ導入
	void AppendData(size_t, size_t, const NOT_TCHAR*, int, size_t nDepth = 0);	// 配列の最後にデータを追加する 2002.04.01 YAZAKI 深さ導入
	void AppendData(size_t nLogicLine, size_t nLogicCol, size_t nLayoutLine, size_t nLayoutCol, const TCHAR*, const TCHAR*, int, size_t nDepth = 0);	/* 配列の最後にデータを追加する 2010.03.01 syat 桁導入*/
	void AppendData(size_t nLogicLine, size_t nLogicCol, size_t nLayoutLine, size_t nLayoutCol, const NOT_TCHAR*, const NOT_TCHAR*, int, size_t nDepth = 0);	/* 配列の最後にデータを追加する 2010.03.01 syat 桁導入*/
	size_t	GetNum(void) {	return nFuncInfoArrNum; }	// 配列要素数を返す
	void Empty(void);
	void DUMP(void);
	void SetAppendText(int info, const std::wstring& s, bool overwrite);
	std::wstring GetAppendText(int info);
	size_t AppendTextLenMax() { return nAppendTextLenMax; }

public:
	SFilePath	szFilePath;	// 解析対象ファイル名
private:
	size_t		nFuncInfoArrNum;	// 配列要素数
	FuncInfo**	ppcFuncInfoArr;	// 配列
	std::map<int, std::wstring>	appendTextArr;	// 追加文字列のリスト
	size_t		nAppendTextLenMax;

private:
	DISALLOW_COPY_AND_ASSIGN(FuncInfoArr);
};

