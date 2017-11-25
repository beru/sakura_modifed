#include "StdAfx.h"
#include <io.h>	// access
#include "ReadManager.h"
#include "EditApp.h"	// AppExitException
#include "window/EditWnd.h"
#include "charset/CodeMediator.h"
#include "io/FileLoad.h"
#include "util/window.h"

/*!
	ファイルを読み込んで格納する（分割読み込みテスト版）
	@version	2.0
	@note	Windows用にコーディングしてある
	@retval	TRUE	正常読み込み
	@retval	FALSE	エラー(またはユーザによるキャンセル?)
*/
CodeConvertResult ReadManager::ReadFile_To_CDocLineMgr(
	DocLineMgr&			docLineMgr,		// [out]
	const LoadInfo&		loadInfo,		// [in]
	FileInfo*			pFileInfo		// [out]
	)
{
	LPCTSTR pszPath = loadInfo.filePath.c_str();

	// 文字コード種別
	const TypeConfigMini* type;
	DocTypeManager().GetTypeConfigMini( loadInfo.nType, &type );
	EncodingType eCharCode = loadInfo.eCharCode;
	if (eCharCode == CODE_AUTODETECT) {
		CodeMediator mediator( type->encoding );
		eCharCode = mediator.CheckKanjiCodeOfFile( pszPath );
	}
	if (!IsValidCodeOrCPType(eCharCode)) {
		eCharCode = type->encoding.eDefaultCodetype;	// デフォルト文字コード
	}
	bool bBom;
	if (eCharCode == type->encoding.eDefaultCodetype) {
		bBom = type->encoding.bDefaultBom;	// デフォルトBOM
	}else {
		bBom = CodeTypeName( eCharCode ).IsBomDefOn();
	}
	pFileInfo->SetCodeSet( eCharCode, bBom );

	// 既存データのクリア
	docLineMgr.DeleteAllLine();

	// 処理中のユーザー操作を可能にする
	if (!::BlockingHook(NULL)) {
		return CodeConvertResult::Failure; //######INTERRUPT
	}

	CodeConvertResult eRet = CodeConvertResult::Complete;

	try {
		FileLoad fl;

		bool bBigFile;
#ifdef _WIN64
		bBigFile = true;
#else
		bBigFile = false;
#endif
		// ファイルを開く
		// ファイルを閉じるにはFileCloseメンバ又はデストラクタのどちらかで処理できます
		fl.FileOpen(type->encoding, pszPath, bBigFile, eCharCode, GetDllShareData().common.file.GetAutoMIMEdecode(), &bBom );
		pFileInfo->SetBomExist( bBom );

		// ファイル時刻の取得
		FILETIME fileTime;
		if (fl.GetFileTime(NULL, NULL, &fileTime)) {
			pFileInfo->SetFileTime( fileTime );
		}

		// ReadLineはファイルから 文字コード変換された1行を読み出します
		// エラー時はthrow Error_FileRead を投げます
		int			nLineNum = 0;
		Eol			eol;
		NativeW		unicodeBuffer;
		CodeConvertResult	eRead;
		while ((eRead = fl.ReadLine( &unicodeBuffer, &eol )) != CodeConvertResult::Failure) {
			if (eRead == CodeConvertResult::LoseSome) {
				eRet = CodeConvertResult::LoseSome;
			}
			const wchar_t* pLine = unicodeBuffer.GetStringPtr();
			size_t nLineLen = unicodeBuffer.GetStringLength();
			++nLineNum;
			DocEditAgent(docLineMgr).AddLineStrX( pLine, nLineLen );
			// 経過通知
			if (nLineNum % 512 == 0) {
				NotifyProgress(fl.GetPercent());
				// 処理中のユーザー操作を可能にする
				if (!::BlockingHook( NULL )) {
					throw AppExitException(); // 中断検出
				}
			}
		}

		// ファイルをクローズする
		fl.FileClose();
	}catch (AppExitException) {
		// WM_QUITが発生した
		return CodeConvertResult::Failure;
	}catch (Error_FileOpen) {
		eRet = CodeConvertResult::Failure;
		if (!fexist(pszPath)) {
			// ファイルがない
			ErrorMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_ERR_DLGDOCLM1),
				pszPath
			);
		}else if (_taccess(pszPath, 4) == -1) {
			// 読み込みアクセス権がない
			ErrorMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_ERR_DLGDOCLM2),
				pszPath
			 );
		}else {
			ErrorMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_ERR_DLGDOCLM3),
				pszPath
			 );
		}
	}catch (Error_FileRead) {
		eRet = CodeConvertResult::Failure;
		ErrorMessage(
			EditWnd::getInstance().GetHwnd(),
			LS(STR_ERR_DLGDOCLM4),
			pszPath
		 );
		// 既存データのクリア
		docLineMgr.DeleteAllLine();
	} // 例外処理終わり

	NotifyProgress(0);
	// 処理中のユーザー操作を可能にする
	if (!::BlockingHook(NULL)) {
		return CodeConvertResult::Failure; //####INTERRUPT
	}

	// 行変更状態をすべてリセット
//	ModifyVisitor().ResetAllModifyFlag(docLineMgr, 0);
	return eRet;
}

