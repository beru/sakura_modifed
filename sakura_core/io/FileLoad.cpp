/*!	@file
	@brief ファイル読み込みクラス

	@author Moca
	@date 2002/08/30 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca, genta
	Copyright (C) 2003, Moca, ryoji
	Copyright (C) 2006, rastiv

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
#include <stdlib.h>
#include <string.h>
#include "_main/global.h"
#include "mem/Memory.h"
#include "Eol.h"
#include "io/FileLoad.h"
#include "charset/charcode.h"
#include "io/IoBridge.h"
#include "charset/CodeFactory.h" ////
#include "charset/CodePage.h"
#include "charset/CodeMediator.h"
#include "util/string_ex2.h"
#include "charset/ESI.h"
#include "window/EditWnd.h"

/*
	@note Win32APIで実装
		2GB以上のファイルは開けない
*/

// ロード用バッファサイズの初期値 */
const int FileLoad::g_nBufSizeDef = 32768;
// (最適値がマシンによって違うのでとりあえず32KB確保する)

// ロード用バッファサイズの設定可能な最低値
// const int g_nBufSizeMin = 1024;

// コンストラクタ
FileLoad::FileLoad()
{
	hFile			= NULL;
	nFileSize		= 0;
	nFileDataLen	= 0;
	CharCode		= CODE_DEFAULT;
	pCodeBase		= nullptr;////
	encodingTrait = ENCODING_TRAIT_ASCII;
	bBomExist		= false;	// Jun. 08, 2003 Moca
	nFlag 		= 0;
	nReadLength	= 0;
	mode			= FileLoadMode::Close;	// Jun. 08, 2003 Moca

	nLineIndex	= -1;

	pReadBuf		= NULL;
	nReadDataLen    = 0;
	nReadBufSize    = 0;
	nReadBufOffSet  = 0;
}

// デストラクタ
FileLoad::~FileLoad(void)
{
	if (hFile) {
		FileClose();
	}
	if (pReadBuf) {
		free(pReadBuf);
	}
	if (pCodeBase) {
		delete pCodeBase;
	}
}

/*!
	ファイルを開く
	@param pFileName [in] ファイル名
	@param bBigFile  [in] 2GB以上のファイルを開くか。Grep=true, 32bit版はその他=falseで運用
	@param CharCode  [in] ファイルの文字コード．
	@param nFlag [in] 文字コードのオプション
	@param pbBomExist [out] BOMの有無
	@date 2003.06.08 Moca CODE_AUTODETECTを指定できるように変更
	@date 2003.07.26 ryoji BOM引数追加
*/
EncodingType FileLoad::FileOpen(
	const EncodingConfig& encode,
	LPCTSTR pFileName,
	bool bBigFile,
	EncodingType charCode,
	int nFlag,
	bool* pbBomExist
	)
{
	encoding = encode;
	ULARGE_INTEGER	fileSize;

	if (hFile) {
		FileClose();
	}
	HANDLE hFile = ::CreateFile(
		pFileName,
		GENERIC_READ,
		// Oct. 18, 2002 genta FILE_SHARE_WRITE 追加
		// 他プロセスが書き込み中のファイルを開けるように
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// 共有
		NULL,						// セキュリティ記述子
		OPEN_EXISTING,				// 作成方法
		FILE_FLAG_SEQUENTIAL_SCAN,	// ファイル属性
		NULL						// テンプレートファイルのハンドル
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		throw Error_FileOpen();
	}
	this->hFile = hFile;

	// GetFileSizeEx は Win2K以上
	fileSize.LowPart = ::GetFileSize( hFile, &fileSize.HighPart );
	if (fileSize.LowPart == 0xFFFFFFFF) {
		DWORD lastError = ::GetLastError();
		if (NO_ERROR != lastError) {
			FileClose();
			throw Error_FileOpen();
		}
	}
	if (!bBigFile && 0x80000000 <= fileSize.QuadPart) {
		// ファイルが大きすぎる(2GB位)
		FileClose();
		throw Error_FileOpen();
	}
	nFileSize = fileSize.QuadPart;

//	mode = FLMODE_OPEN;

	// From Here Jun. 08, 2003 Moca 文字コード判定
	// データ読み込み
	Buffering();

	EncodingType nBomCode = CodeMediator::DetectUnicodeBom(pReadBuf, nReadDataLen);
	if (charCode == CODE_AUTODETECT) {
		if (nBomCode != CODE_NONE) {
			charCode = nBomCode;
		}else {
			CodeMediator mediator(encoding);
			charCode = mediator.CheckKanjiCode(pReadBuf, nReadDataLen);
		}
	}
	// To Here Jun. 08, 2003
	// 不正な文字コードのときはデフォルト(SJIS:無変換)を設定
	if (!IsValidCodeType(charCode)) {
		charCode = CODE_DEFAULT;
	}
	if (charCode != CharCode || pCodeBase == nullptr) {
		CharCode = charCode;
		if (pCodeBase != nullptr) {
			delete pCodeBase;
		}
		pCodeBase = CodeFactory::CreateCodeBase(CharCode, nFlag);
	}
	encodingTrait = CodePage::GetEncodingTrait(CharCode);
	nFlag = nFlag;

	nFileDataLen = nFileSize;
	bool bBom = false;
	if (0 < nReadDataLen) {
		Memory headData(pReadBuf, t_min(nReadDataLen, 10));
		NativeW headUni;
		IoBridge::FileToImpl(headData, &headUni, pCodeBase, nFlag);
		if (1 <= headUni.GetStringLength() && headUni.GetStringPtr()[0] == 0xfeff) {
			bBom = true;
		}
	}
	if (bBom) {
		// Jul. 26, 2003 ryoji BOMの有無をパラメータで返す
		bBomExist = true;
		if (pbBomExist) {
			*pbBomExist = true;
		}
	}else {
		// Jul. 26, 2003 ryoji BOMの有無をパラメータで返す
		if (pbBomExist) {
			*pbBomExist = false;
		}
	}
	
	// To Here Jun. 13, 2003 Moca BOMの除去
	mode = FileLoadMode::Ready;
//	memLine.AllocBuffer(256);
	pCodeBase->GetEol( &memEols[0], EolType::NEL );
	pCodeBase->GetEol( &memEols[1], EolType::LS );
	pCodeBase->GetEol( &memEols[2], EolType::PS );
	bool bEolEx = false;
	int nMaxEolLen = 0;
	for (int k=0; k<(int)_countof(memEols); ++k) {
		if (memEols[k].GetRawLength() != 0) {
			bEolEx = true;
			nMaxEolLen = t_max(nMaxEolLen, memEols[k].GetRawLength());
		}
	}
	this->bEolEx = bEolEx;
	this->nMaxEolLen = nMaxEolLen;
	if (!GetDllShareData().common.edit.bEnableExtEol) {
		bEolEx = false;
	}

	nReadOffset2 = 0;
	nTempResult = CodeConvertResult::Failure;
	lineTemp.SetString(L"");
	return CharCode;
}

/*!
	ファイルを閉じる
	読み込み用バッファとmemLineもクリアされる
*/
void FileLoad::FileClose(void)
{
	ReadBufEmpty();
	if (hFile) {
		::CloseHandle(hFile);
		hFile = NULL;
	}
	if (pCodeBase) {
		delete pCodeBase;
		pCodeBase = nullptr;
	}
	nFileSize		=  0;
	nFileDataLen	=  0;
	CharCode		= CODE_DEFAULT;
	bBomExist		= false; // From Here Jun. 08, 2003
	nFlag 		=  0;
	nReadLength	=  0;
	mode			= FileLoadMode::Close;
	nLineIndex	= -1;
}

/*! 1行読み込み
	UTF-7場合、データ内のNEL,PS,LS等の改行までを1行として取り出す
*/
CodeConvertResult FileLoad::ReadLine(
	NativeW* pUnicodeBuffer,
	Eol* pEol
	)
{
	if (CharCode != CODE_UTF7 && CharCode != CP_UTF7) {
		return ReadLine_core( pUnicodeBuffer, pEol );
	}
	if (nReadOffset2 == lineTemp.GetStringLength()) {
		Eol eol;
		CodeConvertResult e = ReadLine_core(&lineTemp, &eol);
		if (e == CodeConvertResult::Failure) {
			pUnicodeBuffer->_GetMemory()->SetRawDataHoldBuffer( L"", 0 );
			*pEol = eol;
			return CodeConvertResult::Failure;
		}
		nReadOffset2 = 0;
		nTempResult = e;
	}
	int nOffsetTemp = nReadOffset2;
	int nRetLineLen;
	Eol cEolTemp;
	const wchar_t* pRet = GetNextLineW( lineTemp.GetStringPtr(),
										lineTemp.GetStringLength(),
										&nRetLineLen,
										&nReadOffset2,
										&cEolTemp,
										GetDllShareData().common.edit.bEnableExtEol
									  );
	if (lineTemp.GetStringLength() == nReadOffset2 && nOffsetTemp == 0) {
		// 途中に改行がない限りは、swapを使って中身のコピーを省略する
		pUnicodeBuffer->swap(lineTemp);
		if (0 < lineTemp.GetStringLength()) {
			lineTemp._GetMemory()->SetRawDataHoldBuffer(L"", 0);
		}
		nReadOffset2 = 0;
	}else {
		// 改行が途中にあった。必要分をコピー
		pUnicodeBuffer->_GetMemory()->SetRawDataHoldBuffer(L"", 0);
		pUnicodeBuffer->AppendString(pRet, nRetLineLen + cEolTemp.GetLen());
	}
	*pEol = cEolTemp;
	return nTempResult;
}


/*!
	次の論理行を文字コード変換してロードする
	順次アクセス専用
	GetNextLineのような動作をする
	@return	NULL以外	1行を保持しているデータの先頭アドレスを返す。永続的ではない一時的な領域。
			NULL		データがなかった
*/
CodeConvertResult FileLoad::ReadLine_core(
	NativeW*	pUnicodeBuffer,	// [out] UNICODEデータ受け取りバッファ。改行も含めて読み取る。
	Eol*		pEol			// [i/o]
	)
{
	CodeConvertResult eRet = CodeConvertResult::Complete;

#ifdef _DEBUG
	if (mode < FileLoadMode::Ready) {
		MYTRACE(_T("FileLoad::ReadLine(): mode = %d\n"), mode);
		return CodeConvertResult::Failure;
	}
#endif
	// 行データバッファ (文字コード変換無しの生のデータ)
	lineBuffer.SetRawDataHoldBuffer("", 0);

	// 1行取り出し ReadBuf -> memLine
	// Oct. 19, 2002 genta while条件を整理
	int	nBufLineLen;
	int	nEolLen;
	int	nBufferNext;
	for (;;) {
		const char* pLine = GetNextLineCharCode(
			pReadBuf,
			nReadDataLen,    // [in] バッファの有効データサイズ
			&nBufLineLen,      // [out] 改行を含まない長さ
			&nReadBufOffSet, // [i/o] オフセット
			pEol,
			&nEolLen,
			&nBufferNext
		);
		if (!pLine) {
			break;
		}

		// ReadBufから1行を取得するとき、改行コードが欠ける可能性があるため
		if (nReadDataLen <= nReadBufOffSet && FileLoadMode::Ready == mode) {// From Here Jun. 13, 2003 Moca
			int n = 128;
			int nMinAllocSize = lineBuffer.GetRawLength() + nEolLen - nBufferNext + 100;
			while (n < nMinAllocSize) {
				n *= 2;
			}
			lineBuffer.AllocBuffer( n );
			lineBuffer.AppendRawData( pLine, nBufLineLen + nEolLen - nBufferNext );
			nReadBufOffSet -= nBufferNext;
			// バッファロード   File -> ReadBuf
			Buffering();
			if (nBufferNext == 0 && 0 < nEolLen) {
				// ぴったり行出力
				break;
			}
		}else {
			lineBuffer.AppendRawData(pLine, nBufLineLen + nEolLen);
			break;
		}
	}
	nReadLength += lineBuffer.GetRawLength();

	// 文字コード変換 cLineBuffer -> pUnicodeBuffer
	CodeConvertResult eConvertResult = IoBridge::FileToImpl(lineBuffer, pUnicodeBuffer, pCodeBase, nFlag);
	if (eConvertResult == CodeConvertResult::LoseSome) {
		eRet = CodeConvertResult::LoseSome;
	}

	++nLineIndex;

	// 2012.10.21 Moca BOMの除去(UTF-7対応)
	if (nLineIndex == 0) {
		if (bBomExist && 1 <= pUnicodeBuffer->GetStringLength()) {
			if (pUnicodeBuffer->GetStringPtr()[0] == 0xfeff) {
				*pUnicodeBuffer = NativeW(pUnicodeBuffer->GetStringPtr() + 1, pUnicodeBuffer->GetStringLength() - 1);
			}
		}
	}
	if (pUnicodeBuffer->GetStringLength() == LogicInt(0)) {
		eRet = CodeConvertResult::Failure;
	}

	return eRet;
}

/*!
	バッファにデータを読み込む
	@note エラー時は throw する
*/
void FileLoad::Buffering(void)
{
	// メモリー確保
	if (!pReadBuf) {
		int nBufSize = (nFileSize < g_nBufSizeDef) ? (static_cast<int>(nFileSize)) : (g_nBufSizeDef);
		// Borland C++では0バイトのmallocを獲得失敗と見なすため
		// 最低1バイトは取得することで0バイトのファイルを開けるようにする
		if (0 >= nBufSize) {
			nBufSize = 1; // Jun. 08, 2003  BCCのmalloc(0)がNULLを返す仕様に対処
		}

		pReadBuf = (char*) malloc(nBufSize);
		if (!pReadBuf) {
			throw Error_FileRead(); // メモリー確保に失敗
		}
		nReadDataLen = 0;
		nReadBufSize = nBufSize;
		nReadBufOffSet = 0;
	// ReadBuf内にデータが残っている
	}else if (nReadBufOffSet < nReadDataLen) {
		nReadDataLen -= nReadBufOffSet;
		memmove(pReadBuf, &pReadBuf[nReadBufOffSet], nReadDataLen);
		nReadBufOffSet = 0;
	}else {
		nReadBufOffSet = 0;
		nReadDataLen = 0;
	}
	// ファイルの読み込み
	DWORD readSize = Read(&pReadBuf[nReadDataLen], nReadBufSize - nReadDataLen);
	if (readSize == 0) {
		mode = FileLoadMode::ReadBufEnd;	// ファイルなどの終わりに達したらしい
	}
	nReadDataLen += readSize;
}

/*!
	バッファクリア
*/
void FileLoad::ReadBufEmpty(void)
{
	nReadDataLen    = 0;
	nReadBufOffSet  = 0;
}

/*!
	 現在の進行率を取得する
	 @return 0% - 100%  若干誤差が出る
*/
int FileLoad::GetPercent(void) {
	int nRet;
	if (nFileDataLen == 0 || nFileDataLen < nReadLength) {
		nRet = 100;
	}else {
		nRet = static_cast<int>(nReadLength * 100 / nFileDataLen);
	}
	return nRet;
}

/*!
	GetNextLineの汎用文字コード版
*/
const char* FileLoad::GetNextLineCharCode(
	const char*	pData,			// [in]		検索文字列
	int			nDataLen,		// [in]		検索文字列のバイト数
	int*		pnLineLen,		// [out]	1行のバイト数を返すただしEOLは含まない
	int*		pnBgn,			// [i/o]	検索文字列のバイト単位のオフセット位置
	Eol*		pEol,			// [i/o]	EOL
	int*		pnEolLen,		// [out]	EOLのバイト数 (Unicodeで困らないように)
	int*		pnBufferNext	// [out]	次回持越しバッファ長(EOLの断片)
	)
{
	int nbgn = *pnBgn;
	int i;

	pEol->SetType(EolType::None);
	*pnBufferNext = 0;

	if (nDataLen <= nbgn) {
		*pnLineLen = 0;
		*pnEolLen = 0;
		return NULL;
	}
	const unsigned char* pUData = (const unsigned char*)pData; // signedだと符号拡張でNELがおかしくなるので
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
	int nLen = nDataLen;
	int neollen = 0;
	switch (encodingTrait) {
	case ENCODING_TRAIT_ERROR://
	case ENCODING_TRAIT_ASCII:
		{
			static const EolType eEolEx[] = {
				EolType::NEL,
				EolType::LS,
				EolType::PS,
			};
			nLen = nDataLen;
			for (i=nbgn; i<nDataLen; ++i) {
				if (pData[i] == '\r' || pData[i] == '\n') {
					pEol->SetTypeByStringForFile( &pData[i], nDataLen - i );
					neollen = pEol->GetLen();
					break;
				}
				if (bEolEx) {
					size_t k;
					for (k=0; k<_countof(eEolEx); ++k) {
						if (memEols[k].GetRawLength() != 0
							&& i + memEols[k].GetRawLength() - 1 < nDataLen
							&& memcmp(memEols[k].GetRawPtr(), pData+i, memEols[k].GetRawLength()) == 0
						) {
							pEol->SetType(eEolEx[k]);
							neollen = memEols[k].GetRawLength();
							break;
						}
					}
					if (k != _countof(eEolEx)) {
						break;
					}
				}
			}
			// UTF-8のNEL,PS,LS断片の検出
			if (i == nDataLen && bEolEx) {
				for (i=t_max(0, nDataLen - nMaxEolLen - 1); i < nDataLen; ++i) {
					bool bSet = false;
					for (size_t k=0; k<_countof(eEolEx); ++k) {
						int nCompLen = t_min(nDataLen-i, memEols[k].GetRawLength());
						if (nCompLen != 0
							&& memcmp(memEols[k].GetRawPtr(), pData+i, nCompLen) == 0
						) {
							*pnBufferNext = t_max(*pnBufferNext, nCompLen);
							bSet = true;
						}
					}
					if (bSet) {
						break;
					}
				}
				i = nDataLen;
			}
		}
		break;
	case ENCODING_TRAIT_UTF16LE:
		nLen = nDataLen - 1;
		for (i=nbgn; i<nLen; i+=2) {
			wchar_t c = static_cast<wchar_t>((pUData[i + 1] << 8) | pUData[i]);
			if (WCODE::IsLineDelimiter(c, bExtEol)) {
				pEol->SetTypeByStringForFile_uni( &pData[i], nDataLen - i );
				neollen = (Int)pEol->GetLen() * sizeof(wchar_t);
				break;
			}
		}
		break;
	case ENCODING_TRAIT_UTF16BE:
		nLen = nDataLen - 1;
		for (i=nbgn; i<nLen; i+=2) {
			wchar_t c = static_cast<wchar_t>((pUData[i] << 8) | pUData[i + 1]);
			if (WCODE::IsLineDelimiter(c, bExtEol)) {
				pEol->SetTypeByStringForFile_unibe( &pData[i], nDataLen - i );
				neollen = (Int)pEol->GetLen() * sizeof(wchar_t);
				break;
			}
		}
		break;
	case ENCODING_TRAIT_UTF32LE:
		nLen = nDataLen - 3;
		for (i=nbgn; i<nLen; i+=4) {
			wchar_t c = static_cast<wchar_t>((pUData[i+1] << 8) | pUData[i]);
			if (pUData[i+3] == 0x00
				&& pUData[i+2] == 0x00
				&& WCODE::IsLineDelimiter(c, bExtEol)
			) {
				wchar_t c2;
				int eolTempLen;
				if (i + 4 < nLen
					&& pUData[i+7] == 0x00
					&& pUData[i+6] == 0x00
				) {
					c2 = static_cast<wchar_t>((pUData[i+5] << 8) | pUData[i+4]);
					eolTempLen = 2 * sizeof(wchar_t);
				}else {
					c2 = 0x0000;
					eolTempLen = 1 * sizeof(wchar_t);
				}
				wchar_t pDataTmp[2] = {c, c2};
				pEol->SetTypeByStringForFile_uni( reinterpret_cast<char *>(pDataTmp), eolTempLen );
				neollen = (Int)pEol->GetLen() * 4;
				break;
			}
		}
		break;
	case ENCODING_TRAIT_UTF32BE:
		nLen = nDataLen - 3;
		for (i=nbgn; i<nLen; i+=4) {
			wchar_t c = static_cast<wchar_t>((pUData[i+2] << 8) | pUData[i+3]);
			if (pUData[i] == 0x00
				&& pUData[i+1] == 0x00
				&& WCODE::IsLineDelimiter(c, bExtEol)
			) {
				wchar_t c2;
				int eolTempLen;
				if (i + 4 < nLen
					&& pUData[i+4] == 0x00
					&& pUData[i+5] == 0x00
				) {
					c2 = static_cast<wchar_t>((pUData[i+6] << 8) | pUData[i+7]);
					eolTempLen = 2 * sizeof(wchar_t);
				}else {
					c2 = 0x0000;
					eolTempLen = 1 * sizeof(wchar_t);
				}
				wchar_t pDataTmp[2] = {c, c2};
				pEol->SetTypeByStringForFile_uni( reinterpret_cast<char *>(pDataTmp), eolTempLen );
				neollen = (Int)pEol->GetLen() * 4;
				break;
			}
		}
		break;
	case ENCODING_TRAIT_EBCDIC_CRLF:
	case ENCODING_TRAIT_EBCDIC:
		// EOLコード変換しつつ設定
		for (i=nbgn; i<nDataLen; ++i) {
			if (encodingTrait == ENCODING_TRAIT_EBCDIC && bExtEol) {
				if (pData[i] == '\x15') {
					pEol->SetType(EolType::NEL);
					neollen = 1;
					break;
				}
			}
			if (pData[i] == '\x0d' || pData[i] == '\x25') {
				char szEof[3] = {
					(pData[i]  == '\x25' ? '\x0a' : '\x0d'),
					(pData[i+1]== '\x25' ? '\x0a' : 
						(pData[i+1] == '\x0a' ? 0 : // EBCDIC の"\x0aがLFにならないように細工する
							(i + 1 < nDataLen ? pData[i+1] : 0))),
					0
				};
				pEol->SetTypeByStringForFile( szEof, t_min(nDataLen - i, 2) );
				neollen = (Int)pEol->GetLen();
				break;
			}
		}
		break;
	}

	if (neollen < 1) {
		// EOLがなかった場合
		if (i != nDataLen) {
			i = nDataLen;		// 最後の半端なバイトを落とさないように
		}
	}else {
		// CRの場合は、CRLFかもしれないので次のバッファへ送る
		if (*pEol == EolType::CR) {
			*pnBufferNext = neollen;
		}
	}

	*pnBgn = i + neollen;
	*pnLineLen = i - nbgn;
	*pnEolLen = neollen;

	return &pData[nbgn];
}

