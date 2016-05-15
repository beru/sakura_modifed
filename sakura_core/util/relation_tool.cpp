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
	for (int i=0; i<(int)vListenersRef.size(); ++i) {
		vListenersRef[i]->Listen(NULL);
	}
	vListenersRef.clear();
}

void Subject::_AddListener(Listener* pListener)
{
	// 既に追加済みなら何もしない
	for (int i=0; i<(int)vListenersRef.size(); ++i) {
		if (vListenersRef[i] == pListener) {
			return;
		}
	}
	// 追加
	vListenersRef.push_back(pListener);
}

void Subject::_RemoveListener(Listener* pListener)
{
	// 配列から削除
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

	// 古いサブジェクトを解除
	if (pSubjectRef) {
		pSubjectRef->_RemoveListener(this);
		pSubjectRef = nullptr;
	}

	// 新しく設定
	pSubjectRef = pSubject;
	if (pSubjectRef) {
		pSubjectRef->_AddListener(this);
	}

	return pOld;
}

