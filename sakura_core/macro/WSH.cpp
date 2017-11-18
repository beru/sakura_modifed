/*!	@file
	@brief WSH Handler
*/
#include "StdAfx.h"
#include <process.h> // _beginthreadex
#ifdef __MINGW32__
#define INITGUID 1
#endif
#include <ObjBase.h>
#include <InitGuid.h>
#include <ShlDisp.h>
#include "macro/WSH.h"
#include "macro/IfObj.h"
#include "window/EditWnd.h"
#include "util/os.h"
#include "util/module.h"
#include "util/window.h"	// BlockingHook
#include "dlg/DlgCancel.h"
#include "sakura_rc.h"
#ifndef SCRIPT_E_REPORTED
#define	SCRIPT_E_REPORTED	0x80020101L	// ActivScp.h(VS2012)と同じ様な形に変更
#endif

class WSHSite :
	public IActiveScriptSite,
	public IActiveScriptSiteWindow
{
private:
	WSHClient* Client;
	ITypeInfo* typeInfo;
	ULONG refCount;
public:
	WSHSite(WSHClient *AClient)
		:
		Client(AClient),
		refCount(0)
	{
	}

	virtual ULONG _stdcall AddRef() {
		return ++refCount;
	}

	virtual ULONG _stdcall Release() {
		if (--refCount == 0) {
			delete this;
			return 0;
		}
		return refCount;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
	    /* [in] */ REFIID iid,
	    /* [out] */ void ** ppvObject)
	{
		*ppvObject = NULL;

		if (iid == IID_IActiveScriptSiteWindow) {
			*ppvObject = static_cast<IActiveScriptSiteWindow*>(this);
			++refCount;
			return S_OK;
		}

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetLCID(
	    /* [out] */ LCID *plcid) 
	{ 
#ifdef TEST
		cout << "GetLCID" << endl;
#endif
		return E_NOTIMPL; // システムデフォルトを使用
	}

	virtual HRESULT STDMETHODCALLTYPE GetItemInfo(
	    /* [in] */ LPCOLESTR pstrName,
	    /* [in] */ DWORD dwReturnMask,
	    /* [out] */ IUnknown **ppiunkItem,
	    /* [out] */ ITypeInfo **ppti) 
	{
#ifdef TEST
		wcout << L"GetItemInfo:" << pstrName << endl;
#endif
		// 指定された名前のインタフェースオブジェクトを検索
		const WSHClient::List& objects = Client->GetInterfaceObjects();
		for (auto it=objects.begin(); it!=objects.end(); ++it) {
			// Nov. 10, 2003 FILE Win9Xでは、[lstrcmpiW]が無効のため、[_wcsicmp]に修正
			if (_wcsicmp(pstrName, (*it)->name.c_str()) == 0) {
				if (dwReturnMask & SCRIPTINFO_IUNKNOWN) {
					(*ppiunkItem) = *it;
					(*ppiunkItem)->AddRef();
				}
				if (dwReturnMask & SCRIPTINFO_ITYPEINFO) {
					(*it)->GetTypeInfo(0, 0, ppti);
				}
				return S_OK;
			}
		}
		return TYPE_E_ELEMENTNOTFOUND;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDocVersionString(
	    /* [out] */ BSTR *pbstrVersion) 
	{ 
#ifdef TEST
		cout << "GetDocVersionString" << endl;
#endif
		return E_NOTIMPL; 
	}

	virtual HRESULT STDMETHODCALLTYPE OnScriptTerminate(
	    /* [in] */ const VARIANT *pvarResult,
	    /* [in] */ const EXCEPINFO *pexcepinfo) 
	{ 
#ifdef TEST
		cout << "OnScriptTerminate" << endl;
#endif
		return S_OK; 
	}

	virtual HRESULT STDMETHODCALLTYPE OnStateChange(
	    /* [in] */ SCRIPTSTATE ssScriptState) 
	{ 
#ifdef TEST
		cout << "OnStateChange" << endl;
#endif
		return S_OK; 
	}

	virtual HRESULT STDMETHODCALLTYPE OnScriptError(
	  /* [in] */ IActiveScriptError* pscripterror
		)
	{
		EXCEPINFO Info;
		if (pscripterror->GetExceptionInfo(&Info) == S_OK) {
			DWORD Context;
			ULONG Line;
			LONG Pos;
			if (!Info.bstrDescription) {
				Info.bstrDescription = SysAllocString(LSW(STR_ERR_CWSH09));
			}
			if (pscripterror->GetSourcePosition(&Context, &Line, &Pos) == S_OK) {
				std::vector<wchar_t> msgBuf(SysStringLen(Info.bstrDescription) + 128);
				wchar_t* message = &msgBuf[0];
				const wchar_t* szDesc = Info.bstrDescription;
				auto_sprintf(message, L"[Line %d] %ls", Line + 1, szDesc);
				SysReAllocString(&Info.bstrDescription, message);
			}
			Client->Error(Info.bstrDescription, Info.bstrSource);
			SysFreeString(Info.bstrSource);
			SysFreeString(Info.bstrDescription);
			SysFreeString(Info.bstrHelpFile);
		}
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnEnterScript() {
#ifdef TEST
		cout << "OnEnterScript" << endl;
#endif
		return S_OK; 
	}

	virtual HRESULT STDMETHODCALLTYPE OnLeaveScript() {
#ifdef TEST
		cout << "OnLeaveScript" << endl;
#endif
		return S_OK; 
	}

	virtual HRESULT __stdcall GetWindow(
	    /* [out] */ HWND *phwnd)
	{
		*phwnd = EditWnd::getInstance().splitterWnd.GetHwnd();
		return S_OK;
	}

	virtual HRESULT __stdcall EnableModeless(
	    /* [in] */ BOOL fEnable)
	{
		return S_OK;
	}
};

// implementation

WSHClient::WSHClient(
	const wchar_t* AEngine,
	ScriptErrorHandler AErrorHandler,
	void *AData
	)
	: 
	onError(AErrorHandler),
	data(AData),
	isValid(false),
	engine(NULL)
{ 
	// インジェクション対策としてEXEのフォルダに移動する
	CurrentDirectoryBackupPoint dirBack;
	ChangeCurrentDirectoryToExeDir();
	
	CLSID ClassID;
	if (CLSIDFromProgID(AEngine, &ClassID) != S_OK) {
		Error(LSW(STR_ERR_CWSH01));
	}else {
		if (CoCreateInstance(ClassID, 0, CLSCTX_INPROC_SERVER, IID_IActiveScript, reinterpret_cast<void **>(&engine)) != S_OK) {
			Error(LSW(STR_ERR_CWSH02));
		}else {
			IActiveScriptSite* site = new WSHSite(this);
			if (engine->SetScriptSite(site) != S_OK) {
				delete site;
				Error(LSW(STR_ERR_CWSH03));
			}else {
				isValid = true;
			}
		}
	}
}

WSHClient::~WSHClient()
{
	// インタフェースオブジェクトを解放
	for (auto it=ifObjArr.begin(); it!=ifObjArr.end(); ++it) {
		(*it)->Release();
	}
	
	if (engine) {
		engine->Release();
	}
}

// AbortMacroProcのパラメータ構造体
struct AbortMacroParam {
	HANDLE hEvent;
	IActiveScript* pEngine;				// ActiveScript
	int nCancelTimer;
	EditView* view;
};

// WSHマクロ実行を中止するスレッド
static unsigned __stdcall AbortMacroProc(LPVOID lpParameter)
{
	AbortMacroParam* pParam = (AbortMacroParam*) lpParameter;

	// 停止ダイアログ表示前に数秒待つ
	if (::WaitForSingleObject(pParam->hEvent, pParam->nCancelTimer * 1000) == WAIT_TIMEOUT) {
		// 停止ダイアログ表示
		DEBUG_TRACE(_T("AbortMacro: Show Dialog\n"));

		MSG msg;
		DlgCancel dlgCancel;
		HWND hwndDlg = dlgCancel.DoModeless(G_AppInstance(), NULL, IDD_MACRORUNNING);	// エディタビジーでも表示できるよう、親を指定しない
		// ダイアログタイトルとファイル名を設定
		::SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)GSTR_APPNAME);
		::SendMessage(GetDlgItem(hwndDlg, IDC_STATIC_CMD),
			WM_SETTEXT, 0, (LPARAM)pParam->view->GetDocument().docFile.GetFilePath());
		
		bool bCanceled = false;
		for (;;) {
			DWORD dwResult = MsgWaitForMultipleObjects(1, &pParam->hEvent, FALSE, INFINITE, QS_ALLINPUT);
			if (dwResult == WAIT_OBJECT_0) {
				::SendMessage(dlgCancel.GetHwnd(), WM_CLOSE, 0, 0);
			}else if (dwResult == WAIT_OBJECT_0 + 1) {
				while (::PeekMessage(&msg , NULL , 0 , 0, PM_REMOVE)) {
					if (dlgCancel.GetHwnd() && ::IsDialogMessage(dlgCancel.GetHwnd(), &msg)) {
					}else {
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}
			}else {
				// MsgWaitForMultipleObjectsに与えたハンドルのエラー
				break;
			}
			if (!bCanceled && dlgCancel.IsCanceled()) {
				DEBUG_TRACE(_T("Canceld\n"));
				bCanceled = true;
				dlgCancel.CloseDialog(0);
			}
			if (!dlgCancel.GetHwnd()) {
				DEBUG_TRACE(_T("Close\n"));
				break;
			}
		}

		DEBUG_TRACE(_T("AbortMacro: Try Interrupt\n"));
		pParam->pEngine->InterruptScriptThread(SCRIPTTHREADID_BASE, NULL, 0);
		DEBUG_TRACE(_T("AbortMacro: Done\n"));
	}

	DEBUG_TRACE(_T("AbortMacro: Exit\n"));
	return 0;
}


bool WSHClient::Execute(const wchar_t* AScript)
{
	bool bRet = false;
	IActiveScriptParse *Parser;
	if (engine->QueryInterface(IID_IActiveScriptParse, reinterpret_cast<void **>(&Parser)) != S_OK) {
		Error(LSW(STR_ERR_CWSH04));
	}else {
		if (Parser->InitNew() != S_OK) {
			Error(LSW(STR_ERR_CWSH05));
		}else {
			bool bAddNamedItemError = false;

			for (auto it=ifObjArr.begin(); it!=ifObjArr.end(); ++it) {
				DWORD dwFlag = SCRIPTITEM_ISVISIBLE;

				if ((*it)->IsGlobal()) { dwFlag |= SCRIPTITEM_GLOBALMEMBERS; }

				if (engine->AddNamedItem((*it)->Name(), dwFlag) != S_OK) {
					bAddNamedItemError = true;
					Error(LSW(STR_ERR_CWSH06));
					break;
				}
			}
			if (!bAddNamedItemError) {
				// マクロ停止スレッドの起動
				AbortMacroParam sThreadParam;
				sThreadParam.pEngine = engine;
				sThreadParam.nCancelTimer = GetDllShareData().common.macro.nMacroCancelTimer;
				sThreadParam.view = (EditView*)data;

				HANDLE hThread = NULL;
				unsigned int nThreadId = 0;
				if (0 < sThreadParam.nCancelTimer) {
					sThreadParam.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
					hThread = (HANDLE)_beginthreadex(NULL, 0, AbortMacroProc, (LPVOID)&sThreadParam, 0, &nThreadId);
					DEBUG_TRACE(_T("Start AbortMacroProc 0x%08x\n"), nThreadId);
				}

				// マクロ実行
				if (engine->SetScriptState(SCRIPTSTATE_STARTED) != S_OK) {
					Error(LSW(STR_ERR_CWSH07));
				}else {
					HRESULT hr = Parser->ParseScriptText(AScript, 0, 0, 0, 0, 0, SCRIPTTEXT_ISVISIBLE, 0, 0);
					if (hr == SCRIPT_E_REPORTED) {
					/*
						IActiveScriptSite->OnScriptErrorに通知済み。
						中断メッセージが既に表示されてるはず。
					*/
					}else if (hr != S_OK) {
						Error(LSW(STR_ERR_CWSH08));
					}else {
						bRet = true;
					}
				}

				if (0 < sThreadParam.nCancelTimer) {
					::SetEvent(sThreadParam.hEvent);

					// マクロ停止スレッドの終了待ち
					DEBUG_TRACE(_T("Waiting for AbortMacroProc to finish\n"));
					::WaitForSingleObject(hThread, INFINITE); 
					::CloseHandle(hThread);
					::CloseHandle(sThreadParam.hEvent);
				}
			}
		}
		Parser->Release();
	}
	engine->Close();
	return bRet;
}

void WSHClient::Error(BSTR Description, BSTR Source)
{
	if (onError) {
		onError(Description, Source, data);
	}
}

void WSHClient::Error(const wchar_t* Description)
{
	BSTR S = SysAllocString(L"WSH");
	BSTR D = SysAllocString(Description);
	Error(D, S);
	SysFreeString(S);
	SysFreeString(D);
}

// インタフェースオブジェクトの追加
void WSHClient::AddInterfaceObject(IfObj* obj)
{
	if (!obj) return;
	ifObjArr.push_back(obj);
	obj->owner = this;
	obj->AddRef();
}

