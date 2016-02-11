/*
	Copyright (C) 2008, kobake

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

#include "view/colors/CColorStrategy.h"

class Color_Select : public ColorStrategy {
public:
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_SELECT; }
	// �F�ւ�
	virtual void InitStrategyStatus() { }
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool Disp() const { return true; }
	virtual bool EndColor(const CStringRef& cStr, int nPos);

	virtual bool BeginColorEx(const CStringRef& cStr, int nPos, LayoutInt, const Layout*);

	// �C�x���g
	virtual void OnStartScanLogic();

private:
	LayoutInt	m_nSelectLine;
	LogicInt	m_nSelectStart;
	LogicInt	m_nSelectEnd;
};

class Color_Found : public ColorStrategy {
public:
	Color_Found();
	virtual EColorIndexType GetStrategyColor() const {
		return
			this->validColorNum != 0
			? this->highlightColors[ (m_nSearchResult - 1) % this->validColorNum ]
			: COLORIDX_DEFAULT
		;
	}
	// �F�ւ�
	virtual void InitStrategyStatus() { } //############�v����
	virtual bool BeginColor(const CStringRef& cStr, int nPos);
	virtual bool Disp() const { return true; }
	virtual bool EndColor(const CStringRef& cStr, int nPos);
	// �C�x���g
	virtual void OnStartScanLogic();

private:
	int				m_nSearchResult;
	LogicInt		m_nSearchStart;
	LogicInt		m_nSearchEnd;
	EColorIndexType highlightColors[ COLORIDX_SEARCHTAIL - COLORIDX_SEARCH + 1 ]; ///< �`�F�b�N���t���Ă��錟��������F�̔z��B
	unsigned validColorNum; ///< highlightColors�̉��Ԗڂ̗v�f�܂ł��L�����B
};

