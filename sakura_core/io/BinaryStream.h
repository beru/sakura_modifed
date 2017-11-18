#pragma once

#include "Stream.h"

class BinaryInputStream : public Stream {
public:
	BinaryInputStream(LPCTSTR tszFilePath);

public:
	// ストリームの「残り」サイズを取得
	size_t GetLength();

	// データを無変換で読み込む。戻り値は読み込んだバイト数。
	size_t Read(void* pBuffer, size_t nSizeInBytes);
};

class BinaryOutputStream : public OutputStream {
public:
	BinaryOutputStream(LPCTSTR tszFilePath, bool bExceptionMode = false);
};

