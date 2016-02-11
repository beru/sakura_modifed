/*
	Copyright (C) 2007, kobake

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

// 2007.10.23 kobake 作成

#include "util/design_template.h"
#include "uiparts/CSoundSet.h"
#include "uiparts/CImageListMgr.h"
class EditDoc;
class EditWnd;
class LoadAgent;
class SaveAgent;
class VisualProgress;
class MruListener;
class CSMacroMgr;
class PropertyManager;
class GrepAgent;
enum EFunctionCode;

// エディタ部分アプリケーションクラス。CNormalProcess1個につき、1個存在。
class EditApp : public TSingleton<EditApp> {
	friend class TSingleton<EditApp>;
	EditApp() {}
	virtual ~EditApp();

public:
	void Create(HINSTANCE hInst, int);

	// モジュール情報
	HINSTANCE GetAppInstance() const { return m_hInst; }	// インスタンスハンドル取得

	// ウィンドウ情報
	EditWnd* GetEditWindow() { return m_pcEditWnd; }		// ウィンドウ取得

	EditDoc*		GetDocument() { return m_pcEditDoc; }
	ImageListMgr&	GetIcons() { return m_cIcons; }

	bool OpenPropertySheet(int nPageNum);
	bool OpenPropertySheetTypes(int nPageNum, CTypeConfig nSettingType);

public:
	HINSTANCE			m_hInst;

	// ドキュメント
	EditDoc*			m_pcEditDoc;

	// ウィンドウ
	EditWnd*			m_pcEditWnd;

	// IO管理
	LoadAgent*			m_pcLoadAgent;
	SaveAgent*			m_pcSaveAgent;
	VisualProgress*	m_pcVisualProgress;

	// その他ヘルパ
	MruListener*		m_pcMruListener;		// MRU管理
	CSMacroMgr*			m_pcSMacroMgr;			// マクロ管理
private:
	PropertyManager*	m_pcPropertyManager;	// プロパティ管理
public:
	GrepAgent*			m_pcGrepAgent;			// GREPモード
	SoundSet			m_cSoundSet;			// サウンド管理

	// GUIオブジェクト
	ImageListMgr		m_cIcons;				// Image List
};


// WM_QUIT検出例外
class AppExitException : public std::exception {
public:
	const char* what() const throw() { return "AppExitException"; }
};

