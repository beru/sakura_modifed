#include "StdAfx.h"
#include "Convert_ToHankaku.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 半角にできるものは全部半角に変換
bool Converter_ToHankaku::DoConvert(NativeW* pData)
{
	// 全角→半角
	// 濁点等の影響で、最大2倍にまで膨れ上がる可能性があるので、2倍のバッファを確保
	std::vector<wchar_t> buf(pData->GetStringLength() * 2 + 1);
	wchar_t* pBuf = &buf[0];
	size_t nDstLen = 0;
	Convert_ToHankaku(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nDstLen);
	pData->SetString(pBuf, nDstLen);

	return true;
}

