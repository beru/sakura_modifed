#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �萔                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �����R�[�h�Z�b�g���
enum EncodingType {
	CODE_SJIS,						// SJIS				(MS-CP932(Windows-31J), �V�t�gJIS(Shift_JIS))
	CODE_JIS,						// JIS				(MS-CP5022x(ISO-2022-JP-MS)�ł͂Ȃ�)
	CODE_EUC,						// EUC				(MS-CP51932, eucJP-ms(eucJP-open)�ł͂Ȃ�)
	CODE_UNICODE,					// Unicode			(UTF-16 LittleEndian(UCS-2))
	CODE_UTF8,						// UTF-8(UCS-2)
	CODE_UTF7,						// UTF-7(UCS-2)
	CODE_UNICODEBE,					// Unicode BigEndian	(UTF-16 BigEndian(UCS-2))
	CODE_CESU8,						// CESU-8
	CODE_LATIN1,					// Latin1				(Latin1, ����, Windows-1252, Windows Codepage 1252 West European)
	CODE_CODEMAX,
	CODE_CPACP      = 90,
	CODE_CPOEM      = 91,
	CODE_AUTODETECT	= 99,			// �����R�[�h��������
	CODE_ERROR      = -1,			// �G���[
	CODE_NONE       = -1,			// �����o
	CODE_DEFAULT    = CODE_SJIS,	// �f�t�H���g�̕����R�[�h
	/*
		- MS-CP50220 
			Unicode ���� cp50220 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����� JIS X 0208 �̕Љ����ɒu�������
		- MS-CP50221
			Unicode ���� cp50221 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����́AG0 �W���ւ̎w���̃G�X�P�[�v�V�[�P���X ESC (I ��p���ăG���R�[�h�����
		- MS-CP50222
			Unicode ���� cp50222 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����́ASO/SI ��p���ăG���R�[�h�����
		
		�Q�l
		http://legacy-encoding.sourceforge.jp/wiki/
	*/
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �L���ȕ����R�[�h�Z�b�g�Ȃ�true
bool IsValidCodeType(int code);

// �L���ȕ����R�[�h�Z�b�g�Ȃ�true�B�������ASJIS�͏���(�t�@�C���ꗗ�ɕ����R�[�h��[]�t���ŕ\���̂���)
inline bool IsValidCodeTypeExceptSJIS(int code)
{
	return IsValidCodeType(code) && code != CODE_SJIS;
}

inline bool IsValidCodePageEx(int code)
{
	return code == 12000
		|| code == 12001
		|| ::IsValidCodePage(code);
}

void InitCodeSet();
inline bool IsValidCodeOrCPType(int code)
{
	return IsValidCodeType(code) || code == CODE_CPACP || code == CODE_CPOEM || (CODE_CODEMAX <= code && IsValidCodePageEx(code));
}
inline bool IsValidCodeOrCPTypeExceptSJIS(int code)
{
	return IsValidCodeTypeExceptSJIS(code) || code == CODE_CPACP || code == CODE_CPOEM || (CODE_CODEMAX <= code && IsValidCodePageEx(code));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���O                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CodeTypeName {
public:
	CodeTypeName(EncodingType eCodeType) : eCodeType(eCodeType) { InitCodeSet(); }
	CodeTypeName(int eCodeType) : eCodeType((EncodingType)eCodeType) { InitCodeSet(); }
	EncodingType GetCode() const { return eCodeType; }
	LPCTSTR	Normal() const;
	LPCTSTR	Short() const;
	LPCTSTR	Bracket() const;
	bool	UseBom();
	bool	CanDefault();
	bool	IsBomDefOn();
private:
	EncodingType eCodeType;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �R���{�{�b�N�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CodeTypesForCombobox {
public:
	CodeTypesForCombobox() { InitCodeSet(); }
	size_t			GetCount() const;
	EncodingType	GetCode(size_t nIndex) const;
	LPCTSTR		GetName(size_t nIndex) const;
};

