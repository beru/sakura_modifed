#include "StdAfx.h"
#include "WriteManager.h"
#include <list>
#include "doc/logic/DocLineMgr.h"
#include "doc/logic/DocLine.h"
#include "EditApp.h" // AppExitException
#include "window/EditWnd.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "charset/Unicode.h"
#include "io/IoBridge.h"
#include "io/BinaryStream.h"
#include "util/window.h"


/*! バッファ内容をファイルに書き出す (テスト用)

	@note Windows用にコーディングしてある
*/
CodeConvertResult WriteManager::WriteFile_From_CDocLineMgr(
	const DocLineMgr&	pcDocLineMgr,	// [in]
	const SaveInfo&	saveInfo		// [in]
	)
{
	CodeConvertResult nRetVal = CodeConvertResult::Complete;
	std::unique_ptr<CodeBase> pcCodeBase(CodeFactory::CreateCodeBase(saveInfo.eCharCode, 0));

	{
		// 変換テスト
		NativeW buffer = L"abcde";
		Memory tmp;
		CodeConvertResult e = pcCodeBase->UnicodeToCode( buffer, &tmp );
		if (e == CodeConvertResult::Failure) {
			nRetVal=CodeConvertResult::Failure;
			ErrorMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_FILESAVE_CONVERT_ERROR),
				saveInfo.filePath.c_str()
			);
			return nRetVal;
		}
	}


	try {
		// ファイルオープン
		BinaryOutputStream out(saveInfo.filePath, true);

		// 各行出力
		size_t nLineNumber = 0;
		const DocLine*	pDocLine = pcDocLineMgr.GetDocLineTop();
		// 1行目
		{
			++nLineNumber;
			Memory memOutputBuffer;
			{
				NativeW strSrc;
				Memory strBomCheck;
				pcCodeBase->GetBom(&strBomCheck);
				if (saveInfo.bBomExist && 0 < strBomCheck.GetRawLength()) {
					// 1行目にはBOMを付加する。エンコーダでbomがある場合のみ付加する。
					Unicode().GetBom(strSrc._GetMemory());
				}
				if (pDocLine) {
					strSrc.AppendNativeData(pDocLine->_GetDocLineDataWithEOL());
				}
				CodeConvertResult e = pcCodeBase->UnicodeToCode(strSrc, &memOutputBuffer);
				if (e == CodeConvertResult::LoseSome) {
					nRetVal = CodeConvertResult::LoseSome;
				}
				if (e == CodeConvertResult::Failure) {
					nRetVal = CodeConvertResult::Failure;
					ErrorMessage(
						EditWnd::getInstance().GetHwnd(),
						LS(STR_FILESAVE_CONVERT_ERROR),
						saveInfo.filePath.c_str()
					);
					throw Error_FileWrite();
				}
			}
			out.Write(memOutputBuffer.GetRawPtr(), memOutputBuffer.GetRawLength());
			if (pDocLine) {
				pDocLine = pDocLine->GetNextLine();
			}
		}
		while (pDocLine) {
			++nLineNumber;

			// 経過通知
			if (pcDocLineMgr.GetLineCount() > 0 && nLineNumber%1024 == 0) {
				NotifyProgress(nLineNumber * 100 / pcDocLineMgr.GetLineCount());
				// 処理中のユーザー操作を可能にする
				if (!::BlockingHook(NULL)) {
					throw AppExitException(); // 中断検出
				}
			}

			// 1行出力 -> memOutputBuffer
			Memory memOutputBuffer;
			{
				// 書き込み時のコード変換 strSrc -> memOutputBuffer
				CodeConvertResult e = pcCodeBase->UnicodeToCode(
					pDocLine->_GetDocLineDataWithEOL(),
					&memOutputBuffer
				);
				if (e == CodeConvertResult::LoseSome) {
					if (nRetVal == CodeConvertResult::Complete) {
						nRetVal = CodeConvertResult::LoseSome;
					}
				}
				if (e == CodeConvertResult::Failure) {
					nRetVal = CodeConvertResult::Failure;
					ErrorMessage(
						EditWnd::getInstance().GetHwnd(),
						LS(STR_FILESAVE_CONVERT_ERROR),
						saveInfo.filePath.c_str()
					);
					break;
				}
			}

			// ファイルに出力 memOutputBuffer -> fp
			out.Write(memOutputBuffer.GetRawPtr(), memOutputBuffer.GetRawLength());

			// 次の行へ
			pDocLine = pDocLine->GetNextLine();
		}

		// ファイルクローズ
		out.Close();
	}catch (Error_FileOpen) { //########### 現時点では、この例外が発生した場合は正常に動作できない
		ErrorMessage(
			EditWnd::getInstance().GetHwnd(),
			LS(STR_SAVEAGENT_OTHER_APP),
			saveInfo.filePath.c_str()
		);
		nRetVal = CodeConvertResult::Failure;
	}catch (Error_FileWrite) {
		nRetVal = CodeConvertResult::Failure;
	}catch (AppExitException) {
		// 中断検出
		return CodeConvertResult::Failure;
	}
	return nRetVal;
}

