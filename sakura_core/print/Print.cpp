/*!	@file
	@brief 印刷
*/
#include "StdAfx.h"
#include <stdlib.h>
#include <WinSpool.h>
#include "Print.h"
#include "_main/global.h"


const PaperInfo Print::paperInfoArr[] = {
	// 	用紙ID, 幅
	{DMPAPER_A4,                  2100,  2970, _T("A4 (210 x 297 mm)")},
	{DMPAPER_A3,                  2970,  4200, _T("A3 (297 x 420 mm)")},
	{DMPAPER_A4SMALL,             2100,  2970, _T("A4 small(210 x 297 mm)")},
	{DMPAPER_A5,                  1480,  2100, _T("A5 (148 x 210 mm)")},
	{DMPAPER_B4,                  2500,  3540, _T("B4 (250 x 354 mm)")},
	{DMPAPER_B5,                  1820,  2570, _T("B5 (182 x 257 mm)")},
	{DMPAPER_QUARTO,              2150,  2750, _T("Quarto(215 x 275 mm)")},
	{DMPAPER_ENV_DL,              1100,  2200, _T("DL Envelope(110 x 220 mm)")},
	{DMPAPER_ENV_C5,              1620,  2290, _T("C5 Envelope(162 x 229 mm)")},
	{DMPAPER_ENV_C3,              3240,  4580, _T("C3 Envelope(324 x 458 mm)")},
	{DMPAPER_ENV_C4,              2290,  3240, _T("C4 Envelope(229 x 324 mm)")},
	{DMPAPER_ENV_C6,              1140,  1620, _T("C6 Envelope(114 x 162 mm)")},
	{DMPAPER_ENV_C65,             1140,  2290, _T("C65 Envelope(114 x 229 mm)")},
	{DMPAPER_ENV_B4,              2500,  3530, _T("B4 Envelope(250 x 353 mm)")},
	{DMPAPER_ENV_B5,              1760,  2500, _T("B5 Envelope(176 x 250 mm)")},
	{DMPAPER_ENV_B6,              1760,  1250, _T("B6 Envelope(176 x 125 mm)")},
	{DMPAPER_ENV_ITALY,           1100,  2300, _T("Italy Envelope(110 x 230 mm)")},
	{DMPAPER_LETTER,              2159,  2794, _T("Letter (8 1/2 x 11 inch)")},
	{DMPAPER_LEGAL,               2159,  3556, _T("Legal  (8 1/2 x 14 inch)")},
	{DMPAPER_CSHEET,              4318,  5588, _T("C sheet (17 x 22 inch)")},
	{DMPAPER_DSHEET,              5588,  8634, _T("D sheet (22 x 34 inch)")},
	{DMPAPER_ESHEET,              8634, 11176, _T("E sheet (34 x 44 inch)")},
	{DMPAPER_LETTERSMALL,         2159,  2794, _T("Letter Small (8 1/2 x 11 inch)")},
	{DMPAPER_TABLOID,             2794,  4318, _T("Tabloid (11 x 17 inch)")},
	{DMPAPER_LEDGER,              4318,  2794, _T("Ledger  (17 x 11 inch)")},
	{DMPAPER_STATEMENT,           1397,  2159, _T("Statement (5 1/2 x 8 1/2 inch)")},
	{DMPAPER_EXECUTIVE,           1841,  2667, _T("Executive (7 1/4 x 10 1/2 inch)")},
	{DMPAPER_FOLIO,               2159,  3302, _T("Folio (8 1/2 x 13 inch)")},
	{DMPAPER_10X14,               2540,  3556, _T("10x14 inch sheet")},
	{DMPAPER_11X17,               2794,  4318, _T("11x17 inch sheet")},
	{DMPAPER_NOTE,                2159,  2794, _T("Note (8 1/2 x 11 inch)")},
	{DMPAPER_ENV_9,                984,  2254, _T("#9 Envelope  (3 7/8 x 8 7/8 inch)")},
	{DMPAPER_ENV_10,              1047,  2413, _T("#10 Envelope (4 1/8 x 9 1/2 inch)")},
	{DMPAPER_ENV_11,              1143,  2635, _T("#11 Envelope (4 1/2 x 10 3/8 inch)")},
	{DMPAPER_ENV_12,              1206,  2794, _T("#12 Envelope (4 3/4 x 11 inch)")},
	{DMPAPER_ENV_14,              1270,  2921, _T("#14 Envelope (5 x 11 1/2 inch)")},
	{DMPAPER_ENV_MONARCH,          984,  1905, _T("Monarch Envelope (3 7/8 x 7 1/2 inch)")},
	{DMPAPER_ENV_PERSONAL,         920,  1651, _T("6 3/4 Envelope (3 5/8 x 6 1/2 inch)")},
	{DMPAPER_FANFOLD_US,          3778,  2794, _T("US Std Fanfold (14 7/8 x 11 inch)")},
	{DMPAPER_FANFOLD_STD_GERMAN,  2159,  3048, _T("German Std Fanfold   (8 1/2 x 12 inch)")},
	{DMPAPER_FANFOLD_LGL_GERMAN,  2159,  3302, _T("German Legal Fanfold (8 1/2 x 13 inch)")},
};

const size_t Print::nPaperInfoArrNum = _countof(paperInfoArr);



Print::Print(void)
{
	hDevMode	= NULL;
	hDevNames	= NULL;
	return;
}

Print::~Print(void)
{
	// メモリ割り当て済みならば、解放する
	// 2003.05.18 かろと
	if (hDevMode) {
		::GlobalFree(hDevMode);
	}
	if (hDevNames) {
		::GlobalFree(hDevNames);
	}
	hDevMode	= NULL;
	hDevNames	= NULL;
	return;
}



/*! @brief プリンタダイアログを表示して、プリンタを選択する
** 
** @param pPD			[i/o]	プリンタダイアログ構造体
** @param pMYDEVMODE 	[i/o] 	印刷設定
*/
BOOL Print::PrintDlg(
	PRINTDLG* pPD,
	MYDEVMODE* pMYDEVMODE
	)
{
	// デフォルトプリンタが選択されていなければ、選択する
	if (!hDevMode) {
		if (!GetDefaultPrinter(pMYDEVMODE)) {
			return FALSE;
		}
	}

	//
	//  現在のプリンタ設定の必要部分を変更
	//
	DEVMODE* pDEVMODE = (DEVMODE*)::GlobalLock(hDevMode);
	pDEVMODE->dmOrientation		= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize		= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength		= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth		= pMYDEVMODE->dmPaperWidth;
	// PrintDlg()でReAllocされる事を考えて、呼び出す前にUnlock
	::GlobalUnlock(hDevMode);

	// プリンタダイアログを表示して、プリンタを選択
	pPD->lStructSize = sizeof(*pPD);
	pPD->hDevMode = hDevMode;
	pPD->hDevNames = hDevNames;
	if (!::PrintDlg(pPD)) {
		// プリンタを変更しなかった
		return FALSE;
	}

	hDevMode = pPD->hDevMode;
	hDevNames = pPD->hDevNames;

	pDEVMODE = (DEVMODE*)::GlobalLock(hDevMode);
	// プリンタ設定 DEVNAMES用
	DEVNAMES* pDEVNAMES = (DEVNAMES*)::GlobalLock(hDevNames);

	// プリンタドライバ名
	_tcscpy_s(
		pMYDEVMODE->szPrinterDriverName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset
	);
	// プリンタデバイス名
	_tcscpy_s(
		pMYDEVMODE->szPrinterDeviceName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset
	);
	// プリンタポート名
	_tcscpy_s(
		pMYDEVMODE->szPrinterOutputName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset
	);

	// プリンタから得られた、dmFieldsは変更しない
	// プリンタがサポートしないbitをセットすると、プリンタドライバによっては、不安定な動きをする場合がある
	// pMYDEVMODEは、コピーしたいbitで１のものだけセットする
	// →プリンタから得られた dmFieldsが1でないLength,Width情報に、間違った長さが入っているプリンタドライバでは、
	//   縦・横が正しく印刷されない不具合となっていた(2003.07.03 かろと)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation	= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize		= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength	= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth	= pDEVMODE->dmPaperWidth;

	DEBUG_TRACE(_T(" (入力/出力) デバイス ドライバ=[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset);
	DEBUG_TRACE(_T(" (入力/出力) デバイス名=[%ts]\n"),        (TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset);
	DEBUG_TRACE(_T("物理出力メディア (出力ポート) =[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset);
	DEBUG_TRACE(_T("デフォルトのプリンタか=[%d]\n"),          pDEVNAMES->wDefault);

	::GlobalUnlock(hDevMode);
	::GlobalUnlock(hDevNames);
	return TRUE;
}


/*! @brief デフォルトのプリンタを取得し、MYDEVMODE に設定 
** 
** @param pMYDEVMODE 	[out] 	印刷設定
*/
BOOL Print::GetDefaultPrinter(MYDEVMODE* pMYDEVMODE)
{
	PRINTDLG	pd;
	// 2009.08.08 印刷で用紙サイズ、横指定が効かない問題対応 syat
	//// すでに DEVMODEを取得済みなら、何もしない
	//if (hDevMode != NULL) {
	//	return TRUE;
	//}

	// DEVMODEを取得済みでない場合、取得する
	if (!hDevMode) {
		//
		// PRINTDLG構造体を初期化する（ダイアログは表示しないように）
		// PrintDlg()でデフォルトプリンタのデバイス名などを取得する
		//
		memset_raw (&pd, 0, sizeof(pd));
		pd.lStructSize	= sizeof(pd);
		pd.Flags		= PD_RETURNDEFAULT;
		if (!::PrintDlg(&pd)) {
			pMYDEVMODE->bPrinterNotFound = TRUE;	// プリンタがなかったフラグ
			return FALSE;
		}
		pMYDEVMODE->bPrinterNotFound = FALSE;	// プリンタがなかったフラグ

		// 初期化
		memset_raw(pMYDEVMODE, 0, sizeof(*pMYDEVMODE));
		hDevMode = pd.hDevMode;
		hDevNames = pd.hDevNames;
	}

	// MYDEVMODEへのコピー
	DEVMODE* pDEVMODE = (DEVMODE*)::GlobalLock(hDevMode);
	// プリンタ設定 DEVNAMES用
	DEVNAMES* pDEVNAMES = (DEVNAMES*)::GlobalLock(hDevNames);

	// プリンタドライバ名
	_tcscpy_s(
		pMYDEVMODE->szPrinterDriverName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset
	);
	// プリンタデバイス名
	_tcscpy_s(
		pMYDEVMODE->szPrinterDeviceName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset
	);
	// プリンタポート名
	_tcscpy_s(
		pMYDEVMODE->szPrinterOutputName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset
	);

	// プリンタから得られた、dmFieldsは変更しない
	// プリンタがサポートしないbitをセットすると、プリンタドライバによっては、不安定な動きをする場合がある
	// pMYDEVMODEは、コピーしたいbitで１のものだけコピーする
	// →プリンタから得られた dmFieldsが1でないLength,Width情報に、間違った長さが入っているプリンタドライバでは、
	//   縦・横が正しく印刷されない不具合となっていた(2003.07.03 かろと)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation	= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize		= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength	= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth	= pDEVMODE->dmPaperWidth;

	DEBUG_TRACE(_T(" (入力/出力) デバイス ドライバ=[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset);
	DEBUG_TRACE(_T(" (入力/出力) デバイス名=[%ts]\n"),        (TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset);
	DEBUG_TRACE(_T("物理出力メディア (出力ポート) =[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset);
	DEBUG_TRACE(_T("デフォルトのプリンタか=[%d]\n"),          pDEVNAMES->wDefault);

	::GlobalUnlock(hDevMode);
	::GlobalUnlock(hDevNames);
	return TRUE;
}

/*! 
** @brief プリンタをオープンし、hDCを作成する
*/
HDC Print::CreateDC(
	MYDEVMODE*	pMYDEVMODE,
	TCHAR*		pszErrMsg		// エラーメッセージ格納場所
	)
{
	// プリンタが選択されていなければ、NULLを返す
	if (!hDevMode) {
		return NULL;
	}
	HDC	hdc = NULL;
	HANDLE hPrinter = NULL;
	//
	// OpenPrinter()で、デバイス名でプリンタハンドルを取得
	//
	if (!::OpenPrinter(
			pMYDEVMODE->szPrinterDeviceName,		// プリンタデバイス名
			&hPrinter,					// プリンタハンドルのポインタ
			NULL
		)
	) {
		auto_sprintf(
			pszErrMsg,
			LS(STR_ERR_CPRINT01),
			pMYDEVMODE->szPrinterDeviceName	// プリンタデバイス名
		);
		goto end_of_func;
	}

	DEVMODE* pDEVMODE = (DEVMODE*)::GlobalLock(hDevMode);
	pDEVMODE->dmOrientation	= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize	= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength	= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth	= pMYDEVMODE->dmPaperWidth;

	//
	// DocumentProperties()でアプリケーション独自のプリンタ設定に変更する
	//
	::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->szPrinterDeviceName,	// プリンタデバイス名
		pDEVMODE,
		pDEVMODE,
		DM_OUT_BUFFER | DM_IN_BUFFER
	);
	// 指定デバイスに対するデバイス コンテキストを作成します。
	hdc = ::CreateDC(
		pMYDEVMODE->szPrinterDriverName,	// プリンタドライバ名
		pMYDEVMODE->szPrinterDeviceName,	// プリンタデバイス名
		pMYDEVMODE->szPrinterOutputName,	// プリンタポート名
		pDEVMODE
	);
	
	// pMYDEVMODEは、コピーしたいbitで１のものだけコピーする
	// →プリンタから得られた dmFieldsが1でないLength,Width情報に、間違った長さが入っているプリンタドライバでは、
	//   縦・横が正しく印刷されない不具合となっていた(2003.07.03 かろと)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation	= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize		= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength	= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth	= pDEVMODE->dmPaperWidth;

	::GlobalUnlock(hDevMode);

end_of_func:;
	if (hPrinter) {
		::ClosePrinter(hPrinter);
	}

	return hdc;
}


// 印刷/Previewに必要な情報を取得
BOOL Print::GetPrintMetrics(
	MYDEVMODE*	pMYDEVMODE,
	short*		pnPaperAllWidth,	// 用紙幅
	short*		pnPaperAllHeight,	// 用紙高さ
	short*		pnPaperWidth,		// 用紙印刷可能幅
	short*		pnPaperHeight,		// 用紙印刷可能高さ
	short*		pnPaperOffsetLeft,	// 用紙余白左端
	short*		pnPaperOffsetTop,	// 用紙余白上端
	TCHAR*		pszErrMsg			// エラーメッセージ格納場所
	)
{
	BOOL bRet = TRUE;

	// 現在の設定で、用紙の幅、高さを確定し、CreateDCに渡す
	if (!GetPaperSize(pnPaperAllWidth, pnPaperAllHeight, pMYDEVMODE)) {
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);
	}

	// pMYDEVMODEを使って、hdcを取得
	HDC hdc = CreateDC(pMYDEVMODE, pszErrMsg);
	if (!hdc) {
		return FALSE;
	}

	// CreateDC実行によって得られた実際のプリンタの用紙の幅、高さを取得
	if (!GetPaperSize(pnPaperAllWidth, pnPaperAllHeight, pMYDEVMODE)) {
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);
	}

	// マッピング モードの設定
	::SetMapMode(hdc, MM_LOMETRIC);	// MM_LOMETRIC	それぞれの論理単位は 0.1 mm にマップされます。

	// 最小左マージンと最小上マージンを取得(1mm単位)
	POINT	po;
	if (0 < ::Escape(hdc, GETPRINTINGOFFSET, (int)NULL, NULL, (LPPOINT)&po)) {
		::DPtoLP(hdc, &po, 1);
		*pnPaperOffsetLeft = (short)abs(po.x);	// 用紙余白左端
		*pnPaperOffsetTop  = (short)abs(po.y);	// 用紙余白上端
	}else {
		*pnPaperOffsetLeft = 0;	// 用紙余白左端
		*pnPaperOffsetTop  = 0;	// 用紙余白上端
	}

	// 用紙の印刷可能な幅、高さ
	po.x = ::GetDeviceCaps(hdc, HORZRES);	// 用紙印刷可能幅←物理ディスプレイの幅 (mm 単位)
	po.y = ::GetDeviceCaps(hdc, VERTRES);	// 用紙印刷可能高さ←物理ディスプレイの高さ (mm 単位) 
	::DPtoLP(hdc, &po, 1);
	*pnPaperWidth  = (short)abs(po.x);
	*pnPaperHeight = (short)abs(po.y);

	::DeleteDC(hdc);

	return bRet;
}


// 用紙の幅、高さ
BOOL Print::GetPaperSize(
	short*		pnPaperAllWidth,
	short*		pnPaperAllHeight,
	MYDEVMODE*	pDEVMODE
	)
{
	short	nWork;
	
	if (pDEVMODE->dmFields &  DM_PAPERSIZE) {
		const PaperInfo* pi = FindPaperInfo(pDEVMODE->dmPaperSize);
		if (pi) {
			*pnPaperAllWidth = pi->nAllWidth;
			*pnPaperAllHeight = pi->nAllHeight;
		}else {
			// 2001.12.21 hor マウスでクリックしたままリスト外に出るとここにくるけど、
			//	異常ではないので FALSE を返すことにする
			return FALSE;
		}
	}
	if (pDEVMODE->dmFields & DM_PAPERLENGTH && pDEVMODE->dmPaperLength != 0) {
		// pDEVMODE->dmPaperLengthは1/10mm単位である
		*pnPaperAllHeight = pDEVMODE->dmPaperLength/* * 10*/;
	}else {
		pDEVMODE->dmPaperLength = *pnPaperAllHeight;
		pDEVMODE->dmFields |= DM_PAPERLENGTH;
	}
	if (pDEVMODE->dmFields & DM_PAPERWIDTH && pDEVMODE->dmPaperWidth != 0) {
		// pDEVMODE->dmPaperWidthは1/10mm単位である
		*pnPaperAllWidth = pDEVMODE->dmPaperWidth/* * 10*/;
	}else {
		pDEVMODE->dmPaperWidth = *pnPaperAllWidth;
		pDEVMODE->dmFields |= DM_PAPERWIDTH;
	}
	// 用紙の方向
	if (pDEVMODE->dmOrientation == DMORIENT_LANDSCAPE) {
		nWork = *pnPaperAllWidth;
		*pnPaperAllWidth = *pnPaperAllHeight;
		*pnPaperAllHeight = nWork;
	}
	return TRUE;
}


// 印刷 ジョブ開始
BOOL Print::PrintOpen(
	TCHAR*		pszJobName,
	MYDEVMODE*	pMYDEVMODE,
	HDC*		phdc,
	TCHAR*		pszErrMsg		// エラーメッセージ格納場所
	)
{
	BOOL bRet = TRUE;
	DOCINFO di = {0};
	// 
	// hdcを取得
	//
	HDC hdc = CreateDC(pMYDEVMODE, pszErrMsg);
	if (!hdc) {
		bRet = FALSE;
		goto end_of_func;
	}

	// マッピング モードの設定
	::SetMapMode(hdc, MM_LOMETRIC);	// MM_LOMETRIC		それぞれの論理単位は、0.1 mm にマップされます。

	//
	//  印刷ジョブ開始
	//
	di.cbSize = sizeof(di);
	di.lpszDocName = pszJobName;
	di.lpszOutput  = NULL;
	di.lpszDatatype = NULL;
	di.fwType = 0;
	if (0 >= ::StartDoc(hdc, &di)) {
		auto_sprintf(
			pszErrMsg,
			LS(STR_ERR_CPRINT02),
			pMYDEVMODE->szPrinterDeviceName	// プリンタデバイス名
		);
		bRet = FALSE;
		goto end_of_func;
	}

	*phdc = hdc;

end_of_func:;

	return bRet;
}


// 印刷 ページ開始
void Print::PrintStartPage(HDC hdc)
{
	::StartPage(hdc);
}


// 印刷 ページ終了
void Print::PrintEndPage(HDC hdc)
{
	::EndPage(hdc);

}


// 印刷 ジョブ終了
void Print::PrintClose(HDC hdc)
{
	::EndDoc(hdc);
	::DeleteDC(hdc);
}


// 用紙の名前を取得
TCHAR* Print::GetPaperName(int nPaperSize, TCHAR* pszPaperName)
{
	const PaperInfo* paperInfo = FindPaperInfo(nPaperSize);
	if (paperInfo) {
		_tcscpy(pszPaperName, paperInfo->pszName);
	}else {
		_tcscpy(pszPaperName, LS(STR_ERR_CPRINT03));
	}
	return pszPaperName;
}

/*!
	用紙情報の取得
*/
const PaperInfo* Print::FindPaperInfo(int id)
{
	for (int i=0; i<nPaperInfoArrNum; ++i) {
		if (paperInfoArr[i].nId == id) {
			return &(paperInfoArr[i]);
		}
	}
	return NULL;
}


/*!	@brief PrintSettingの初期化

	ここではmdmDevModeの プリンタ設定は取得・初期化しない
*/
void Print::SettingInitialize(PrintSetting& pPrintSetting, const TCHAR* settingName)
{
	_tcscpy_s(pPrintSetting.szPrintSettingName, settingName);		// 印刷設定の名前
	_tcscpy(pPrintSetting.szPrintFontFaceHan, _T("ＭＳ 明朝"));		// 印刷フォント
	_tcscpy(pPrintSetting.szPrintFontFaceZen, _T("ＭＳ 明朝"));		// 印刷フォント
	pPrintSetting.bColorPrint = false;			// カラー印刷
	pPrintSetting.nPrintFontWidth = 12;			// 印刷フォント幅(1/10mm単位)
	pPrintSetting.nPrintFontHeight = pPrintSetting.nPrintFontWidth * 2;	// 印刷フォント高さ(1/10mm単位単位)
	pPrintSetting.nPrintDansuu = 1;				// 段組の段数
	pPrintSetting.nPrintDanSpace = 70; 			// 段と段の隙間(1/10mm)
	pPrintSetting.bPrintWordWrap = true;		// 英文ワードラップする
	pPrintSetting.bPrintKinsokuHead = false;	// 行頭禁則する
	pPrintSetting.bPrintKinsokuTail = false;	// 行末禁則する
	pPrintSetting.bPrintKinsokuRet  = false;	// 改行文字をぶら下げる
	pPrintSetting.bPrintKinsokuKuto = false;	// 
	pPrintSetting.bPrintLineNumber = false;		// 行番号を印刷する
	pPrintSetting.nPrintLineSpacing = 30;		// 印刷フォント行間 文字の高さに対する割合(%)
	pPrintSetting.nPrintMarginTY = 100;			// 印刷用紙マージン 上(1/10mm単位)
	pPrintSetting.nPrintMarginBY = 200;			// 印刷用紙マージン 下(1/10mm単位)
	pPrintSetting.nPrintMarginLX = 200;			// 印刷用紙マージン 左(1/10mm単位)
	pPrintSetting.nPrintMarginRX = 100;			// 印刷用紙マージン 右(1/10mm単位)
	pPrintSetting.nPrintPaperOrientation = DMORIENT_PORTRAIT;	// 用紙方向 DMORIENT_PORTRAIT (1) または DMORIENT_LANDSCAPE (2)
	pPrintSetting.nPrintPaperSize = DMPAPER_A4;	// 用紙サイズ
	// プリンタ設定 DEVMODE用
	// プリンタ設定を取得するのはコストがかかるので、後ほど
	//	print.GetDefaultPrinterInfo(&(pPrintSetting.mdmDevMode));
	pPrintSetting.bHeaderUse[0] = TRUE;
	pPrintSetting.bHeaderUse[1] = FALSE;
	pPrintSetting.bHeaderUse[2] = FALSE;
	wcscpy(pPrintSetting.szHeaderForm[0], L"$f");
	wcscpy(pPrintSetting.szHeaderForm[1], L"");
	wcscpy(pPrintSetting.szHeaderForm[2], L"");
	pPrintSetting.bFooterUse[0] = TRUE;
	pPrintSetting.bFooterUse[1] = FALSE;
	pPrintSetting.bFooterUse[2] = FALSE;
	wcscpy(pPrintSetting.szFooterForm[0], L"");
	wcscpy(pPrintSetting.szFooterForm[1], L"- $p -");
	wcscpy(pPrintSetting.szFooterForm[2], L"");
}


/*!
	印字可能桁数の計算
*/
int Print::CalculatePrintableColumns(PrintSetting& ps, int nPaperAllWidth, int nLineNumberColumns)
{
	int nPrintablePaperWidth = nPaperAllWidth - ps.nPrintMarginLX - ps.nPrintMarginRX;
	if (nPrintablePaperWidth < 0) { return 0; }

	int nPrintSpaceWidth = (ps.nPrintDansuu - 1) * ps.nPrintDanSpace
						 + (ps.nPrintDansuu) * (nLineNumberColumns * ps.nPrintFontWidth);
	if (nPrintablePaperWidth < nPrintSpaceWidth) { return 0; }

	int nEnableColumns =
		(nPrintablePaperWidth - nPrintSpaceWidth
		) / ps.nPrintFontWidth / ps.nPrintDansuu;	// 印字可能桁数/ページ
	return nEnableColumns;
}


/*!
	印字可能行数の計算
*/
int Print::CalculatePrintableLines(
	PrintSetting& ps,
	int nPaperAllHeight
	)
{
	int nPrintablePaperHeight = nPaperAllHeight - ps.nPrintMarginTY - ps.nPrintMarginBY;
	if (nPrintablePaperHeight < 0) { return 0; }

	int nPrintSpaceHeight = (ps.nPrintFontHeight * ps.nPrintLineSpacing / 100);

	int nEnableLines =
		(nPrintablePaperHeight - CalcHeaderHeight(ps)*2 - CalcFooterHeight(ps)*2 + nPrintSpaceHeight) /
		(ps.nPrintFontHeight + nPrintSpaceHeight);	// 印字可能行数/ページ
	if (nEnableLines < 0) { return 0; }
	return nEnableLines;
}


/*!
	ヘッダ高さの計算(行送り分こみ)
*/
int Print::CalcHeaderHeight(PrintSetting& ps)
{
	if (ps.szHeaderForm[0][0] == _T('\0')
		&& ps.szHeaderForm[1][0] == _T('\0')
		&& ps.szHeaderForm[2][0] == _T('\0')
	) {
		// 使ってなければ 0
		return 0;
	}

	int nHeight;
	if (ps.lfHeader.lfFaceName[0] == _T('\0')) {
		// フォント指定無し
		nHeight = ps.nPrintFontHeight;
	}else {
		// フォントのサイズ計算(pt->1/10mm)
		nHeight = ps.nHeaderPointSize * 254 / 720;
	}
	return nHeight * (ps.nPrintLineSpacing + 100) / 100;	// 行送り計算
}

/*!
	フッタ高さの計算(行送り分こみ)
*/
int Print::CalcFooterHeight(PrintSetting& ps)
{
	if (ps.szFooterForm[0][0] == _T('\0')
	 && ps.szFooterForm[1][0] == _T('\0')
	 && ps.szFooterForm[2][0] == _T('\0')
	) {
		// 使ってなければ 0
		return 0;
	}

	int nHeight;
	if (ps.lfFooter.lfFaceName[0] == _T('\0')) {
		// フォント指定無し
		nHeight = ps.nPrintFontHeight;
	}else {
		// フォントのサイズ計算(pt->1/10mm)
		nHeight = ps.nFooterPointSize * 254 / 720;
	}
	return nHeight * (ps.nPrintLineSpacing + 100) / 100;	// 行送り計算
}

