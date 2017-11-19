#include "StdAfx.h"
#include "Convert_SpaceToTab.h"
#include "charset/charcode.h"
#include "Eol.h"
#include "util/string_ex2.h"

// �󔒁�TAB�ϊ��B�P�Ƃ̃X�y�[�X�͕ϊ����Ȃ�
bool Converter_SpaceToTab::DoConvert(NativeW* pData)
{
	using namespace WCODE;

	const wchar_t* pLine;
	size_t nLineLen;
	size_t nBgn;
	size_t nPosDes;
	int		nPosX;
	Eol	eol;

	bool	bSpace = false;	// �X�y�[�X�̏��������ǂ���
	int		j;
	int		nStartPos;

	nBgn = 0;
	nPosDes = 0;
	// �ϊ���ɕK�v�ȃo�C�g���𒲂ׂ�
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, bExtEol))) {
		if (0 < nLineLen) {
			nPosDes += nLineLen;
		}
		nPosDes += eol.GetLen();
	}
	if (0 >= nPosDes) {
		return false;
	}
	std::vector<wchar_t> des(nPosDes + 1);
	wchar_t* pDes = &des[0];
	nBgn = 0;
	nPosDes = 0;
	// CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ�
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, bExtEol))) {
		if (0 < nLineLen) {
			// �擪�s�ɂ��Ă͊J�n���ʒu���l������i����ɐ܂�Ԃ��֘A�̑΍􂪕K�v�H�j
			nPosX = (pData->GetStringPtr() == pLine)? nStartColumn: 0;	// ��������i�ɑΉ�����\�����ʒu
			bSpace = false;	// ���O���X�y�[�X��
			nStartPos = 0;	// �X�y�[�X�̐擪
			size_t i;
			for (i=0; i<nLineLen; ++i) {
				if (pLine[i] == SPACE || pLine[i] == TAB) {
					if (!bSpace) {
						nStartPos = nPosX;
					}
					bSpace = true;
					if (pLine[i] == SPACE) {
						++nPosX;
					}else if (pLine[i] == TAB) {
						nPosX += nTabWidth - (nPosX % nTabWidth);
					}
				}else {
					if (bSpace) {
						if ((nPosX - nStartPos == 1) && (pLine[i - 1] == SPACE)) {
							pDes[nPosDes] = SPACE;
							++nPosDes;
						}else {
							for (j = nStartPos / nTabWidth; j < (nPosX / nTabWidth); ++j) {
								pDes[nPosDes] = TAB;
								++nPosDes;
								nStartPos += nTabWidth - (nStartPos % nTabWidth);
							}
							// �ϊ����TAB��1������Ȃ��ꍇ�ɃX�y�[�X���l�߂�����
							// �o�b�t�@���͂ݏo���̂��C��
							for (j=nStartPos; j<nPosX; ++j) {
								pDes[nPosDes] = SPACE;
								++nPosDes;
							}
						}
					}
					++nPosX;
					if (WCODE::IsZenkaku(pLine[i])) ++nPosX;		// �S�p��������Ή�
					pDes[nPosDes] = pLine[i];
					++nPosDes;
					bSpace = false;
				}
			}
			//for (; i<nLineLen; ++i) {
			//	pDes[nPosDes] = pLine[i];
			//	++nPosDes;
			//}
			if (bSpace) {
				if ((nPosX-nStartPos == 1) && (pLine[i - 1] == SPACE)) {
					pDes[nPosDes] = SPACE;
					++nPosDes;
				}else {
					//for (j = nStartPos - 1; (j + nTabWidth) <= nPosX + 1; j += nTabWidth) {
					for (j=nStartPos/nTabWidth; j<(nPosX/nTabWidth); ++j) {
						pDes[nPosDes] = TAB;
						++nPosDes;
						nStartPos += nTabWidth - (nStartPos % nTabWidth);
					}
					// 2003.08.05 Moca
					// �ϊ����TAB��1������Ȃ��ꍇ�ɃX�y�[�X���l�߂�����
					// �o�b�t�@���͂ݏo���̂��C��
					for (j=nStartPos; j<nPosX; ++j) {
						pDes[nPosDes] = SPACE;
						++nPosDes;
					}
				}
			}
		}

		// �s���̏���
		auto_memcpy(&pDes[nPosDes], eol.GetValue2(), eol.GetLen());
		nPosDes += eol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pData->SetString(pDes, nPosDes);
	return true;
}

