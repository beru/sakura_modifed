#include "StdAfx.h"
#include "io/CIoBridge.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "CEol.h"

// ���������̃G���R�[�h�֕ϊ�
CodeConvertResult IoBridge::FileToImpl(
	const Memory&	src,		// [in]  �ϊ���������
	NativeW*		pDst,		// [out] �ϊ��惁����(UNICODE)
	CodeBase*		pCode,		// [in]  �ϊ����������̕����R�[�h
	int				nFlag		// [in]  bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
	)
{
	// �C�ӂ̕����R�[�h����Unicode�֕ϊ�����
	CodeConvertResult ret = pCode->CodeToUnicode(src, pDst);

	// ����
	return ret;
}

CodeConvertResult IoBridge::ImplToFile(
	const NativeW&	src,		// [in]  �ϊ���������(UNICODE)
	Memory*			pDst,		// [out] �ϊ��惁����
	CodeBase*		pCode		// [in]  �ϊ��惁�����̕����R�[�h
	)
{
	// Unicode����C�ӂ̕����R�[�h�֕ϊ�����
	CodeConvertResult ret = pCode->UnicodeToCode(src, pDst);

	// ����
	return ret;
}


