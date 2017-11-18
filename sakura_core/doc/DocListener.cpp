/*
	ObserverパターンのEditDoc特化版。
	DocSubjectは観察され、CDocListnerは観察を行う。
	観察の開始は DocListener::Listen で行う。

	$Note:
		Listener (Observer) と Subject のリレーション管理は
		ジェネリックな汎用モジュールに分離できる。
*/
#include "StdAfx.h"
#include <map>
#include "doc/DocListener.h"
#include "doc/EditDoc.h"

bool LoadInfo::IsSamePath(LPCTSTR pszPath) const
{
	return _tcsicmp(this->filePath, pszPath) == 0;
}
bool SaveInfo::IsSamePath(LPCTSTR pszPath) const
{
	return _tcsicmp(this->filePath, pszPath) == 0;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       DocSubject                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//(1)

DocSubject::~DocSubject()
{
}


#define DEF_NOTIFY(NAME) CallbackResultType DocSubject::Notify##NAME() \
{ \
	size_t n = GetListenerCount(); \
	for (size_t i=0; i<n; ++i) { \
		CallbackResultType eRet = GetListener(i)->On##NAME(); \
		if (eRet != CallbackResultType::Continue) return eRet; \
	} \
	return CallbackResultType::Continue; \
}

#define DEF_NOTIFY2(NAME, ARGTYPE) CallbackResultType DocSubject::Notify##NAME(ARGTYPE a) \
{ \
	size_t n = GetListenerCount(); \
	for (size_t i=0; i<n; ++i) { \
		CallbackResultType eRet = GetListener(i)->On##NAME(a); \
		if (eRet != CallbackResultType::Continue) return eRet; \
	} \
	return CallbackResultType::Continue; \
}

#define VOID_NOTIFY(NAME) void DocSubject::Notify##NAME() \
{ \
	size_t n = GetListenerCount(); \
	for (size_t i=0; i<n; ++i) { \
		GetListener(i)->On##NAME(); \
	} \
}

#define VOID_NOTIFY2(NAME, ARGTYPE) void DocSubject::Notify##NAME(ARGTYPE a) \
{ \
	size_t n = GetListenerCount(); \
	for (size_t i=0; i<n; ++i) { \
		GetListener(i)->On##NAME(a); \
	} \
}

//######仮
#define CORE_NOTIFY2(NAME, ARGTYPE) LoadResultType DocSubject::Notify##NAME(ARGTYPE a) \
{ \
	size_t n = GetListenerCount(); \
	LoadResultType eRet = LoadResultType::Failure; \
	for (size_t i=0; i<n; ++i) { \
		LoadResultType e = GetListener(i)->On##NAME(a); \
		if (e == LoadResultType::NoImplement) continue; \
		if (e == LoadResultType::Failure) return e; \
		eRet = e; \
	} \
	return eRet; \
}

DEF_NOTIFY2(CheckLoad, LoadInfo*)
VOID_NOTIFY2(BeforeLoad, LoadInfo*)
CORE_NOTIFY2(Load, const LoadInfo&)
VOID_NOTIFY2(Loading, int)
VOID_NOTIFY2(AfterLoad, const LoadInfo&)
VOID_NOTIFY2(FinalLoad, LoadResultType)

DEF_NOTIFY2(CheckSave, SaveInfo*)
DEF_NOTIFY2(PreBeforeSave, SaveInfo*)
VOID_NOTIFY2(BeforeSave, const SaveInfo&)
VOID_NOTIFY2(Save, const SaveInfo&)
VOID_NOTIFY2(Saving, int)
VOID_NOTIFY2(AfterSave, const SaveInfo&)
VOID_NOTIFY2(FinalSave, SaveResultType)

DEF_NOTIFY(BeforeClose)


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       DocListener                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// (多)

DocListener::DocListener(DocSubject* pDoc)
{
	if (!pDoc) {
		pDoc = EditDoc::GetInstance(0); //$$ インチキ
	}
	assert(pDoc);
	Listen(pDoc);
}

DocListener::~DocListener()
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      DocListenerEx                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "doc/EditDoc.h"

EditDoc* DocListenerEx::GetListeningDoc() const
{
	return static_cast<EditDoc*>(DocListener::GetListeningDoc());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ProgressSubject                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void ProgressSubject::NotifyProgress(size_t nPer)
{
	size_t n = GetListenerCount();
	for (size_t i=0; i<n; ++i) {
		GetListener(i)->OnProgress(nPer);
	}
}

