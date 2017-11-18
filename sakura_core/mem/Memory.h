/*!	@file
	@brief �������o�b�t�@�N���X
*/

#pragma once

// �t�@�C�������R�[�h�Z�b�g���ʎ��̐�ǂݍő�T�C�Y
#define CheckKanjiCode_MAXREADLENGTH 16384

#include "_main/global.h"

// �������o�b�t�@�N���X
class Memory {
	// �R���X�g���N�^�E�f�X�g���N�^
public:
	Memory();
	Memory(const Memory& rhs);
	Memory(const void* pData, size_t nDataLenBytes);
	virtual ~Memory();
protected:
	void _init_members();

	// �C���^�[�t�F�[�X
public:
	void AllocBuffer(size_t);											// �o�b�t�@�T�C�Y�̒����B�K�v�ɉ����Ċg�傷��B
	void SetRawData(const void* pData, size_t nDataLen);				// �o�b�t�@�̓��e��u��������
	void SetRawData(const Memory&);										// �o�b�t�@�̓��e��u��������
	void SetRawDataHoldBuffer(const void* pData, size_t nDataLen);	// �o�b�t�@�̓��e��u��������(�o�b�t�@��ێ�)
	void SetRawDataHoldBuffer(const Memory&);						// �o�b�t�@�̓��e��u��������(�o�b�t�@��ێ�)
	void AppendRawData(const void* pData, size_t nDataLen);			// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
	void AppendRawData(const Memory*);								// �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����
	void Clean() { _Empty(); }
	void Clear() { _Empty(); }

	inline const void* GetRawPtr(size_t* pnLength) const;			// �f�[�^�ւ̃|�C���^�ƒ����Ԃ�
	inline void* GetRawPtr(size_t* pnLength);						// �f�[�^�ւ̃|�C���^�ƒ����Ԃ�
	inline const void* GetRawPtr() const { return pRawData; } // �f�[�^�ւ̃|�C���^��Ԃ�
	inline void* GetRawPtr() { return pRawData; }				// �f�[�^�ւ̃|�C���^��Ԃ�
	size_t GetRawLength() const { return nRawLen; }				// �f�[�^����Ԃ��B�o�C�g�P�ʁB

	// ���Z�q
	const Memory& operator = (const Memory&);

	// ��r
	static int IsEqual(Memory&, Memory&);	// ���������e��

	// �ϊ��֐�
	static void SwapHLByte(char*, const size_t); // ���L�֐���static�֐���
	void SwapHLByte();					// Byte����������
	bool SwabHLByte( const Memory& );	// Byte����������(�R�s�[��)


protected:
	/*
	||  �����w���p�֐�
	*/
	void _Empty(void); // �������BpRawData��NULL�ɂȂ�B
	void _AddData(const void*, size_t);
public:
	void _AppendSz(const char* str);
	void _SetRawLength(size_t nLength);
	void swap(Memory& left) {
		std::swap(nDataBufSize, left.nDataBufSize);
		std::swap(pRawData, left.pRawData);
		std::swap(nRawLen, left.nRawLen);
	}
	size_t capacity() const {
		if (nDataBufSize) {
			assert(nDataBufSize >= 2);
		}
		return nDataBufSize ? nDataBufSize - 2: 0;
	}

#ifdef _DEBUG
protected:
	typedef char* PCHAR;
	PCHAR& _DebugGetPointerRef() { return pRawData; } // �f�o�b�O�p�B�o�b�t�@�|�C���^�̎Q�Ƃ�Ԃ��B
#endif

private: // 2002/2/10 aroka �A�N�Z�X���ύX
	/*
	|| �����o�ϐ�
	*/
	char*	pRawData;		// �o�b�t�@
	size_t	nRawLen;		// �f�[�^�T�C�Y(nDataBufSize�ȓ�)�B�o�C�g�P�ʁB
	size_t	nDataBufSize;	//�o�b�t�@�T�C�Y�B�o�C�g�P�ʁB
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     inline�֐��̎���                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
inline
const void* Memory::GetRawPtr(size_t* pnLength) const // �f�[�^�ւ̃|�C���^�ƒ����Ԃ�
{
	if (pnLength) *pnLength = GetRawLength();
	return pRawData;
}

inline
void* Memory::GetRawPtr(size_t* pnLength) // �f�[�^�ւ̃|�C���^�ƒ����Ԃ�
{
	if (pnLength) *pnLength = GetRawLength();
	return pRawData;
}

