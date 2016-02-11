/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include <io.h>	// access
#include "CReadManager.h"
#include "CEditApp.h"	// AppExitException
#include "window/CEditWnd.h"
#include "charset/CCodeMediator.h"
#include "io/CFileLoad.h"
#include "util/window.h"

/*!
	ファイルを読み込んで格納する（分割読み込みテスト版）
	@version	2.0
	@note	Windows用にコーディングしてある
	@retval	TRUE	正常読み込み
	@retval	FALSE	エラー(またはユーザによるキャンセル?)
	@date	2002/08/30 Moca 旧ReadFileを元に作成 ファイルアクセスに関する部分をFileLoadで行う
	@date	2003/07/26 ryoji BOMの状態の取得を追加
*/
CodeConvertResult ReadManager::ReadFile_To_CDocLineMgr(
	DocLineMgr*		pcDocLineMgr,	// [out]
	const LoadInfo&	sLoadInfo,		// [in]
	FileInfo*			pFileInfo		// [out]
	)
{
	LPCTSTR pszPath = sLoadInfo.cFilePath.c_str();

	// 文字コード種別
	const TypeConfigMini* type;
	DocTypeManager().GetTypeConfigMini( sLoadInfo.nType, &type );
	ECodeType eCharCode = sLoadInfo.eCharCode;
	if (eCharCode == CODE_AUTODETECT) {
		CodeMediator cmediator( type->m_encoding );
		eCharCode = cmediator.CheckKanjiCodeOfFile( pszPath );
	}
	if (!IsValidCodeOrCPType(eCharCode)) {
		eCharCode = type->m_encoding.m_eDefaultCodetype;	// 2011.01.24 ryoji デフォルト文字コード
	}
	bool bBom;
	if (eCharCode == type->m_encoding.m_eDefaultCodetype) {
		bBom = type->m_encoding.m_bDefaultBom;	// 2011.01.24 ryoji デフォルトBOM
	}else {
		bBom = CodeTypeName( eCharCode ).IsBomDefOn();
	}
	pFileInfo->SetCodeSet( eCharCode, bBom );

	// 既存データのクリア
	pcDocLineMgr->DeleteAllLine();

	// 処理中のユーザー操作を可能にする
	if (!::BlockingHook(NULL)) {
		return CodeConvertResult::Failure; //######INTERRUPT
	}

	CodeConvertResult eRet = CodeConvertResult::Complete;

	try {
		FileLoad cfl(type->m_encoding);

		bool bBigFile;
#ifdef _WIN64
		bBigFile = true;
#else
		bBigFile = false;
#endif
		// ファイルを開く
		// ファイルを閉じるにはFileCloseメンバ又はデストラクタのどちらかで処理できます
		//	Jul. 28, 2003 ryoji BOMパラメータ追加
		cfl.FileOpen( pszPath, bBigFile, eCharCode, GetDllShareData().m_common.m_sFile.GetAutoMIMEdecode(), &bBom );
		pFileInfo->SetBomExist( bBom );

		// ファイル時刻の取得
		FILETIME fileTime;
		if (cfl.GetFileTime(NULL, NULL, &fileTime)) {
			pFileInfo->SetFileTime( fileTime );
		}

		// ReadLineはファイルから 文字コード変換された1行を読み出します
		// エラー時はthrow Error_FileRead を投げます
		int				nLineNum = 0;
		Eol			cEol;
		NativeW		cUnicodeBuffer;
		CodeConvertResult	eRead;
		while ((eRead = cfl.ReadLine( &cUnicodeBuffer, &cEol )) != CodeConvertResult::Failure) {
			if (eRead == CodeConvertResult::LoseSome) {
				eRet = CodeConvertResult::LoseSome;
			}
			const wchar_t* pLine = cUnicodeBuffer.GetStringPtr();
			int nLineLen = cUnicodeBuffer.GetStringLength();
			++nLineNum;
			DocEditAgent(pcDocLineMgr).AddLineStrX( pLine, nLineLen );
			// 経過通知
			if (nLineNum % 512 == 0) {
				NotifyProgress(cfl.GetPercent());
				// 処理中のユーザー操作を可能にする
				if (!::BlockingHook( NULL )) {
					throw AppExitException(); // 中断検出
				}
			}
		}

		// ファイルをクローズする
		cfl.FileClose();
	}catch (AppExitException) {
		// WM_QUITが発生した
		return CodeConvertResult::Failure;
	}catch (Error_FileOpen) {
		eRet = CodeConvertResult::Failure;
		if (!fexist(pszPath)) {
			// ファイルがない
			ErrorMessage(
				EditWnd::getInstance()->GetHwnd(),
				LS(STR_ERR_DLGDOCLM1),	// Mar. 24, 2001 jepro 若干修正
				pszPath
			);
		}else if (_taccess(pszPath, 4) == -1) {
			// 読み込みアクセス権がない
			ErrorMessage(
				EditWnd::getInstance()->GetHwnd(),
				LS(STR_ERR_DLGDOCLM2),
				pszPath
			 );
		}else {
			ErrorMessage(
				EditWnd::getInstance()->GetHwnd(),
				LS(STR_ERR_DLGDOCLM3),
				pszPath
			 );
		}
	}catch (Error_FileRead) {
		eRet = CodeConvertResult::Failure;
		ErrorMessage(
			EditWnd::getInstance()->GetHwnd(),
			LS(STR_ERR_DLGDOCLM4),
			pszPath
		 );
		// 既存データのクリア
		pcDocLineMgr->DeleteAllLine();
	} // 例外処理終わり

	NotifyProgress(0);
	// 処理中のユーザー操作を可能にする
	if (!::BlockingHook(NULL)) {
		return CodeConvertResult::Failure; //####INTERRUPT
	}

	// 行変更状態をすべてリセット
//	CModifyVisitor().ResetAllModifyFlag(pcDocLineMgr, 0);
	return eRet;
}

