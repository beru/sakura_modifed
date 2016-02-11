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
CodeConvertResult WriteManager::WriteFile_From_CDocLineMgr(
	const DocLineMgr&	pcDocLineMgr,	// [in]
	const SaveInfo&	sSaveInfo		// [in]
	)
{
	CodeConvertResult nRetVal = CodeConvertResult::Complete;
	std::unique_ptr<CodeBase> pcCodeBase(CodeFactory::CreateCodeBase(sSaveInfo.eCharCode, 0));

	{
		// �ϊ��e�X�g
		NativeW buffer = L"abcde";
		Memory tmp;
		CodeConvertResult e = pcCodeBase->UnicodeToCode( buffer, &tmp );
		if (e == CodeConvertResult::Failure) {
			nRetVal=CodeConvertResult::Failure;
			ErrorMessage(
				EditWnd::getInstance()->GetHwnd(),
				LS(STR_FILESAVE_CONVERT_ERROR),
				sSaveInfo.filePath.c_str()
			);
			return nRetVal;
		}
	}


	try {
		// �t�@�C���I�[�v��
		BinaryOutputStream out(sSaveInfo.filePath, true);

		// �e�s�o��
		int nLineNumber = 0;
		const DocLine*	pDocLine = pcDocLineMgr.GetDocLineTop();
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
				if (pDocLine) {
					cstrSrc.AppendNativeData(pDocLine->_GetDocLineDataWithEOL());
				}
				CodeConvertResult e = pcCodeBase->UnicodeToCode(cstrSrc, &cmemOutputBuffer);
				if (e == CodeConvertResult::LoseSome) {
					nRetVal = CodeConvertResult::LoseSome;
				}
				if (e == CodeConvertResult::Failure) {
					nRetVal = CodeConvertResult::Failure;
					ErrorMessage(
						EditWnd::getInstance()->GetHwnd(),
						LS(STR_FILESAVE_CONVERT_ERROR),
						sSaveInfo.filePath.c_str()
					);
					throw Error_FileWrite();
				}
			}
			out.Write(cmemOutputBuffer.GetRawPtr(), cmemOutputBuffer.GetRawLength());
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

			// 1�s�o�� -> cmemOutputBuffer
			Memory cmemOutputBuffer;
			{
				// �������ݎ��̃R�[�h�ϊ� cstrSrc -> cmemOutputBuffer
				CodeConvertResult e = pcCodeBase->UnicodeToCode(
					pDocLine->_GetDocLineDataWithEOL(),
					&cmemOutputBuffer
				);
				if (e == CodeConvertResult::LoseSome) {
					if (nRetVal == CodeConvertResult::Complete) {
						nRetVal = CodeConvertResult::LoseSome;
					}
				}
				if (e == CodeConvertResult::Failure) {
					nRetVal = CodeConvertResult::Failure;
					ErrorMessage(
						EditWnd::getInstance()->GetHwnd(),
						LS(STR_FILESAVE_CONVERT_ERROR),
						sSaveInfo.filePath.c_str()
					);
					break;
				}
			}

			// �t�@�C���ɏo�� cmemOutputBuffer -> fp
			out.Write(cmemOutputBuffer.GetRawPtr(), cmemOutputBuffer.GetRawLength());

			// ���̍s��
			pDocLine = pDocLine->GetNextLine();
		}

		// �t�@�C���N���[�Y
		out.Close();
	}catch (Error_FileOpen) { //########### �����_�ł́A���̗�O�����������ꍇ�͐���ɓ���ł��Ȃ�
		ErrorMessage(
			EditWnd::getInstance()->GetHwnd(),
			LS(STR_SAVEAGENT_OTHER_APP),
			sSaveInfo.filePath.c_str()
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

