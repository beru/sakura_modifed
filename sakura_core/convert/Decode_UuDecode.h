#pragma once

#include "convert/Decode.h"

// Unix-to-Unix Decode

class Decode_UuDecode : public Decode {

	TCHAR aFilename[_MAX_PATH];
public:
	bool DoDecode(const NativeW& data, Memory* pDst);
	void CopyFilename(TCHAR* pDst) const { _tcscpy(pDst, aFilename); }
};

