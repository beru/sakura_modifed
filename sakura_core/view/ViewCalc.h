#pragma once

/*
	X�l�̒P�ʕϊ��֐��Q�B
*/

class Layout;
class DocLine;
class EditView;

class ViewCalc {
protected:
	// �O���ˑ�
	size_t GetTabSpace() const;

public:
	ViewCalc(const EditView& owner) : owner(owner) { }
	virtual ~ViewCalc() {}

	// �P�ʕϊ�: ���C�A�E�g�����W�b�N
	size_t LineColumnToIndex (const Layout*  pLayout,  size_t nColumn) const;		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1		// @@@ 2002.09.28 YAZAKI
	size_t LineColumnToIndex (const DocLine* pDocLine, size_t nColumn) const;		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1		// @@@ 2002.09.28 YAZAKI
	size_t LineColumnToIndex2(const Layout*  pLayout,  size_t nColumn, size_t* pnLineAllColLen) const;	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0		// @@@ 2002.09.28 YAZAKI

	// �P�ʕϊ�: ���W�b�N�����C�A�E�g
	size_t LineIndexToColumn (const Layout*  pLayout,  size_t nIndex) const;		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�	// @@@ 2002.09.28 YAZAKI
	size_t LineIndexToColumn (const DocLine* pLayout,  size_t nIndex) const;		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�	// @@@ 2002.09.28 YAZAKI

private:
	const EditView& owner;
};

