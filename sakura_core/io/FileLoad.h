/*!	@file
	@brief メモリバッファクラスへのファイル入力クラス

	@author Moca
	@date 2002/08/30 新規作成
*/
/*
	Copyright (C) 2002, Moca, genta
	Copyright (C) 2003, Moca, ryoji

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

#include <Windows.h>
#include "Stream.h" //CError_FileOpen
#include "charset/CodeBase.h"
#include "charset/CodePage.h"
#include "util/design_template.h"

// VC6添付のヘッダで定義されてません
#ifndef INVALID_SET_FILE_POINTER
 #define INVALID_SET_FILE_POINTER 0xFFFFFFFF
#endif // INVALID_SET_FILE_POINTER

struct EncodingConfig;
class CodeBase;

/*!
	文字コードを変換してデータを行単位で取得するためのクラス
	@note 明示的にFileOpenメンバを呼び出さないと使えない
		ファイルポインタを共有すると困るので、クラスのコピー禁止
*/
class FileLoad {
public:

	FileLoad();
	~FileLoad(void);

	// Jul. 26, 2003 ryoji BOM引数追加
	EncodingType FileOpen(
		const EncodingConfig& encode,
		LPCTSTR pFileName,
		bool bBigFile,
		EncodingType charCode,
		int nFlag,
		bool* pbBomExist = nullptr
	);		// 指定文字コードでファイルをオープンする
	void FileClose(void);					// 明示的にファイルをクローズする

	// 1行データをロードする 順アクセス用
	CodeConvertResult ReadLine(
		NativeW*	pUnicodeBuffer,	// [out] UNICODEデータ受け取りバッファ
		Eol*		pEol			// [i/o]
	);

// 未実装関数郡
//	cosnt char* ReadAtLine(int, int*, Eol*); // 指定行目をロードする
//	cosnt wchar_t* ReadAtLineW(int, int*, Eol*); // 指定行目をロードする(Unicode版)
//	bool ReadIgnoreLine(void); // 1行読み飛ばす

	// ファイルの日時を取得する
	BOOL GetFileTime(FILETIME*, FILETIME*, FILETIME*); // inline

	// Jun. 08, 2003 Moca
	// 開いたファイルにはBOMがあるか？
	bool IsBomExist(void) { return bBomExist; }

	// 現在の進行率を取得する(0% - 100%) 若干誤差が出る
	int GetPercent(void);

	// ファイルサイズを取得する
	inline LONGLONG GetFileSize( void ){ return nFileSize; }

	static const int g_nBufSizeDef; // ロード用バッファサイズの初期値
//	static const int g_nBufSizeMin; // ロード用バッファサイズの設定可能な最低値

protected:
	// Oct. 19, 2002 genta スペルミス修正
//	void SeekBegin(void);		// ファイルの先頭位置に移動する(BOMを考慮する)
	void Buffering(void);		// バッファにデータをロードする
	void ReadBufEmpty(void);	// バッファを空にする

	// GetLextLine の 文字コード考慮版
	const char* GetNextLineCharCode( const char*, size_t, size_t*, size_t*, Eol*, size_t*, size_t* );
	CodeConvertResult ReadLine_core( NativeW*, Eol* );

	int Read(void*, size_t); // inline
	DWORD FilePointer(DWORD, DWORD); // inline

	// メンバオブジェクト
	EncodingConfig encoding;

//	LPTSTR	pszFileName;		// ファイル名
	HANDLE		hFile;			// ファイルハンドル
	LONGLONG	nFileSize;		// ファイルサイズ(64bit)
	LONGLONG	nFileDataLen;	// ファイルデータ長からBOM長を引いたバイト数
	LONGLONG	nReadLength;	// 現在までにロードしたデータの合計バイト数(BOM長を含まない)
	int			nLineIndex;		// 現在ロードしている論理行(0開始)
	EncodingType	CharCode;	// 文字コード
	CodeBase*	pCodeBase;	////
	EEncodingTrait	encodingTrait;
	Memory			memEols[3];
	bool	bEolEx;		// CR/LF以外のEOLが有効か
	size_t	nMaxEolLen;	// EOLの長さ
	bool	bBomExist;	// ファイルのBOMが付いているか Jun. 08, 2003 Moca 
	int		nFlag;		// 文字コードの変換オプション
	// Jun. 13, 2003 Moca
	// 状態をenumとしてわかりやすく．
	enum class FileLoadMode {
		Close,		// 初期状態
		Open,		// ファイルオープンのみ
		Ready,		// 順アクセスOK
		ReadBufEnd	// ファイルの終端までバッファに入れた
	};
	FileLoadMode	mode;	// 現在の読み込み状態

	// 読み込みバッファ系
	char*	pReadBuf;			// 読み込みバッファへのポインタ
	size_t	nReadBufSize;		// 読み込みバッファの実際に確保しているサイズ
	size_t	nReadDataLen;		// 読み込みバッファの有効データサイズ
	size_t	nReadBufOffSet;		// 読み込みバッファ中のオフセット(次の行頭位置)
//	int		nReadBufSumSize;	// 今までにバッファに読み込んだデータの合計サイズ
	Memory lineBuffer;
	NativeW lineTemp;
	size_t nReadOffset2;
	CodeConvertResult nTempResult;

private:
	DISALLOW_COPY_AND_ASSIGN(FileLoad);
}; // class FileLoad

// インライン関数郡

// public
inline BOOL FileLoad::GetFileTime(FILETIME* pftCreate, FILETIME* pftLastAccess, FILETIME* pftLastWrite) {
	return ::GetFileTime(hFile, pftCreate, pftLastAccess, pftLastWrite);
}

// protected
inline int FileLoad::Read(void* pBuf, size_t nSize) {
	DWORD ReadSize;
	if (!::ReadFile(hFile, pBuf, (DWORD)nSize, &ReadSize, NULL)) {
		throw Error_FileRead();
	}
	return (int)ReadSize;
}

// protected
inline DWORD FileLoad::FilePointer(DWORD offset, DWORD origin) {
	DWORD fp;
	if ((fp = ::SetFilePointer(hFile, offset, NULL, FILE_BEGIN)) == INVALID_SET_FILE_POINTER) {
		throw Error_FileRead();
	}
	return fp;
}

