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

#ifndef _PLUGIN_SQLITE_H_
#define _PLUGIN_SQLITE_H_

#include <windows.h>
#include <string>
#include "CDllLoader.h"

///////////////////////////////////////////////////////////////////////////////
typedef struct sqlite3      sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
typedef __int64             sqlite3_int64;
typedef unsigned __int64    sqlite3_uint64;
typedef void (*sqlite3_destructor_type)(void*);
#define SQLITE_STATIC       ((sqlite3_destructor_type)0)
#define SQLITE_TRANSIENT    ((sqlite3_destructor_type)-1)

#define SQLITE_OK           0	//Successful result
#define SQLITE_ROW          100	//sqlite3_step() has another row ready
#define SQLITE_DONE         101	//sqlite3_step() has finished executing

///////////////////////////////////////////////////////////////////////////////
class CSqlite : public CDllLoader
{
public:
	CSqlite();
	virtual ~CSqlite();

protected:
	virtual BOOL RegisterEntries();

private:
	typedef int (__cdecl *API_sqlite3_bind_int)(sqlite3_stmt* pStmt, int index, int data);
	typedef int (__cdecl *API_sqlite3_bind_text16)(sqlite3_stmt* pStmt, int index, const void* data, int length, void(*destructor)(void*));
	typedef int (__cdecl *API_sqlite3_close)(sqlite3* db);
	typedef int (__cdecl *API_sqlite3_column_int)(sqlite3_stmt* pStmt, int iCol);
	typedef const void* (__cdecl *API_sqlite3_column_name16)(sqlite3_stmt* pStmt, int N);
	typedef const void* (__cdecl *API_sqlite3_column_text16)(sqlite3_stmt* pStmt, int iCol);
	typedef const void* (__cdecl *API_sqlite3_errmsg16)(sqlite3* db);
	typedef int (__cdecl *API_sqlite3_exec)(sqlite3* db, const char* sql, int (*callback)(void*, int, char**, char**), void* arg, char** errmsg);
	typedef int (__cdecl *API_sqlite3_extended_errcode)(sqlite3* db);
	typedef int (__cdecl *API_sqlite3_finalize)(sqlite3_stmt* pStmt);
	typedef int (__cdecl *API_sqlite3_open16)(const void* filename, sqlite3** ppDb);
	typedef int (__cdecl *API_sqlite3_prepare16)(sqlite3* db, const void* zSql, int nByte, sqlite3_stmt** ppStmt, const void** psTail);
	typedef int (__cdecl *API_sqlite3_reset)(sqlite3_stmt* pStmt);
	typedef int (__cdecl *API_sqlite3_step)(sqlite3_stmt* pStmt);

	API_sqlite3_bind_int			m_fn_sqlite3_bind_int;
	API_sqlite3_bind_text16			m_fn_sqlite3_bind_text16;
	API_sqlite3_close				m_fn_sqlite3_close;
	API_sqlite3_column_int			m_fn_sqlite3_column_int;
	API_sqlite3_column_name16		m_fn_sqlite3_column_name16;
	API_sqlite3_column_text16		m_fn_sqlite3_column_text16;
	API_sqlite3_errmsg16			m_fn_sqlite3_errmsg16;
	API_sqlite3_exec				m_fn_sqlite3_exec;
	API_sqlite3_finalize			m_fn_sqlite3_finalize;
	API_sqlite3_open16				m_fn_sqlite3_open16;
	API_sqlite3_prepare16			m_fn_sqlite3_prepare16;
	API_sqlite3_reset				m_fn_sqlite3_reset;
	API_sqlite3_step				m_fn_sqlite3_step;

public:
	int sqlite3_bind_int(sqlite3_stmt* pStmt, int index, int data){
		return m_fn_sqlite3_bind_int(pStmt, index, data);
	}

	int sqlite3_bind_text16(sqlite3_stmt* pStmt, int index, const void* data, int length, void(*destructor)(void*)){
		return m_fn_sqlite3_bind_text16(pStmt, index, data, length, destructor);
	}

	int sqlite3_close(sqlite3* db){
		return m_fn_sqlite3_close(db);
	}

	int sqlite3_column_int(sqlite3_stmt* pStmt, int iCol){
		return m_fn_sqlite3_column_int(pStmt, iCol);
	}

	const wchar_t* sqlite3_column_name16(sqlite3_stmt* pStmt, int N){
		return (const wchar_t*)m_fn_sqlite3_column_name16(pStmt, N);
	}

	const wchar_t* sqlite3_column_text16(sqlite3_stmt* pStmt, int iCol){
		return (const wchar_t*)m_fn_sqlite3_column_text16(pStmt, iCol);
	}

	const wchar_t* sqlite3_errmsg16(sqlite3* db){
		return (const wchar_t*)m_fn_sqlite3_errmsg16(db);
	}

	int sqlite3_exec(sqlite3* db, const char* sql, int (*callback)(void*, int, char**, char**), void* arg, char** errmsg){
		return m_fn_sqlite3_exec(db, sql, callback, arg, errmsg);
	}

	int sqlite3_finalize(sqlite3_stmt* pStmt){
		return m_fn_sqlite3_finalize(pStmt);
	}

	int sqlite3_open16(const void* filename, sqlite3** ppDb){
		return m_fn_sqlite3_open16(filename, ppDb);
	}

	int sqlite3_prepare16(sqlite3* db, const void* zSql, int nByte, sqlite3_stmt** ppStmt, const void** psTail){
		return m_fn_sqlite3_prepare16(db, zSql, nByte, ppStmt, psTail);
	}

	int sqlite3_reset(sqlite3_stmt* pStmt){
		return m_fn_sqlite3_reset(pStmt);
	}

	int sqlite3_step(sqlite3_stmt* pStmt){
		return m_fn_sqlite3_step(pStmt);
	}

};

#endif	//_PLUGIN_SQLITE_H_
