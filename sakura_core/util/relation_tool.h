/*
	関連の管理
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

