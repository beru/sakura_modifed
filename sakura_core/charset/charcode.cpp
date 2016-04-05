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

#include "StdAfx.h"
#include "charset/charcode.h"

#include "env/DllSharedData.h"

// �L�[���[�h�L�����N�^
const unsigned char gm_keyword_char[128] = {
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
		��Ǔ_��
		2008.04.27 kobake CLayoutMgr::IsKutoTen ���番��

		@param[in] c1 ���ׂ镶��1�o�C�g��
		@param[in] c2 ���ׂ镶��2�o�C�g��
		@retval true ��Ǔ_�ł���
		@retval false ��Ǔ_�łȂ�
	*/
	bool IsKutoten(wchar_t wc)
	{
		// ��Ǔ_��`
		static const wchar_t* KUTOTEN =
			L"��,."
			L"�B�A�C�D"
		;

		const wchar_t* p;
		for (p = KUTOTEN; *p; ++p) {
			if (*p == wc) return true;
		}
		return false;
	}
#endif


	/*!
		UNICODE�������̃L���b�V���N���X�B
		1����������2�r�b�g�ŁA�l��ۑ����Ă����B
		00:��������
		01:���p
		10:�S�p
		11:-
	*/
	class LocalCache {
	public:
		LocalCache()
		{
			// LOGFONT�̏�����
			memset(&m_lf, 0, sizeof(m_lf));

			// HDC �̏�����
			HDC hdc = GetDC(NULL);
			m_hdc = CreateCompatibleDC(hdc);
			ReleaseDC(NULL, hdc);

			m_hFont = NULL;
			m_hFontOld = NULL;
			m_pCache = 0;
		}
		~LocalCache()
		{
			// -- -- ��n�� -- -- //
			if (m_hFont) {
				SelectObject(m_hdc, m_hFontOld);
				DeleteObject(m_hFont);
			}
			DeleteDC(m_hdc);
		}
		// �ď�����
		void Init(const LOGFONT& lf)
		{
			if (m_hFontOld) {
				m_hFontOld = (HFONT)SelectObject(m_hdc, m_hFontOld);
				DeleteObject(m_hFontOld);
			}

			m_lf = lf;

			m_hFont = ::CreateFontIndirect(&lf);
			m_hFontOld = (HFONT)SelectObject(m_hdc, m_hFont);

			// -- -- ���p� -- -- //
			GetTextExtentPoint32W_AnyBuild(m_hdc, L"x", 1, &m_han_size);
		}
		void SelectCache(CharWidthCache* pCache)
		{
			m_pCache = pCache;
		}
		void Clear()
		{
			assert(m_pCache != 0);
			// �L���b�V���̃N���A
			memcpy(m_pCache->lfFaceName, m_lf.lfFaceName, sizeof(m_lf.lfFaceName));
			memset(m_pCache->bCharWidthCache, 0, sizeof(m_pCache->bCharWidthCache));
			m_pCache->nCharWidthCacheTest = 0x12345678;
		}
		bool IsSameFontFace(const LOGFONT& lf)
		{
			assert(m_pCache != 0);
			return (memcmp(m_pCache->lfFaceName, lf.lfFaceName, sizeof(lf.lfFaceName)) == 0);
		}
		void SetCache(wchar_t c, bool cache_value)
		{
			int v = cache_value ? 0x1 : 0x2;
			m_pCache->bCharWidthCache[c/4] &= ~(0x3<< ((c%4)*2)); // �Y���ӏ��N���A
			m_pCache->bCharWidthCache[c/4] |=  (v  << ((c%4)*2)); // �Y���ӏ��Z�b�g
		}
		bool GetCache(wchar_t c) const
		{
			return _GetRaw(c) == 0x1;
		}
		bool ExistCache(wchar_t c) const
		{
			assert(m_pCache->nCharWidthCacheTest == 0x12345678);
			return _GetRaw(c) != 0x0;
		}
		bool CalcHankakuByFont(wchar_t c)
		{
			SIZE size = {m_han_size.cx * 2, 0}; // �֐������s�����Ƃ��̂��Ƃ��l���A�S�p���ŏ��������Ă���
			GetTextExtentPoint32W_AnyBuild(m_hdc, &c, 1, &size);
			return (size.cx <= m_han_size.cx);
		}
	protected:
		int _GetRaw(wchar_t c) const
		{
			return (m_pCache->bCharWidthCache[c/4]>>((c%4)*2))&0x3;
		}
	private:
		HDC					m_hdc;
		HFONT				m_hFontOld;
		HFONT				m_hFont;
		SIZE				m_han_size;
		LOGFONT				m_lf;				// 2008/5/15 Uchi
		CharWidthCache*	m_pCache;
	};

	class LocalCacheSelector {
	public:
		LocalCacheSelector()
		{
			m_pCache = &m_localCache[0];
			for (int i=0; i<(int)CharWidthFontMode::Max; ++i) {
				m_parCache[i] = 0;
			}
			m_lastEditCacheMode = CharWidthCacheMode::Neutral;
		}
		~LocalCacheSelector()
		{
			for (int i=0; i<(int)CharWidthFontMode::Max; ++i) {
				delete m_parCache[i];
				m_parCache[i] = 0;
			}
		}
		void Init(const LOGFONT& lf, CharWidthFontMode fMode)
	 	{
			// Fontface���ύX����Ă�����L���b�V�����N���A����	2013.04.08 aroka
			m_localCache[(int)fMode].Init(lf);
			if (!m_localCache[(int)fMode].IsSameFontFace(lf)) {
				m_localCache[(int)fMode].Clear();
			}
		}
		void Select(CharWidthFontMode fMode, CharWidthCacheMode cMode)
		{
			CharWidthCacheMode cmode = (cMode == CharWidthCacheMode::Neutral) ? m_lastEditCacheMode : cMode;

			m_pCache = &m_localCache[(int)fMode];
			if (cmode == CharWidthCacheMode::Share) {
				m_pCache->SelectCache(&(GetDllShareData().charWidth));
			}else {
				if (m_parCache[(int)fMode] == 0) {
					m_parCache[(int)fMode] = new CharWidthCache;
				}
				m_pCache->SelectCache(m_parCache[(int)fMode]);
			}
			if (fMode == CharWidthFontMode::Edit) { m_lastEditCacheMode = cmode; }
		}
		LocalCache* GetCache() { return m_pCache; }
	private:
		LocalCache* m_pCache;
		LocalCache m_localCache[3];
		CharWidthCache* m_parCache[3];
		CharWidthCacheMode m_lastEditCacheMode;
	private:
		DISALLOW_COPY_AND_ASSIGN(LocalCacheSelector);
	};

	static LocalCacheSelector selector;


	// �������̓��I�v�Z�B���p�Ȃ�true�B
	bool CalcHankakuByFont(wchar_t c)
	{
		LocalCache* pCache = selector.GetCache();
		// -- -- �L���b�V�������݂���΁A��������̂܂ܕԂ� -- -- //
		if (pCache->ExistCache(c)) {
			return pCache->GetCache(c);
		}

		// -- -- ���Δ�r -- -- //
		bool value;
		value = pCache->CalcHankakuByFont(c);

		// -- -- �L���b�V���X�V -- -- //
		pCache->SetCache(c, value);

		return pCache->GetCache(c);
	}
}

// �������̓��I�v�Z�p�L���b�V���̏������B	2007/5/18 Uchi
void InitCharWidthCache(const LOGFONT& lf, CharWidthFontMode fMode)
{
	WCODE::selector.Init(lf, fMode);
}

// �������̓��I�v�Z�p�L���b�V���̑I��	2013.04.08 aroka
void SelectCharWidthCache(CharWidthFontMode fMode, CharWidthCacheMode cMode)
{
	assert(fMode == CharWidthFontMode::Edit || cMode == CharWidthCacheMode::Local);

	WCODE::selector.Select(fMode, cMode);
}
