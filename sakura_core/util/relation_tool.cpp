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
	for (int i=0; i<(int)m_vListenersRef.size(); ++i) {
		m_vListenersRef[i]->Listen(NULL);
	}
	m_vListenersRef.clear();
}

void Subject::_AddListener(Listener* pListener)
{
	// ���ɒǉ��ς݂Ȃ牽�����Ȃ�
	for (int i=0; i<(int)m_vListenersRef.size(); ++i) {
		if (m_vListenersRef[i] == pListener) {
			return;
		}
	}
	// �ǉ�
	m_vListenersRef.push_back(pListener);
}

void Subject::_RemoveListener(Listener* pListener)
{
	// �z�񂩂�폜
	for (int i=0; i<(int)m_vListenersRef.size(); ++i) {
		if (m_vListenersRef[i] == pListener) {
			m_vListenersRef.erase(m_vListenersRef.begin() + i);
			break;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         Listener                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Listener::Listener()
: m_pSubjectRef(NULL)
{
}

Listener::~Listener()
{
	Listen(NULL);
}

Subject* Listener::Listen(Subject* pSubject)
{
	Subject* pOld = GetListeningSubject();

	// �Â��T�u�W�F�N�g������
	if (m_pSubjectRef) {
		m_pSubjectRef->_RemoveListener(this);
		m_pSubjectRef = NULL;
	}

	// �V�����ݒ�
	m_pSubjectRef = pSubject;
	if (m_pSubjectRef) {
		m_pSubjectRef->_AddListener(this);
	}

	return pOld;
}

