/*!	@file
	@brief Ruby

	Rubyを利用するためのインターフェース

	@author Plugins developers
	@date 2013.12.25
*/
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

#include "StdAfx.h"
#include "CRuby.h"
#include "CPluginService.h"

///////////////////////////////////////////////////////////////////////////////
CRuby::CRuby()
{
	m_fn_ruby_sysinit               = NULL;
	m_fn_ruby_init                  = NULL;
	m_fn_ruby_init_loadpath         = NULL;
	m_fn_ruby_cleanup               = NULL;
	m_fn_rb_load                    = NULL;
	m_fn_rb_load_protect            = NULL;
	m_fn_rb_str_new                 = NULL;
	m_fn_rb_str_new2                = NULL;
	m_fn_rb_eval_string_protect     = NULL;
	m_fn_rb_errinfo                 = NULL;
	m_fn_rb_set_errinfo             = NULL;
	m_fn_rb_obj_as_string           = NULL;
	m_fn_rb_sourceline              = NULL;
	m_fn_rb_sourcefile              = NULL;
	m_fn_rb_string_value_cstr       = NULL;
	m_fn_rb_define_global_function  = NULL;
	m_fn_rb_define_class            = NULL;
	m_fn_rb_define_method           = NULL;
	m_fn_rb_define_protected_method = NULL;
	m_fn_rb_define_private_method   = NULL;
	m_fn_rb_define_singleton_method = NULL;
	m_fn_rb_singleton_class         = NULL;
	m_fn_rb_iv_set                  = NULL;
	m_fn_rb_iv_get                  = NULL;
	m_fn_rb_raise                   = NULL;
	m_fn_rb_fix2int                 = NULL;
	m_fn_rb_int2inum                = NULL;
	m_fn_rb_uint2inum               = NULL;
	m_fn_rb_check_type              = NULL;
	m_fn_rb_str_concat				= NULL;
	m_fn_rb_inspect					= NULL;
	m_fn_rb_str_cat2				= NULL;

	m_val_ruby_api_version          = NULL;
	m_val_ruby_version              = NULL;
	m_val_rb_cObject                = NULL;
	m_val_rb_eRuntimeError          = NULL;
	m_val_rb_eTypeError             = NULL;
}

///////////////////////////////////////////////////////////////////////////////
CRuby::~CRuby()
{
}

///////////////////////////////////////////////////////////////////////////////
/*!	DLLを初期化する
*/
BOOL CRuby::RegisterEntries()
{
	//DLLが持っている関数を準備する
	const ImportTable table[] = {
		{ &m_fn_ruby_sysinit,				"ruby_sysinit" },
		{ &m_fn_ruby_init,					"ruby_init" },
		{ &m_fn_ruby_init_loadpath,			"ruby_init_loadpath" },
		{ &m_fn_ruby_cleanup,				"ruby_cleanup" },
		{ &m_fn_rb_load,					"rb_load" },
		{ &m_fn_rb_load_protect,			"rb_load_protect" },
		{ &m_fn_rb_str_new,					"rb_str_new" },
		{ &m_fn_rb_str_new2,				"rb_str_new2" },
		{ &m_fn_rb_eval_string_protect,		"rb_eval_string_protect" },
		{ &m_fn_rb_errinfo,					"rb_errinfo" },
		{ &m_fn_rb_set_errinfo,				"rb_set_errinfo" },
		{ &m_fn_rb_obj_as_string,			"rb_obj_as_string" },
		{ &m_fn_rb_sourceline,				"rb_sourceline" },
		{ &m_fn_rb_sourcefile,				"rb_sourcefile" },
		{ &m_fn_rb_string_value_cstr,		"rb_string_value_cstr" },
		{ &m_fn_rb_define_global_function,	"rb_define_global_function" },
		{ &m_fn_rb_define_class,			"rb_define_class" },
		{ &m_fn_rb_define_method,			"rb_define_method" },
		{ &m_fn_rb_define_protected_method,	"rb_define_protected_method" },
		{ &m_fn_rb_define_private_method,	"rb_define_private_method" },
		{ &m_fn_rb_define_singleton_method,	"rb_define_singleton_method" },
		{ &m_fn_rb_singleton_class,			"rb_singleton_class" },
		{ &m_fn_rb_iv_set,					"rb_iv_set" },
		{ &m_fn_rb_iv_get,					"rb_iv_get" },
		{ &m_fn_rb_raise,					"rb_raise" },
		{ &m_fn_rb_fix2int,					"rb_fix2int" },
		{ &m_fn_rb_int2inum,				"rb_int2inum" },
		{ &m_fn_rb_uint2inum,				"rb_uint2inum" },
		{ &m_fn_rb_check_type,				"rb_check_type" },
		{ &m_fn_rb_str_concat,				"rb_str_concat" },
		{ &m_fn_rb_inspect,					"rb_inspect" },
		{ &m_fn_rb_str_cat2,				"rb_str_cat2" },

		{ &m_val_ruby_api_version,			"ruby_api_version" },
		{ &m_val_ruby_version,				"ruby_version" },
		{ &m_val_rb_cObject,				"rb_cObject" },
		{ &m_val_rb_eRuntimeError,			"rb_eRuntimeError" },
		{ &m_val_rb_eTypeError,				"rb_eTypeError" },

		{ NULL, 0 }
	};

	return CDllLoader::RegisterEntries(table);
}
