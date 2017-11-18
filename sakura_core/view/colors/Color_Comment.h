#pragma once

#include "view/colors/ColorStrategy.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class Color_LineComment : public ColorStrategy {
public:
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_COMMENT; }
	virtual void InitStrategyStatus() {}
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool EndColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return pTypeData->colorInfoArr[COLORIDX_COMMENT].bDisp; }
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class Color_BlockComment : public ColorStrategy {
public:
	Color_BlockComment(EColorIndexType nType) : type(nType), nCommentEnd(0) {}
	virtual void Update(void)
	{
		const EditDoc* pEditDoc = EditDoc::GetInstance(0);
		pTypeData = &pEditDoc->docType.GetDocumentAttribute();
		pBlockComment = &pTypeData->blockComments[type - COLORIDX_BLOCK1];
	}
	virtual EColorIndexType GetStrategyColor() const { return type; }
	virtual void InitStrategyStatus() { nCommentEnd = 0; }
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool EndColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return pTypeData->colorInfoArr[COLORIDX_COMMENT].bDisp; }
private:
	EColorIndexType type;
	const BlockComment* pBlockComment;
	size_t nCommentEnd;
};

