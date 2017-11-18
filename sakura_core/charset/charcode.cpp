#include "StdAfx.h"
#include "charset/charcode.h"

#include "env/DllSharedData.h"

// キーワードキャラクタ
const unsigned char g_keyword_char[128] = {
//  0         1         2         3         4         5         6         7         8         9         A         B         C         D         E         F             : 0123456789ABCDEF
	CK_NULL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_TAB,   CK_LF,    CK_CTRL,  CK_CTRL,  CK_CR,    CK_CTRL,  CK_CTRL,  // 0: ................
	CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  // 1: ................
	CK_SPACE, CK_ETC,   CK_ETC,   CK_UDEF,  CK_UDEF,  CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   // 2:  !"#$%&'()*+,-./
	CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   // 3: 0123456789:;<=>?
	CK_UDEF,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  // 4: @ABCDEFGHIJKLMNO
	CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_ETC,   CK_UDEF,  CK_ETC,   CK_ETC,   CK_CSYM,  // 5: PQRSTUVWXYZ[\]^_
	CK_ETC,   CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  // 6: `abcdefghijklmno
	CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_CTRL,  // 7: pqrstuvwxyz{|}~.
	// 0: not-keyword, 1:__iscsym(), 2:user-define
};

namespace WCODE
{
	bool CalcHankakuByFont(wchar_t);

#if 0
	/*!
		句読点か
		2008.04.27 kobake CLayoutMgr::IsKutoTen から分離

		@param[in] c1 調べる文字1バイト目
		@param[in] c2 調べる文字2バイト目
		@retval true 句読点である
		@retval false 句読点でない
	*/
	bool IsKutoten(wchar_t wc)
	{
		// 句読点定義
		static const wchar_t* KUTOTEN =
			L"｡､,."
			L"。、，．"
		;

		const wchar_t* p;
		for (p = KUTOTEN; *p; ++p) {
			if (*p == wc) return true;
		}
		return false;
	}
#endif


	/*!
		UNICODE文字情報のキャッシュクラス。
		1文字当たり2ビットで、値を保存しておく。
		00:未初期化
		01:半角
		10:全角
		11:-
	*/
	class LocalCache {
	public:
		LocalCache()
		{
			// LOGFONTの初期化
			memset(&lf, 0, sizeof(lf));

			// HDC の初期化
			HDC hdc = GetDC(NULL);
			this->hdc = CreateCompatibleDC(hdc);
			ReleaseDC(NULL, hdc);

			hFont = NULL;
			hFontOld = NULL;
			pCache = 0;
		}
		~LocalCache()
		{
			// -- -- 後始末 -- -- //
			if (hFont) {
				SelectObject(hdc, hFontOld);
				DeleteObject(hFont);
			}
			DeleteDC(hdc);
		}
		// 再初期化
		void Init(const LOGFONT& lf)
		{
			if (hFontOld) {
				hFontOld = (HFONT)SelectObject(hdc, hFontOld);
				DeleteObject(hFontOld);
			}

			this->lf = lf;

			hFont = ::CreateFontIndirect(&lf);
			hFontOld = (HFONT)SelectObject(hdc, hFont);

			// -- -- 半角基準 -- -- //
			GetTextExtentPoint32W_AnyBuild(hdc, L"x", 1, &han_size);
		}
		void SelectCache(CharWidthCache* pCache)
		{
			this->pCache = pCache;
		}
		void Clear()
		{
			assert(pCache != 0);
			// キャッシュのクリア
			memcpy(pCache->lfFaceName, lf.lfFaceName, sizeof(lf.lfFaceName));
			memset(pCache->bCharWidthCache, 0, sizeof(pCache->bCharWidthCache));
			pCache->nCharWidthCacheTest = 0x12345678;
		}
		bool IsSameFontFace(const LOGFONT& lf)
		{
			assert(pCache != 0);
			return (memcmp(pCache->lfFaceName, lf.lfFaceName, sizeof(lf.lfFaceName)) == 0);
		}
		void SetCache(wchar_t c, bool cache_value)
		{
			int v = cache_value ? 0x1 : 0x2;
			pCache->bCharWidthCache[c/4] &= ~(0x3<< ((c%4)*2)); // 該当箇所クリア
			pCache->bCharWidthCache[c/4] |=  (v  << ((c%4)*2)); // 該当箇所セット
		}
		bool GetCache(wchar_t c) const
		{
			return _GetRaw(c) == 0x1;
		}
		bool ExistCache(wchar_t c) const
		{
			assert(pCache->nCharWidthCacheTest == 0x12345678);
			return _GetRaw(c) != 0x0;
		}
		bool CalcHankakuByFont(wchar_t c)
		{
			SIZE size = {han_size.cx * 2, 0}; // 関数が失敗したときのことを考え、全角幅で初期化しておく
			GetTextExtentPoint32W_AnyBuild(hdc, &c, 1, &size);
			return (size.cx <= han_size.cx);
		}
	protected:
		int _GetRaw(wchar_t c) const
		{
			return (pCache->bCharWidthCache[c/4]>>((c%4)*2))&0x3;
		}
	private:
		HDC					hdc;
		HFONT				hFontOld;
		HFONT				hFont;
		SIZE				han_size;
		LOGFONT				lf;
		CharWidthCache*	pCache;
	};

	class LocalCacheSelector {
	public:
		LocalCacheSelector()
		{
			pCache = &localCache[0];
			for (size_t i=0; i<(size_t)CharWidthFontMode::Max; ++i) {
				parCache[i] = 0;
			}
			lastEditCacheMode = CharWidthCacheMode::Neutral;
		}
		~LocalCacheSelector()
		{
			for (size_t i=0; i<(size_t)CharWidthFontMode::Max; ++i) {
				delete parCache[i];
				parCache[i] = 0;
			}
		}
		void Init(const LOGFONT& lf, CharWidthFontMode fMode)
	 	{
			// Fontfaceが変更されていたらキャッシュをクリアする
			localCache[(int)fMode].Init(lf);
			if (!localCache[(int)fMode].IsSameFontFace(lf)) {
				localCache[(int)fMode].Clear();
			}
		}
		void Select(CharWidthFontMode fMode, CharWidthCacheMode cMode)
		{
			CharWidthCacheMode cmode = (cMode == CharWidthCacheMode::Neutral) ? lastEditCacheMode : cMode;

			pCache = &localCache[(int)fMode];
			if (cmode == CharWidthCacheMode::Share) {
				pCache->SelectCache(&(GetDllShareData().charWidth));
			}else {
				if (parCache[(int)fMode] == 0) {
					parCache[(int)fMode] = new CharWidthCache;
				}
				pCache->SelectCache(parCache[(int)fMode]);
			}
			if (fMode == CharWidthFontMode::Edit) { lastEditCacheMode = cmode; }
		}
		LocalCache* GetCache() { return pCache; }
	private:
		LocalCache* pCache;
		LocalCache localCache[3];
		CharWidthCache* parCache[3];
		CharWidthCacheMode lastEditCacheMode;
	private:
		DISALLOW_COPY_AND_ASSIGN(LocalCacheSelector);
	};

	static LocalCacheSelector selector;


	// 文字幅の動的計算。半角ならtrue。
	bool CalcHankakuByFont(wchar_t c)
	{
		LocalCache* pCache = selector.GetCache();
		// -- -- キャッシュが存在すれば、それをそのまま返す -- -- //
		if (pCache->ExistCache(c)) {
			return pCache->GetCache(c);
		}

		// -- -- 相対比較 -- -- //
		bool value;
		value = pCache->CalcHankakuByFont(c);

		// -- -- キャッシュ更新 -- -- //
		pCache->SetCache(c, value);

		return pCache->GetCache(c);
	}
}

// 文字幅の動的計算用キャッシュの初期化。
void InitCharWidthCache(const LOGFONT& lf, CharWidthFontMode fMode)
{
	WCODE::selector.Init(lf, fMode);
}

// 文字幅の動的計算用キャッシュの選択
void SelectCharWidthCache(CharWidthFontMode fMode, CharWidthCacheMode cMode)
{
	assert(fMode == CharWidthFontMode::Edit || cMode == CharWidthCacheMode::Local);

	WCODE::selector.Select(fMode, cMode);
}
