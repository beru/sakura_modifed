#include "StdAfx.h"
#include "CStream.h"
#include <string>

//	::fflush(m_hFile);
// ネットワーク上のファイルを扱っている場合、
// 書き込み後にFlushを行うとデットロックが発生することがあるので、
// Close時に::fflushを呼び出してはいけません。
// 詳細：http://www.microsoft.com/japan/support/faq/KBArticles2.asp?URL=/japan/support/kb/articles/jp288/7/94.asp


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  ファイル属性操作クラス                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class FileAttribute {
public:
	FileAttribute(const TCHAR* tszPath)
		:
		m_strPath(tszPath),
		m_bAttributeChanged(false),
		m_dwAttribute(0)
	{
	}

	// 指定属性を取り除く
	void PopAttribute(DWORD dwPopAttribute)
	{
		if (m_bAttributeChanged) {
			return; // 既に取り除き済み
		}
		m_dwAttribute = ::GetFileAttributes(m_strPath.c_str());
		if (m_dwAttribute != (DWORD)-1) {
			if (m_dwAttribute & dwPopAttribute) {
				DWORD dwNewAttribute = m_dwAttribute & ~dwPopAttribute;
				::SetFileAttributes(m_strPath.c_str(), dwNewAttribute);
				m_bAttributeChanged = true;
			}
		}
	}
	
	// 属性を元に戻す
	void RestoreAttribute()
	{
		if (m_bAttributeChanged) {
			::SetFileAttributes(m_strPath.c_str(), m_dwAttribute);
		}
		m_bAttributeChanged = false;
		m_dwAttribute = 0;
	}
private:
	std::tstring	m_strPath;
	bool			m_bAttributeChanged;
	DWORD			m_dwAttribute;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Stream::Stream(
	const TCHAR* tszPath,
	const TCHAR* tszMode,
	bool bExceptionMode
	)
{
	m_fp = NULL;
	m_pFileAttribute = NULL;
	m_bExceptionMode = bExceptionMode;
	Open(tszPath, tszMode);
}

/*
Stream::Stream()
{
	m_fp = NULL;
	m_pFileAttribute = NULL;
	m_bExceptionMode = false;
}
*/

Stream::~Stream()
{
	Close();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    オープン・クローズ                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
void Stream::Open(
	const TCHAR* tszPath,
	const TCHAR* tszMode
	)
{
	Close(); // 既に開いていたら、一度閉じる

	// 属性変更：隠しorシステムファイルはCの関数で読み書きできないので属性を変更する
	m_pFileAttribute = new FileAttribute(tszPath);
	m_pFileAttribute->PopAttribute(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

	// オープン
	m_fp = _tfopen(tszPath, tszMode);
	if (!m_fp) {
		Close(); // 属性復元
	}

	// エラー処理
	if (!m_fp && IsExceptionMode()) {
		throw Error_FileOpen();
	}
}

void Stream::Close()
{
	// クローズ
	if (m_fp) {
		fclose(m_fp);
		m_fp=NULL;
	}

	// 属性復元
	if (m_pFileAttribute) {
		m_pFileAttribute->RestoreAttribute();
		SAFE_DELETE(m_pFileAttribute);
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           操作                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Stream::SeekSet(	// シーク
	long offset	// ストリーム先頭からのオフセット 
	)
{
	fseek(m_fp, offset, SEEK_SET);
}

void Stream::SeekEnd(  // シーク
	long offset // ストリーム終端からのオフセット
	)
{
	fseek(m_fp, offset, SEEK_END);
}

