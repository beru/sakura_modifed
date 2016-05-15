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

#include "view/colors/ColorStrategy.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class Color_LineComment : public ColorStrategy {
public:
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_COMMENT; }
	virtual void InitStrategyStatus() {}
	virtual bool BeginColor(const StringRef& str, int nPos);
	virtual bool EndColor(const StringRef& str, int nPos);
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
	virtual bool BeginColor(const StringRef& str, int nPos);
	virtual bool EndColor(const StringRef& str, int nPos);
	virtual bool Disp() const { return pTypeData->colorInfoArr[COLORIDX_COMMENT].bDisp; }
private:
	EColorIndexType type;
	const BlockComment* pBlockComment;
	int nCommentEnd;
};

