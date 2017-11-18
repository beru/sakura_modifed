#pragma once

// 定数
enum class CodeConvertResult {
	Complete, // データを失うことなく変換が完了した。
	LoseSome, // 変換が完了したが、一部のデータが失われた。
	Failure,  // 何らかの原因により失敗した。
};

class Memory;
class NativeW;
struct CommonSetting_StatusBar;
enum class EolType;

/*!
	文字コード基底クラス。
	
	ここで言う「特定コード」とは、
	CodeBaseを継承した子クラスが定める、一意の文字コードのことです。
*/
class CodeBase {
public:
	virtual ~CodeBase() {}
//	virtual bool IsCode(const Memory* pMem) {return false;}  // 特定コードであればtrue

	// 文字コード変換
	virtual CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst) = 0;	// 特定コード → UNICODE    変換
	virtual CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst) = 0;	// UNICODE    → 特定コード 変換
	// UNICODE    → 特定コード 変換
	virtual CodeConvertResult UnicodeToCode(const StringRef& src, Memory* pDst) {
		NativeW mem(src.GetPtr(), src.GetLength());
		return UnicodeToCode(mem, pDst);
	}

	// ファイル形式
	virtual void GetBom(Memory* pMemBom);											// BOMデータ取得
	void GetEol(Memory* pMemEol, EolType eEolType) { S_GetEol(pMemEol, eEolType); }	// 改行データ取得

	// 文字コード表示用
	virtual CodeConvertResult UnicodeToHex(const wchar_t* pSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);			// UNICODE → Hex 変換

	// 変換エラー処理（１バイト <-> U+D800 から U+D8FF）
	static size_t BinToText(const unsigned char*, const size_t, unsigned short*);
	static int TextToBin(const unsigned short);

	// MIME Header デコーダ
	static bool MIMEHeaderDecode(const char*, const size_t, Memory*, const EncodingType);

	static void S_GetEol(Memory* pMemEol, EolType eEolType);	// 改行データ取得
	
protected:

};

/*!
	バイナリ１バイトを U+DC00 から U+DCFF までに対応付ける
*/
inline size_t CodeBase::BinToText(const unsigned char* pSrc, const size_t nLen, unsigned short* pDst)
{
	size_t i;

	for (i=0; i<nLen; ++i) {
		pDst[i] = static_cast<unsigned short>(pSrc[i]) + 0xdc00;
	}

	return i;
}


/*!
	U+DC00 から U+DCFF からバイナリ1バイトを復元
*/
inline int CodeBase::TextToBin(const unsigned short cSrc)
{
	return static_cast<int>((cSrc - 0xdc00) & 0x00ff);
}

