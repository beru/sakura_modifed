#include "StdAfx.h"
#include "CodeChecker.h"
#include "charset/CodePage.h"
#include "io/IoBridge.h"
#include "charset/CodeFactory.h" ////
#include "charset/Unicode.h"

#include "doc/EditDoc.h"
#include "doc/logic/DocLineMgr.h"
#include "window/EditWnd.h"
#include "util/string_ex.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �Z�[�u���`�F�b�N                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// DocLineMgr���ێ�����f�[�^�ɈقȂ���s�R�[�h�����݂��Ă��邩�ǂ������肷��
static bool _CheckSavingEolcode(
	const DocLineMgr& pcDocLineMgr,
	Eol eolType
	)
{
	bool bMix = false;
	if (eolType == EolType::None) {	// ���s�R�[�h�ϊ��Ȃ�
		Eol eolCheck;	// ��r�Ώ�EOL
		const DocLine* pDocLine = pcDocLineMgr.GetDocLineTop();
		if (pDocLine) {
			eolCheck = pDocLine->GetEol();
		}
		while (pDocLine) {
			Eol eol = pDocLine->GetEol();
			if (eol != eolCheck && eol != EolType::None) {
				bMix = true;
				break;
			}
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return bMix;
}

// DocLineMgr���ێ�����f�[�^���w�蕶���R�[�h�ň��S�ɕۑ��ł��邩�ǂ������肷��
static CodeConvertResult _CheckSavingCharcode(
	const DocLineMgr& pcDocLineMgr,
	EncodingType eCodeType,
	LogicPoint& point,
	NativeW& wc
	)
{
	const DocLine* pDocLine = pcDocLineMgr.GetDocLineTop();
	const bool bCodePageMode = IsValidCodeOrCPType(eCodeType) && !IsValidCodeType(eCodeType);
	CodeBase* pCodeBase = CodeFactory::CreateCodeBase(eCodeType, 0);
	Memory memTmp;	// �o�b�t�@���ė��p
	NativeW memTmp2;
	LogicInt nLine = LogicInt(0);
	while (pDocLine) {
		// �R�[�h�ϊ� pDocLine -> memTmp
		CodeConvertResult e = IoBridge::ImplToFile(
			pDocLine->_GetDocLineDataWithEOL(),
			&memTmp,
			pCodeBase
		);
		if (bCodePageMode) {
			// �R�[�h�y�[�W��CodeConvertResult::LoseSome��Ԃ��Ȃ��̂ŁA�����ŕ������r����
			CodeConvertResult e2 = IoBridge::FileToImpl(
				memTmp,
				&memTmp2,
				pCodeBase,
				0
			);
			const int nDocLineLen = (Int)pDocLine->GetLengthWithEOL();
			const int nConvertLen = (Int)memTmp2.GetStringLength();
			const int nDataMinLen = t_min(nDocLineLen, nConvertLen);
			const wchar_t* p = pDocLine->GetPtr();
			const wchar_t* r = memTmp2.GetStringPtr();
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
				const WCHAR* pLine = pDocLine->GetPtr();
				const LogicInt nLineLen = pDocLine->GetLengthWithEOL();
				LogicInt chars = NativeW::GetSizeOfChar( pLine, nLineLen, 0 );
				LogicInt nPos = LogicInt(0);
				NativeW mem;
				while (0 < chars) {
					mem.SetStringHoldBuffer( pLine + nPos, chars );
					CodeConvertResult e2 = IoBridge::ImplToFile(
						mem,
						&memTmp,
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
		pDocLine = pDocLine->GetNextLine();
		++nLine;
	}
	delete pCodeBase;
	return CodeConvertResult::Complete;
}


CallbackResultType CodeChecker::OnCheckSave(SaveInfo* pSaveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// ���s�R�[�h�����݂��Ă��邩�ǂ�������
	bool bTmpResult = false;
	if (pDoc->docType.GetDocumentAttribute().bChkEnterAtEnd) {
		bTmpResult = _CheckSavingEolcode(
			pDoc->docLineMgr, pSaveInfo->eol
		);
	}

	// ���[�U�₢���킹
	if (bTmpResult) {
		int nDlgResult = MYMESSAGEBOX(
			EditWnd::getInstance().GetHwnd(),
			MB_YESNOCANCEL | MB_ICONWARNING,
			GSTR_APPNAME,
			LS(STR_CODECHECKER_EOL_UNIFY),
			pDoc->docEditor.GetNewLineCode().GetName()
		);
		switch (nDlgResult) {
		case IDYES:		pSaveInfo->eol = pDoc->docEditor.GetNewLineCode(); break; // ����
		case IDNO:		break; // ���s
		case IDCANCEL:	return CallbackResultType::Interrupt; // ���f
		}
	}

	// �w�蕶���R�[�h�ň��S�ɕۑ��ł��邩�ǂ�������
	LogicPoint point;
	NativeW memChar(L"", 0);
	CodeConvertResult nTmpResult = _CheckSavingCharcode(
		pDoc->docLineMgr, pSaveInfo->eCharCode,
		point, memChar
	);

	// ���[�U�₢���킹
	if (nTmpResult == CodeConvertResult::LoseSome) {
		TCHAR szCpName[100];
		TCHAR szLineNum[60];  // 123��
		TCHAR szCharCode[12]; // U+12ab or 1234abcd
		CodePage::GetNameNormal(szCpName, pSaveInfo->eCharCode);
		_tcscpy( szCharCode, _T("") );
		_tcscpy( szLineNum, _T("") );
		if (point.x == -1) {
			memChar.SetString(LSW(STR_ERR_CSHAREDATA22));
		}else {
			auto_sprintf( szLineNum, _T("%d"), (int)((Int)point.x) + 1 );
			_tcscat( szLineNum, LS(STR_DLGFNCLST_LIST_COL) );
			Unicode().UnicodeToHex( memChar.GetStringPtr(), memChar.GetStringLength(),
				szCharCode, &GetDllShareData().common.statusBar );
		}
		int nDlgResult = MYMESSAGEBOX(
			EditWnd::getInstance().GetHwnd(),
			MB_YESNOCANCEL | MB_ICONWARNING,
			GSTR_APPNAME,
			LS(STR_CODECHECKER_CONFORM_LOSESOME),
			szCpName,
			(int)((Int)point.y + 1),
			szLineNum,
			memChar.GetStringPtr(),
			szCharCode
		);
		switch (nDlgResult) {
		case IDYES:		break; // ���s
		case IDNO:		return CallbackResultType::Interrupt; // ���f
		case IDCANCEL:
			{
				LogicPoint pt(point.x < 0 ? LogicInt(0) : point.x, point.y);
				pDoc->pEditWnd->GetActiveView().GetCommander().Command_MoveCursor(pt, 0);
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
		ErrorMessage(EditWnd::getInstance().GetHwnd(), LS(STR_CODECHECKER_LOSESOME_SAVE));
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ���[�h���`�F�b�N                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CodeChecker::OnFinalLoad(LoadResultType eLoadResult)
{
	if (eLoadResult == LoadResultType::LoseSome) {
		ErrorMessage(
			EditWnd::getInstance().GetHwnd(),
			LS(STR_CODECHECKER_LOSESOME_ROAD)
		);
	}
}

