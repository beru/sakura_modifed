#pragma once

#include "_os/OsVersionInfo.h"
#include "sakura_rc.h"

#ifndef IDC_HAND
#define IDC_HAND	MAKEINTRESOURCE(32649)
#endif

inline
void SetHandCursor ()
{
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
	}
}

