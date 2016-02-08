/*
	ObserverパターンのEditDoc特化版。
	DocSubjectは観察され、CDocListnerは観察を行う。
	観察の開始は DocListener::Listen で行う。

	$Note:
		Listener (Observer) と Subject のリレーション管理は
		ジェネリックな汎用モジュールに分離できる。
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
#include "StdAfx.h"
#include <map>
#include "doc/CDocListener.h"
#include "doc/CEditDoc.h"

bool LoadInfo::IsSamePath(LPCTSTR pszPath) const
{
	return _tcsicmp(this->cFilePath, pszPath) == 0;
}
bool SaveInfo::IsSamePath(LPCTSTR pszPath) const
{
	return _tcsicmp(this->cFilePath, pszPath) == 0;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       DocSubject                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//(1)

DocSubject::~DocSubject()
{
}


#define DEF_NOTIFY(NAME) ECallbackResult DocSubject::Notify##NAME() \
{ \
	int n = GetListenerCount(); \
	for (int i=0; i<n; ++i) { \
		ECallbackResult eRet = GetListener(i)->On##NAME(); \
		if (eRet != CALLBACK_CONTINUE) return eRet; \
	} \
	return CALLBACK_CONTINUE; \
}

#define DEF_NOTIFY2(NAME, ARGTYPE) ECallbackResult DocSubject::Notify##NAME(ARGTYPE a) \
{ \
	int n = GetListenerCount(); \
	for (int i=0; i<n; ++i) { \
		ECallbackResult eRet = GetListener(i)->On##NAME(a); \
		if (eRet != CALLBACK_CONTINUE) return eRet; \
	} \
	return CALLBACK_CONTINUE; \
}

#define VOID_NOTIFY(NAME) void DocSubject::Notify##NAME() \
{ \
	int n = GetListenerCount(); \
	for (int i=0; i<n; ++i) { \
		GetListener(i)->On##NAME(); \
	} \
}

#define VOID_NOTIFY2(NAME, ARGTYPE) void DocSubject::Notify##NAME(ARGTYPE a) \
{ \
	int n = GetListenerCount(); \
	for (int i=0; i<n; ++i) { \
		GetListener(i)->On##NAME(a); \
	} \
}

//######仮
#define CORE_NOTIFY2(NAME, ARGTYPE) ELoadResult DocSubject::Notify##NAME(ARGTYPE a) \
{ \
	int n = GetListenerCount(); \
	ELoadResult eRet = LOADED_FAILURE; \
	for (int i=0; i<n; ++i) { \
		ELoadResult e = GetListener(i)->On##NAME(a); \
		if (e == LOADED_NOIMPLEMENT) continue; \
		if (e == LOADED_FAILURE) return e; \
		eRet = e; \
	} \
	return eRet; \
}

DEF_NOTIFY2(CheckLoad, LoadInfo*)
VOID_NOTIFY2(BeforeLoad, LoadInfo*)
CORE_NOTIFY2(Load, const LoadInfo&)
VOID_NOTIFY2(Loading, int)
VOID_NOTIFY2(AfterLoad, const LoadInfo&)
VOID_NOTIFY2(FinalLoad, ELoadResult)

DEF_NOTIFY2(CheckSave, SaveInfo*)
DEF_NOTIFY2(PreBeforeSave, SaveInfo*)
VOID_NOTIFY2(BeforeSave, const SaveInfo&)
VOID_NOTIFY2(Save, const SaveInfo&)
VOID_NOTIFY2(Saving, int)
VOID_NOTIFY2(AfterSave, const SaveInfo&)
VOID_NOTIFY2(FinalSave, ESaveResult)

DEF_NOTIFY(BeforeClose)


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       DocListener                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// (多)

DocListener::DocListener(DocSubject* pcDoc)
{
	if (!pcDoc) {
		pcDoc = EditDoc::GetInstance(0); //$$ インチキ
	}
	assert(pcDoc);
	Listen(pcDoc);
}

DocListener::~DocListener()
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      DocListenerEx                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "doc/CEditDoc.h"

EditDoc* DocListenerEx::GetListeningDoc() const
{
	return static_cast<EditDoc*>(DocListener::GetListeningDoc());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ProgressSubject                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void ProgressSubject::NotifyProgress(int nPer)
{
	int n = GetListenerCount();
	for (int i=0; i<n; ++i) {
		GetListener(i)->OnProgress(nPer);
	}
}

