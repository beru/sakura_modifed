/*!	@file
	@brief �t�@�C���ǂݍ��݃N���X

	@author Moca
	@date 2002/08/30 �V�K�쐬
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
	@note Win32API�Ŏ���
		2GB�ȏ�̃t�@�C���͊J���Ȃ�
*/

// ���[�h�p�o�b�t�@�T�C�Y�̏����l */
const int FileLoad::g_nBufSizeDef = 32768;
// (�œK�l���}�V���ɂ���ĈႤ�̂łƂ肠����32KB�m�ۂ���)

// ���[�h�p�o�b�t�@�T�C�Y�̐ݒ�\�ȍŒ�l
// const int g_nBufSizeMin = 1024;

// �R���X�g���N�^
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

// �f�X�g���N�^
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
	�t�@�C�����J��
	@param pFileName [in] �t�@�C����
	@param bBigFile  [in] 2GB�ȏ�̃t�@�C�����J�����BGrep=true, 32bit�ł͂��̑�=false�ŉ^�p
	@param CharCode  [in] �t�@�C���̕����R�[�h�D
	@param nFlag [in] �����R�[�h�̃I�v�V����
	@param pbBomExist [out] BOM�̗L��
	@date 2003.06.08 Moca CODE_AUTODETECT���w��ł���悤�ɕύX
	@date 2003.07.26 ryoji BOM�����ǉ�
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
		// Oct. 18, 2002 genta FILE_SHARE_WRITE �ǉ�
		// ���v���Z�X���������ݒ��̃t�@�C�����J����悤��
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// ���L
		NULL,						// �Z�L�����e�B�L�q�q
		OPEN_EXISTING,				// �쐬���@
		FILE_FLAG_SEQUENTIAL_SCAN,	// �t�@�C������
		NULL						// �e���v���[�g�t�@�C���̃n���h��
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		throw Error_FileOpen();
	}
	this->hFile = hFile;

	// GetFileSizeEx �� Win2K�ȏ�
	fileSize.LowPart = ::GetFileSize( hFile, &fileSize.HighPart );
	if (fileSize.LowPart == 0xFFFFFFFF) {
		DWORD lastError = ::GetLastError();
		if (NO_ERROR != lastError) {
			FileClose();
			throw Error_FileOpen();
		}
	}
	if (!bBigFile && 0x80000000 <= fileSize.QuadPart) {
		// �t�@�C�����傫������(2GB��)
		FileClose();
		throw Error_FileOpen();
	}
	nFileSize = fileSize.QuadPart;

//	mode = FLMODE_OPEN;

	// From Here Jun. 08, 2003 Moca �����R�[�h����
	// �f�[�^�ǂݍ���
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
	// �s���ȕ����R�[�h�̂Ƃ��̓f�t�H���g(SJIS:���ϊ�)��ݒ�
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
		// Jul. 26, 2003 ryoji BOM�̗L�����p�����[�^�ŕԂ�
		bBomExist = true;
		if (pbBomExist) {
			*pbBomExist = true;
		}
	}else {
		// Jul. 26, 2003 ryoji BOM�̗L�����p�����[�^�ŕԂ�
		if (pbBomExist) {
			*pbBomExist = false;
		}
	}
	
	// To Here Jun. 13, 2003 Moca BOM�̏���
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
	�t�@�C�������
	�ǂݍ��ݗp�o�b�t�@��memLine���N���A�����
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

/*! 1�s�ǂݍ���
	UTF-7�ꍇ�A�f�[�^����NEL,PS,LS���̉��s�܂ł�1�s�Ƃ��Ď��o��
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
		// �r���ɉ��s���Ȃ�����́Aswap���g���Ē��g�̃R�s�[���ȗ�����
		pUnicodeBuffer->swap(lineTemp);
		if (0 < lineTemp.GetStringLength()) {
			lineTemp._GetMemory()->SetRawDataHoldBuffer(L"", 0);
		}
		nReadOffset2 = 0;
	}else {
		// ���s���r���ɂ������B�K�v�����R�s�[
		pUnicodeBuffer->_GetMemory()->SetRawDataHoldBuffer(L"", 0);
		pUnicodeBuffer->AppendString(pRet, nRetLineLen + cEolTemp.GetLen());
	}
	*pEol = cEolTemp;
	return nTempResult;
}


/*!
	���̘_���s�𕶎��R�[�h�ϊ����ă��[�h����
	�����A�N�Z�X��p
	GetNextLine�̂悤�ȓ��������
	@return	NULL�ȊO	1�s��ێ����Ă���f�[�^�̐擪�A�h���X��Ԃ��B�i���I�ł͂Ȃ��ꎞ�I�ȗ̈�B
			NULL		�f�[�^���Ȃ�����
*/
CodeConvertResult FileLoad::ReadLine_core(
	NativeW*	pUnicodeBuffer,	// [out] UNICODE�f�[�^�󂯎��o�b�t�@�B���s���܂߂ēǂݎ��B
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
	// �s�f�[�^�o�b�t�@ (�����R�[�h�ϊ������̐��̃f�[�^)
	lineBuffer.SetRawDataHoldBuffer("", 0);

	// 1�s���o�� ReadBuf -> memLine
	// Oct. 19, 2002 genta while�����𐮗�
	int	nBufLineLen;
	int	nEolLen;
	int	nBufferNext;
	for (;;) {
		const char* pLine = GetNextLineCharCode(
			pReadBuf,
			nReadDataLen,    // [in] �o�b�t�@�̗L���f�[�^�T�C�Y
			&nBufLineLen,      // [out] ���s���܂܂Ȃ�����
			&nReadBufOffSet, // [i/o] �I�t�Z�b�g
			pEol,
			&nEolLen,
			&nBufferNext
		);
		if (!pLine) {
			break;
		}

		// ReadBuf����1�s���擾����Ƃ��A���s�R�[�h��������\�������邽��
		if (nReadDataLen <= nReadBufOffSet && FileLoadMode::Ready == mode) {// From Here Jun. 13, 2003 Moca
			int n = 128;
			int nMinAllocSize = lineBuffer.GetRawLength() + nEolLen - nBufferNext + 100;
			while (n < nMinAllocSize) {
				n *= 2;
			}
			lineBuffer.AllocBuffer( n );
			lineBuffer.AppendRawData( pLine, nBufLineLen + nEolLen - nBufferNext );
			nReadBufOffSet -= nBufferNext;
			// �o�b�t�@���[�h   File -> ReadBuf
			Buffering();
			if (nBufferNext == 0 && 0 < nEolLen) {
				// �҂�����s�o��
				break;
			}
		}else {
			lineBuffer.AppendRawData(pLine, nBufLineLen + nEolLen);
			break;
		}
	}
	nReadLength += lineBuffer.GetRawLength();

	// �����R�[�h�ϊ� cLineBuffer -> pUnicodeBuffer
	CodeConvertResult eConvertResult = IoBridge::FileToImpl(lineBuffer, pUnicodeBuffer, pCodeBase, nFlag);
	if (eConvertResult == CodeConvertResult::LoseSome) {
		eRet = CodeConvertResult::LoseSome;
	}

	++nLineIndex;

	// 2012.10.21 Moca BOM�̏���(UTF-7�Ή�)
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
	�o�b�t�@�Ƀf�[�^��ǂݍ���
	@note �G���[���� throw ����
*/
void FileLoad::Buffering(void)
{
	// �������[�m��
	if (!pReadBuf) {
		int nBufSize = (nFileSize < g_nBufSizeDef) ? (static_cast<int>(nFileSize)) : (g_nBufSizeDef);
		// Borland C++�ł�0�o�C�g��malloc���l�����s�ƌ��Ȃ�����
		// �Œ�1�o�C�g�͎擾���邱�Ƃ�0�o�C�g�̃t�@�C�����J����悤�ɂ���
		if (0 >= nBufSize) {
			nBufSize = 1; // Jun. 08, 2003  BCC��malloc(0)��NULL��Ԃ��d�l�ɑΏ�
		}

		pReadBuf = (char*) malloc(nBufSize);
		if (!pReadBuf) {
			throw Error_FileRead(); // �������[�m�ۂɎ��s
		}
		nReadDataLen = 0;
		nReadBufSize = nBufSize;
		nReadBufOffSet = 0;
	// ReadBuf���Ƀf�[�^���c���Ă���
	}else if (nReadBufOffSet < nReadDataLen) {
		nReadDataLen -= nReadBufOffSet;
		memmove(pReadBuf, &pReadBuf[nReadBufOffSet], nReadDataLen);
		nReadBufOffSet = 0;
	}else {
		nReadBufOffSet = 0;
		nReadDataLen = 0;
	}
	// �t�@�C���̓ǂݍ���
	DWORD readSize = Read(&pReadBuf[nReadDataLen], nReadBufSize - nReadDataLen);
	if (readSize == 0) {
		mode = FileLoadMode::ReadBufEnd;	// �t�@�C���Ȃǂ̏I���ɒB�����炵��
	}
	nReadDataLen += readSize;
}

/*!
	�o�b�t�@�N���A
*/
void FileLoad::ReadBufEmpty(void)
{
	nReadDataLen    = 0;
	nReadBufOffSet  = 0;
}

/*!
	 ���݂̐i�s�����擾����
	 @return 0% - 100%  �኱�덷���o��
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
	GetNextLine�̔ėp�����R�[�h��
*/
const char* FileLoad::GetNextLineCharCode(
	const char*	pData,			// [in]		����������
	int			nDataLen,		// [in]		����������̃o�C�g��
	int*		pnLineLen,		// [out]	1�s�̃o�C�g����Ԃ�������EOL�͊܂܂Ȃ�
	int*		pnBgn,			// [i/o]	����������̃o�C�g�P�ʂ̃I�t�Z�b�g�ʒu
	Eol*		pEol,			// [i/o]	EOL
	int*		pnEolLen,		// [out]	EOL�̃o�C�g�� (Unicode�ō���Ȃ��悤��)
	int*		pnBufferNext	// [out]	���񎝉z���o�b�t�@��(EOL�̒f��)
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
	const unsigned char* pUData = (const unsigned char*)pData; // signed���ƕ����g����NEL�����������Ȃ�̂�
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
			// UTF-8��NEL,PS,LS�f�Ђ̌��o
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
		// EOL�R�[�h�ϊ����ݒ�
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
						(pData[i+1] == '\x0a' ? 0 : // EBCDIC ��"\x0a��LF�ɂȂ�Ȃ��悤�ɍ׍H����
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
		// EOL���Ȃ������ꍇ
		if (i != nDataLen) {
			i = nDataLen;		// �Ō�̔��[�ȃo�C�g�𗎂Ƃ��Ȃ��悤��
		}
	}else {
		// CR�̏ꍇ�́ACRLF��������Ȃ��̂Ŏ��̃o�b�t�@�֑���
		if (*pEol == EolType::CR) {
			*pnBufferNext = neollen;
		}
	}

	*pnBgn = i + neollen;
	*pnLineLen = i - nbgn;
	*pnEolLen = neollen;

	return &pData[nbgn];
}

