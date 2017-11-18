#pragma once

class FileAttribute;

// 例外
class Error_FileOpen {};	// 例外：ファイルオープンに失敗
class Error_FileWrite {};	// 例外：ファイル書き込み失敗
class Error_FileRead {};	// 例外：ファイル読み込み失敗

// ストリーム基底クラス
class Stream {
public:
	// コンストラクタ・デストラクタ
	Stream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode = false);
//	Stream();
	virtual ~Stream();

	// 演算子
	operator bool() const { return Good(); }

	// オープン・クローズ
	void Open(const TCHAR* tszPath, const TCHAR* tszMode);
	void Close();

	// 操作
	void SeekSet(	// シーク
		long offset	// ストリーム先頭からのオフセット 
	);
	void SeekEnd(	// シーク
		long offset // ストリーム終端からのオフセット
	);

	// 状態
	virtual bool Good() const { return fp && !Eof(); }
	bool Eof() const { return !fp || feof(fp); }

	// ファイルハンドル
	FILE* GetFp() const { return fp; }

	// モード
	bool IsExceptionMode() const { return bExceptionMode; }
private:
	FILE*			fp;
	FileAttribute*	pFileAttribute;
	bool			bExceptionMode;
};


class OutputStream : public Stream {
public:
	OutputStream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode = false)
		:
		Stream(tszPath, tszMode, bExceptionMode)
	{
	}

	// データを無変換で書き込む。戻り値は書き込んだバイト数。
	size_t Write(const void* pBuffer, size_t nSizeInBytes) {
		size_t nRet = fwrite(pBuffer, 1, nSizeInBytes, GetFp());
		if (nRet != nSizeInBytes && IsExceptionMode()) {
			throw Error_FileWrite();
		}
		return nRet;
	}
};

