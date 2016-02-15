/*!	@file
	@brief 指差しカーソル

	@author Uchi
	@date 2013年1月29日
*/
/*
	Copyright (C) 2013, Uchi

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

