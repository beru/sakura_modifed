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
class CSubject;
class Listener;

// 複数のListenerからウォッチされる
class CSubject {
public:
	// コンストラクタ・デストラクタ
	CSubject();
	virtual ~CSubject();

	// 公開インターフェース
	int GetListenerCount() const { return (int)m_vListenersRef.size(); }
	Listener* GetListener(int nIndex) const { return m_vListenersRef[nIndex]; }

public:
	// 管理用
	void _AddListener(Listener* pcListener);
	void _RemoveListener(Listener* pcListener);

private:
	std::vector<Listener*> m_vListenersRef;
};

// 1つのCSubjectをウォッチする
class Listener {
public:
	Listener();
	virtual ~Listener();

	// 公開インターフェース
	CSubject* Listen(CSubject* pcSubject); // 直前にウォッチしていたサブジェクトを返す
	CSubject* GetListeningSubject() const { return m_pcSubjectRef; }

private:
	CSubject* m_pcSubjectRef;
};


template <class LISTENER>
class CSubjectT : public CSubject {
public:
	LISTENER* GetListener(int nIndex) const {
		return static_cast<LISTENER*>(CSubject::GetListener(nIndex));
	}
};

template <class SUBJECT>
class CListenerT : public Listener {
public:
	SUBJECT* Listen(SUBJECT* pcSubject) {
		return static_cast<SUBJECT*>(Listener::Listen(static_cast<CSubject*>(pcSubject)));
	}
	SUBJECT* GetListeningSubject() const {
		return static_cast<SUBJECT*>(Listener::GetListeningSubject());
	}
};

