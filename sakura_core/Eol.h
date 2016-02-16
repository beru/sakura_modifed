/*! @file
	@brief End of Line種別の管理

	@author genta
	@date 2000/5/15 新規作成
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, frozen, Moca

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

#pragma once

#include "_main/global.h"

// 2002/09/22 Moca EOL_CRLF_UNICODEを廃止
// 行終端子の種類
enum class EolType {
	None,			// 
	CRLF,			// 0d0a
	LF,				// 0a
	CR,				// 0d
	NEL,			// 85
	LS,				// 2028
	PS,				// 2029
	CodeMax,		//
	Unknown = -1	//
};

#define EOL_TYPE_NUM	(int)EolType::CodeMax // 8

// 行終端子の配列
extern const EolType gm_pnEolTypeArr[EOL_TYPE_NUM];

#include "basis/SakuraBasis.h"

/*!
	@brief 行末の改行コードを管理

	管理とは言ってもオブジェクト化することで安全に設定を行えたり関連情報の取得を
	オブジェクトに対するメソッドで行えるだけだが、グローバル変数への参照を
	クラス内部に閉じこめることができるのでそれなりに意味はあると思う。
*/
class Eol {
public:
	// コンストラクタ・デストラクタ
	Eol() { m_eEolType = EolType::None; }
	Eol(EolType t) { SetType(t); }

	// 比較
	bool operator == (EolType t) const { return GetType() == t; }
	bool operator != (EolType t) const { return GetType() != t; }

	// 代入
	const Eol& operator = (const Eol& t) { m_eEolType = t.m_eEolType; return *this; }

	// 型変換
	operator EolType() const { return GetType(); }

	// 設定
	bool SetType(EolType t);	//	Typeの設定
	void SetTypeByString(const wchar_t* pszData, int nDataLen);
	void SetTypeByString(const char* pszData, int nDataLen);

	// 設定（ファイル読み込み時に使用）
	void SetTypeByStringForFile(const char* pszData, int nDataLen) { SetTypeByString(pszData, nDataLen); }
	void SetTypeByStringForFile_uni(const char* pszData, int nDataLen);
	void SetTypeByStringForFile_unibe(const char* pszData, int nDataLen);

	// 取得
	EolType		GetType()	const { return m_eEolType; }		// 現在のTypeを取得
	LogicInt		GetLen()	const;	// 現在のEOL長を取得。文字単位。
	const TCHAR*	GetName()	const;	// 現在のEOLの名称取得
	const wchar_t*	GetValue2()	const;	// 現在のEOL文字列先頭へのポインタを取得
	//#####

	bool IsValid() const {
		return m_eEolType >= EolType::CRLF && m_eEolType < EolType::CodeMax;
	}

private:
	EolType	m_eEolType;	// 改行コードの種類
};

