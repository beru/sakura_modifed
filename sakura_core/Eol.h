#pragma once

// End of Line種別の管理

#include "_main/global.h"

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

#define EOL_TYPE_NUM	(size_t)EolType::CodeMax // 8

// 行終端子の配列
extern const EolType g_pnEolTypeArr[EOL_TYPE_NUM];

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
	Eol() { eEolType = EolType::None; }
	Eol(EolType t) { SetType(t); }

	// 比較
	bool operator == (EolType t) const { return GetType() == t; }
	bool operator != (EolType t) const { return GetType() != t; }

	// 代入
	const Eol& operator = (const Eol& t) { eEolType = t.eEolType; return *this; }

	// 型変換
	operator EolType() const { return GetType(); }

	// 設定
	bool SetType(EolType t);	//	Typeの設定
	void SetTypeByString(const wchar_t* pszData, size_t nDataLen);
	void SetTypeByString(const char* pszData, size_t nDataLen);

	// 設定（ファイル読み込み時に使用）
	void SetTypeByStringForFile(const char* pszData, size_t nDataLen) { SetTypeByString(pszData, nDataLen); }
	void SetTypeByStringForFile_uni(const char* pszData, size_t nDataLen);
	void SetTypeByStringForFile_unibe(const char* pszData, size_t nDataLen);

	// 取得
	EolType			GetType()	const { return eEolType; }		// 現在のTypeを取得
	size_t			GetLen()	const;	// 現在のEOL長を取得。文字単位。
	const TCHAR*	GetName()	const;	// 現在のEOLの名称取得
	const wchar_t*	GetValue2()	const;	// 現在のEOL文字列先頭へのポインタを取得
	//#####

	bool IsValid() const {
		return eEolType >= EolType::CRLF && eEolType < EolType::CodeMax;
	}

private:
	EolType	eEolType;	// 改行コードの種類
};

