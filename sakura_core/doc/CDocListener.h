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
enum class SaveResultType {
	OK,
	Failure,
	Interrupt,	// 中断された
	LoseSome,	// 文字の一部が失われた
};

//###
enum class LoadResultType {
	OK,
	Failure,
	Interrupt,		// 中断された
	LoseSome,		// 文字の一部が失われた

	// 特殊
	NoImplement,	// 実装無し
};

//###
enum class CallbackResultType {
	Continue,	// 続ける
	Interrupt,	// 中断
};

//###
struct LoadInfo {
	// 入力
	FilePath	cFilePath;
	ECodeType	eCharCode;
	bool		bViewMode;
	bool		bWritableNoMsg; //<! 書き込み禁止メッセージを表示しない
	TypeConfigNum	nType;

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
		const FilePath&	_cFilePath,
		ECodeType			_eCodeType,
		bool				_bReadOnly,
		TypeConfigNum		_nType = TypeConfigNum(-1)
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
	FilePath	cFilePath;	// 保存ファイル名
	ECodeType	eCharCode;	// 保存文字コードセット
	bool		bBomExist;	// 保存時BOM付加
	bool		bChgCodeSet;// 文字コードセット変更	2013/5/19 Uchi
	Eol		cEol;		// 保存改行コード

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
		const FilePath& _cFilePath,
		ECodeType _eCodeType,
		const Eol& _cEol,
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
	CallbackResultType NotifyCheckLoad	(LoadInfo* pLoadInfo);
	void NotifyBeforeLoad			(LoadInfo* sLoadInfo);
	LoadResultType NotifyLoad		(const LoadInfo& sLoadInfo);
	void NotifyLoading				(int nPer);
	void NotifyAfterLoad			(const LoadInfo& sLoadInfo);
	void NotifyFinalLoad			(LoadResultType eLoadResult);

	// セーブ前後
	CallbackResultType NotifyCheckSave	(SaveInfo* pSaveInfo);
	CallbackResultType NotifyPreBeforeSave(SaveInfo* pSaveInfo);
	void NotifyBeforeSave			(const SaveInfo& sSaveInfo);
	void NotifySave					(const SaveInfo& sSaveInfo);
	void NotifySaving				(int nPer);
	void NotifyAfterSave			(const SaveInfo& sSaveInfo);
	void NotifyFinalSave			(SaveResultType eSaveResult);

	// クローズ前後
	CallbackResultType NotifyBeforeClose();
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
	virtual CallbackResultType	OnCheckLoad	(LoadInfo* pLoadInfo)		{ return CallbackResultType::Continue; }	// 本当にロードを行うかの判定を行う
	virtual void			OnBeforeLoad(LoadInfo* sLoadInfo)			{ return ; }	// ロード事前処理
	virtual LoadResultType		OnLoad		(const LoadInfo& sLoadInfo) { return LoadResultType::NoImplement; }	// ロード処理
	virtual void			OnLoading	(int nPer)						{ return ; }	// ロード処理の経過情報を受信
	virtual void			OnAfterLoad	(const LoadInfo& sLoadInfo) 	{ return ; }	// ロード事後処理
	virtual void			OnFinalLoad	(LoadResultType eLoadResult)	{ return ; }	// ロードフローの最後に必ず呼ばれる

	// セーブ前後
	virtual CallbackResultType OnCheckSave	(SaveInfo* pSaveInfo)		{ return CallbackResultType::Continue; }	// 本当にセーブを行うかの判定を行う
	virtual CallbackResultType OnPreBeforeSave	(SaveInfo* pSaveInfo)	{ return CallbackResultType::Continue; }	// セーブ事前おまけ処理 ($$ 仮)
	virtual void			OnBeforeSave(const SaveInfo& sSaveInfo)		{ return ; }	// セーブ事前処理
	virtual void			OnSave		(const SaveInfo& sSaveInfo)		{ return ; }	// セーブ処理
	virtual void			OnSaving	(int nPer)						{ return ; }	// セーブ処理の経過情報を受信
	virtual void			OnAfterSave	(const SaveInfo& sSaveInfo)		{ return ; }	// セーブ事後処理
	virtual void			OnFinalSave	(SaveResultType eSaveResult)	{ return ; }	// セーブフローの最後に必ず呼ばれる

	// クローズ前後
	virtual CallbackResultType OnBeforeClose()							{ return CallbackResultType::Continue; }
};

// GetListeningDocの利便性をアップ
class EditDoc;
class DocListenerEx : public DocListener {
public:
	DocListenerEx(DocSubject* pcDoc = NULL) : DocListener(pcDoc) { }
	EditDoc* GetListeningDoc() const;
};


#include <exception>
class FlowInterruption : public std::exception {
public:
	const char* what() const throw() { return "FlowInterruption"; }
};

