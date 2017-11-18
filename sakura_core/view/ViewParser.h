#pragma once

class EditView;

// �i����̓N���X
class ViewParser {
public:
	ViewParser(const EditView& editView) : editView(editView) { }
	virtual ~ViewParser() {}

	// �J�[�\�����O�̒P����擾
	size_t GetLeftWord(NativeW* pMemWord, int nMaxWordLen) const;

	// �L�����b�g�ʒu�̒P����擾
	// 2006.03.24 fon
	bool GetCurrentWord(NativeW* pMemWord) const;

private:
	const EditView& editView;
};

