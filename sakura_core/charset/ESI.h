#pragma once

struct EncodingConfig;

#include "_main/global.h"

// �����R�[�h�̔��蒲�����鎞�Ɏg��������

struct tagEncodingInfo {
	EncodingType eCodeID;  // �����R�[�h���ʔԍ�
	int nSpecific;	// �]���l1
	int nPoints;	// �]���l2
};
typedef struct tagEncodingInfo  MBCODE_INFO, WCCODE_INFO;

/*
	���@�]���l�̎g�����@��

	SJIS, JIS, EUCJP, UTF-8, UTF-7 �̏ꍇ�F

	typedef�� MBCODE_INFO
	�]���l�P �� �ŗL�o�C�g��
	�]���l�Q �� �|�C���g���i���L�o�C�g�� �| �s���o�C�g���j

	UTF-16 UTF-16BE �̏ꍇ�F

	typedef�� WCCODE_INFO
	�]���l�P �� ���C�h�����̉��s�̌�
	�]���l�Q �� �s���o�C�g��
*/

static const DWORD ESI_NOINFORMATION		= 0;
static const DWORD ESI_MBC_DETECTED			= 1;
static const DWORD ESI_WC_DETECTED			= 2;
static const DWORD ESI_NODETECTED			= 4;


// ���C�h�����̂Q��ނ�����̂̊i�[�ʒu
enum EStoreID4WCInfo {
	ESI_WCIDX_UTF16LE,
	ESI_WCIDX_UTF16BE,
	ESI_WCIDX_MAX,
};
// BOM �^�C�v
enum class BOMType {
	Unknown = -1,
	LittleEndian =0,
	BigEndian = 1,
};



/*!
	�����R�[�h�𒲍����鎞�ɐ�������i�[�N���X
//*/

class ESI {
public:

	virtual ~ESI() { ; }
	explicit ESI(const EncodingConfig& ref) : pEncodingConfig(&ref) {
		dwStatus = ESI_NOINFORMATION;
		nTargetDataLen = -1;
		eMetaName = CODE_NONE;
	}

	// �������ʂ̏����i�[
	void SetInformation(const char*, const size_t);

protected:

	// �Y�����Ɏg����D�揇�ʕ\���쐬
	void InitPriorityTable(void);

	// **** �S��
	// �}���`�o�C�g�n��UNICODE�n�Ƃł��ꂼ����̊i�[�悪�Ⴄ�B
	// �ȉ��̊֐��ŋz������
	int GetIndexById(const EncodingType) const; // �����R�[�hID ������i�[��C���f�b�N�X�𐶐�

	// �f�[�^�Z�b�^/�Q�b�^�[
	void SetEvaluation(const EncodingType, const int, const int);
	void GetEvaluation(const EncodingType, int*, int *) const;

	// �����ΏۂƂȂ����f�[�^�̒����i8bit �P�ʁj
	size_t nTargetDataLen;

	// ���茋�ʂ��i�[�������
	unsigned int dwStatus;

public:

	// dwStatus �̃Z�b�^�[�^�Q�b�^�[
	void SetStatus(DWORD inf) { dwStatus |= inf; }
	DWORD GetStatus(void) const { return dwStatus; }

	// nTargetDataLen �̃Z�b�^�[�^�Q�b�^�[
protected:
	void SetDataLen(size_t n) { nTargetDataLen = n; }
public:
	size_t GetDataLen(void) const { return nTargetDataLen; }

protected:
	/*
		������̕����R�[�h�������W����
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
	// **** �}���`�o�C�g����֌W�̕ϐ����̑�
	//
	static const size_t NUM_OF_MBCODE = (CODE_CODEMAX - 2);
	MBCODE_INFO aMbcInfo[NUM_OF_MBCODE];   // SJIS, JIS, EUCJP, UTF8, UTF7 ���i�D��x�ɏ]���Ċi�[�����j
	MBCODE_INFO* apMbcInfo[NUM_OF_MBCODE]; // �]�����Ƀ\�[�g���ꂽ SJIS, JIS, EUCJP, UTF8, UTF7, CESU8 �̏��
	size_t nMbcSjisHankata;                   // SJIS ���p�J�^�J�i�̃o�C�g��
	size_t nMbcEucZenHirakata;                // EUC �S�p�Ђ炪�ȃJ�^�J�i�̃o�C�g��
	size_t nMbcEucZen;                        // EUC �S�p�̃o�C�g��

	// �}���`�o�C�g�n�̑{�����ʂ��A�|�C���g���傫�����Ƀ\�[�g�B �\�[�g�������ʂ́AapMbcInfo �Ɋi�[
	void SortMBCInfo(void);

	// EUC �� SJIS �����̃g�b�v�Q�ɏオ���Ă��邩�ǂ���
	bool IsAmbiguousEucAndSjis(void) {
		// EUC �� SJIS ���g�b�v2�ɏオ������
		// ���AEUC �� SJIS �̃|�C���g���������̂Ƃ�
		return (
			(apMbcInfo[0]->eCodeID == CODE_SJIS && apMbcInfo[1]->eCodeID == CODE_EUC
			|| apMbcInfo[1]->eCodeID == CODE_SJIS && apMbcInfo[0]->eCodeID == CODE_EUC
			)
			&& apMbcInfo[0]->nPoints == apMbcInfo[1]->nPoints
		);
	}

	// SJIS �� UTF-8 �����̃g�b�v2�ɏオ���Ă��邩�ǂ���
	bool IsAmbiguousUtf8AndCesu8(void) {
		// UTF-8 �� SJIS ���g�b�v2�ɏオ������
		// ���AUTF-8 �� SJIS �̃|�C���g���������̂Ƃ�
		return (
			(apMbcInfo[0]->eCodeID == CODE_UTF8 && apMbcInfo[1]->eCodeID == CODE_CESU8
			|| apMbcInfo[1]->eCodeID == CODE_UTF8 && apMbcInfo[0]->eCodeID == CODE_CESU8
			)
			&& apMbcInfo[0]->nPoints == apMbcInfo[1]->nPoints
		);
	}

protected:
	void GuessEucOrSjis(void);		// EUC �� SJIS ���𔻒�
	void GuessUtf8OrCesu8(void);	// UTF-8 �� CESU-8 ���𔻒�
public:
	//
	// 	**** UTF-16 ����֌W�̕ϐ����̑�
	//
	WCCODE_INFO aWcInfo[ESI_WCIDX_MAX];	// UTF-16 LE/BE ���
	BOMType eWcBomType;					// pWcInfo ���琄������� BOM �̎��
	EncodingType eMetaName;				// �G���R�[�f�B���O������̎�ޔ���

	BOMType GetBOMType(void) const { return eWcBomType; }
	EncodingType GetMetaName() const { return eMetaName; }

protected:
	// BOM�̎�ނ𐄑����� eWcBomType ��ݒ�
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

