#pragma once

#include <Windows.h> // SIZE

class Size : public SIZE {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Size() {} // ���������Ȃ�
	Size(int _cx, int _cy) { cx = _cx; cy = _cy; }
	Size(const SIZE& rhs) { cx = rhs.cx; cy = rhs.cy; }

	// �֐�
	void Set(int _cx, int _cy) { cx = _cx; cy = _cy; }

	// ���Z�q
	bool operator == (const SIZE& rhs) const { return cx == rhs.cx && cy == rhs.cy; }
	bool operator != (const SIZE& rhs) const { return !(operator == (rhs)); }
};

