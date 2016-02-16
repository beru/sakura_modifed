#include "StdAfx.h"
#include "MyRect.h"

Rect MergeRect(const Rect& rc1, const Rect& rc2)
{
	return Rect(
		t_min(rc1.left,		rc2.left),
		t_min(rc1.top,		rc2.top),
		t_max(rc1.right,	rc2.right),
		t_max(rc1.bottom,	rc2.bottom)
	);
}
