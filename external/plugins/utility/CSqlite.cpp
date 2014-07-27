/*
	Copyright (C) 2013, Plugins developers

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

#include "stdafx.h"
#include <windows.h>
#include "CSqlite.h"

///////////////////////////////////////////////////////////////////////////////
CSqlite::CSqlite()
{
	m_fn_sqlite3_bind_int      = NULL;
	m_fn_sqlite3_bind_text16   = NULL;
	m_fn_sqlite3_close         = NULL;
	m_fn_sqlite3_column_int    = NULL;
	m_fn_sqlite3_column_name16 = NULL;
	m_fn_sqlite3_column_text16 = NULL;
	m_fn_sqlite3_errmsg16      = NULL;
	m_fn_sqlite3_exec          = NULL;
	m_fn_sqlite3_finalize      = NULL;
	m_fn_sqlite3_open16        = NULL;
	m_fn_sqlite3_prepare16     = NULL;
	m_fn_sqlite3_reset         = NULL;
	m_fn_sqlite3_step          = NULL;
}

///////////////////////////////////////////////////////////////////////////////
CSqlite::~CSqlite()
{
}

///////////////////////////////////////////////////////////////////////////////
BOOL CSqlite::RegisterEntries()
{
	const ImportTable table[] = {
		{ &m_fn_sqlite3_bind_int,		"sqlite3_bind_int" },
		{ &m_fn_sqlite3_bind_text16,	"sqlite3_bind_text16" },
		{ &m_fn_sqlite3_close,			"sqlite3_close" },
		{ &m_fn_sqlite3_column_int,		"sqlite3_column_int" },
		{ &m_fn_sqlite3_column_name16,	"sqlite3_column_name16" },
		{ &m_fn_sqlite3_column_text16,	"sqlite3_column_text16" },
		{ &m_fn_sqlite3_errmsg16,		"sqlite3_errmsg16" },
		{ &m_fn_sqlite3_exec,			"sqlite3_exec" },
		{ &m_fn_sqlite3_finalize,		"sqlite3_finalize" },
		{ &m_fn_sqlite3_open16,			"sqlite3_open16" },
		{ &m_fn_sqlite3_prepare16,		"sqlite3_prepare16" },
		{ &m_fn_sqlite3_reset,			"sqlite3_reset" },
		{ &m_fn_sqlite3_step,			"sqlite3_step" },
		{ NULL, NULL }
	};

	return CDllLoader::RegisterEntries(table);
}
