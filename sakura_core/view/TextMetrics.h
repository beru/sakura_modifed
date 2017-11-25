#pragma once

#include <vector>

class TextMetrics;

class TextMetrics {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	TextMetrics();
	virtual ~TextMetrics();
	void CopyTextMetricsStatus(TextMetrics* pDst) const;
	void Update(HFONT hFont);

	// �ݒ�
	void SetHankakuWidth(int nHankakuWidth);   // ���p�����̕���ݒ�B�P�ʂ̓s�N�Z���B
	void SetHankakuHeight(int nHankakuHeight); // ���p�����̏c����ݒ�B�P�ʂ̓s�N�Z���B
	void SetHankakuDx(int nHankakuDx);         // ���p�����̕����Ԋu��ݒ�B�P�ʂ̓s�N�Z���B
	void SetHankakuDy(int nHankakuDy);         // ���p�����̍s�Ԋu��ݒ�B�P�ʂ̓s�N�Z���B

	// �擾
	int GetHankakuWidth() const { return nCharWidth; }		// ���p�����̉������擾�B�P�ʂ̓s�N�Z���B
	int GetHankakuHeight() const { return nCharHeight; }	// ���p�����̏c�����擾�B�P�ʂ̓s�N�Z���B
	int GetHankakuDx() const { return nDxBasis; }			// ���p�����̕����Ԋu���擾�B�P�ʂ̓s�N�Z���B
	int GetZenkakuDx() const { return nDxBasis*2; }			// �S�p�����̕����Ԋu���擾�B�P�ʂ̓s�N�Z���B
	int GetHankakuDy() const { return nDyBasis; }			// Y���������Ԋu�B�����c���{�s�Ԋu�B�P�ʂ̓s�N�Z���B

	// �Œ蕶��x���̃��C�A�E�g�����擾����
	int GetLayoutXDefault(int chars = 1) const {
		return chars;
	}

	// �����Ԋu�z����擾
	const int* GetDxArray_AllHankaku() const { return anHankakuDx; } // ���p������̕����Ԋu�z����擾�B�v�f����64�B
	const int* GetDxArray_AllZenkaku() const { return anZenkakuDx; } // ���p������̕����Ԋu�z����擾�B�v�f����64�B

	// �w�肵��������ɂ�蕶���Ԋu�z��𐶐�����B
	static
	const int* GenerateDxArray(
		std::vector<int>* vResultArray, // [out] �����Ԋu�z��̎󂯎��R���e�i
		const wchar_t* pText,           // [in]  ������
		size_t nLength,					// [in]  ������
		int	nHankakuDx,					// [in]  ���p�����̕����Ԋu
		int	nTabSpace = 8,				// [in]  TAB��
		int	nIndent = 0					// [in]  �C���f���g
	);

	// ������̃s�N�Z������Ԃ��B
	static
	size_t CalcTextWidth(
		const wchar_t*	pText,		// ������
		size_t			nLength,	// ������
		const int*		pnDx		// �����Ԋu�̓������z��
	);

	// ������̃s�N�Z������Ԃ��B
	static
	size_t CalcTextWidth2(
		const wchar_t*	pText,		// ������
		size_t			nLength,	// ������
		int				nHankakuDx	// ���p�����̕����Ԋu
	);

private:
//	HDC hdc; // �v�Z�ɗp����f�o�C�X�R���e�L�X�g
	int	nCharWidth;				// ���p�����̉���
	int nCharHeight;    		// ���p�����̏c��
	int nDxBasis;       		// ���p�����̕����Ԋu (����+��)
	int nDyBasis;       		// ���p�����̍s�Ԋu (�c��+��)
	int anHankakuDx[64];		// ���p�p�����Ԋu�z��
	int anZenkakuDx[64];		// �S�p�p�����Ԋu�z��
};

