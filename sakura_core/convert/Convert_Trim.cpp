#include "StdAfx.h"
#include "Convert_Trim.h"
#include "convert_util.h"
#include "Eol.h"
#include "charset/charcode.h"
#include "util/string_ex2.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! TRIM Step2
	ConvMemory ���� �߂��Ă�����̏����D
	CMemory.cpp�̂Ȃ��ɒu���Ȃ��ق����ǂ����ȂƎv���Ă�����ɒu���܂����D
	
	@author hor
	@date 2001.12.03 hor    �V�K�쐬
	@date 2007.10.18 kobake Converter_Trim�Ɉړ�
*/
bool Converter_Trim::DoConvert(NativeW* pData)
{
	const wchar_t*	pLine;
	int			nLineLen;
	int			nBgn;
	int			i, j;
	int			nPosDes;
	Eol			eol;
	int			nCharChars;

	nBgn = 0;
	nPosDes = 0;
	// �ϊ���ɕK�v�ȃo�C�g���𒲂ׂ�
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, m_bExtEol))) { // 2002/2/10 aroka CMemory�ύX
		if (0 < nLineLen) {
			nPosDes += nLineLen;
		}
		nPosDes += eol.GetLen();
	}
	if (0 >= nPosDes) {
		return true;
	}
	std::vector<wchar_t> des(nPosDes + 1);
	wchar_t* pDes = &des[0];
	nBgn = 0;
	nPosDes = 0;
	// LTRIM
	if (m_bLeft) {
		while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, m_bExtEol))) { // 2002/2/10 aroka CMemory�ύX
			if (0 < nLineLen) {
				for (i=0; i<=nLineLen; ++i) {
					if (WCODE::IsBlank(pLine[i])) {
						continue;
					}else {
						break;
					}
				}
				if (nLineLen-i>0) {
					wmemcpy(&pDes[nPosDes], &pLine[i], nLineLen);
					nPosDes += nLineLen - i;
				}
			}
			wmemcpy(&pDes[nPosDes], eol.GetValue2(), eol.GetLen());
			nPosDes += eol.GetLen();
		}
	}else {
		// RTRIM
		while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, m_bExtEol))) { // 2002/2/10 aroka CMemory�ύX
			if (0 < nLineLen) {
				// 2005.10.11 ryoji �E����k��̂ł͂Ȃ�������T���悤�ɏC���i"��@" �̉E�Q�o�C�g���S�p�󔒂Ɣ��肳�����̑Ώ��j
				i = j = 0;
				while (i < nLineLen) {
					nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, i);
					if (!WCODE::IsBlank(pLine[i])) {
						j = i + nCharChars;
					}
					i += nCharChars;
				}
				if (j > 0) {
					wmemcpy(&pDes[nPosDes], &pLine[0], j);
					nPosDes += j;
				}
			}
			wmemcpy(&pDes[nPosDes], eol.GetValue2(), eol.GetLen());
			nPosDes += eol.GetLen();
		}
	}
	pDes[nPosDes] = L'\0';

	pData->SetString(pDes, nPosDes);
	return true;
}


