#include "StdAfx.h"
#include "charset/CCodeMediator.h"
#include "charset/charcode.h"
#include "charset/CESI.h"
#include "io/CBinaryStream.h"
#include "types/CType.h"


/*!
	������̐擪��Unicode�nBOM���t���Ă��邩�H

	@retval CODE_UNICODE   UTF-16 LE
	@retval CODE_UTF8      UTF-8
	@retval CODE_UNICODEBE UTF-16 BE
	@retval CODE_NONE      �����o

	@date 2007.08.11 charcode.cpp ����ړ�
*/
ECodeType CodeMediator::DetectUnicodeBom(const char* pS, const int nLen)
{
	uchar_t* pBuf;

	if (!pS) { return CODE_NONE; }

	pBuf = (uchar_t*) pS;
	if (2 <= nLen) {
		if (pBuf[0] == 0xff && pBuf[1] == 0xfe) {
			return CODE_UNICODE;
		}
		if (pBuf[0] == 0xfe && pBuf[1] == 0xff) {
			return CODE_UNICODEBE;
		}
		if (3 <= nLen) {
			if (pBuf[0] == 0xef && pBuf[1] == 0xbb && pBuf[2] == 0xbf) {
				return CODE_UTF8;
			}
		}
	}
	if (4 <= nLen) {
		if (memcmp(pBuf, "+/v", 3) == 0
			&& (pBuf[3] == '8' || pBuf[3] == '9' || pBuf[3] == '+' || pBuf[3] == '/')
		) {
			return CODE_UTF7;
		}
	}
	return CODE_NONE;
}


/*!
	SJIS, JIS, EUCJP, UTF-8, UTF-7 �𔻒� (��)

	@return SJIS, JIS, EUCJP, UTF-8, UTF-7 �̉��ꂩ�� ID ��Ԃ��D

	@note �K�؂Ȍ��o���s��ꂽ�ꍇ�́Am_dwStatus �� CESI_MB_DETECTED �t���O���i�[�����B
*/
ECodeType CodeMediator::DetectMBCode(ESI* pEsi)
{
//	pEsi->m_dwStatus = ESI_NOINFORMATION;

	if (pEsi->GetDataLen() < (pEsi->m_apMbcInfo[0]->nSpecific - pEsi->m_apMbcInfo[0]->nPoints) * 2000) {
		// �s���o�C�g�̊������A�S�̂� 0.05% �����ł��邱�Ƃ��m�F�B
		// �S�̂�0.05%�قǂ̕s���o�C�g�́A��������B
		pEsi->SetStatus(ESI_NODETECTED);
		return CODE_NONE;
	}
	if (pEsi->m_apMbcInfo[0]->nPoints <= 0) {
		pEsi->SetStatus(ESI_NODETECTED);
		return CODE_NONE;
	}

	/*
		����󋵂��m�F
	*/
	pEsi->SetStatus(ESI_MBC_DETECTED);
	return pEsi->m_apMbcInfo[0]->eCodeID;
}


/*!
	UTF-16 LE/BE �𔻒�.

	@retval CODE_UNICODE    UTF-16 LE �����o���ꂽ
	@retval CODE_UNICODEBE  UTF-16 BE �����o���ꂽ
	@retval 0               UTF-16 LE/BE �Ƃ��Ɍ��o����Ȃ�����

*/
ECodeType CodeMediator::DetectUnicode(ESI* pEsi)
{
//	pEsi->m_dwStatus = ESI_NOINFORMATION;

	BOMType bomType = pEsi->GetBOMType();
	if (bomType == BOMType::Unknown) {
		pEsi->SetStatus(ESI_NODETECTED);
		return CODE_NONE;
	}

	// 1�s�̕��ό�����200�𒴂��Ă���ꍇ��Unicode�����o�Ƃ���
	int nDataLen = pEsi->GetDataLen();
	int nLineBreak = pEsi->m_aWcInfo[(int)bomType].nSpecific;  // ���s���� nLineBreak�Ɏ擾
	if (static_cast<double>(nDataLen) / nLineBreak > 200) {
		pEsi->SetStatus(ESI_NODETECTED);
		return CODE_NONE;
	}

	pEsi->SetStatus(ESI_WC_DETECTED);
	return pEsi->m_aWcInfo[(int)bomType].eCodeID;
}


/*
	���{��R�[�h�Z�b�g����
*/
ECodeType CodeMediator::CheckKanjiCode(ESI* pEsi)
{
	/*
		����󋵂́A
		DetectMBCode(), DetectUnicode() ����
		esi.m_dwStatus �ɋL�^����B
	*/

	if (!pEsi) {
		return CODE_DEFAULT;
	}
	if (pEsi->GetMetaName() != CODE_NONE) {
		return pEsi->GetMetaName();
	}
	ECodeType nret;
	nret = DetectUnicode( pEsi );
	if (nret != CODE_NONE && pEsi->GetStatus() != ESI_NODETECTED) {
		return nret;
	}
	nret = DetectMBCode(pEsi);
	if (nret != CODE_NONE && pEsi->GetStatus() != ESI_NODETECTED) {
		return nret;
	}

	// �f�t�H���g�����R�[�h��Ԃ�
	return pEsi->m_pEncodingConfig->m_eDefaultCodetype;
}


/*
	���{��R�[�h�Z�b�g����

	�߂�l�z2007.08.14 kobake �߂�l��int����ECodeType�֕ύX
	SJIS		CODE_SJIS
	JIS			CODE_JIS
	EUC			CODE_EUC
	Unicode		CODE_UNICODE
	UTF-8		CODE_UTF8
	UTF-7		CODE_UTF7
	UnicodeBE	CODE_UNICODEBE
*/
ECodeType CodeMediator::CheckKanjiCode(const char* pBuf, int nBufLen)
{
	ESI esi(*m_pEncodingConfig);

	/*
		����󋵂́A
		DetectMBCode(), DetectUnicode() ����
		esi.m_dwStatus �ɋL�^����B
	*/

	esi.SetInformation(pBuf, nBufLen/*, CODE_SJIS*/);
	return CheckKanjiCode(&esi);
}



/*
|| �t�@�C���̓��{��R�[�h�Z�b�g����
||
|| �y�߂�l�z2007.08.14 kobake �߂�l��int����ECodeType�֕ύX
||	SJIS		CODE_SJIS
||	JIS			CODE_JIS
||	EUC			CODE_EUC
||	Unicode		CODE_UNICODE
||	UTF-8		CODE_UTF8
||	UTF-7		CODE_UTF7
||	UnicodeBE	CODE_UNICODEBE
||	�G���[		CODE_ERROR
*/
ECodeType CodeMediator::CheckKanjiCodeOfFile(const TCHAR* pszFile)
{
	// �I�[�v��
	BinaryInputStream in(pszFile);
	if (!in) {
		return CODE_ERROR;
	}

	// �f�[�^���擾
	int nBufLen = in.GetLength();
	if (nBufLen > CheckKanjiCode_MAXREADLENGTH) {
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}

	// 0�o�C�g�Ȃ�^�C�v�ʂ̃f�t�H���g�ݒ�
	if (nBufLen == 0) {
		return m_pEncodingConfig->m_eDefaultCodetype;
	}

	// �f�[�^�m��
	Memory mem;
	mem.AllocBuffer(nBufLen);
	void* pBuf = mem.GetRawPtr();

	// �ǂݍ���
	nBufLen = in.Read(pBuf, nBufLen);

	// �N���[�Y
	in.Close();

	// ���{��R�[�h�Z�b�g����
	ECodeType nCodeType = DetectUnicodeBom(reinterpret_cast<const char*>(pBuf), nBufLen);
	if (nCodeType == CODE_NONE) {
		// Unicode BOM �͌��o����܂���ł����D
		nCodeType = CheckKanjiCode(reinterpret_cast<const char*>(pBuf), nBufLen);
	}

	return nCodeType;
}

