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
	Copyright (C) 2013, Uchi

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
#pragma once

class DocListener;
#include "basis/CMyString.h"
#include "charset/charset.h"
#include "CEol.h"
#include "types/CType.h"
#include "util/relation_tool.h"

//###
enum ESaveResult {
	SAVED_OK,
	SAVED_FAILURE,
	SAVED_INTERRUPT,	// ���f���ꂽ
	SAVED_LOSESOME,		// �����̈ꕔ������ꂽ
};

//###
enum ELoadResult {
	LOADED_OK,
	LOADED_FAILURE,
	LOADED_INTERRUPT,	// ���f���ꂽ
	LOADED_LOSESOME,	// �����̈ꕔ������ꂽ

	// ����
	LOADED_NOIMPLEMENT,	// ��������
};

//###
enum ECallbackResult {
	CALLBACK_CONTINUE,	// ������
	CALLBACK_INTERRUPT,	// ���f
};

//###
struct LoadInfo {
	// ����
	CFilePath	cFilePath;
	ECodeType	eCharCode;
	bool		bViewMode;
	bool		bWritableNoMsg; //<! �������݋֎~���b�Z�[�W��\�����Ȃ�
	CTypeConfig	nType;

	// ���[�h
	bool		bRequestReload;	// �����[�h�v��

	// �o��
	bool		bOpened;

	LoadInfo()
		:
		cFilePath(_T("")),
		eCharCode(CODE_AUTODETECT),
		bViewMode(false),
		bWritableNoMsg(false),
		nType(-1),
		bRequestReload(false),
		bOpened(false)
	{
	}

	LoadInfo(
		const CFilePath&	_cFilePath,
		ECodeType			_eCodeType,
		bool				_bReadOnly,
		CTypeConfig			_nType = CTypeConfig(-1)
	)
		:
		cFilePath(_cFilePath),
		eCharCode(_eCodeType),
		bViewMode(_bReadOnly),
		bWritableNoMsg(false),
		nType(_nType),
		bRequestReload(false),
		bOpened(false)
	{
	}

	// �t�@�C���p�X�̔�r
	bool IsSamePath(LPCTSTR pszPath) const;
};

struct SaveInfo {
	CFilePath	cFilePath;	// �ۑ��t�@�C����
	ECodeType	eCharCode;	// �ۑ������R�[�h�Z�b�g
	bool		bBomExist;	// �ۑ���BOM�t��
	bool		bChgCodeSet;// �����R�[�h�Z�b�g�ύX	2013/5/19 Uchi
	CEol		cEol;		// �ۑ����s�R�[�h

	// ���[�h
	bool		bOverwriteMode;	// �㏑���v��

	SaveInfo()
		:
		cFilePath(_T("")),
		eCharCode(CODE_AUTODETECT),
		bBomExist(false),
		bChgCodeSet(false),
		cEol(EOL_NONE),
		bOverwriteMode(false)
	{
	}

	SaveInfo(
		const CFilePath& _cFilePath,
		ECodeType _eCodeType,
		const CEol& _cEol,
		bool _bBomExist
	)
		: 
		cFilePath(_cFilePath),
		eCharCode(_eCodeType),
		bBomExist(_bBomExist),
		bChgCodeSet(false),
		cEol(_cEol),
		bOverwriteMode(false)
	{
	}

	// �t�@�C���p�X�̔�r
	bool IsSamePath(LPCTSTR pszPath) const;
};


class ProgressListener;

// ������ProgressSubject����E�H�b�`�����
class ProgressSubject : public SubjectT<ProgressListener> {
public:
	virtual ~ProgressSubject() {}
	void NotifyProgress(int nPer);
};

// 1��ProgressSubject���E�H�b�`����
class ProgressListener : public ListenerT<ProgressSubject> {
public:
	virtual ~ProgressListener() {}
	virtual void OnProgress(int nPer) = 0;
};

// Subject�͕�����Listener����ώ@�����
class DocSubject : public SubjectT<DocListener> {
public:
	virtual ~DocSubject();

	// ���[�h�O��
	ECallbackResult NotifyCheckLoad	(LoadInfo* pLoadInfo);
	void NotifyBeforeLoad			(LoadInfo* sLoadInfo);
	ELoadResult NotifyLoad			(const LoadInfo& sLoadInfo);
	void NotifyLoading				(int nPer);
	void NotifyAfterLoad			(const LoadInfo& sLoadInfo);
	void NotifyFinalLoad			(ELoadResult eLoadResult);

	// �Z�[�u�O��
	ECallbackResult NotifyCheckSave	(SaveInfo* pSaveInfo);
	ECallbackResult NotifyPreBeforeSave(SaveInfo* pSaveInfo);
	void NotifyBeforeSave			(const SaveInfo& sSaveInfo);
	void NotifySave					(const SaveInfo& sSaveInfo);
	void NotifySaving				(int nPer);
	void NotifyAfterSave			(const SaveInfo& sSaveInfo);
	void NotifyFinalSave			(ESaveResult eSaveResult);

	// �N���[�Y�O��
	ECallbackResult NotifyBeforeClose();
};

// Listener��1��Subject���ώ@����
class DocListener : public ListenerT<DocSubject> {
public:
	DocListener(DocSubject* pcDoc = NULL);
	virtual ~DocListener();

	// -- -- ���� -- -- //
	DocSubject* GetListeningDoc() const { return GetListeningSubject(); }

	// -- -- �e��C�x���g -- -- //
	// ���[�h�O��
	virtual ECallbackResult	OnCheckLoad	(LoadInfo* pLoadInfo)		{ return CALLBACK_CONTINUE; }	// �{���Ƀ��[�h���s�����̔�����s��
	virtual void			OnBeforeLoad(LoadInfo* sLoadInfo)		{ return ; }	// ���[�h���O����
	virtual ELoadResult		OnLoad		(const LoadInfo& sLoadInfo) { return LOADED_NOIMPLEMENT; }	// ���[�h����
	virtual void			OnLoading	(int nPer)					{ return ; }	// ���[�h�����̌o�ߏ�����M
	virtual void			OnAfterLoad	(const LoadInfo& sLoadInfo) { return ; }	// ���[�h���㏈��
	virtual void			OnFinalLoad	(ELoadResult eLoadResult)	{ return ; }	// ���[�h�t���[�̍Ō�ɕK���Ă΂��

	// �Z�[�u�O��
	virtual ECallbackResult OnCheckSave	(SaveInfo* pSaveInfo)		{ return CALLBACK_CONTINUE; }	// �{���ɃZ�[�u���s�����̔�����s��
	virtual ECallbackResult OnPreBeforeSave	(SaveInfo* pSaveInfo)	{ return CALLBACK_CONTINUE; }	// �Z�[�u���O���܂����� ($$ ��)
	virtual void			OnBeforeSave(const SaveInfo& sSaveInfo) { return ; }	// �Z�[�u���O����
	virtual void			OnSave		(const SaveInfo& sSaveInfo) { return ; }	// �Z�[�u����
	virtual void			OnSaving	(int nPer)					{ return ; }	// �Z�[�u�����̌o�ߏ�����M
	virtual void			OnAfterSave	(const SaveInfo& sSaveInfo) { return ; }	// �Z�[�u���㏈��
	virtual void			OnFinalSave	(ESaveResult eSaveResult)	{ return ; }	// �Z�[�u�t���[�̍Ō�ɕK���Ă΂��

	// �N���[�Y�O��
	virtual ECallbackResult OnBeforeClose()							{ return CALLBACK_CONTINUE; }
};

// GetListeningDoc�̗��֐����A�b�v
class EditDoc;
class DocListenerEx : public DocListener {
public:
	DocListenerEx(DocSubject* pcDoc = NULL) : DocListener(pcDoc) { }
	EditDoc* GetListeningDoc() const;
};


#include <exception>
class CFlowInterruption : public std::exception {
public:
	const char* what() const throw() { return "CFlowInterruption"; }
};

