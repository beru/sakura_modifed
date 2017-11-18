#pragma once

#include "io/File.h"
#include "util/fileUtil.h"
class EditDoc;

//####�{���͂����ɂ���ׂ��ł͖���
struct FileInfo {
	friend class DocFile;
protected:
	EncodingType	eCharCode;
	bool			bBomExist;
	EncodingType	eCharCodeLoad;
	bool			bBomExistLoad;
	FileTime		fileTime;

public:
	FileInfo() {
		eCharCode = eCharCodeLoad = CODE_DEFAULT;
		bBomExist = bBomExistLoad = false;
		fileTime.ClearFILETIME();
	}
	void	SetCodeSet(EncodingType eSet, bool bBom)	{ eCharCode = eCharCodeLoad = eSet; bBomExist = bBomExistLoad = bBom; }	// �����R�[�h�Z�b�g��ݒ�
	void	SetBomExist(bool bBom)						{ bBomExist = bBomExistLoad = bBom; }	// BOM�t����ݒ�
	void	SetFileTime(FILETIME& Time)					{ fileTime.SetFILETIME(Time); }
};

class DocFile : public File {
public:
	DocFile(EditDoc& doc) : doc(doc) {}

	void			SetCodeSet(EncodingType eCodeSet, bool bBomExist)		{ fileInfo.SetCodeSet(eCodeSet, bBomExist); }	// �����R�[�h�Z�b�g��ݒ�
	void			SetCodeSetChg(EncodingType eCodeSet, bool bBomExist)	{ fileInfo.eCharCode = eCodeSet; fileInfo.bBomExist = bBomExist; }	// �����R�[�h�Z�b�g��ݒ�(�����R�[�h�w��p)
	EncodingType	GetCodeSet() const			{ return fileInfo.eCharCode; }		// �����R�[�h�Z�b�g���擾
	void			SetBomDefoult()				{ fileInfo.bBomExist= CodeTypeName(fileInfo.eCharCode).IsBomDefOn(); }	// BOM�t���̃f�t�H���g�l��ݒ肷��
	void			CancelChgCodeSet()			{ fileInfo.eCharCode = fileInfo.eCharCodeLoad; fileInfo.bBomExist = fileInfo.bBomExistLoad; }		// �����R�[�h�Z�b�g1�̕ύX���L�����Z������
	bool			IsBomExist() const			{ return fileInfo.bBomExist; }		// �ۑ�����BOM��t�����邩�ǂ������擾
	bool			IsChgCodeSet() const		{ return (!IsFileTimeZero()) && ((fileInfo.eCharCode != fileInfo.eCharCodeLoad) || (fileInfo.bBomExist != fileInfo.bBomExistLoad)); }		// �����R�[�h�Z�b�g���ύX���ꂽ���H

	FileTime&		GetFileTime()					{ return fileInfo.fileTime; }
	void			ClearFileTime()					{ fileInfo.fileTime.ClearFILETIME(); }
	bool			IsFileTimeZero() const			{ return fileInfo.fileTime.IsZero(); }	// �V�K�t�@�C���H
	const SYSTEMTIME	GetFileSysTime() const		{ return fileInfo.fileTime.GetSYSTEMTIME(); }
	void			SetFileTime(FILETIME& Time)		{ fileInfo.fileTime.SetFILETIME(Time); }

	const TCHAR*	GetFileName() const					{ return GetFileTitlePointer(GetFilePath()); }	// �t�@�C����(�p�X�Ȃ�)���擾
	const TCHAR*	GetSaveFilePath(void) const;
	void			SetSaveFilePath(LPCTSTR pszPath)	{ szSaveFilePath.Assign(pszPath); }
public: //####
	EditDoc&	doc;
	FileInfo	fileInfo;
	FilePath	szSaveFilePath;	// �ۑ����̃t�@�C���̃p�X�i�}�N���p�j	// 2006.09.04 ryoji
};

