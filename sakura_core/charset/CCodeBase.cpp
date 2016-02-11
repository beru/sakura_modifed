// ���̍s�͕��������΍�̂��߂̂��̂ł��B
#include "StdAfx.h"
#include "CCodeBase.h"
#include "charcode.h"
#include "convert/convert_util2.h"
#include "charset/codechecker.h"
#include "CEol.h"

// ��ˑ�����
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

void CodeBase::GetBom(Memory* pcmemBom) { pcmemBom->Clear(); }					// BOM�f�[�^�擾

// �\���p16�\��	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
CodeConvertResult CodeBase::UnicodeToHex(
	const wchar_t* cSrc,
	const int iSLen,
	TCHAR* pDst,
	const CommonSetting_Statusbar* psStatusbar
	)
{
	if (IsUTF16High(cSrc[0]) && iSLen >= 2 && IsUTF16Low(cSrc[1])) {
		// �T���Q�[�g�y�A
		if (psStatusbar->m_bDispSPCodepoint) {
			auto_sprintf(pDst, _T("U+%05X"), 0x10000 + ((cSrc[0] & 0x3FF)<<10) + (cSrc[1] & 0x3FF));
		}else {
			auto_sprintf(pDst, _T("%04X%04X"), cSrc[0], cSrc[1]);
		}
	}else {
		auto_sprintf(pDst, _T("U+%04X"), cSrc[0]);
	}

	return CodeConvertResult::Complete;
}


/*!
	MIME �f�R�[�_�[

	@param[out] pcMem �f�R�[�h�ς݂̕�������i�[
*/
bool CodeBase::MIMEHeaderDecode(
	const char* pSrc,
	const int nSrcLen,
	Memory* pcMem,
	const ECodeType eCodetype
	)
{
	ECodeType ecodetype;
	int nskip_bytes;

	// �\�[�X���擾
	pcMem->AllocBuffer(nSrcLen);
	char* pdst = reinterpret_cast<char*>(pcMem->GetRawPtr());
	if (!pdst) {
		pcMem->SetRawData("", 0);
		return false;
	}

	Memory cmembuf;
	int i = 0;
	int j = 0;
	while (i < nSrcLen) {
		if (pSrc[i] != '=') {
			pdst[j] = pSrc[i];
			++i;
			++j;
			continue;
		}
		nskip_bytes = _DecodeMimeHeader(&pSrc[i], nSrcLen - i, &cmembuf, &ecodetype);
		if (nskip_bytes < 1) {
			pdst[j] = pSrc[i];
			++i;
			++j;
		}else {
			if (ecodetype == eCodetype) {
				// eChartype �� ecodetype �ƈ�v���Ă���ꍇ�ɂ����A
				// �ϊ����ʂ��R�s�[
				memcpy(&pdst[j], cmembuf.GetRawPtr(), cmembuf.GetRawLength());
				i += nskip_bytes;
				j += cmembuf.GetRawLength();
			}else {
				memcpy(&pdst[j], &pSrc[i], nskip_bytes);
				i += nskip_bytes;
				j += nskip_bytes;
			}
		}
	}

	pcMem->_SetRawLength(j);
	return true;
}

/*!
	���s�f�[�^�擾
*/
// CShiftJis���ړ� 2010/6/13 Uchi
void CodeBase::S_GetEol(
	Memory* pcmemEol,
	EolType eolType
	)
{
	static const struct{
		const char* szData;
		int nLen;
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
	pcmemEol->SetRawData(aEolTable[(int)eolType].szData, aEolTable[(int)eolType].nLen);
}

