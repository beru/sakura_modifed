/*!	@file
	@brief UxTheme 動的ロード

	UxTheme (Windows thmeme manager) への動的アクセスクラス
*/
#pragma once

#if defined(_MSC_VER) && _MSC_VER >= 1500
#include <vsstyle.h>
#else
typedef HANDLE HTHEME;
enum TABPARTS {
	TABP_TABITEM = 1,
	TABP_TABITEMLEFTEDGE = 2,
	TABP_TABITEMRIGHTEDGE = 3,
	TABP_TABITEMBOTHEDGE = 4,
	TABP_TOPTABITEM = 5,
	TABP_TOPTABITEMLEFTEDGE = 6,
	TABP_TOPTABITEMRIGHTEDGE = 7,
	TABP_TOPTABITEMBOTHEDGE = 8,
	TABP_PANE = 9,
	TABP_BODY = 10,
	TABP_AEROWIZARDBODY = 11,
};
enum TABITEMSTATES {
	TIS_NORMAL = 1,
	TIS_HOT = 2,
	TIS_SELECTED = 3,
	TIS_DISABLED = 4,
	TIS_FOCUSED = 5,
};
#endif
#include "DllHandler.h"
#include "util/design_template.h"

/*!
	@brief UxTheme 動的ロード

	UxTheme コンポーネントの動的ロードをサポートするクラス
*/
class UxTheme :
	public TSingleton<UxTheme>,
	public DllImp
{
	friend class TSingleton<UxTheme>;
	UxTheme();
	virtual ~UxTheme();

protected:
	bool bInitialized;

	bool InitThemeDll( TCHAR* str = NULL );
	virtual bool InitDllImp();
	virtual LPCTSTR GetDllNameImp(int nIndex);

protected:
	// UxTheme API Entry Points
	BOOL (WINAPI* pfnIsThemeActive)( VOID );
	HRESULT (WINAPI* pfnSetWindowTheme)( HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList );
	HTHEME (WINAPI* pfnOpenThemeData)( HWND hwnd, LPCWSTR pszClassList );
	HRESULT (WINAPI* pfnDrawThemeBackground)( HTHEME htheme, HDC hdc, int iPartId, int iStateId, RECT* prc, RECT* prcClip );
	HRESULT (WINAPI* pfnDrawThemeParentBackground)( HWND hwnd, HDC hdc, RECT* prc );
	HRESULT (WINAPI* pfnIsThemeBackgroundPartiallyTransparent)( HTHEME htheme, int iPartId, int iStateId );

public:
	// UxTheme API Wrapper Functions
	BOOL IsThemeActive( VOID );
	HRESULT SetWindowTheme( HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList );
	HTHEME OpenThemeData( HWND hwnd, LPCWSTR pszClassList );
	HRESULT DrawThemeBackground( HTHEME htheme, HDC hdc, int iPartId, int iStateId, RECT* prc, RECT* prcClip );
	HRESULT DrawThemeParentBackground( HWND hwnd, HDC hdc, RECT* prc );
	HRESULT IsThemeBackgroundPartiallyTransparent( HTHEME htheme, int iPartId, int iStateId );
};

