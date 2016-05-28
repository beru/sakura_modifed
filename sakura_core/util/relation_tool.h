/*
	関連の管理
*/
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include <vector>
class Subject;
class Listener;

// 複数のListenerからウォッチされる
class Subject {
public:
	// コンストラクタ・デストラクタ
	Subject();
	virtual ~Subject();

	// 公開インターフェース
	size_t GetListenerCount() const { return vListenersRef.size(); }
	Listener* GetListener(size_t nIndex) const { return vListenersRef[nIndex]; }

public:
	// 管理用
	void _AddListener(Listener* pListener);
	void _RemoveListener(Listener* pListener);

private:
	std::vector<Listener*> vListenersRef;
};

// 1つのSubjectをウォッチする
class Listener {
public:
	Listener();
	virtual ~Listener();

	// 公開インターフェース
	Subject* Listen(Subject* pSubject); // 直前にウォッチしていたサブジェクトを返す
	Subject* GetListeningSubject() const { return pSubjectRef; }

private:
	Subject* pSubjectRef;
};


template <class LISTENER>
class SubjectT : public Subject {
public:
	LISTENER* GetListener(size_t nIndex) const {
		return static_cast<LISTENER*>(Subject::GetListener(nIndex));
	}
};

template <class SUBJECT>
class ListenerT : public Listener {
public:
	SUBJECT* Listen(SUBJECT* pSubject) {
		return static_cast<SUBJECT*>(Listener::Listen(static_cast<Subject*>(pSubject)));
	}
	SUBJECT* GetListeningSubject() const {
		return static_cast<SUBJECT*>(Listener::GetListeningSubject());
	}
};

