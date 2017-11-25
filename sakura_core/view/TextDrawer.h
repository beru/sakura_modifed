#pragma once

class TextMetrics;
class TextArea;
class ViewFont;
class Eol;
class EditView;
class Layout;
#include "DispPos.h"

class Graphics;

class TextDrawer {
public:
	TextDrawer(const EditView& editView) : editView(editView) { }
	virtual ~TextDrawer() {}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �O���ˑ�                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �̈�̃C���X�^���X�����߂�
	const TextArea& GetTextArea() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �C���^�[�t�F�[�X                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// ���ۂɂ� pX �� nX ���X�V�����B
	void DispText(HDC hdc, DispPos* pDispPos, const wchar_t* pData, size_t nLength, bool bTransparent = false) const; // �e�L�X�g�\��

	// �m�[�g���`��
	void DispNoteLine(Graphics& gr, int nTop, int nBottom, int nLeft, int nRight) const;

	// -- -- �w�茅�c���`�� -- -- //
	// �w�茅�c���`��֐�
	void DispVerticalLines(Graphics& gr, int nTop, int nBottom, int nLeftCol, int nRightCol) const;

	// -- -- �܂�Ԃ����c���`�� -- -- //
	void DispWrapLine(Graphics& gr, int nTop, int nBottom) const;

	// -- -- �s�ԍ� -- -- //
	void DispLineNumber(Graphics& gr, int nLineNum, int y) const;		// �s�ԍ��\��

private:
	const EditView& editView;
};

