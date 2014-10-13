/*!	@file
	@brief assert�֐�

*/
/*
	Copyright (C) 2007, kobake

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

// 2007.08.30 kobake �ǉ�
#ifdef assert
#undef assert
#endif

#ifdef _DEBUG
	void debug_output(const char* str, ...);
	void debug_exit();
	void warning_point();

	#define assert(exp) \
	{ \
		if (!(exp)) { \
			debug_output("!assert: %hs(%d): %hs\n", __FILE__, __LINE__, #exp); \
			debug_exit(); \
		} \
	}

	#define assert_warning(exp) \
	{ \
		if (!(exp)) { \
			debug_output("!warning: %hs(%d): %hs\n", __FILE__, __LINE__, #exp); \
			warning_point(); \
		} \
	}

#else
	#define assert(exp)
	#define assert_warning(exp)
#endif

