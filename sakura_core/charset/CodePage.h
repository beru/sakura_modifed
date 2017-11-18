#pragma once

#include "CodeBase.h"
#include <vector>
#include <utility>
#include <string>
#include "ShiftJis.h"

enum EEncodingTrait {
	ENCODING_TRAIT_ERROR, // error
	ENCODING_TRAIT_ASCII,// ASCII comportible 1byte
	ENCODING_TRAIT_UTF16LE,// UTF-16LE
	ENCODING_TRAIT_UTF16BE,// UTF-16BE
	ENCODING_TRAIT_UTF32LE,// UTF-32LE 0123
	ENCODING_TRAIT_UTF32BE,// UTF-32BE 3210
	ENCODING_TRAIT_EBCDIC_CRLF,// EBCDIC/CR,LF
	ENCODING_TRAIT_EBCDIC,// EBCDIC/CR,LF,NEL
};


/*
	�V�X�e���R�[�h�y�[�W�ɂ�镶���R�[�h�ϊ�
*/
class CodePage : public CodeBase{
public:
	CodePage(int codepageEx) : nCodePageEx(codepageEx) { }
	
	//CodeBase�C���^�[�t�F�[�X
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return CPToUnicode(src, pDst, nCodePageEx); }	// ����R�[�h �� UNICODE    �ϊ�
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToCP(src, pDst, nCodePageEx); }	// UNICODE    �� ����R�[�h �ϊ�
	void GetEol(Memory* pMemEol, EolType eolType);	// ���s�f�[�^�擾
	void GetBom(Memory* pMemBom);	// BOM�f�[�^�擾
	CodeConvertResult UnicodeToHex(const wchar_t* cSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);			// UNICODE �� Hex �ϊ�

public:
	//����
	static CodeConvertResult CPToUnicode(const Memory& src, NativeW* pDst, int codepageEx);		// CodePage  �� Unicode�R�[�h�ϊ� 
	static CodeConvertResult UnicodeToCP(const NativeW& src, Memory* pDst, int codepageEx);		// Unicode   �� CodePage�R�[�h�ϊ�

	typedef std::vector<std::pair<int, std::wstring>> CodePageList;
	
	//GUI�p�⏕�֐�
	static CodePage::CodePageList& GetCodePageList();
	static int GetNameNormal(LPTSTR outName, int charcodeEx);
	static int GetNameShort(LPTSTR outName, int charcodeEx);
	static int GetNameLong(LPTSTR outName, int charcodeEx);
	static int GetNameBracket(LPTSTR outName, int charcodeEx);
	static int AddComboCodePages(HWND hwnd, HWND combo, int nSelCode);
	
	//CP�⏕���
	static EEncodingTrait GetEncodingTrait(int charcodeEx);
	
protected:
	// ����
	static CodeConvertResult CPToUni(const char*, const size_t, wchar_t*, size_t, size_t&, UINT);
	static CodeConvertResult UniToCP(const wchar_t*, const size_t, char*, size_t, size_t&, UINT);
	
	int nCodePageEx;
	
	static BOOL CALLBACK CallBackEnumCodePages( LPCTSTR );

	static size_t MultiByteToWideChar2(UINT, int, const char*, size_t, wchar_t*, size_t);
	static size_t WideCharToMultiByte2(UINT, int, const wchar_t*, size_t, char*, size_t);
	static size_t S_UTF32LEToUnicode(const char*, size_t, wchar_t*, size_t);
	static size_t S_UTF32BEToUnicode(const char*, size_t, wchar_t*, size_t);
	static size_t S_UnicodeToUTF32LE(const wchar_t*, size_t, char*, size_t);
	static size_t S_UnicodeToUTF32BE(const wchar_t*, size_t, char*, size_t);
};

