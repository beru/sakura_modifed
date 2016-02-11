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
//                     セーブ時チェック                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! DocLineMgrが保持するデータに異なる改行コードが混在しているかどうか判定する
static bool _CheckSavingEolcode(
	const DocLineMgr& pcDocLineMgr,
	Eol cEolType
	)
{
	bool bMix = false;
	if (cEolType == EOL_NONE) {	// 改行コード変換なし
		Eol cEolCheck;	// 比較対象EOL
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

//! DocLineMgrが保持するデータを指定文字コードで安全に保存できるかどうか判定する
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
	Memory cmemTmp;	// バッファを再利用
	NativeW cmemTmp2;
	LogicInt nLine = LogicInt(0);
	while (pcDocLine) {
		// コード変換 pcDocLine -> cmemTmp
		CodeConvertResult e = IoBridge::ImplToFile(
			pcDocLine->_GetDocLineDataWithEOL(),
			&cmemTmp,
			pCodeBase
		);
		if (bCodePageMode) {
			// コードページはCodeConvertResult::LoseSomeを返さないので、自分で文字列比較する
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
				// 変換できなかった位置の1文字取得
				wc.SetString( p + nPos, (Int)NativeW::GetSizeOfChar( p, nDocLineLen, nPos ) );
				delete pCodeBase;
				return CodeConvertResult::LoseSome;
			}
		}
		if (e != CodeConvertResult::Complete) {
			if (e == CodeConvertResult::LoseSome) {
				// 行内の位置を特定
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

		// 次の行へ
		pcDocLine = pcDocLine->GetNextLine();
		++nLine;
	}
	delete pCodeBase;
	return CodeConvertResult::Complete;
}


CallbackResultType CodeChecker::OnCheckSave(SaveInfo* pSaveInfo)
{
	EditDoc* pcDoc = GetListeningDoc();

	// 改行コードが混在しているかどうか判定
	bool bTmpResult = false;
	if (pcDoc->m_cDocType.GetDocumentAttribute().m_bChkEnterAtEnd) {
		bTmpResult = _CheckSavingEolcode(
			pcDoc->m_cDocLineMgr, pSaveInfo->cEol
		);
	}

	// ユーザ問い合わせ
	if (bTmpResult) {
		int nDlgResult = MYMESSAGEBOX(
			EditWnd::getInstance()->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONWARNING,
			GSTR_APPNAME,
			LS(STR_CODECHECKER_EOL_UNIFY),
			pcDoc->m_cDocEditor.GetNewLineCode().GetName()
		);
		switch (nDlgResult) {
		case IDYES:		pSaveInfo->cEol = pcDoc->m_cDocEditor.GetNewLineCode(); break; // 統一
		case IDNO:		break; // 続行
		case IDCANCEL:	return CallbackResultType::Interrupt; // 中断
		}
	}

	// 指定文字コードで安全に保存できるかどうか判定
	LogicPoint point;
	NativeW cmemChar(L"", 0);
	CodeConvertResult nTmpResult = _CheckSavingCharcode(
		pcDoc->m_cDocLineMgr, pSaveInfo->eCharCode,
		point, cmemChar
	);

	// ユーザ問い合わせ
	if (nTmpResult == CodeConvertResult::LoseSome) {
		TCHAR szCpName[100];
		TCHAR  szLineNum[60];  // 123桁
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
		case IDYES:		break; // 続行
		case IDNO:		return CallbackResultType::Interrupt; // 中断
		case IDCANCEL:
			{
				LogicPoint pt(point.x < 0 ? LogicInt(0) : point.x, point.y);
				pcDoc->m_pcEditWnd->GetActiveView().GetCommander().Command_MOVECURSOR(pt, 0);
			}
			return CallbackResultType::Interrupt; //中断
		}
	}
	return CallbackResultType::Continue;
}

void CodeChecker::OnFinalSave(SaveResultType eSaveResult)
{
	// カキコ結果
	if (eSaveResult == SaveResultType::LoseSome) {
		ErrorMessage(EditWnd::getInstance()->GetHwnd(), LS(STR_CODECHECKER_LOSESOME_SAVE));
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ロード時チェック                        //
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

