#pragma once

#include "view/colors/ColorStrategy.h"

class Color_Select : public ColorStrategy {
public:
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_SELECT; }
	// �F�ւ�
	virtual void InitStrategyStatus() { }
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return true; }
	virtual bool EndColor(const StringRef& str, size_t nPos);

	virtual bool BeginColorEx(const StringRef& str, int nPos, int, const Layout*);

	// �C�x���g
	virtual void OnStartScanLogic();

private:
	int nSelectLine;
	int nSelectStart;
	int nSelectEnd;
};

class Color_Found : public ColorStrategy {
public:
	Color_Found();
	virtual EColorIndexType GetStrategyColor() const {
		return
			this->validColorNum != 0
			? this->highlightColors[ (nSearchResult - 1) % this->validColorNum ]
			: COLORIDX_DEFAULT
		;
	}
	// �F�ւ�
	virtual void InitStrategyStatus() { } //############�v����
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return true; }
	virtual bool EndColor(const StringRef& str, size_t nPos);
	// �C�x���g
	virtual void OnStartScanLogic();

private:
	int nSearchResult;
	int nSearchStart;
	int nSearchEnd;
	EColorIndexType highlightColors[ COLORIDX_SEARCHTAIL - COLORIDX_SEARCH + 1 ]; ///< �`�F�b�N���t���Ă��錟��������F�̔z��B
	unsigned validColorNum; ///< highlightColors�̉��Ԗڂ̗v�f�܂ł��L�����B
};

