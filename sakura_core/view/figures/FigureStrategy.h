#pragma once

#include <vector>
#include "view/colors/ColorStrategy.h" // ColorStrategyInfo


//$$���C�A�E�g�\�z�t���[(DoLayout)�� Figure �ōs���Ɛ������₷��
class Figure {
public:
	virtual ~Figure() {}
	virtual bool DrawImp(ColorStrategyInfo& csInfo) = 0;
	virtual bool Match(const wchar_t* pText, int nTextLen) const = 0;

	// �F�����\���Ώ۔���
	virtual bool Disp(void) const = 0;

	// �ݒ�X�V
	virtual void Update(void) {
		EditDoc* pEditDoc = EditDoc::GetInstance(0);
		pTypeData = &pEditDoc->docType.GetDocumentAttribute();
	}
protected:
	const TypeConfig* pTypeData;
};

// �ʏ�e�L�X�g�`��
class Figure_Text : public Figure {
public:
	bool DrawImp(ColorStrategyInfo& csInfo) override;
	bool Match(const wchar_t* pText, int nTextLen) const {
		return true;
	}

	// �F�����\���Ώ۔���
	virtual bool Disp(void) const {
		return true;
	}
};

// �e��󔒁i���p�󔒁^�S�p�󔒁^�^�u�^���s�j�`��p�̊�{�N���X
class FigureSpace : public Figure {
public:
	virtual bool DrawImp(ColorStrategyInfo& csInfo) override;
protected:
	virtual void DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const = 0;
	virtual EColorIndexType GetColorIdx(void) const = 0;

	// �F�����\���Ώ۔���
	virtual bool Disp(void) const {
		EColorIndexType nColorIndex = GetColorIdx();
		return pTypeData->colorInfoArr[nColorIndex].bDisp;
	}

	virtual void Update(void) {
		Figure::Update();

		EColorIndexType nColorIndex = GetColorIdx();
		if (pTypeData->colorInfoArr[nColorIndex].bDisp) {
			nDispColorIndex = nColorIndex;
		}else {
			nDispColorIndex = COLORIDX_TEXT;
		}
	}

	EColorIndexType GetDispColorIdx(void) const { return nDispColorIndex; }

	// �����⏕
	bool DrawImp_StyleSelect(ColorStrategyInfo& csInfo);
	void DrawImp_StylePop(ColorStrategyInfo& csInfo);
	void DrawImp_DrawUnderline(ColorStrategyInfo& csInfo, DispPos&);

protected:
	EColorIndexType nDispColorIndex;
};

