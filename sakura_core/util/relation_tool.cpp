#include "StdAfx.h"
#include "relation_tool.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         Subject                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Subject::Subject()
{
}

Subject::~Subject()
{
	// ���X�i������
	for (int i=0; i<(int)vListenersRef.size(); ++i) {
		vListenersRef[i]->Listen(NULL);
	}
	vListenersRef.clear();
}

void Subject::_AddListener(Listener* pListener)
{
	// ���ɒǉ��ς݂Ȃ牽�����Ȃ�
	for (int i=0; i<(int)vListenersRef.size(); ++i) {
		if (vListenersRef[i] == pListener) {
			return;
		}
	}
	// �ǉ�
	vListenersRef.push_back(pListener);
}

void Subject::_RemoveListener(Listener* pListener)
{
	// �z�񂩂�폜
	for (int i=0; i<(int)vListenersRef.size(); ++i) {
		if (vListenersRef[i] == pListener) {
			vListenersRef.erase(vListenersRef.begin() + i);
			break;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         Listener                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Listener::Listener()
	:
	pSubjectRef(nullptr)
{
}

Listener::~Listener()
{
	Listen(nullptr);
}

Subject* Listener::Listen(Subject* pSubject)
{
	Subject* pOld = GetListeningSubject();

	// �Â��T�u�W�F�N�g������
	if (pSubjectRef) {
		pSubjectRef->_RemoveListener(this);
		pSubjectRef = nullptr;
	}

	// �V�����ݒ�
	pSubjectRef = pSubject;
	if (pSubjectRef) {
		pSubjectRef->_AddListener(this);
	}

	return pOld;
}

