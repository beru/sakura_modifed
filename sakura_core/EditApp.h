#pragma once

#include "util/design_template.h"
#include "uiparts/SoundSet.h"
#include "uiparts/ImageListMgr.h"
class EditDoc;
class EditWnd;
class LoadAgent;
class SaveAgent;
class VisualProgress;
class MruListener;
class SMacroMgr;
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
	HINSTANCE GetAppInstance() const { return hInst; }	// インスタンスハンドル取得

	// ウィンドウ情報
	EditWnd* GetEditWindow() { return pEditWnd; }		// ウィンドウ取得

	EditDoc*		GetDocument() { return pEditDoc; }
	ImageListMgr&	GetIcons() { return icons; }

	bool OpenPropertySheet(int nPageNum);
	bool OpenPropertySheetTypes(int nPageNum, TypeConfigNum nSettingType);

public:
	HINSTANCE			hInst;

	// ドキュメント
	EditDoc*			pEditDoc;

	// ウィンドウ
	EditWnd*			pEditWnd;

	// IO管理
	LoadAgent*			pLoadAgent;
	SaveAgent*			pSaveAgent;
	VisualProgress*		pVisualProgress;

	// その他ヘルパ
	MruListener*		pMruListener;		// MRU管理
	SMacroMgr*			pSMacroMgr;			// マクロ管理
private:
	PropertyManager*	pPropertyManager;	// プロパティ管理
public:
	GrepAgent*			pGrepAgent;			// GREPモード
	SoundSet			soundSet;			// サウンド管理

	// GUIオブジェクト
	ImageListMgr		icons;				// Image List
};


// WM_QUIT検出例外
class AppExitException : public std::exception {
public:
	const char* what() const throw() { return "AppExitException"; }
};

