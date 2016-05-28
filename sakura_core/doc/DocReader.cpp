#include "StdAfx.h"
#include "DocReader.h"
#include "logic/DocLine.h"
#include "logic/DocLineMgr.h"

/* �S�s�f�[�^��Ԃ�
	���s�R�[�h�́ACRLF�ɓ��ꂳ���B
	@retval �S�s�f�[�^�Bfree�ŊJ�����Ȃ���΂Ȃ�Ȃ��B
	@note   Debug�ł̃e�X�g�ɂ̂ݎg�p���Ă���B
*/
wchar_t* DocReader::GetAllData(size_t* pnDataLen)
{
	const DocLine* pDocLine = pDocLineMgr->GetDocLineTop();
	size_t nDataLen = 0;
	while (pDocLine) {
		// Oct. 7, 2002 YAZAKI
		nDataLen += pDocLine->GetLengthWithoutEOL() + 2;	// \r\n��ǉ����ĕԂ����� + 2����B
		pDocLine = pDocLine->GetNextLine();
	}
	
	wchar_t* pData = (wchar_t*)malloc((nDataLen + 1) * sizeof(wchar_t));
	if (!pData) {
		TopErrorMessage(NULL, LS(STR_ERR_DLGDOCLM6), nDataLen + 1);
		return NULL;
	}
	pDocLine = pDocLineMgr->GetDocLineTop();
	
	nDataLen = 0;
	while (pDocLine) {
		// Oct. 7, 2002 YAZAKI
		size_t nLineLen = pDocLine->GetLengthWithoutEOL();
		if (0 < nLineLen) {
			wmemcpy(&pData[nDataLen], pDocLine->GetPtr(), nLineLen);
			nDataLen += nLineLen;
		}
		pData[nDataLen++] = L'\r';
		pData[nDataLen++] = L'\n';
		pDocLine = pDocLine->GetNextLine();
	}
	pData[nDataLen] = L'\0';
	*pnDataLen = nDataLen;
	return pData;
}

const wchar_t* DocReader::GetLineStr(size_t nLine, size_t* pnLineLen)
{
	const DocLine* pDocLine = pDocLineMgr->GetLine(nLine);
	if (!pDocLine) {
		*pnLineLen = 0;
		return NULL;
	}
	// 2002/2/10 aroka CMemory �̃����o�ϐ��ɒ��ڃA�N�Z�X���Ȃ�(inline������Ă���̂ő��x�I�Ȗ��͂Ȃ�)
	return pDocLine->GetDocLineStrWithEOL(pnLineLen);
}

/*!
	�w�肳�ꂽ�s�ԍ��̕�����Ɖ��s�R�[�h�������������擾
	
	@author Moca
	@date 2003.06.22
*/
const wchar_t* DocReader::GetLineStrWithoutEOL(size_t nLine, size_t* pnLineLen)
{
	const DocLine* pDocLine = pDocLineMgr->GetLine(nLine);
	if (!pDocLine) {
		*pnLineLen = 0;
		return NULL;
	}
	*pnLineLen = pDocLine->GetLengthWithoutEOL();
	return pDocLine->GetPtr();
}


/*! ���A�N�Z�X���[�h�F�擪�s�𓾂�

	@param pnLineLen [out] �s�̒������Ԃ�B
	@return 1�s�ڂ̐擪�ւ̃|�C���^�B
	�f�[�^��1�s���Ȃ��Ƃ��́A����0�A�|�C���^NULL���Ԃ�B

*/
const wchar_t* DocReader::GetFirstLinrStr(size_t* pnLineLen)
{
	const wchar_t* pszLine;
	if (pDocLineMgr->GetLineCount() == 0) {
		pszLine = NULL;
		*pnLineLen = 0;
	}else {
		pszLine = pDocLineMgr->GetDocLineTop()->GetDocLineStrWithEOL(pnLineLen);
		pDocLineMgr->pDocLineCurrent = const_cast<DocLine*>(pDocLineMgr->GetDocLineTop()->GetNextLine());
	}
	return pszLine;
}


/*!
	���A�N�Z�X���[�h�F���̍s�𓾂�

	@param pnLineLen [out] �s�̒������Ԃ�B
	@return ���s�̐擪�ւ̃|�C���^�B
	GetFirstLinrStr()���Ăяo����Ă��Ȃ���NULL���Ԃ�

*/
const wchar_t* DocReader::GetNextLinrStr(size_t* pnLineLen)
{
	const wchar_t* pszLine;
	if (!pDocLineMgr->pDocLineCurrent) {
		pszLine = NULL;
		*pnLineLen = 0;
	}else {
		pszLine = pDocLineMgr->pDocLineCurrent->GetDocLineStrWithEOL(pnLineLen);
		pDocLineMgr->pDocLineCurrent = pDocLineMgr->pDocLineCurrent->GetNextLine();
	}
	return pszLine;
}

