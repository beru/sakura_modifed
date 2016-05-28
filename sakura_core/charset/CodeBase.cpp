// ���̍s�͕��������΍�̂��߂̂��̂ł��B
#include "StdAfx.h"
#include "CodeBase.h"
#include "charcode.h"
#include "convert/convert_util2.h"
#include "charset/codecheck.h"
#include "Eol.h"

// ��ˑ�����
#include "env/ShareData.h"
#include "env/DllSharedData.h"

void CodeBase::GetBom(Memory* pMemBom) { pMemBom->Clear(); }					// BOM�f�[�^�擾

// �\���p16�\��	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
CodeConvertResult CodeBase::UnicodeToHex(
	const wchar_t* pSrc,
	size_t iSLen,
	TCHAR* pDst,
	const CommonSetting_StatusBar* psStatusbar
	)
{
	if (IsUTF16High(pSrc[0]) && iSLen >= 2 && IsUTF16Low(pSrc[1])) {
		// �T���Q�[�g�y�A
		if (psStatusbar->bDispSPCodepoint) {
			auto_sprintf(pDst, _T("U+%05X"), 0x10000 + ((pSrc[0] & 0x3FF)<<10) + (pSrc[1] & 0x3FF));
		}else {
			auto_sprintf(pDst, _T("%04X%04X"), pSrc[0], pSrc[1]);
		}
	}else {
		auto_sprintf(pDst, _T("U+%04X"), pSrc[0]);
	}

	return CodeConvertResult::Complete;
}


/*!
	MIME �f�R�[�_�[

	@param[out] pMem �f�R�[�h�ς݂̕�������i�[
*/
bool CodeBase::MIMEHeaderDecode(
	const char* pSrc,
	const size_t nSrcLen,
	Memory* pMem,
	const EncodingType codetype
	)
{
	EncodingType ecodetype;
	int nskip_bytes;

	// �\�[�X���擾
	pMem->AllocBuffer(nSrcLen);
	char* pdst = reinterpret_cast<char*>(pMem->GetRawPtr());
	if (!pdst) {
		pMem->SetRawData("", 0);
		return false;
	}

	Memory membuf;
	size_t i = 0;
	size_t j = 0;
	while (i < nSrcLen) {
		if (pSrc[i] != '=') {
			pdst[j] = pSrc[i];
			++i;
			++j;
			continue;
		}
		nskip_bytes = _DecodeMimeHeader(&pSrc[i], nSrcLen - i, &membuf, &ecodetype);
		if (nskip_bytes < 1) {
			pdst[j] = pSrc[i];
			++i;
			++j;
		}else {
			if (ecodetype == codetype) {
				// eChartype �� ecodetype �ƈ�v���Ă���ꍇ�ɂ����A
				// �ϊ����ʂ��R�s�[
				memcpy(&pdst[j], membuf.GetRawPtr(), membuf.GetRawLength());
				i += nskip_bytes;
				j += membuf.GetRawLength();
			}else {
				memcpy(&pdst[j], &pSrc[i], nskip_bytes);
				i += nskip_bytes;
				j += nskip_bytes;
			}
		}
	}

	pMem->_SetRawLength(j);
	return true;
}

/*!
	���s�f�[�^�擾
*/
// CShiftJis���ړ� 2010/6/13 Uchi
void CodeBase::S_GetEol(
	Memory* pMemEol,
	EolType eolType
	)
{
	static const struct{
		const char* szData;
		size_t nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		{ "",			0 },	// EolType::None
		{ "\x0d\x0a",	2 },	// EolType::CRLF
		{ "\x0a",		1 },	// EolType::LF
		{ "\x0d",		1 },	// EolType::CR
		{ "",			0 },	// EolType::NEL
		{ "",			0 },	// EolType::LS
		{ "",			0 },	// EolType::PS
	};
	pMemEol->SetRawData(aEolTable[(int)eolType].szData, aEolTable[(int)eolType].nLen);
}

