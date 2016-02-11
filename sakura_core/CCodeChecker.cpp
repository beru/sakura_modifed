#include "StdAfx.h"
#include "CCodeChecker.h"
#include "charset/CCodePage.h"
#include "io/CIoBridge.h"
#include "charset/CCodeFactory.h" ////
#include "charset/CUnicode.h"

#include "doc/CEditDoc.h"
#include "doc/logic/CDocLineMgr.h"
#include "window/CEditWnd.h"
#include "util/string_ex.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �Z�[�u���`�F�b�N                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! DocLineMgr���ێ�����f�[�^�ɈقȂ���s�R�[�h�����݂��Ă��邩�ǂ������肷��
static bool _CheckSavingEolcode(
	const DocLineMgr& pcDocLineMgr,
	Eol cEolType
	)
{
	bool bMix = false;
	if (cEolType == EOL_NONE) {	// ���s�R�[�h�ϊ��Ȃ�
		Eol cEolCheck;	// ��r�Ώ�EOL
		const DocLine* pcDocLine = pcDocLineMgr.GetDocLineTop();
		if (pcDocLine) {
			cEolCheck = pcDocLine->GetEol();
		}
		while (pcDocLine) {
			Eol cEol = pcDocLine->GetEol();
			if (cEol != cEolCheck && cEol != EOL_NONE) {
				bMix = true;
				break;
			}
			pcDocLine = pcDocLine->GetNextLine();
		}
	}
	return bMix;
}

//! DocLineMgr���ێ�����f�[�^���w�蕶���R�[�h�ň��S�ɕۑ��ł��邩�ǂ������肷��
static CodeConvertResult _CheckSavingCharcode(
	const DocLineMgr& pcDocLineMgr,
	ECodeType eCodeType,
	LogicPoint& point,
	NativeW& wc
	)
{
	const DocLine*	pcDocLine = pcDocLineMgr.GetDocLineTop();
	const bool bCodePageMode = IsValidCodeOrCPType(eCodeType) && !IsValidCodeType(eCodeType);
	CodeBase* pCodeBase = CodeFactory::CreateCodeBase(eCodeType, 0);
	Memory cmemTmp;	// �o�b�t�@���ė��p
	NativeW cmemTmp2;
	LogicInt nLine = LogicInt(0);
	while (pcDocLine) {
		// �R�[�h�ϊ� pcDocLine -> cmemTmp
		CodeConvertResult e = IoBridge::ImplToFile(
			pcDocLine->_GetDocLineDataWithEOL(),
			&cmemTmp,
			pCodeBase
		);
		if (bCodePageMode) {
			// �R�[�h�y�[�W��CodeConvertResult::LoseSome��Ԃ��Ȃ��̂ŁA�����ŕ������r����
			CodeConvertResult e2 = IoBridge::FileToImpl(
				cmemTmp,
				&cmemTmp2,
				pCodeBase,
				0
			);
			const int nDocLineLen = (Int)pcDocLine->GetLengthWithEOL();
			const int nConvertLen = (Int)cmemTmp2.GetStringLength();
			const int nDataMinLen = t_min(nDocLineLen, nConvertLen);
			const wchar_t* p = pcDocLine->GetPtr();
			const wchar_t* r = cmemTmp2.GetStringPtr();
			int nPos = -1;
			for (int i=0; i<nDataMinLen; ++i) {
				if (p[i] != r[i]) {
					nPos = i;
					break;
				}
			}
			if (nPos == -1 && nDocLineLen != nConvertLen) {
				nPos = nDataMinLen;
			}
			if (nPos != -1) {
				point.y = nLine;
				point.x = LogicInt(nPos);
				// �ϊ��ł��Ȃ������ʒu��1�����擾
				wc.SetString( p + nPos, (Int)NativeW::GetSizeOfChar( p, nDocLineLen, nPos ) );
				delete pCodeBase;
				return CodeConvertResult::LoseSome;
			}
		}
		if (e != CodeConvertResult::Complete) {
			if (e == CodeConvertResult::LoseSome) {
				// �s���̈ʒu�����
				point.y = nLine;
				point.x = LogicInt(-1);
				const WCHAR* pLine = pcDocLine->GetPtr();
				const LogicInt nLineLen = pcDocLine->GetLengthWithEOL();
				LogicInt chars = NativeW::GetSizeOfChar( pLine, nLineLen, 0 );
				LogicInt nPos = LogicInt(0);
				NativeW mem;
				while (0 < chars) {
					mem.SetStringHoldBuffer( pLine + nPos, chars );
					CodeConvertResult e2 = IoBridge::ImplToFile(
						mem,
						&cmemTmp,
						pCodeBase
					);
					if (e2 == CodeConvertResult::LoseSome) {
						point.x = nPos;
						wc = mem;
						break;
					}
					nPos += chars;
					chars = NativeW::GetSizeOfChar( pLine, nLineLen, nPos );
				}
			}
			delete pCodeBase;
			return e;
		}

		// ���̍s��
		pcDocLine = pcDocLine->GetNextLine();
		++nLine;
	}
	delete pCodeBase;
	return CodeConvertResult::Complete;
}


CallbackResultType CodeChecker::OnCheckSave(SaveInfo* pSaveInfo)
{
	EditDoc* pcDoc = GetListeningDoc();

	// ���s�R�[�h�����݂��Ă��邩�ǂ�������
	bool bTmpResult = false;
	if (pcDoc->m_cDocType.GetDocumentAttribute().m_bChkEnterAtEnd) {
		bTmpResult = _CheckSavingEolcode(
			pcDoc->m_cDocLineMgr, pSaveInfo->cEol
		);
	}

	// ���[�U�₢���킹
	if (bTmpResult) {
		int nDlgResult = MYMESSAGEBOX(
			EditWnd::getInstance()->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONWARNING,
			GSTR_APPNAME,
			LS(STR_CODECHECKER_EOL_UNIFY),
			pcDoc->m_cDocEditor.GetNewLineCode().GetName()
		);
		switch (nDlgResult) {
		case IDYES:		pSaveInfo->cEol = pcDoc->m_cDocEditor.GetNewLineCode(); break; // ����
		case IDNO:		break; // ���s
		case IDCANCEL:	return CallbackResultType::Interrupt; // ���f
		}
	}

	// �w�蕶���R�[�h�ň��S�ɕۑ��ł��邩�ǂ�������
	LogicPoint point;
	NativeW cmemChar(L"", 0);
	CodeConvertResult nTmpResult = _CheckSavingCharcode(
		pcDoc->m_cDocLineMgr, pSaveInfo->eCharCode,
		point, cmemChar
	);

	// ���[�U�₢���킹
	if (nTmpResult == CodeConvertResult::LoseSome) {
		TCHAR szCpName[100];
		TCHAR  szLineNum[60];  // 123��
		TCHAR  szCharCode[12]; // U+12ab or 1234abcd
		CodePage::GetNameNormal(szCpName, pSaveInfo->eCharCode);
		_tcscpy( szCharCode, _T("") );
		_tcscpy( szLineNum, _T("") );
		if (point.x == -1) {
			cmemChar.SetString(LSW(STR_ERR_CSHAREDATA22));
		}else {
			auto_sprintf( szLineNum, _T("%d"), (int)((Int)point.x) + 1 );
			_tcscat( szLineNum, LS(STR_DLGFNCLST_LIST_COL) );
			Unicode().UnicodeToHex( cmemChar.GetStringPtr(), cmemChar.GetStringLength(),
				szCharCode, &GetDllShareData().m_common.m_sStatusbar );
		}
		int nDlgResult = MYMESSAGEBOX(
			EditWnd::getInstance()->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONWARNING,
			GSTR_APPNAME,
			LS(STR_CODECHECKER_CONFORM_LOSESOME),
			szCpName,
			(int)((Int)point.y + 1),
			szLineNum,
			cmemChar.GetStringPtr(),
			szCharCode
		);
		switch (nDlgResult) {
		case IDYES:		break; // ���s
		case IDNO:		return CallbackResultType::Interrupt; // ���f
		case IDCANCEL:
			{
				LogicPoint pt(point.x < 0 ? LogicInt(0) : point.x, point.y);
				pcDoc->m_pcEditWnd->GetActiveView().GetCommander().Command_MOVECURSOR(pt, 0);
			}
			return CallbackResultType::Interrupt; //���f
		}
	}
	return CallbackResultType::Continue;
}

void CodeChecker::OnFinalSave(SaveResultType eSaveResult)
{
	// �J�L�R����
	if (eSaveResult == SaveResultType::LoseSome) {
		ErrorMessage(EditWnd::getInstance()->GetHwnd(), LS(STR_CODECHECKER_LOSESOME_SAVE));
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ���[�h���`�F�b�N                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CodeChecker::OnFinalLoad(LoadResultType eLoadResult)
{
	if (eLoadResult == LoadResultType::LoseSome) {
		ErrorMessage(
			EditWnd::getInstance()->GetHwnd(),
			LS(STR_CODECHECKER_LOSESOME_ROAD)
		);
	}
}

