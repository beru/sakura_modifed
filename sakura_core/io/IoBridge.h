#pragma once

#include "mem/Memory.h"
#include "charset/CodeBase.h"

class IoBridge {
public:
	// ���������̃G���R�[�h�֕ϊ�
	static CodeConvertResult FileToImpl(
		const Memory&	src,			// [in]  �ϊ���������
		NativeW*		pDst,			// [out] �ϊ��惁����(UNICODE)
		CodeBase*		pCodeBase,		// [in]  �ϊ����������̕����R�[�h�N���X
		int				nFlag			// [in]  bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
	);

	// �t�@�C���̃G���R�[�h�֕ύX
	static CodeConvertResult ImplToFile(
		const NativeW&		src,		// [in]  �ϊ���������(UNICODE)
		Memory*				pDst,		// [out] �ϊ��惁����
		CodeBase*			pCodeBase	// [in]  �ϊ��惁�����̕����R�[�h�N���X
	);
};

