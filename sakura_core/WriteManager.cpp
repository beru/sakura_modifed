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


/*! �o�b�t�@���e���t�@�C���ɏ����o�� (�e�X�g�p)

	@note Windows�p�ɃR�[�f�B���O���Ă���
*/
CodeConvertResult WriteManager::WriteFile_From_CDocLineMgr(
	const DocLineMgr&	pcDocLineMgr,	// [in]
	const SaveInfo&	saveInfo		// [in]
	)
{
	CodeConvertResult nRetVal = CodeConvertResult::Complete;
	std::unique_ptr<CodeBase> pcCodeBase(CodeFactory::CreateCodeBase(saveInfo.eCharCode, 0));

	{
		// �ϊ��e�X�g
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
		// �t�@�C���I�[�v��
		BinaryOutputStream out(saveInfo.filePath, true);

		// �e�s�o��
		size_t nLineNumber = 0;
		const DocLine*	pDocLine = pcDocLineMgr.GetDocLineTop();
		// 1�s��
		{
			++nLineNumber;
			Memory memOutputBuffer;
			{
				NativeW strSrc;
				Memory strBomCheck;
				pcCodeBase->GetBom(&strBomCheck);
				if (saveInfo.bBomExist && 0 < strBomCheck.GetRawLength()) {
					// 1�s�ڂɂ�BOM��t������B�G���R�[�_��bom������ꍇ�̂ݕt������B
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

			// �o�ߒʒm
			if (pcDocLineMgr.GetLineCount() > 0 && nLineNumber%1024 == 0) {
				NotifyProgress(nLineNumber * 100 / pcDocLineMgr.GetLineCount());
				// �������̃��[�U�[������\�ɂ���
				if (!::BlockingHook(NULL)) {
					throw AppExitException(); // ���f���o
				}
			}

			// 1�s�o�� -> memOutputBuffer
			Memory memOutputBuffer;
			{
				// �������ݎ��̃R�[�h�ϊ� strSrc -> memOutputBuffer
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

			// �t�@�C���ɏo�� memOutputBuffer -> fp
			out.Write(memOutputBuffer.GetRawPtr(), memOutputBuffer.GetRawLength());

			// ���̍s��
			pDocLine = pDocLine->GetNextLine();
		}

		// �t�@�C���N���[�Y
		out.Close();
	}catch (Error_FileOpen) { //########### �����_�ł́A���̗�O�����������ꍇ�͐���ɓ���ł��Ȃ�
		ErrorMessage(
			EditWnd::getInstance().GetHwnd(),
			LS(STR_SAVEAGENT_OTHER_APP),
			saveInfo.filePath.c_str()
		);
		nRetVal = CodeConvertResult::Failure;
	}catch (Error_FileWrite) {
		nRetVal = CodeConvertResult::Failure;
	}catch (AppExitException) {
		// ���f���o
		return CodeConvertResult::Failure;
	}
	return nRetVal;
}

