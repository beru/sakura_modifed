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

void Subject::_AddListener(Listener* pListener)
{
	// 既に追加済みなら何もしない
	for (int i=0; i<(int)m_vListenersRef.size(); ++i) {
		if (m_vListenersRef[i] == pListener) {
			return;
		}
	}
	// 追加
	m_vListenersRef.push_back(pListener);
}

void Subject::_RemoveListener(Listener* pListener)
{
	// 配列から削除
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

	// 古いサブジェクトを解除
	if (m_pSubjectRef) {
		m_pSubjectRef->_RemoveListener(this);
		m_pSubjectRef = NULL;
	}

	// 新しく設定
	m_pSubjectRef = pSubject;
	if (m_pSubjectRef) {
		m_pSubjectRef->_AddListener(this);
	}

	return pOld;
}

