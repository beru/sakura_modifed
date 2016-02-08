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
	SAVED_INTERRUPT,	// 中断された
	SAVED_LOSESOME,		// 文字の一部が失われた
};

//###
enum ELoadResult {
	LOADED_OK,
	LOADED_FAILURE,
	LOADED_INTERRUPT,	// 中断された
	LOADED_LOSESOME,	// 文字の一部が失われた

	// 特殊
	LOADED_NOIMPLEMENT,	// 実装無し
};

//###
enum ECallbackResult {
	CALLBACK_CONTINUE,	// 続ける
	CALLBACK_INTERRUPT,	// 中断
};

//###
struct LoadInfo {
	// 入力
	CFilePath	cFilePath;
	ECodeType	eCharCode;
	bool		bViewMode;
	bool		bWritableNoMsg; //<! 書き込み禁止メッセージを表示しない
	CTypeConfig	nType;

	// モード
	bool		bRequestReload;	// リロード要求

	// 出力
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

	// ファイルパスの比較
	bool IsSamePath(LPCTSTR pszPath) const;
};

struct SaveInfo {
	CFilePath	cFilePath;	// 保存ファイル名
	ECodeType	eCharCode;	// 保存文字コードセット
	bool		bBomExist;	// 保存時BOM付加
	bool		bChgCodeSet;// 文字コードセット変更	2013/5/19 Uchi
	CEol		cEol;		// 保存改行コード

	// モード
	bool		bOverwriteMode;	// 上書き要求

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

	// ファイルパスの比較
	bool IsSamePath(LPCTSTR pszPath) const;
};


class ProgressListener;

// 複数のProgressSubjectからウォッチされる
class ProgressSubject : public SubjectT<ProgressListener> {
public:
	virtual ~ProgressSubject() {}
	void NotifyProgress(int nPer);
};

// 1つのProgressSubjectをウォッチする
class ProgressListener : public ListenerT<ProgressSubject> {
public:
	virtual ~ProgressListener() {}
	virtual void OnProgress(int nPer) = 0;
};

// Subjectは複数のListenerから観察される
class DocSubject : public SubjectT<DocListener> {
public:
	virtual ~DocSubject();

	// ロード前後
	ECallbackResult NotifyCheckLoad	(LoadInfo* pLoadInfo);
	void NotifyBeforeLoad			(LoadInfo* sLoadInfo);
	ELoadResult NotifyLoad			(const LoadInfo& sLoadInfo);
	void NotifyLoading				(int nPer);
	void NotifyAfterLoad			(const LoadInfo& sLoadInfo);
	void NotifyFinalLoad			(ELoadResult eLoadResult);

	// セーブ前後
	ECallbackResult NotifyCheckSave	(SaveInfo* pSaveInfo);
	ECallbackResult NotifyPreBeforeSave(SaveInfo* pSaveInfo);
	void NotifyBeforeSave			(const SaveInfo& sSaveInfo);
	void NotifySave					(const SaveInfo& sSaveInfo);
	void NotifySaving				(int nPer);
	void NotifyAfterSave			(const SaveInfo& sSaveInfo);
	void NotifyFinalSave			(ESaveResult eSaveResult);

	// クローズ前後
	ECallbackResult NotifyBeforeClose();
};

// Listenerは1つのSubjectを観察する
class DocListener : public ListenerT<DocSubject> {
public:
	DocListener(DocSubject* pcDoc = NULL);
	virtual ~DocListener();

	// -- -- 属性 -- -- //
	DocSubject* GetListeningDoc() const { return GetListeningSubject(); }

	// -- -- 各種イベント -- -- //
	// ロード前後
	virtual ECallbackResult	OnCheckLoad	(LoadInfo* pLoadInfo)		{ return CALLBACK_CONTINUE; }	// 本当にロードを行うかの判定を行う
	virtual void			OnBeforeLoad(LoadInfo* sLoadInfo)		{ return ; }	// ロード事前処理
	virtual ELoadResult		OnLoad		(const LoadInfo& sLoadInfo) { return LOADED_NOIMPLEMENT; }	// ロード処理
	virtual void			OnLoading	(int nPer)					{ return ; }	// ロード処理の経過情報を受信
	virtual void			OnAfterLoad	(const LoadInfo& sLoadInfo) { return ; }	// ロード事後処理
	virtual void			OnFinalLoad	(ELoadResult eLoadResult)	{ return ; }	// ロードフローの最後に必ず呼ばれる

	// セーブ前後
	virtual ECallbackResult OnCheckSave	(SaveInfo* pSaveInfo)		{ return CALLBACK_CONTINUE; }	// 本当にセーブを行うかの判定を行う
	virtual ECallbackResult OnPreBeforeSave	(SaveInfo* pSaveInfo)	{ return CALLBACK_CONTINUE; }	// セーブ事前おまけ処理 ($$ 仮)
	virtual void			OnBeforeSave(const SaveInfo& sSaveInfo) { return ; }	// セーブ事前処理
	virtual void			OnSave		(const SaveInfo& sSaveInfo) { return ; }	// セーブ処理
	virtual void			OnSaving	(int nPer)					{ return ; }	// セーブ処理の経過情報を受信
	virtual void			OnAfterSave	(const SaveInfo& sSaveInfo) { return ; }	// セーブ事後処理
	virtual void			OnFinalSave	(ESaveResult eSaveResult)	{ return ; }	// セーブフローの最後に必ず呼ばれる

	// クローズ前後
	virtual ECallbackResult OnBeforeClose()							{ return CALLBACK_CONTINUE; }
};

// GetListeningDocの利便性をアップ
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

