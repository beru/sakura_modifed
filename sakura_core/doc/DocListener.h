/*
	Observer�p�^�[����EditDoc�����ŁB
	DocSubject�͊ώ@����ACDocListner�͊ώ@���s���B
	�ώ@�̊J�n�� DocListener::Listen �ōs���B

	$Note:
		Listener (Observer) �� Subject �̃����[�V�����Ǘ���
		�W�F�l���b�N�Ȕėp���W���[���ɕ����ł���B
*/
#pragma once

class DocListener;
#include "basis/MyString.h"
#include "charset/charset.h"
#include "Eol.h"
#include "types/Type.h"
#include "util/relation_tool.h"

//###
enum class SaveResultType {
	OK,
	Failure,
	Interrupt,	// ���f���ꂽ
	LoseSome,	// �����̈ꕔ������ꂽ
};

//###
enum class LoadResultType {
	OK,
	Failure,
	Interrupt,		// ���f���ꂽ
	LoseSome,		// �����̈ꕔ������ꂽ

	// ����
	NoImplement,	// ��������
};

//###
enum class CallbackResultType {
	Continue,	// ������
	Interrupt,	// ���f
};

//###
struct LoadInfo {
	// ����
	FilePath		filePath;
	EncodingType	eCharCode;
	bool			bViewMode;
	bool			bWritableNoMsg; // �������݋֎~���b�Z�[�W��\�����Ȃ�
	TypeConfigNum	nType;

	// ���[�h
	bool		bRequestReload;	// �����[�h�v��

	// �o��
	bool		bOpened;

	LoadInfo()
		:
		filePath(_T("")),
		eCharCode(CODE_AUTODETECT),
		bViewMode(false),
		bWritableNoMsg(false),
		nType(-1),
		bRequestReload(false),
		bOpened(false)
	{
	}

	LoadInfo(
		const FilePath&	filePath,
		EncodingType	codeType,
		bool			bReadOnly,
		TypeConfigNum	nType = TypeConfigNum(-1)
	)
		:
		filePath(filePath),
		eCharCode(codeType),
		bViewMode(bReadOnly),
		bWritableNoMsg(false),
		nType(nType),
		bRequestReload(false),
		bOpened(false)
	{
	}

	// �t�@�C���p�X�̔�r
	bool IsSamePath(LPCTSTR pszPath) const;
};

struct SaveInfo {
	FilePath		filePath;	// �ۑ��t�@�C����
	EncodingType	eCharCode;	// �ۑ������R�[�h�Z�b�g
	bool			bBomExist;	// �ۑ���BOM�t��
	bool			bChgCodeSet;// �����R�[�h�Z�b�g�ύX
	Eol				eol;		// �ۑ����s�R�[�h

	// ���[�h
	bool		bOverwriteMode;	// �㏑���v��

	SaveInfo()
		:
		filePath(_T("")),
		eCharCode(CODE_AUTODETECT),
		bBomExist(false),
		bChgCodeSet(false),
		eol(EolType::None),
		bOverwriteMode(false)
	{
	}

	SaveInfo(
		const FilePath&	filePath,
		EncodingType	codeType,
		const Eol&		eol,
		bool			bBomExist
	)
		: 
		filePath(filePath),
		eCharCode(codeType),
		bBomExist(bBomExist),
		bChgCodeSet(false),
		eol(eol),
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
	void NotifyProgress(size_t nPer);
};

// 1��ProgressSubject���E�H�b�`����
class ProgressListener : public ListenerT<ProgressSubject> {
public:
	virtual ~ProgressListener() {}
	virtual void OnProgress(size_t nPer) = 0;
};

// Subject�͕�����Listener����ώ@�����
class DocSubject : public SubjectT<DocListener> {
public:
	virtual ~DocSubject();

	// ���[�h�O��
	CallbackResultType NotifyCheckLoad	(LoadInfo* pLoadInfo);
	void NotifyBeforeLoad			(LoadInfo* pLoadInfo);
	LoadResultType NotifyLoad		(const LoadInfo& loadInfo);
	void NotifyLoading				(int nPer);
	void NotifyAfterLoad			(const LoadInfo& loadInfo);
	void NotifyFinalLoad			(LoadResultType eLoadResult);

	// �Z�[�u�O��
	CallbackResultType NotifyCheckSave	(SaveInfo* pSaveInfo);
	CallbackResultType NotifyPreBeforeSave(SaveInfo* pSaveInfo);
	void NotifyBeforeSave			(const SaveInfo& saveInfo);
	void NotifySave					(const SaveInfo& saveInfo);
	void NotifySaving				(int nPer);
	void NotifyAfterSave			(const SaveInfo& saveInfo);
	void NotifyFinalSave			(SaveResultType eSaveResult);

	// �N���[�Y�O��
	CallbackResultType NotifyBeforeClose();
};

// Listener��1��Subject���ώ@����
class DocListener : public ListenerT<DocSubject> {
public:
	DocListener(DocSubject* pDoc = nullptr);
	virtual ~DocListener();

	// -- -- ���� -- -- //
	DocSubject* GetListeningDoc() const { return GetListeningSubject(); }

	// -- -- �e��C�x���g -- -- //
	// ���[�h�O��
	virtual CallbackResultType	OnCheckLoad	(LoadInfo* pLoadInfo)		{ return CallbackResultType::Continue; }	// �{���Ƀ��[�h���s�����̔�����s��
	virtual void				OnBeforeLoad(LoadInfo* loadInfo)		{ return ; }	// ���[�h���O����
	virtual LoadResultType		OnLoad		(const LoadInfo& loadInfo)	{ return LoadResultType::NoImplement; }	// ���[�h����
	virtual void			OnLoading	(size_t nPer)						{ return ; }	// ���[�h�����̌o�ߏ�����M
	virtual void			OnAfterLoad	(const LoadInfo& loadInfo) 		{ return ; }	// ���[�h���㏈��
	virtual void			OnFinalLoad	(LoadResultType eLoadResult)	{ return ; }	// ���[�h�t���[�̍Ō�ɕK���Ă΂��

	// �Z�[�u�O��
	virtual CallbackResultType OnCheckSave	(SaveInfo* pSaveInfo)		{ return CallbackResultType::Continue; }	// �{���ɃZ�[�u���s�����̔�����s��
	virtual CallbackResultType OnPreBeforeSave	(SaveInfo* pSaveInfo)	{ return CallbackResultType::Continue; }	// �Z�[�u���O���܂����� ($$ ��)
	virtual void			OnBeforeSave(const SaveInfo& saveInfo)		{ return ; }	// �Z�[�u���O����
	virtual void			OnSave		(const SaveInfo& saveInfo)		{ return ; }	// �Z�[�u����
	virtual void			OnSaving	(size_t nPer)						{ return ; }	// �Z�[�u�����̌o�ߏ�����M
	virtual void			OnAfterSave	(const SaveInfo& saveInfo)		{ return ; }	// �Z�[�u���㏈��
	virtual void			OnFinalSave	(SaveResultType eSaveResult)	{ return ; }	// �Z�[�u�t���[�̍Ō�ɕK���Ă΂��

	// �N���[�Y�O��
	virtual CallbackResultType OnBeforeClose()							{ return CallbackResultType::Continue; }
};

// GetListeningDoc�̗��֐����A�b�v
class EditDoc;
class DocListenerEx : public DocListener {
public:
	DocListenerEx(DocSubject* pDoc = nullptr) : DocListener(pDoc) { }
	EditDoc* GetListeningDoc() const;
};


#include <exception>
class FlowInterruption : public std::exception {
public:
	const char* what() const throw() { return "FlowInterruption"; }
};

