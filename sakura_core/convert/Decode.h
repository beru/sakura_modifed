#pragma once

// デコーダーのインターフェース

class Decode {
public:
	virtual ~Decode() { }

	// インターフェース
	bool CallDecode(const NativeW& pData, Memory* pDest)
	{
		bool bRet = DoDecode(pData, pDest);
		if (!bRet) {
			ErrorMessage(NULL, LS(STR_CONVERT_ERR));
			pDest->SetRawData("", 0);
			return false;
		}
		return true;
	}

	// 実装
	virtual bool DoDecode(const NativeW& pData, Memory* pDest) = 0;

};

