#include "StdAfx.h"
#include "DispPos.h"
#include "doc/layout/Layout.h"

// $$$高速化
void DispPos::ForwardLayoutLineRef(int nOffsetLine)
{
	nLineRef += nOffsetLine;
	// キャッシュ更新
	int n = nOffsetLine;
	if (pLayoutRef) {
		while (n > 0 && pLayoutRef) {
			pLayoutRef = pLayoutRef->GetNextLayout();
			--n;
		}
		while (n < 0 && pLayoutRef) {
			pLayoutRef = pLayoutRef->GetPrevLayout();
			++n;
		}
	}else {
		pLayoutRef = EditDoc::GetInstance(0)->layoutMgr.SearchLineByLayoutY(nLineRef);
	}
}

