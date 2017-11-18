#pragma once

#include "doc/DocListener.h" // LoadInfo
#include "Eol.h"

class EditDoc;

class DocFileOperation {
public:
	DocFileOperation(EditDoc& doc) : doc(doc) { }

	// ���b�N
	bool _ToDoLock() const;
	void DoFileLock(bool bMsg = true);
	void DoFileUnlock();
	
	// ���[�hUI
	bool OpenFileDialog(
		HWND				hwndParent,
		const TCHAR*		pszOpenFolder,	// [in]  NULL�ȊO���w�肷��Ə����t�H���_���w��ł���
		LoadInfo*			pLoadInfo,		// [in/out] ���[�h���
		std::vector<std::tstring>&	files
	);

	// ���[�h�t���[
	bool DoLoadFlow(LoadInfo* pLoadInfo);
	bool FileLoad(
		LoadInfo*	pLoadInfo			// [in/out]
	);
	bool FileLoadWithoutAutoMacro(
		LoadInfo*	pLoadInfo			// [in/out]
	);
	void ReloadCurrentFile(				// ����t�@�C���̍ăI�[�v�� Jul. 26, 2003 ryoji BOM�I�v�V�����ǉ�
		EncodingType	nCharCode		// [in] �����R�[�h���
	);

	
	// �Z�[�uUI
	bool SaveFileDialog(SaveInfo* pSaveInfo);	//�u�t�@�C������t���ĕۑ��v�_�C�A���O
	bool SaveFileDialog(LPTSTR szPath);			//�u�t�@�C������t���ĕۑ��v�_�C�A���O

	// �Z�[�u�t���[
	bool DoSaveFlow(SaveInfo* pSaveInfo);
	bool FileSaveAs(
		const wchar_t*	filename = NULL,
		EncodingType	eCodeType = CODE_NONE,
		EolType			eEolType = EolType::None,
		bool			bDialog = true
	);	// �_�C�A���O�Ńt�@�C��������͂����A�ۑ��B	// 2006.12.30 ryoji
	bool FileSave();			// �㏑���ۑ��B�t�@�C�������w�肳��Ă��Ȃ�������_�C�A���O�œ��͂𑣂��B	// 2006.12.30 ryoji

	// �N���[�Y
	bool FileClose();			// ����(����)	// 2006.12.30 ryoji

	// ���̑�
	void FileCloseOpen(				// ���ĊJ��	// 2006.12.30 ryoji
		const LoadInfo& loadInfo = LoadInfo(_T(""), CODE_AUTODETECT, false)
	);

private:
	EditDoc& doc;
};

