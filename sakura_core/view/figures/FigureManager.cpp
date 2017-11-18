#include "StdAfx.h"
#include "FigureManager.h"
#include "Figure_Tab.h"
#include "Figure_HanSpace.h"
#include "Figure_ZenSpace.h"
#include "Figure_Eol.h"
#include "Figure_CtrlCode.h"

FigureManager::FigureManager()
{
	vFigures.push_back(new Figure_Tab());
	vFigures.push_back(new Figure_HanSpace());
	vFigures.push_back(new Figure_ZenSpace());
	vFigures.push_back(new Figure_Eol());
	vFigures.push_back(new Figure_CtrlCode());
	vFigures.push_back(new Figure_HanBinary());
	vFigures.push_back(new Figure_ZenBinary());
	vFigures.push_back(new Figure_Text());

	OnChangeSetting();
}

FigureManager::~FigureManager()
{
	vFiguresDisp.clear();

	int size = (int)vFigures.size();
	for (int i=0; i<size; ++i) {
		SAFE_DELETE(vFigures[i]);
	}
	vFigures.clear();
}

//$$ �������\
Figure& FigureManager::GetFigure(const wchar_t* pText, int nTextLen)
{
	int size = (int)vFiguresDisp.size();
	for (int i=0; i<size; ++i) {
		Figure* pFigure = vFiguresDisp[i];
		if (pFigure->Match(pText, nTextLen)) {
			return *pFigure;
		}
	}

	assert(0);
	return *vFiguresDisp.back();
}

// �ݒ�X�V
void FigureManager::OnChangeSetting(void)
{
	vFiguresDisp.clear();

	int size = (int)vFigures.size();
	for (int i=0; i<size; ++i) {
		vFigures[i]->Update();
		// �F�����\���Ώۂ݂̂�o�^
		if (vFigures[i]->Disp()) {
			vFiguresDisp.push_back(vFigures[i]);
		}
	}
}

