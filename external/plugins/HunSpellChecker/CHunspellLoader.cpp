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
#include "CHunspellLoader.h"

///////////////////////////////////////////////////////////////////////////////
CHunspellLoader::CHunspellLoader()
{
	m_fn_Hunspell_create           = NULL;
	m_fn_Hunspell_create_key       = NULL;
	m_fn_Hunspell_destroy          = NULL;
	m_fn_Hunspell_spell            = NULL;
	m_fn_Hunspell_get_dic_encoding = NULL;
	m_fn_Hunspell_suggest          = NULL;
	m_fn_Hunspell_analyze          = NULL;
	m_fn_Hunspell_stem             = NULL;
	m_fn_Hunspell_stem2            = NULL;
	m_fn_Hunspell_generate         = NULL;
	m_fn_Hunspell_generate2        = NULL;
	m_fn_Hunspell_add              = NULL;
	m_fn_Hunspell_add_with_affix   = NULL;
	m_fn_Hunspell_remove           = NULL;
	m_fn_Hunspell_free_list        = NULL;
}

///////////////////////////////////////////////////////////////////////////////
CHunspellLoader::~CHunspellLoader()
{
}

///////////////////////////////////////////////////////////////////////////////
BOOL CHunspellLoader::RegisterEntries()
{
	const ImportTable table[] = {
		{ &m_fn_Hunspell_create,			"Hunspell_create" },
		{ &m_fn_Hunspell_create_key,		"Hunspell_create_key" },
		{ &m_fn_Hunspell_destroy,			"Hunspell_destroy" },
		{ &m_fn_Hunspell_spell,				"Hunspell_spell" },
		{ &m_fn_Hunspell_get_dic_encoding,	"Hunspell_get_dic_encoding" },
		{ &m_fn_Hunspell_suggest,			"Hunspell_suggest" },
		{ &m_fn_Hunspell_analyze,			"Hunspell_analyze" },
		{ &m_fn_Hunspell_stem,				"Hunspell_stem" },
		{ &m_fn_Hunspell_stem2,				"Hunspell_stem2" },
		{ &m_fn_Hunspell_generate,			"Hunspell_generate" },
		{ &m_fn_Hunspell_generate2,			"Hunspell_generate2" },
		{ &m_fn_Hunspell_add,				"Hunspell_add" },
		{ &m_fn_Hunspell_add_with_affix,	"Hunspell_add_with_affix" },
		{ &m_fn_Hunspell_remove,			"Hunspell_remove" },
		{ &m_fn_Hunspell_free_list,			"Hunspell_free_list" },
		{ NULL, NULL }
	};

	return CDllLoader::RegisterEntries(table);
}
