#pragma once

#include <vector>
#include <algorithm> // find

// vectorにちょっと機能を追加した版
template <class T>
class vector_ex : public std::vector<T> {
public:
	using std::vector<T>::begin;
	using std::vector<T>::end;
	using std::vector<T>::push_back;

public:
	// -- -- インターフェース -- -- //
	// 要素を探す。見つかればtrue。
	bool exist(const T& t) const {
		return std::find(begin(), end(), t) != end();
	}

	// 要素を追加。ただし重複した要素は弾く。
	bool push_back_unique(const T& t) {
		if (!exist(t)) {
			push_back(t);
			return true;
		}
		return false;
	}
};

