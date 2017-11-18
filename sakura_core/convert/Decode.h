#pragma once

// �f�R�[�_�[�̃C���^�[�t�F�[�X

class Decode {
public:
	virtual ~Decode() { }

	// �C���^�[�t�F�[�X
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

	// ����
	virtual bool DoDecode(const NativeW& pData, Memory* pDest) = 0;

};

