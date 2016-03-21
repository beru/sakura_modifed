/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Uchi

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
	DocFile(EditDoc& doc) : m_doc(doc) {}

	void			SetCodeSet(EncodingType eCodeSet, bool bBomExist)		{ m_fileInfo.SetCodeSet(eCodeSet, bBomExist); }	// 文字コードセットを設定
	void			SetCodeSetChg(EncodingType eCodeSet, bool bBomExist)	{ m_fileInfo.eCharCode = eCodeSet; m_fileInfo.bBomExist = bBomExist; }	// 文字コードセットを設定(文字コード指定用)
	EncodingType	GetCodeSet() const			{ return m_fileInfo.eCharCode; }		// 文字コードセットを取得
	void			SetBomDefoult()				{ m_fileInfo.bBomExist= CodeTypeName(m_fileInfo.eCharCode).IsBomDefOn(); }	// BOM付加のデフォルト値を設定する
	void			CancelChgCodeSet()			{ m_fileInfo.eCharCode = m_fileInfo.eCharCodeLoad; m_fileInfo.bBomExist = m_fileInfo.bBomExistLoad; }		// 文字コードセット1の変更をキャンセルする
	bool			IsBomExist() const			{ return m_fileInfo.bBomExist; }		// 保存時にBOMを付加するかどうかを取得
	bool			IsChgCodeSet() const		{ return (!IsFileTimeZero()) && ((m_fileInfo.eCharCode != m_fileInfo.eCharCodeLoad) || (m_fileInfo.bBomExist != m_fileInfo.bBomExistLoad)); }		// 文字コードセットが変更されたか？

	FileTime&		GetFileTime()					{ return m_fileInfo.fileTime; }
	void			ClearFileTime()					{ m_fileInfo.fileTime.ClearFILETIME(); }
	bool			IsFileTimeZero() const			{ return m_fileInfo.fileTime.IsZero(); }	// 新規ファイル？
	const SYSTEMTIME	GetFileSysTime() const		{ return m_fileInfo.fileTime.GetSYSTEMTIME(); }
	void			SetFileTime(FILETIME& Time)		{ m_fileInfo.fileTime.SetFILETIME(Time); }

	const TCHAR*	GetFileName() const					{ return GetFileTitlePointer(GetFilePath()); }	// ファイル名(パスなし)を取得
	const TCHAR*	GetSaveFilePath(void) const;
	void			SetSaveFilePath(LPCTSTR pszPath)	{ m_szSaveFilePath.Assign(pszPath); }
public: //####
	EditDoc&	m_doc;
	FileInfo	m_fileInfo;
	FilePath	m_szSaveFilePath;	// 保存時のファイルのパス（マクロ用）	// 2006.09.04 ryoji
};

