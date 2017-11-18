/*
	�֘A�̊Ǘ�
*/
#pragma once

#include <vector>
class Subject;
class Listener;

// ������Listener����E�H�b�`�����
class Subject {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Subject();
	virtual ~Subject();

	// ���J�C���^�[�t�F�[�X
	size_t GetListenerCount() const { return vListenersRef.size(); }
	Listener* GetListener(size_t nIndex) const { return vListenersRef[nIndex]; }

public:
	// �Ǘ��p
	void _AddListener(Listener* pListener);
	void _RemoveListener(Listener* pListener);

private:
	std::vector<Listener*> vListenersRef;
};

// 1��Subject���E�H�b�`����
class Listener {
public:
	Listener();
	virtual ~Listener();

	// ���J�C���^�[�t�F�[�X
	Subject* Listen(Subject* pSubject); // ���O�ɃE�H�b�`���Ă����T�u�W�F�N�g��Ԃ�
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

