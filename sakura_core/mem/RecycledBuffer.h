// �ꎞ�I�ȃ������u���b�N�����[�e�[�V�������Ďg���܂킷���߂̃��m
// Get�Ŏ擾�����������u���b�N�́A�u������x�̊��ԁv�㏑������Ȃ����Ƃ��ۏႳ���B
// ���́u���ԁv�Ƃ́AGet���Ă�ł���ēxCHAIN_COUNT��AGet���Ăяo���܂ł̊Ԃł���B
// �擾�����������u���b�N��RecycledBuffer�̊Ǘ����ɂ��邽�߁A������Ă͂����Ȃ��B
#pragma once

class RecycledBuffer {
// �R���t�B�O
private:
	static const int BLOCK_SIZE  = 1024;	// �u���b�N�T�C�Y�B�o�C�g�P�ʁB
	static const int CHAIN_COUNT = 64;		// �ė��p�\�ȃu���b�N���B

// �R���X�g���N�^�E�f�X�g���N�^
public:
	RecycledBuffer() {
		current=0;
	}

// �C���^�[�t�F�[�X
public:
	// �ꎞ�I�Ɋm�ۂ��ꂽ�������u���b�N���擾�B���̃������u���b�N��������Ă͂����Ȃ��B
	template <class T>
	T* GetBuffer(
		size_t* nCount // [out] �̈�̗v�f�����󂯎��BT�P�ʁB
		)
	{
		if (nCount) {
			*nCount = BLOCK_SIZE / sizeof(T);
		}
		current = (current + 1) % CHAIN_COUNT;
		return reinterpret_cast<T*>(buf[current]);
	}

	// �̈�̗v�f�����擾�BT�P��
	template <class T>
	size_t GetMaxCount() const {
		return BLOCK_SIZE / sizeof(T);
	}


// �����o�ϐ�
private:
	BYTE buf[CHAIN_COUNT][BLOCK_SIZE];
	int  current;
};

class RecycledBufferDynamic {
// �R���t�B�O
private:
	static const int CHAIN_COUNT = 64;   // �ė��p�\�ȃu���b�N���B

// �R���X�g���N�^�E�f�X�g���N�^
public:
	RecycledBufferDynamic() {
		current = 0;
		for (size_t i=0; i<_countof(buf); ++i) {
			buf[i] = NULL;
		}
	}
	~RecycledBufferDynamic() {
		for (size_t i=0; i<_countof(buf); ++i) {
			if (buf[i]) delete[] buf[i];
		}
	}

// �C���^�[�t�F�[�X
public:
	// �ꎞ�I�Ɋm�ۂ��ꂽ�������u���b�N���擾�B���̃������u���b�N��������Ă͂����Ȃ��B
	template <class T>
	T* GetBuffer(
		size_t nCount // [in] �m�ۂ���v�f���BT�P�ʁB
	)
	{
		current = (current + 1) % CHAIN_COUNT;

		// �������m��
		if (buf[current]) {
			delete[] buf[current];
		}
		buf[current] = new BYTE[nCount * sizeof(T)];

		return reinterpret_cast<T*>(buf[current]);
	}

// �����o�ϐ�
private:
	BYTE* buf[CHAIN_COUNT];
	size_t current;
};

