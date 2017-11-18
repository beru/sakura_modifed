#pragma once

#include <vector>
#include <algorithm> // find

// vector�ɂ�����Ƌ@�\��ǉ�������
template <class T>
class vector_ex : public std::vector<T> {
public:
	using std::vector<T>::begin;
	using std::vector<T>::end;
	using std::vector<T>::push_back;

public:
	// -- -- �C���^�[�t�F�[�X -- -- //
	// �v�f��T���B�������true�B
	bool exist(const T& t) const {
		return std::find(begin(), end(), t) != end();
	}

	// �v�f��ǉ��B�������d�������v�f�͒e���B
	bool push_back_unique(const T& t) {
		if (!exist(t)) {
			push_back(t);
			return true;
		}
		return false;
	}
};

