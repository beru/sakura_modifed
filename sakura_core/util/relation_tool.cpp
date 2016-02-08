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
	// リスナを解除
	for (int i=0; i<(int)m_vListenersRef.size(); ++i) {
		m_vListenersRef[i]->Listen(NULL);
	}
	m_vListenersRef.clear();
}

void Subject::_AddListener(Listener* pcListener)
{
	// 既に追加済みなら何もしない
	for (int i=0; i<(int)m_vListenersRef.size(); ++i) {
		if (m_vListenersRef[i] == pcListener) {
			return;
		}
	}
	// 追加
	m_vListenersRef.push_back(pcListener);
}

void Subject::_RemoveListener(Listener* pcListener)
{
	// 配列から削除
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

	// 古いサブジェクトを解除
	if (m_pcSubjectRef) {
		m_pcSubjectRef->_RemoveListener(this);
		m_pcSubjectRef = NULL;
	}

	// 新しく設定
	m_pcSubjectRef = pcSubject;
	if (m_pcSubjectRef) {
		m_pcSubjectRef->_AddListener(this);
	}

	return pOld;
}

