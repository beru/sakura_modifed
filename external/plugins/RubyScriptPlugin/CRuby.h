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

#ifndef _CRUBY_H_
#define _CRUBY_H_

#include <stdio.h>
#include <objbase.h>
#include <ObjIdl.h>
#include "CDllLoader.h"
#include "CPluginService.h"

///////////////////////////////////////////////////////////////////////////////
typedef uintptr_t VALUE;
#define RUBY_NULL        ((VALUE)4)
#define RUBY_TYPE_STRING 0x05
#define RUBY_TYPE_FIXNUM 0x15

///////////////////////////////////////////////////////////////////////////////
/*!	msvcrt-ruby200.dllをサポートするクラス
*/
class CRuby : public CDllLoader
{
public:
	CRuby();
	virtual ~CRuby();

protected:
	virtual BOOL RegisterEntries();

private:
	//ruby api
	typedef void  (__cdecl *API_ruby_sysinit)(int*, char***);
	typedef void  (__cdecl *API_ruby_init)();
	typedef void  (__cdecl *API_ruby_init_loadpath)();
	typedef int   (__cdecl *API_ruby_cleanup)(volatile int);
	typedef void  (__cdecl *API_rb_load)(VALUE, int);
	typedef void  (__cdecl *API_rb_load_protect)(VALUE, int, int*);
	typedef VALUE (__cdecl *API_rb_str_new)(const char*, long);
	typedef VALUE (__cdecl *API_rb_str_new2)(const char*);
	typedef VALUE (__cdecl *API_rb_eval_string_protect)(const char*, int*);
	typedef VALUE (__cdecl *API_rb_errinfo)();
	typedef void  (__cdecl *API_rb_set_errinfo)(VALUE);
	typedef VALUE (__cdecl *API_rb_obj_as_string)(VALUE);
	typedef int   (__cdecl *API_rb_sourceline)(void);
	typedef const char* (__cdecl *API_rb_sourcefile)(void);
//	typedef int   (__cdecl *API_rb_type)(VALUE);	//inline function
	typedef char* (__cdecl *API_rb_string_value_cstr)(volatile VALUE*);
	typedef void  (__cdecl *API_rb_define_global_function)(const char*, VALUE(*)(...), int);
	typedef VALUE (__cdecl *API_rb_define_class)(const char* name, VALUE);
	typedef void  (__cdecl *API_rb_define_method)(VALUE, const char*,VALUE(*)(...), int);
	typedef void  (__cdecl *API_rb_define_protected_method)(VALUE, const char*, VALUE (*)(...), int);
	typedef void  (__cdecl *API_rb_define_private_method)(VALUE, const char*, VALUE (*)(...), int);
	typedef void  (__cdecl *API_rb_define_singleton_method)(VALUE, const char*, VALUE(*)(...), int);
	typedef VALUE (__cdecl *API_rb_singleton_class)(VALUE);
	typedef VALUE (__cdecl *API_rb_iv_set)(VALUE, const char*, VALUE);
	typedef VALUE (__cdecl *API_rb_iv_get)(VALUE, const char*);
	typedef void  (__cdecl *API_rb_raise)(VALUE, const char*, ...);
	typedef long  (__cdecl *API_rb_fix2int)(VALUE);
	typedef void  (__cdecl *API_rb_check_type)(VALUE, int);
	typedef VALUE (__cdecl *API_rb_int2inum)(long);
	typedef VALUE (__cdecl *API_rb_uint2inum)(VALUE);
	typedef VALUE (__cdecl *API_rb_str_concat)(VALUE, VALUE);
	typedef VALUE (__cdecl *API_rb_inspect)(VALUE);
	typedef VALUE (__cdecl *API_rb_str_cat2)(VALUE, const char*);

	API_ruby_sysinit				m_fn_ruby_sysinit;
	API_ruby_init					m_fn_ruby_init;
	API_ruby_init_loadpath			m_fn_ruby_init_loadpath;
	API_ruby_cleanup				m_fn_ruby_cleanup;
	API_rb_load						m_fn_rb_load;
	API_rb_load_protect				m_fn_rb_load_protect;
	API_rb_str_new					m_fn_rb_str_new;
	API_rb_str_new2					m_fn_rb_str_new2;
	API_rb_eval_string_protect		m_fn_rb_eval_string_protect;
	API_rb_errinfo					m_fn_rb_errinfo;
	API_rb_set_errinfo				m_fn_rb_set_errinfo;
	API_rb_obj_as_string			m_fn_rb_obj_as_string;
	API_rb_sourceline				m_fn_rb_sourceline;
	API_rb_sourcefile				m_fn_rb_sourcefile;
	API_rb_string_value_cstr		m_fn_rb_string_value_cstr;
	API_rb_define_global_function	m_fn_rb_define_global_function;
	API_rb_define_class				m_fn_rb_define_class;
	API_rb_define_method			m_fn_rb_define_method;
	API_rb_define_protected_method	m_fn_rb_define_protected_method;
	API_rb_define_private_method	m_fn_rb_define_private_method;
	API_rb_define_singleton_method	m_fn_rb_define_singleton_method;
	API_rb_singleton_class			m_fn_rb_singleton_class;
	API_rb_iv_set					m_fn_rb_iv_set;
	API_rb_iv_get					m_fn_rb_iv_get;
	API_rb_raise					m_fn_rb_raise;
	API_rb_fix2int					m_fn_rb_fix2int;
	API_rb_int2inum					m_fn_rb_int2inum;
	API_rb_uint2inum				m_fn_rb_uint2inum;
	API_rb_check_type				m_fn_rb_check_type;
	API_rb_str_concat				m_fn_rb_str_concat;
	API_rb_inspect					m_fn_rb_inspect;
	API_rb_str_cat2					m_fn_rb_str_cat2;
	const int*						m_val_ruby_api_version;
	const char*						m_val_ruby_version;
	VALUE*							m_val_rb_cObject;
	VALUE*							m_val_rb_eRuntimeError;
	VALUE*							m_val_rb_eTypeError;

public:
	//API
	void ruby_sysinit(int* argc, char*** argv){
		m_fn_ruby_sysinit(argc, argv);
	}
	void ruby_init(){
		m_fn_ruby_init();
	}
	void ruby_init_loadpath(){
		m_fn_ruby_init_loadpath();
	}
	int ruby_cleanup(volatile int state){
		return m_fn_ruby_cleanup(state);
	}
	void rb_load(VALUE fname, int wrap){
		m_fn_rb_load(fname, wrap);
	}
	void rb_load_protect(VALUE fname, int wrap, int* state){
		m_fn_rb_load_protect(fname, wrap, state);
	}
	VALUE rb_str_new(const char* ptr, long len){
		return m_fn_rb_str_new(ptr, len);
	}
	VALUE rb_str_new2(const char* ptr){
		return m_fn_rb_str_new2(ptr);
	}
	VALUE rb_eval_string_protect(const char* str, int* state){
		return m_fn_rb_eval_string_protect(str, state);
	}
	VALUE rb_errinfo(){
		return m_fn_rb_errinfo();
	}
	void rb_set_errinfo(VALUE obj){
		m_fn_rb_set_errinfo(obj);
	}
	VALUE rb_obj_as_string(VALUE obj){
		return m_fn_rb_obj_as_string(obj);
	}
	int rb_sourceline(void){
		return m_fn_rb_sourceline();
	}
	const char* rb_sourcefile(void){
		return m_fn_rb_sourcefile();
	}
	char* rb_string_value_cstr(volatile VALUE* str){
		return m_fn_rb_string_value_cstr(str);
	}
	void rb_define_global_function(const char* name, VALUE(*func)(...), int argc){
		m_fn_rb_define_global_function(name, func, argc);
	}
	VALUE rb_define_class(const char* name, VALUE super){
		return m_fn_rb_define_class(name, super);
	}
	void rb_define_method(VALUE klass, const char* name,VALUE(*func)(...), int argc){
		m_fn_rb_define_method(klass, name, func, argc);
	}
	void rb_define_protected_method(VALUE klass, const char* name, VALUE (*func)(...), int argc){
		m_fn_rb_define_protected_method(klass, name, func, argc);
	}
	void rb_define_private_method(VALUE klass, const char* name, VALUE (*func)(...), int argc){
		m_fn_rb_define_private_method(klass, name, func, argc);
	}
	void rb_define_singleton_method(VALUE obj, const char* name, VALUE(*func)(...), int argc){
		m_fn_rb_define_singleton_method(obj, name, func, argc);
	}
	VALUE rb_singleton_class(VALUE obj){
		return m_fn_rb_singleton_class(obj);
	}
	VALUE rb_iv_set(VALUE obj, const char* name, VALUE val){
		return m_fn_rb_iv_set(obj, name, val);
	}
	VALUE rb_iv_get(VALUE obj, const char* name){
		return m_fn_rb_iv_get(obj, name);
	}
	void rb_raise(VALUE obj, const char* str){	//固定長引数として利用する
		return m_fn_rb_raise(obj, str);
	}
	long rb_fix2int(VALUE obj){
		return m_fn_rb_fix2int(obj);
	}
	void rb_check_type(VALUE obj, int type){
		m_fn_rb_check_type(obj, type);
		//not reached
	}
	VALUE rb_int2inum(long value){
		return m_fn_rb_int2inum(value);
	}
	VALUE rb_uint2inum(VALUE value){
		return m_fn_rb_uint2inum(value);
	}
	VALUE rb_str_concat(VALUE str1, VALUE str2){
		return m_fn_rb_str_concat(str1, str2);
	}
	VALUE rb_inspect(VALUE obj){
		return m_fn_rb_inspect(obj);
	}
	VALUE rb_str_cat2(VALUE obj, const char* str){
		return m_fn_rb_str_cat2(obj, str);
	}

	VALUE rb_rcObject(){
		return *m_val_rb_cObject;
	}
	VALUE rb_eRuntimeError(){
		return *m_val_rb_eRuntimeError;
	}
	VALUE rb_eTypeError(){
		return *m_val_rb_eTypeError;
	}

	//Alias
	char* StringValueCStr(VALUE& v){
		//RSTRING(err)->as.heap.ptrと等価
		return rb_string_value_cstr(&v);
	}
};

#endif	//_CRUBY_H_
