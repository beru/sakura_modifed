#include "StdAfx.h"
#include "convert/Decode_Base64Decode.h"
#include "charset/charcode.h"
#include "convert/convert_util2.h"
#include "charset/codecheck.h"

// Base64�f�R�[�h
bool Decode_Base64Decode::DoDecode(const NativeW& src, Memory* pDst)
{
	using namespace WCODE;

	const size_t BUFFER_SIZE = 1024;  // �o�b�t�@�T�C�Y�B�P�ȏ�̐������S�̔{���ŁB
	const size_t _BUFSIZE = ((BUFFER_SIZE + 3) / 4) * 4;

	const wchar_t* pSrc;
	size_t nSrcLen;
	char *pw, *pw_base;
	wchar_t buffer[_BUFSIZE];
	size_t i, j;
	wchar_t c = 0;

	pSrc = src.GetStringPtr();
	nSrcLen = src.GetStringLength();
	pDst->AllocBuffer(nSrcLen);  // �������݃o�b�t�@���m��
	pw_base = pw = reinterpret_cast<char*>(pDst->GetRawPtr());

	i = 0;  // pSrc �̓Y����
	do {
		j = 0;
		for (; i<nSrcLen; ++i) {
		// �o�b�t�@�ɕ��������߂郋�[�v
			c = pSrc[i];
			if (IsLineDelimiterBasic(c) || c == TAB || c == SPACE) {
				continue;
			}
			if (j == _BUFSIZE || c == LTEXT('=')) {
				break;
			}
			if (!IsBase64(c)) {
				return false;
			}
			buffer[j] = static_cast<char>(c & 0xff);
			++j;
		}
		pw += _DecodeBase64(&buffer[0], j, pw);
	}while (i < nSrcLen && c != LTEXT('='));

	//if (!CheckBase64Padbit(&buffer[0], j)) {
	//	return false;
	//}

	pDst->_SetRawLength(pw - pw_base);
	return true;
}

