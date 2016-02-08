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

void Subject::_AddListener(Listener* pcListener)
{
	// ���ɒǉ��ς݂Ȃ牽�����Ȃ�
	for (int i=0; i<(int)m_vListenersRef.size(); ++i) {
		if (m_vListenersRef[i] == pcListener) {
			return;
		}
	}
	// �ǉ�
	m_vListenersRef.push_back(pcListener);
}

void Subject::_RemoveListener(Listener* pcListener)
{
	// �z�񂩂�폜
	for (int i=0; i<(int)m_vListenersRef.size(); ++i) {
		if (m_vListenersRef[i] == pcListener) {
			m_vListenersRef.erase(m_vListenersRef.begin() + i);
			break;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         Listener                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

Listener::Listener()
: m_pcSubjectRef(NULL)
{
}

Listener::~Listener()
{
	Listen(NULL);
}

Subject* Listener::Listen(Subject* pcSubject)
{
	Subject* pOld = GetListeningSubject();

	// �Â��T�u�W�F�N�g������
	if (m_pcSubjectRef) {
		m_pcSubjectRef->_RemoveListener(this);
		m_pcSubjectRef = NULL;
	}

	// �V�����ݒ�
	m_pcSubjectRef = pcSubject;
	if (m_pcSubjectRef) {
		m_pcSubjectRef->_AddListener(this);
	}

	return pOld;
}

