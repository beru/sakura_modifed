#pragma once

class EditView;


// �N���b�s���O�̈���v�Z����ۂ̃t���O
enum class PaintAreaType {
	LineNumber = (1 << 0), // �s�ԍ�
	Ruler      = (1 << 1), // ���[���[
	Body       = (1 << 2), // �{��

	// ����
	All        = PaintAreaType::LineNumber | PaintAreaType::Ruler | PaintAreaType::Body, // �����
};

class EditView_Paint {
public:
	virtual EditView& GetEditView() = 0;
	virtual const EditView& GetEditView() const = 0;

public:
	virtual ~EditView_Paint() {}
	void Call_OnPaint(
		int nPaintFlag,   // �`�悷��̈��I������
		bool bUseMemoryDC // ������DC���g�p����
	);
};

