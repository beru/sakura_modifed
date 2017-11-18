#pragma once

#include <vector>
#include "util/design_template.h"
#include "FigureStrategy.h"

class FigureManager : public TSingleton<FigureManager> {
	friend class TSingleton<FigureManager>;
	FigureManager();
	virtual ~FigureManager();

public:
	// �`�悷��Figure���擾
	//	@param	pText	�Ώە�����̐擪
	//	@param	nTextLen	pText����s���܂ł̒���(������CRLF==2)
	Figure& GetFigure(const wchar_t* pText, int nTextLen);

	// �ݒ�ύX
	void OnChangeSetting(void);

private:
	std::vector<Figure*>	vFigures;
	std::vector<Figure*>	vFiguresDisp;	// �F�����\���Ώ�
};

