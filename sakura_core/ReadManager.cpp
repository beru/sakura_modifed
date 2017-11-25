#include "StdAfx.h"
#include <io.h>	// access
#include "ReadManager.h"
#include "EditApp.h"	// AppExitException
#include "window/EditWnd.h"
#include "charset/CodeMediator.h"
#include "io/FileLoad.h"
#include "util/window.h"

/*!
	�t�@�C����ǂݍ���Ŋi�[����i�����ǂݍ��݃e�X�g�Łj
	@version	2.0
	@note	Windows�p�ɃR�[�f�B���O���Ă���
	@retval	TRUE	����ǂݍ���
	@retval	FALSE	�G���[(�܂��̓��[�U�ɂ��L�����Z��?)
*/
CodeConvertResult ReadManager::ReadFile_To_CDocLineMgr(
	DocLineMgr&			docLineMgr,		// [out]
	const LoadInfo&		loadInfo,		// [in]
	FileInfo*			pFileInfo		// [out]
	)
{
	LPCTSTR pszPath = loadInfo.filePath.c_str();

	// �����R�[�h���
	const TypeConfigMini* type;
	DocTypeManager().GetTypeConfigMini( loadInfo.nType, &type );
	EncodingType eCharCode = loadInfo.eCharCode;
	if (eCharCode == CODE_AUTODETECT) {
		CodeMediator mediator( type->encoding );
		eCharCode = mediator.CheckKanjiCodeOfFile( pszPath );
	}
	if (!IsValidCodeOrCPType(eCharCode)) {
		eCharCode = type->encoding.eDefaultCodetype;	// �f�t�H���g�����R�[�h
	}
	bool bBom;
	if (eCharCode == type->encoding.eDefaultCodetype) {
		bBom = type->encoding.bDefaultBom;	// �f�t�H���gBOM
	}else {
		bBom = CodeTypeName( eCharCode ).IsBomDefOn();
	}
	pFileInfo->SetCodeSet( eCharCode, bBom );

	// �����f�[�^�̃N���A
	docLineMgr.DeleteAllLine();

	// �������̃��[�U�[������\�ɂ���
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
		// �t�@�C�����J��
		// �t�@�C�������ɂ�FileClose�����o���̓f�X�g���N�^�̂ǂ��炩�ŏ����ł��܂�
		fl.FileOpen(type->encoding, pszPath, bBigFile, eCharCode, GetDllShareData().common.file.GetAutoMIMEdecode(), &bBom );
		pFileInfo->SetBomExist( bBom );

		// �t�@�C�������̎擾
		FILETIME fileTime;
		if (fl.GetFileTime(NULL, NULL, &fileTime)) {
			pFileInfo->SetFileTime( fileTime );
		}

		// ReadLine�̓t�@�C������ �����R�[�h�ϊ����ꂽ1�s��ǂݏo���܂�
		// �G���[����throw Error_FileRead �𓊂��܂�
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
			// �o�ߒʒm
			if (nLineNum % 512 == 0) {
				NotifyProgress(fl.GetPercent());
				// �������̃��[�U�[������\�ɂ���
				if (!::BlockingHook( NULL )) {
					throw AppExitException(); // ���f���o
				}
			}
		}

		// �t�@�C�����N���[�Y����
		fl.FileClose();
	}catch (AppExitException) {
		// WM_QUIT����������
		return CodeConvertResult::Failure;
	}catch (Error_FileOpen) {
		eRet = CodeConvertResult::Failure;
		if (!fexist(pszPath)) {
			// �t�@�C�����Ȃ�
			ErrorMessage(
				EditWnd::getInstance().GetHwnd(),
				LS(STR_ERR_DLGDOCLM1),
				pszPath
			);
		}else if (_taccess(pszPath, 4) == -1) {
			// �ǂݍ��݃A�N�Z�X�����Ȃ�
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
		// �����f�[�^�̃N���A
		docLineMgr.DeleteAllLine();
	} // ��O�����I���

	NotifyProgress(0);
	// �������̃��[�U�[������\�ɂ���
	if (!::BlockingHook(NULL)) {
		return CodeConvertResult::Failure; //####INTERRUPT
	}

	// �s�ύX��Ԃ����ׂă��Z�b�g
//	ModifyVisitor().ResetAllModifyFlag(docLineMgr, 0);
	return eRet;
}

