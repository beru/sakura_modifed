#include "StdAfx.h"
#include <vector>
#include "StdApi.h"
#include "charset/charcode.h"
#include "_os/OsVersionInfo.h"

using namespace std;

namespace ApiWrap {

	/*!
		MakeSureDirectoryPathExists の UNICODE 版。
		szDirPath で指定されたすべてのディレクトリを作成します。
		ディレクトリの記述は、ルートから開始します。

		@param DirPath
			有効なパス名を指定する、null で終わる文字列へのポインタを指定します。
			パスの最後のコンポーネントがファイル名ではなくディレクトリである場合、
			文字列の最後に円記号（\）を記述しなければなりません。 

		@returns
			関数が成功すると、TRUE が返ります。
			関数が失敗すると、FALSE が返ります。

		@note
			指定された各ディレクトリがまだ存在しない場合、それらのディレクトリを順に作成します。
			一部のディレクトリのみを作成した場合、この関数は FALSE を返します。
	*/
	BOOL MakeSureDirectoryPathExistsW(LPCWSTR szDirPath)
	{
		const wchar_t* p = szDirPath - 1;
		for (;;) {
			p = wcschr(p + 1, L'\\');
			if (!p) break; // '\\'を走査し終わったので終了

			// 先頭からpまでの部分文字列 -> szBuf
			wchar_t szBuf[_MAX_PATH];
			wcsncpy_s(szBuf, _countof(szBuf), szDirPath, p - szDirPath);

			// 存在するか
			int nAcc = _waccess(szBuf, 0);
			if (nAcc == 0) continue; // 存在するなら、次へ

			// ディレクトリ作成
			int nDir = _wmkdir(szBuf);
			if (nDir == -1) return FALSE; // エラーが発生したので、FALSEを返す
		}
		return TRUE;
	}

	LPWSTR CharNextW_AnyBuild(
		LPCWSTR lpsz
	)
	{
		//$$ サロゲートペア無視
		if (*lpsz) return const_cast<LPWSTR>(lpsz + 1);
		else return const_cast<LPWSTR>(lpsz);
	}

	LPWSTR CharPrevW_AnyBuild(
		LPCWSTR lpszStart,
		LPCWSTR lpszCurrent
	)
	{
		//$$ サロゲートペア無視
		if (lpszCurrent > lpszStart) return const_cast<LPWSTR>(lpszCurrent - 1);
		else return const_cast<LPWSTR>(lpszStart);
	}

#if 1
	BOOL GetTextExtentPoint32W_AnyBuild(
		HDC		hdc, 
		LPCWSTR	lpString, 
		int		cbString, 
		LPSIZE	lpSize
	)
	{
		vector<char> buf;
		wcstombs_vector(lpString, cbString, &buf);
		return GetTextExtentPoint32A(
			hdc,
			&buf[0],
			(int)buf.size() - 1,
			lpSize
		);
	}
#endif

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    描画API 不具合ラップ                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	/*
		VistaでSetPixelが動かないため、代替関数を用意。

		参考：http://forums.microsoft.com/MSDN-JA/ShowPost.aspx?PostID=3228018&SiteID=7
		> Vista で Aero を OFF にすると SetPixel がうまく動かないそうです。
		> しかも、SP1 でも修正されていないとか。
	*/
	void SetPixelSurely(HDC hdc, int x, int y, COLORREF c)
	{
		if (!IsWinVista_or_later()) {
		// Vistaより前：SetPixel直呼び出し
			::SetPixel(hdc, x, y, c);
		}else {
		// Vista以降：SetPixelエミュレート
			static HPEN hPen = NULL;
			static COLORREF clrPen = 0;
			if (hPen && c != clrPen) {
				DeleteObject(hPen);
				hPen = NULL;
			}
			// ペン生成
			if (!hPen) {
				hPen = CreatePen(PS_SOLID, 1, clrPen = c);
			}
			// 描画
			HPEN hpnOld = (HPEN)SelectObject(hdc, hPen);
			::MoveToEx(hdc, x, y, NULL);
			::LineTo(hdc, x + 1, y + 1);
			SelectObject(hdc, hpnOld);
		}
	}
}

