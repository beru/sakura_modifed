/*
	Observer�p�^�[����EditDoc�����ŁB
	DocSubject�͊ώ@����ACDocListner�͊ώ@���s���B
	�ώ@�̊J�n�� DocListener::Listen �ōs���B

	$Note:
		Listener (Observer) �� Subject �̃����[�V�����Ǘ���
		�W�F�l���b�N�Ȕėp���W���[���ɕ����ł���B
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

//######��
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
// (��)

DocListener::DocListener(DocSubject* pDoc)
{
	if (!pDoc) {
		pDoc = EditDoc::GetInstance(0); //$$ �C���`�L
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

