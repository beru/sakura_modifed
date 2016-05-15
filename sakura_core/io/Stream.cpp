#include "StdAfx.h"
#include "Stream.h"
#include <string>

//	::fflush(hFile);
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
		strPath(tszPath),
		bAttributeChanged(false),
		dwAttribute(0)
	{
	}

	// 指定属性を取り除く
	void PopAttribute(DWORD dwPopAttribute)
	{
		if (bAttributeChanged) {
			return; // 既に取り除き済み
		}
		dwAttribute = ::GetFileAttributes(strPath.c_str());
		if (dwAttribute != (DWORD)-1) {
			if (dwAttribute & dwPopAttribute) {
				DWORD dwNewAttribute = dwAttribute & ~dwPopAttribute;
				::SetFileAttributes(strPath.c_str(), dwNewAttribute);
				bAttributeChanged = true;
			}
		}
	}
	
	// 属性を元に戻す
	void RestoreAttribute()
	{
		if (bAttributeChanged) {
			::SetFileAttributes(strPath.c_str(), dwAttribute);
		}
		bAttributeChanged = false;
		dwAttribute = 0;
	}
private:
	std::tstring	strPath;
	bool			bAttributeChanged;
	DWORD			dwAttribute;
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
	fp = nullptr;
	pFileAttribute = NULL;
	this->bExceptionMode = bExceptionMode;
	Open(tszPath, tszMode);
}

/*
Stream::Stream()
{
	fp = nullptr;
	pFileAttribute = NULL;
	bExceptionMode = false;
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
	pFileAttribute = new FileAttribute(tszPath);
	pFileAttribute->PopAttribute(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

	// オープン
	fp = _tfopen(tszPath, tszMode);
	if (!fp) {
		Close(); // 属性復元
	}

	// エラー処理
	if (!fp && IsExceptionMode()) {
		throw Error_FileOpen();
	}
}

void Stream::Close()
{
	// クローズ
	if (fp) {
		fclose(fp);
		fp = nullptr;
	}

	// 属性復元
	if (pFileAttribute) {
		pFileAttribute->RestoreAttribute();
		SAFE_DELETE(pFileAttribute);
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           操作                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Stream::SeekSet(	// シーク
	long offset	// ストリーム先頭からのオフセット 
	)
{
	fseek(fp, offset, SEEK_SET);
}

void Stream::SeekEnd(  // シーク
	long offset // ストリーム終端からのオフセット
	)
{
	fseek(fp, offset, SEEK_END);
}

