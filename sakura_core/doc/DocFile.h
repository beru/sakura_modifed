#pragma once

#include "io/File.h"
#include "util/fileUtil.h"
class EditDoc;

//####本来はここにあるべきでは無い
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
	void	SetCodeSet(EncodingType eSet, bool bBom)	{ eCharCode = eCharCodeLoad = eSet; bBomExist = bBomExistLoad = bBom; }	// 文字コードセットを設定
	void	SetBomExist(bool bBom)						{ bBomExist = bBomExistLoad = bBom; }	// BOM付加を設定
	void	SetFileTime(FILETIME& Time)					{ fileTime.SetFILETIME(Time); }
};

class DocFile : public File {
public:
	DocFile(EditDoc& doc) : doc(doc) {}

	void			SetCodeSet(EncodingType eCodeSet, bool bBomExist)		{ fileInfo.SetCodeSet(eCodeSet, bBomExist); }	// 文字コードセットを設定
	void			SetCodeSetChg(EncodingType eCodeSet, bool bBomExist)	{ fileInfo.eCharCode = eCodeSet; fileInfo.bBomExist = bBomExist; }	// 文字コードセットを設定(文字コード指定用)
	EncodingType	GetCodeSet() const			{ return fileInfo.eCharCode; }		// 文字コードセットを取得
	void			SetBomDefoult()				{ fileInfo.bBomExist= CodeTypeName(fileInfo.eCharCode).IsBomDefOn(); }	// BOM付加のデフォルト値を設定する
	void			CancelChgCodeSet()			{ fileInfo.eCharCode = fileInfo.eCharCodeLoad; fileInfo.bBomExist = fileInfo.bBomExistLoad; }		// 文字コードセット1の変更をキャンセルする
	bool			IsBomExist() const			{ return fileInfo.bBomExist; }		// 保存時にBOMを付加するかどうかを取得
	bool			IsChgCodeSet() const		{ return (!IsFileTimeZero()) && ((fileInfo.eCharCode != fileInfo.eCharCodeLoad) || (fileInfo.bBomExist != fileInfo.bBomExistLoad)); }		// 文字コードセットが変更されたか？

	FileTime&		GetFileTime()					{ return fileInfo.fileTime; }
	void			ClearFileTime()					{ fileInfo.fileTime.ClearFILETIME(); }
	bool			IsFileTimeZero() const			{ return fileInfo.fileTime.IsZero(); }	// 新規ファイル？
	const SYSTEMTIME	GetFileSysTime() const		{ return fileInfo.fileTime.GetSYSTEMTIME(); }
	void			SetFileTime(FILETIME& Time)		{ fileInfo.fileTime.SetFILETIME(Time); }

	const TCHAR*	GetFileName() const					{ return GetFileTitlePointer(GetFilePath()); }	// ファイル名(パスなし)を取得
	const TCHAR*	GetSaveFilePath(void) const;
	void			SetSaveFilePath(LPCTSTR pszPath)	{ szSaveFilePath.Assign(pszPath); }
public: //####
	EditDoc&	doc;
	FileInfo	fileInfo;
	FilePath	szSaveFilePath;	// 保存時のファイルのパス（マクロ用）	// 2006.09.04 ryoji
};

