/*!	@file
	@brief 印刷関連
*/
#pragma once

#include <WinSpool.h>
#include <CommDlg.h> // PRINTDLG

struct MYDEVMODE {
	BOOL	bPrinterNotFound;	// プリンタがなかったフラグ
	TCHAR	szPrinterDriverName[_MAX_PATH + 1];	// プリンタドライバ名
	TCHAR	szPrinterDeviceName[_MAX_PATH + 1];	// プリンタデバイス名
	TCHAR	szPrinterOutputName[_MAX_PATH + 1];	// プリンタポート名
	DWORD	dmFields;
	short	dmOrientation;
	short	dmPaperSize;
	short	dmPaperLength;
	short	dmPaperWidth;
	short	dmScale;
	short	dmCopies;
	short	dmDefaultSource;
	short	dmPrintQuality;
	short	dmColor;
	short	dmDuplex;
	short	dmYResolution;
	short	dmTTOption;
	short	dmCollate;
	BCHAR	dmFormName[CCHFORMNAME];
	WORD	dmLogPixels;
	DWORD	dmBitsPerPel;
	DWORD	dmPelsWidth;
	DWORD	dmPelsHeight;
	DWORD	dmDisplayFlags;
	DWORD	dmDisplayFrequency;
};

// 用紙情報
struct PaperInfo {
	int				nId;			// 用紙ID
	short			nAllWidth;		// 幅 (0.1mm単位)
	short			nAllHeight;		// 高さ (0.1mm単位)
	const TCHAR*	pszName;		// 用紙名称
};

struct PrintSetting;

// 印刷設定
#define POS_LEFT	0
#define POS_CENTER	1
#define POS_RIGHT	2
#define HEADER_MAX	100
#define FOOTER_MAX	HEADER_MAX
struct PrintSetting {
	TCHAR		szPrintSettingName[32 + 1];			// 印刷設定の名前
	TCHAR		szPrintFontFaceHan[LF_FACESIZE];	// 印刷フォント
	TCHAR		szPrintFontFaceZen[LF_FACESIZE];	// 印刷フォント
	int			nPrintFontWidth;					// 印刷フォント幅(1/10mm単位単位)
	int			nPrintFontHeight;					// 印刷フォント高さ(1/10mm単位単位)
	int			nPrintDansuu;						// 段組の段数
	int			nPrintDanSpace;						// 段と段の隙間(1/10mm単位)
	int			nPrintLineSpacing;					// 印刷フォント行間 文字の高さに対する割合(%)
	int			nPrintMarginTY;						// 印刷用紙マージン 上(mm単位)
	int			nPrintMarginBY;						// 印刷用紙マージン 下(mm単位)
	int			nPrintMarginLX;						// 印刷用紙マージン 左(mm単位)
	int			nPrintMarginRX;						// 印刷用紙マージン 右(mm単位)
	short		nPrintPaperOrientation;				// 用紙方向 DMORIENT_PORTRAIT (1) または DMORIENT_LANDSCAPE (2)
	short		nPrintPaperSize;					// 用紙サイズ
	bool		bColorPrint;						// カラー印刷			// 2013/4/26 Uchi
	bool		bPrintWordWrap;						// 英文ワードラップする
	bool		bPrintKinsokuHead;					// 行頭禁則する
	bool		bPrintKinsokuTail;					// 行末禁則する
	bool		bPrintKinsokuRet;					// 改行文字のぶら下げ
	bool		bPrintKinsokuKuto;					// 句読点のぶらさげ
	bool		bPrintLineNumber;					// 行番号を印刷する

	MYDEVMODE	mdmDevMode;							// プリンタ設定 DEVMODE用
	BOOL		bHeaderUse[3];						// ヘッダが使われているか？
	EDIT_CHAR	szHeaderForm[3][HEADER_MAX];		// 0:左寄せヘッダ。1:中央寄せヘッダ。2:右寄せヘッダ。
	BOOL		bFooterUse[3];						// フッタが使われているか？
	EDIT_CHAR	szFooterForm[3][FOOTER_MAX];		// 0:左寄せフッタ。1:中央寄せフッタ。2:右寄せフッタ。

	// ヘッダ/フッタのフォント(lfFaceNameが設定されていなければ半角/全角フォントを使用)
	LOGFONT		lfHeader;							// ヘッダフォント用LOGFONT構造体
	int 		nHeaderPointSize;					// ヘッダフォントポイントサイズ
	LOGFONT		lfFooter;							// フッタフォント用LOGFONT構造体
	int 		nFooterPointSize;					// フッタフォントポイントサイズ
};


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 印刷関連機能

	オブジェクト指向でないクラス
*/
class Print {
public:
	static const PaperInfo paperInfoArr[];	// 用紙情報一覧
	static const size_t nPaperInfoArrNum; // 用紙情報一覧の要素数

	/*
	||	static関数群
	*/
	static void SettingInitialize(PrintSetting&, const TCHAR* settingName);

	static TCHAR* GetPaperName(int, TCHAR*);	// 用紙の名前を取得
	// 用紙の幅、高さ
	static BOOL GetPaperSize(
		short*		pnPaperAllWidth,
		short*		pnPaperAllHeight,
		MYDEVMODE*	pDEVMODE
	);
	// 印字可能桁・行の計算
	static int CalculatePrintableColumns(PrintSetting&, int width, int nLineNumberColumns);
	static int CalculatePrintableLines(PrintSetting&, int height);

	// ヘッダ・フッタの高さ計算
	static int CalcHeaderHeight(PrintSetting&);
	static int CalcFooterHeight(PrintSetting&);
public:
	/*
	||  Constructors
	*/
	Print();
	~Print();

	/*
	||  Attributes & Operations
	*/
	BOOL GetDefaultPrinter(MYDEVMODE* pMYDEVMODE);		// デフォルトのプリンタ情報を取得
	BOOL PrintDlg(PRINTDLG* pd, MYDEVMODE* pMYDEVMODE);	// プリンタ情報を取得
	// 印刷/Previewに必要な情報を取得
	BOOL GetPrintMetrics(
		MYDEVMODE*	pMYDEVMODE,
		short*		pnPaperAllWidth,	// 用紙幅
		short*		pnPaperAllHeight,	// 用紙高さ
		short*		pnPaperWidth,		// 用紙印刷可能幅
		short*		pnPaperHeight,		// 用紙印刷可能高さ
		short*		pnPaperOffsetLeft,	// 用紙余白左端
		short*		pnPaperOffsetTop,	// 用紙余白上端
		TCHAR*		pszErrMsg			// エラーメッセージ格納場所
	);

	// 印刷 ジョブ開始
	BOOL PrintOpen(
		TCHAR*		pszJobName,
		MYDEVMODE*	pMYDEVMODE,
		HDC*		phdc,
		TCHAR*		pszErrMsg		// エラーメッセージ格納場所
	);
	void PrintStartPage(HDC);		// 印刷 ページ開始
	void PrintEndPage(HDC);		// 印刷 ページ終了
	void PrintClose(HDC);			// 印刷 ジョブ終了

protected:
	/*
	||  実装ヘルパ関数
	*/
	// DC作成する
	HDC CreateDC(MYDEVMODE* pMYDEVMODE, TCHAR* pszErrMsg);
	
	static const PaperInfo* FindPaperInfo(int id);
private:
	/*
	||  メンバ変数
	*/
	HGLOBAL	hDevMode;							// 現在プリンタのDEVMODEへのメモリハンドル
	HGLOBAL	hDevNames;						// 現在プリンタのDEVNAMESへのメモリハンドル
};

