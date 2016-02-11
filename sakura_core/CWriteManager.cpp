#include "StdAfx.h"
#include "CWriteManager.h"
#include <list>
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"
#include "CEditApp.h" // AppExitException
#include "window/CEditWnd.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "charset/CUnicode.h"
#include "io/CIoBridge.h"
#include "io/CBinaryStream.h"
#include "util/window.h"


/*! �o�b�t�@���e���t�@�C���ɏ����o�� (�e�X�g�p)

	@note Windows�p�ɃR�[�f�B���O���Ă���
	@date 2003.07.26 ryoji BOM�����ǉ�
*/
EConvertResult WriteManager::WriteFile_From_CDocLineMgr(
	const DocLineMgr&	pcDocLineMgr,	// [in]
	const SaveInfo&	sSaveInfo		// [in]
	)
{
	EConvertResult nRetVal = RESULT_COMPLETE;
	std::unique_ptr<CodeBase> pcCodeBase(CodeFactory::CreateCodeBase(sSaveInfo.eCharCode, 0));

	{
		// �ϊ��e�X�g
		NativeW buffer = L"abcde";
		Memory tmp;
		EConvertResult e = pcCodeBase->UnicodeToCode( buffer, &tmp );
		if (e == RESULT_FAILURE) {
			nRetVal=RESULT_FAILURE;
			ErrorMessage(
				EditWnd::getInstance()->GetHwnd(),
				LS(STR_FILESAVE_CONVERT_ERROR),
				sSaveInfo.cFilePath.c_str()
			);
			return nRetVal;
		}
	}


	try {
		// �t�@�C���I�[�v��
		BinaryOutputStream out(sSaveInfo.cFilePath, true);

		// �e�s�o��
		int nLineNumber = 0;
		const DocLine*	pcDocLine = pcDocLineMgr.GetDocLineTop();
		// 1�s��
		{
			++nLineNumber;
			Memory cmemOutputBuffer;
			{
				NativeW cstrSrc;
				Memory cstrBomCheck;
				pcCodeBase->GetBom(&cstrBomCheck);
				if (sSaveInfo.bBomExist && 0 < cstrBomCheck.GetRawLength()) {
					// 1�s�ڂɂ�BOM��t������B�G���R�[�_��bom������ꍇ�̂ݕt������B
					Unicode().GetBom(cstrSrc._GetMemory());
				}
				if (pcDocLine) {
					cstrSrc.AppendNativeData(pcDocLine->_GetDocLineDataWithEOL());
				}
				EConvertResult e = pcCodeBase->UnicodeToCode(cstrSrc, &cmemOutputBuffer);
				if (e == RESULT_LOSESOME) {
					nRetVal = RESULT_LOSESOME;
				}
				if (e == RESULT_FAILURE) {
					nRetVal = RESULT_FAILURE;
					ErrorMessage(
						EditWnd::getInstance()->GetHwnd(),
						LS(STR_FILESAVE_CONVERT_ERROR),
						sSaveInfo.cFilePath.c_str()
					);
					throw Error_FileWrite();
				}
			}
			out.Write(cmemOutputBuffer.GetRawPtr(), cmemOutputBuffer.GetRawLength());
			if (pcDocLine) {
				pcDocLine = pcDocLine->GetNextLine();
			}
		}
		while (pcDocLine) {
			++nLineNumber;

			// �o�ߒʒm
			if (pcDocLineMgr.GetLineCount() > 0 && nLineNumber%1024 == 0) {
				NotifyProgress(nLineNumber * 100 / pcDocLineMgr.GetLineCount());
				// �������̃��[�U�[������\�ɂ���
				if (!::BlockingHook(NULL)) {
					throw AppExitException(); // ���f���o
				}
			}

			// 1�s�o�� -> cmemOutputBuffer
			Memory cmemOutputBuffer;
			{
				// �������ݎ��̃R�[�h�ϊ� cstrSrc -> cmemOutputBuffer
				EConvertResult e = pcCodeBase->UnicodeToCode(
					pcDocLine->_GetDocLineDataWithEOL(),
					&cmemOutputBuffer
				);
				if (e == RESULT_LOSESOME) {
					if (nRetVal == RESULT_COMPLETE) {
						nRetVal = RESULT_LOSESOME;
					}
				}
				if (e == RESULT_FAILURE) {
					nRetVal = RESULT_FAILURE;
					ErrorMessage(
						EditWnd::getInstance()->GetHwnd(),
						LS(STR_FILESAVE_CONVERT_ERROR),
						sSaveInfo.cFilePath.c_str()
					);
					break;
				}
			}

			// �t�@�C���ɏo�� cmemOutputBuffer -> fp
			out.Write(cmemOutputBuffer.GetRawPtr(), cmemOutputBuffer.GetRawLength());

			// ���̍s��
			pcDocLine = pcDocLine->GetNextLine();
		}

		// �t�@�C���N���[�Y
		out.Close();
	}catch (Error_FileOpen) { //########### �����_�ł́A���̗�O�����������ꍇ�͐���ɓ���ł��Ȃ�
		ErrorMessage(
			EditWnd::getInstance()->GetHwnd(),
			LS(STR_SAVEAGENT_OTHER_APP),
			sSaveInfo.cFilePath.c_str()
		);
		nRetVal = RESULT_FAILURE;
	}catch (Error_FileWrite) {
		nRetVal = RESULT_FAILURE;
	}catch (AppExitException) {
		// ���f���o
		return RESULT_FAILURE;
	}
	return nRetVal;
}

