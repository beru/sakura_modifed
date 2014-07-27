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

#ifndef _PLUGIN_HUNSPELL_LOADER_H_
#define _PLUGIN_HUNSPELL_LOADER_H_

#include <windows.h>
#include <string>
#include "CDllLoader.h"

typedef struct Hunhandle Hunhandle;

#define HUNSPELL_DLL_NAME	L"libhunspell.dll"

///////////////////////////////////////////////////////////////////////////////
class CHunspellLoader : public CDllLoader
{
public:
	CHunspellLoader();
	virtual ~CHunspellLoader();

protected:
	virtual BOOL RegisterEntries();

private:
	typedef Hunhandle* (__cdecl *API_Hunspell_create          )(LPCSTR lpszAffpath, LPCSTR lpszDpath);
	typedef Hunhandle* (__cdecl *API_Hunspell_create_key      )(LPCSTR lpszAffpath, LPCSTR lpszDpath, LPCSTR lpszKey);
	typedef void       (__cdecl *API_Hunspell_destroy         )(Hunhandle* lpHunspell);
	typedef int        (__cdecl *API_Hunspell_spell           )(Hunhandle* lpHunspell, LPCSTR lpszWord);
	typedef LPSTR      (__cdecl *API_Hunspell_get_dic_encoding)(Hunhandle* lpHunspell);
	typedef int        (__cdecl *API_Hunspell_suggest         )(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord);
	typedef int        (__cdecl *API_Hunspell_analyze         )(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord);
	typedef int        (__cdecl *API_Hunspell_stem            )(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord);
	typedef int        (__cdecl *API_Hunspell_stem2           )(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPSTR* desc, int n);
	typedef int        (__cdecl *API_Hunspell_generate        )(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord, LPCSTR lpszWord2);
	typedef int        (__cdecl *API_Hunspell_generate2       )(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord, LPSTR* desc, int n);
	typedef int        (__cdecl *API_Hunspell_add             )(Hunhandle* lpHunspell, LPCSTR lpszWord);
	typedef int        (__cdecl *API_Hunspell_add_with_affix  )(Hunhandle* lpHunspell, LPCSTR lpszWord, LPCSTR lpszExample);
	typedef int        (__cdecl *API_Hunspell_remove          )(Hunhandle* lpHunspell, LPCSTR lpszWord);
	typedef void       (__cdecl *API_Hunspell_free_list       )(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, int n);

	API_Hunspell_create				m_fn_Hunspell_create;
	API_Hunspell_create_key			m_fn_Hunspell_create_key;
	API_Hunspell_destroy			m_fn_Hunspell_destroy;
	API_Hunspell_spell				m_fn_Hunspell_spell;
	API_Hunspell_get_dic_encoding	m_fn_Hunspell_get_dic_encoding;
	API_Hunspell_suggest			m_fn_Hunspell_suggest;
	API_Hunspell_analyze			m_fn_Hunspell_analyze;
	API_Hunspell_stem				m_fn_Hunspell_stem;
	API_Hunspell_stem2				m_fn_Hunspell_stem2;
	API_Hunspell_generate			m_fn_Hunspell_generate;
	API_Hunspell_generate2			m_fn_Hunspell_generate2;
	API_Hunspell_add				m_fn_Hunspell_add;
	API_Hunspell_add_with_affix		m_fn_Hunspell_add_with_affix;
	API_Hunspell_remove				m_fn_Hunspell_remove;
	API_Hunspell_free_list			m_fn_Hunspell_free_list;

public:
	Hunhandle* Hunspell_create(LPCSTR lpszAffpath, LPCSTR lpszDpath){
		return m_fn_Hunspell_create(lpszAffpath, lpszDpath);
	}
	Hunhandle* Hunspell_create_key(LPCSTR lpszAffpath, LPCSTR lpszDpath, LPCSTR lpszKey){
		return m_fn_Hunspell_create_key(lpszAffpath, lpszDpath, lpszKey);
	}
	void Hunspell_destroy(Hunhandle* lpHunspell){
		m_fn_Hunspell_destroy(lpHunspell);
	}
	int Hunspell_spell(Hunhandle* lpHunspell, LPCSTR lpszWord){
		return m_fn_Hunspell_spell(lpHunspell, lpszWord);
	}
	LPSTR Hunspell_get_dic_encoding(Hunhandle* lpHunspell){
		return m_fn_Hunspell_get_dic_encoding(lpHunspell);
	}
	int Hunspell_suggest(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord){
		return m_fn_Hunspell_suggest(lpHunspell, lplplpszSuggestList, lpszWord);
	}
	int Hunspell_analyze(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord){
		return m_fn_Hunspell_analyze(lpHunspell, lplplpszSuggestList, lpszWord);
	}
	int Hunspell_stem(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord){
		return m_fn_Hunspell_stem(lpHunspell, lplplpszSuggestList, lpszWord);
	}
	int Hunspell_stem2(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPSTR* desc, int n){
		return m_fn_Hunspell_stem2(lpHunspell, lplplpszSuggestList, desc, n);
	}
	int Hunspell_generate(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord, LPCSTR lpszWord2){
		return m_fn_Hunspell_generate(lpHunspell, lplplpszSuggestList, lpszWord, lpszWord2);
	}
	int Hunspell_generate2(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, LPCSTR lpszWord, LPSTR* desc, int n){
		return m_fn_Hunspell_generate2(lpHunspell, lplplpszSuggestList, lpszWord, desc, n);
	}
	int Hunspell_add(Hunhandle* lpHunspell, LPCSTR lpszWord){
		return m_fn_Hunspell_add(lpHunspell, lpszWord);
	}
	int Hunspell_add_with_affix(Hunhandle* lpHunspell, LPCSTR lpszWord, LPCSTR lpszExample){
		return m_fn_Hunspell_add_with_affix(lpHunspell, lpszWord, lpszExample);
	}
	int Hunspell_remove(Hunhandle* lpHunspell, LPCSTR lpszWord){
		return m_fn_Hunspell_remove(lpHunspell, lpszWord);
	}
	void Hunspell_free_list(Hunhandle* lpHunspell, LPSTR** lplplpszSuggestList, int n){
		m_fn_Hunspell_free_list(lpHunspell, lplplpszSuggestList, n);
	}

};

#endif	//_PLUGIN_HUNSPELL_LOADER_H_
