#pragma once

struct EncodingConfig;

#include "_main/global.h"

// 文字コードの判定調査する時に使う情報入れ

struct tagEncodingInfo {
	EncodingType eCodeID;  // 文字コード識別番号
	int nSpecific;	// 評価値1
	int nPoints;	// 評価値2
};
typedef struct tagEncodingInfo  MBCODE_INFO, WCCODE_INFO;

/*
	○　評価値の使い方　○

	SJIS, JIS, EUCJP, UTF-8, UTF-7 の場合：

	typedef名 MBCODE_INFO
	評価値１ → 固有バイト数
	評価値２ → ポイント数（特有バイト数 − 不正バイト数）

	UTF-16 UTF-16BE の場合：

	typedef名 WCCODE_INFO
	評価値１ → ワイド文字の改行の個数
	評価値２ → 不正バイト数
*/

static const DWORD ESI_NOINFORMATION		= 0;
static const DWORD ESI_MBC_DETECTED			= 1;
static const DWORD ESI_WC_DETECTED			= 2;
static const DWORD ESI_NODETECTED			= 4;


// ワイド文字の２種類あるものの格納位置
enum EStoreID4WCInfo {
	ESI_WCIDX_UTF16LE,
	ESI_WCIDX_UTF16BE,
	ESI_WCIDX_MAX,
};
// BOM タイプ
enum class BOMType {
	Unknown = -1,
	LittleEndian =0,
	BigEndian = 1,
};



/*!
	文字コードを調査する時に生じる情報格納クラス
//*/

class ESI {
public:

	virtual ~ESI() { ; }
	explicit ESI(const EncodingConfig& ref) : pEncodingConfig(&ref) {
		dwStatus = ESI_NOINFORMATION;
		nTargetDataLen = -1;
		eMetaName = CODE_NONE;
	}

	// 調査結果の情報を格納
	void SetInformation(const char*, const size_t);

protected:

	// 添え字に使われる優先順位表を作成
	void InitPriorityTable(void);

	// **** 全般
	// マルチバイト系とUNICODE系とでそれぞれ情報の格納先が違う。
	// 以下の関数で吸収する
	int GetIndexById(const EncodingType) const; // 文字コードID から情報格納先インデックスを生成

	// データセッタ/ゲッター
	void SetEvaluation(const EncodingType, const int, const int);
	void GetEvaluation(const EncodingType, int*, int *) const;

	// 調査対象となったデータの長さ（8bit 単位）
	size_t nTargetDataLen;

	// 判定結果を格納するもの
	unsigned int dwStatus;

public:

	// dwStatus のセッター／ゲッター
	void SetStatus(DWORD inf) { dwStatus |= inf; }
	DWORD GetStatus(void) const { return dwStatus; }

	// nTargetDataLen のセッター／ゲッター
protected:
	void SetDataLen(size_t n) { nTargetDataLen = n; }
public:
	size_t GetDataLen(void) const { return nTargetDataLen; }

protected:
	/*
		文字列の文字コード情報を収集する
	*/
	void ScanCode(const char*, const size_t);

	void GetEncodingInfo_sjis(const char*, size_t);
	void GetEncodingInfo_jis(const char*, size_t);
	void GetEncodingInfo_eucjp(const char*, size_t);
	void GetEncodingInfo_utf8(const char*, size_t);
	void GetEncodingInfo_utf7(const char*, size_t);
	void GetEncodingInfo_cesu8(const char*, size_t);
	void GetEncodingInfo_uni(const char*, size_t);
	void GetEncodingInfo_latin1(const char*, size_t);
	void GetEncodingInfo_meta(const char *, size_t);


	bool _CheckUtf16Eol(const char* pS, size_t nLen, bool bbig_endian);
	inline bool _CheckUtf16EolLE(const char* p, size_t n) { return _CheckUtf16Eol(p, n, false); }
	inline bool _CheckUtf16EolBE(const char* p, size_t n) { return _CheckUtf16Eol(p, n, true); }

public:
	//
	// **** マルチバイト判定関係の変数その他
	//
	static const size_t NUM_OF_MBCODE = (CODE_CODEMAX - 2);
	MBCODE_INFO aMbcInfo[NUM_OF_MBCODE];   // SJIS, JIS, EUCJP, UTF8, UTF7 情報（優先度に従って格納される）
	MBCODE_INFO* apMbcInfo[NUM_OF_MBCODE]; // 評価順にソートされた SJIS, JIS, EUCJP, UTF8, UTF7, CESU8 の情報
	size_t nMbcSjisHankata;                   // SJIS 半角カタカナのバイト数
	size_t nMbcEucZenHirakata;                // EUC 全角ひらがなカタカナのバイト数
	size_t nMbcEucZen;                        // EUC 全角のバイト数

	// マルチバイト系の捜査結果を、ポイントが大きい順にソート。 ソートした結果は、apMbcInfo に格納
	void SortMBCInfo(void);

	// EUC と SJIS が候補のトップ２に上がっているかどうか
	bool IsAmbiguousEucAndSjis(void) {
		// EUC と SJIS がトップ2に上がった時
		// かつ、EUC と SJIS のポイント数が同数のとき
		return (
			(apMbcInfo[0]->eCodeID == CODE_SJIS && apMbcInfo[1]->eCodeID == CODE_EUC
			|| apMbcInfo[1]->eCodeID == CODE_SJIS && apMbcInfo[0]->eCodeID == CODE_EUC
			)
			&& apMbcInfo[0]->nPoints == apMbcInfo[1]->nPoints
		);
	}

	// SJIS と UTF-8 が候補のトップ2に上がっているかどうか
	bool IsAmbiguousUtf8AndCesu8(void) {
		// UTF-8 と SJIS がトップ2に上がった時
		// かつ、UTF-8 と SJIS のポイント数が同数のとき
		return (
			(apMbcInfo[0]->eCodeID == CODE_UTF8 && apMbcInfo[1]->eCodeID == CODE_CESU8
			|| apMbcInfo[1]->eCodeID == CODE_UTF8 && apMbcInfo[0]->eCodeID == CODE_CESU8
			)
			&& apMbcInfo[0]->nPoints == apMbcInfo[1]->nPoints
		);
	}

protected:
	void GuessEucOrSjis(void);		// EUC か SJIS かを判定
	void GuessUtf8OrCesu8(void);	// UTF-8 か CESU-8 かを判定
public:
	//
	// 	**** UTF-16 判定関係の変数その他
	//
	WCCODE_INFO aWcInfo[ESI_WCIDX_MAX];	// UTF-16 LE/BE 情報
	BOMType eWcBomType;					// pWcInfo から推測される BOM の種類
	EncodingType eMetaName;				// エンコーディング名からの種類判別

	BOMType GetBOMType(void) const { return eWcBomType; }
	EncodingType GetMetaName() const { return eMetaName; }

protected:
	// BOMの種類を推測して eWcBomType を設定
	void GuessUtf16Bom(void);
	EncodingType AutoDetectByXML( const char*, size_t );
	EncodingType AutoDetectByHTML( const char*, size_t );
	EncodingType AutoDetectByCoding( const char*, size_t );
	
public:
	const EncodingConfig* pEncodingConfig;

#ifdef _DEBUG
public:
	static void GetDebugInfo(const char*, const int, NativeT*);
#endif
};

