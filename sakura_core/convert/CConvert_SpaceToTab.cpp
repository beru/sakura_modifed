#include "StdAfx.h"
#include "CConvert_SpaceToTab.h"
#include "charset/charcode.h"
#include "CEol.h"
#include "util/string_ex2.h"

// �󔒁�TAB�ϊ��B�P�Ƃ̃X�y�[�X�͕ϊ����Ȃ�
bool Converter_SpaceToTab::DoConvert(NativeW* pData)
{
	using namespace WCODE;

	const wchar_t* pLine;
	int		nLineLen;
	int		nBgn;
	int		i;
	int		nPosDes;
	int		nPosX;
	Eol	cEol;

	bool	bSpace = false;	// �X�y�[�X�̏��������ǂ���
	int		j;
	int		nStartPos;

	nBgn = 0;
	nPosDes = 0;
	// �ϊ���ɕK�v�ȃo�C�g���𒲂ׂ�
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol))) {
		if (0 < nLineLen) {
			nPosDes += nLineLen;
		}
		nPosDes += cEol.GetLen();
	}
	if (0 >= nPosDes) {
		return false;
	}
	std::vector<wchar_t> des(nPosDes + 1);
	wchar_t* pDes = &des[0];
	nBgn = 0;
	nPosDes = 0;
	// CRLF�ŋ�؂���u�s�v��Ԃ��BCRLF�͍s���ɉ����Ȃ�
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &cEol, m_bExtEol))) {
		if (0 < nLineLen) {
			// �擪�s�ɂ��Ă͊J�n���ʒu���l������i����ɐ܂�Ԃ��֘A�̑΍􂪕K�v�H�j
			nPosX = (pData->GetStringPtr() == pLine)? m_nStartColumn: 0;	// ��������i�ɑΉ�����\�����ʒu
			bSpace = false;	// ���O���X�y�[�X��
			nStartPos = 0;	// �X�y�[�X�̐擪
			for (i=0; i<nLineLen; ++i) {
				if (SPACE == pLine[i] || TAB == pLine[i]) {
					if (!bSpace) {
						nStartPos = nPosX;
					}
					bSpace = true;
					if (SPACE == pLine[i]) {
						++nPosX;
					}else if (TAB == pLine[i]) {
						nPosX += m_nTabWidth - (nPosX % m_nTabWidth);
					}
				}else {
					if (bSpace) {
						if ((nPosX - nStartPos == 1) && (pLine[i - 1] == SPACE)) {
							pDes[nPosDes] = SPACE;
							++nPosDes;
						}else {
							for (j = nStartPos / m_nTabWidth; j < (nPosX / m_nTabWidth); ++j) {
								pDes[nPosDes] = TAB;
								++nPosDes;
								nStartPos += m_nTabWidth - (nStartPos % m_nTabWidth);
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
					++nPosX;
					if (WCODE::IsZenkaku(pLine[i])) ++nPosX;		// �S�p��������Ή� 2008.10.17 matsumo
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
					//for (j = nStartPos - 1; (j + m_nTabWidth) <= nPosX + 1; j += m_nTabWidth) {
					for (j=nStartPos/m_nTabWidth; j<(nPosX/m_nTabWidth); ++j) {
						pDes[nPosDes] = TAB;
						++nPosDes;
						nStartPos += m_nTabWidth - (nStartPos % m_nTabWidth);
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
		auto_memcpy(&pDes[nPosDes], cEol.GetValue2(), cEol.GetLen());
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = L'\0';

	pData->SetString(pDes, nPosDes);
	return true;
}

