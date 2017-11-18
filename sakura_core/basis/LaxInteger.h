#pragma once

// �^�`�F�b�N�̊ɂ������^
class LaxInteger {
private:
	typedef LaxInteger Me;

public:
	// �R���X�g���N�^�E�f�X�g���N�^
	LaxInteger() { value = 0; }
	LaxInteger(const Me& rhs) { value = rhs.value; }
	LaxInteger(int value) { this->value = value; }

	// �Öق̕ϊ�
	operator const int&() const { return value; }
	operator       int&()       { return value; }

private:
	int value;
};

