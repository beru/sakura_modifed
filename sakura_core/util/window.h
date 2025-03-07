#pragma once

/*!
	@brief 画面 DPI スケーリング
	@note 96 DPI ピクセルを想定しているデザインをどれだけスケーリングするか
*/
class DPI {
	static void Init() {
		if (!bInitialized) {
			HDC hDC = GetDC(NULL);
			nDpiX = GetDeviceCaps(hDC, LOGPIXELSX);
			nDpiY = GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(NULL, hDC);
			bInitialized = true;
		}
	}
	static int nDpiX;
	static int nDpiY;
	static bool bInitialized;
public:
	static int ScaleX(int x) {Init(); return ::MulDiv(x, nDpiX, 96);}
	static int ScaleY(int y) {Init(); return ::MulDiv(y, nDpiY, 96);}
	static int UnscaleX(int x) {Init(); return ::MulDiv(x, 96, nDpiX);}
	static int UnscaleY(int y) {Init(); return ::MulDiv(y, 96, nDpiY);}
	static void ScaleRect(LPRECT lprc) {
		lprc->left = ScaleX(lprc->left);
		lprc->right = ScaleX(lprc->right);
		lprc->top = ScaleY(lprc->top);
		lprc->bottom = ScaleY(lprc->bottom);
	}
	static void UnscaleRect(LPRECT lprc) {
		lprc->left = UnscaleX(lprc->left);
		lprc->right = UnscaleX(lprc->right);
		lprc->top = UnscaleY(lprc->top);
		lprc->bottom = UnscaleY(lprc->bottom);
	}
	static int PointsToPixels(int pt, int ptMag = 1) {Init(); return ::MulDiv(pt, nDpiY, 72 * ptMag);}	// ptMag: 引数のポイント数にかかっている倍率
	static int PixelsToPoints(int px, int ptMag = 1) {Init(); return ::MulDiv(px * ptMag, 72, nDpiY);}	// ptMag: 戻り値のポイント数にかける倍率
};

inline int DpiScaleX(int x) {return DPI::ScaleX(x);}
inline int DpiScaleY(int y) {return DPI::ScaleY(y);}
inline int DpiUnscaleX(int x) {return DPI::UnscaleX(x);}
inline int DpiUnscaleY(int y) {return DPI::UnscaleY(y);}
inline void DpiScaleRect(LPRECT lprc) {DPI::ScaleRect(lprc);}
inline void DpiUnscaleRect(LPRECT lprc) {DPI::UnscaleRect(lprc);}
inline int DpiPointsToPixels(int pt, int ptMag = 1) {return DPI::PointsToPixels(pt, ptMag);}
inline int DpiPixelsToPoints(int px, int ptMag = 1) {return DPI::PixelsToPoints(px, ptMag);}

void ActivateFrameWindow(HWND);	// アクティブにする

/*
||	処理中のユーザー操作を可能にする
||	ブロッキングフック(?)(メッセージ配送)
*/
BOOL BlockingHook(HWND hwndDlgCancel);


#ifndef GA_PARENT
#define GA_PARENT		1
#define GA_ROOT			2
#define GA_ROOTOWNER	3
#endif
#define GA_ROOTOWNER2	100


HWND MyGetAncestor(HWND hWnd, UINT gaFlags);	// 指定したウィンドウの祖先のハンドルを取得する


// チェックボックス
inline void CheckDlgButtonBool(HWND hDlg, int nIDButton, bool bCheck) {
	::CheckDlgButton(hDlg, nIDButton, bCheck ? BST_CHECKED : BST_UNCHECKED);
}
inline bool IsDlgButtonCheckedBool(HWND hDlg, int nIDButton) {
	return (::IsDlgButtonChecked(hDlg, nIDButton) & BST_CHECKED) != 0;
}

// ダイアログアイテムの有効化
inline bool DlgItem_Enable(HWND hwndDlg, int nIDDlgItem, bool nEnable) {
	return ::EnableWindow(::GetDlgItem(hwndDlg, nIDDlgItem), nEnable ? TRUE : FALSE) != FALSE;
}

// 幅計算補助クラス
// 最大の幅を報告します
class TextWidthCalc {
public:
	TextWidthCalc(HWND hParentDlg, int nID);
	TextWidthCalc(HWND hwndThis);
	TextWidthCalc(HFONT font);
	TextWidthCalc(HDC hdc);
	virtual ~TextWidthCalc();
	void Reset() { nCx = 0; nExt = 0; }
	void SetCx(int cx = 0) { nCx = cx; }
	void SetDefaultExtend(int extCx = 0) { nExt = 0; }
	bool SetWidthIfMax(int width);
	bool SetWidthIfMax(int width, int extCx);
	bool SetTextWidthIfMax(LPCTSTR pszText);
	bool SetTextWidthIfMax(LPCTSTR pszText, int extCx);
	int GetTextWidth(LPCTSTR pszText) const;
	int GetTextHeight() const;
	HDC GetDC() const{ return hDC; }
	int GetCx() { return nCx; }
	// 算出方法がよく分からないので定数にしておく
	// 制御不要なら ListViewはLVSCW_AUTOSIZE等推奨
	enum StaticMagicNambers {
		// スクロールバーとアイテムの間の隙間
		WIDTH_MARGIN_SCROLLBER = 8,
		// リストビューヘッダ マージン
		WIDTH_LV_HEADER = 17,
		// リストビューのマージン
		WIDTH_LV_ITEM_NORMAL  = 14,
		// リストビューのチェックボックスとマージンの幅
		WIDTH_LV_ITEM_CHECKBOX = 30,
	};
private:
	HWND  hwnd;
	HDC   hDC;
	HFONT hFont;
	HFONT hFontOld;
	int nCx;
	int nExt;
	bool  bHDCComp;
	bool  bFromDC;
};

class FontAutoDeleter {
public:
	FontAutoDeleter();
	~FontAutoDeleter();
	void SetFont(HFONT hfontOld, HFONT hfont, HWND hwnd);
	void ReleaseOnDestroy();
	// void Release();

private:
	HFONT hFontOld;
	HFONT hFont;
	HWND  hwnd;
};

class DCFont
{
public:
	DCFont(LOGFONT& font, HWND hwnd = NULL) {
		this->hwnd = hwnd;
		this->hDC = ::GetDC(hwnd);
		HFONT hFont = ::CreateFontIndirect(&font);
		hFontOld = (HFONT)::SelectObject(hDC, hFont);
	}
	~DCFont() {
		if (hDC) {
			::SelectObject(hDC, hFontOld);
			::ReleaseDC(hwnd, hDC);
			hDC = NULL;
			::DeleteObject(hFont);
			hFont = NULL;
		}
	}
	HDC GetHDC() { return hDC; }
private:
	HWND  hwnd;
	HDC   hDC;
	HFONT hFontOld;
	HFONT hFont;
};

