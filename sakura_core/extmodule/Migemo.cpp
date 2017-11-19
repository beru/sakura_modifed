/*!	@file
	@brief C/Migemo インターフェース
*/
#include "StdAfx.h"
#include <io.h>
#include "Migemo.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "charset/Utf8.h"
#include "util/module.h"
#include "util/fileUtil.h"

/*! @brief PCRE メタキャラクタのエスケープ処理を行う。
 （Migemo::migemo_setproc_int2char の引数として使用）
 @param[in] in 入力文字コード(unsigned int)
 @param[out] out 出力バイト列(unsigned char*)
 @return 出力された文字列のバイト数。
  0を返せばデフォルトのプロシージャが実行される。
*/
static int __cdecl pcre_int2char(unsigned int in, unsigned char* out);

int __cdecl pcre_char2int_sjis(const unsigned char*, unsigned int*);
int __cdecl pcre_char2int_utf8(const unsigned char*, unsigned int*);
int __cdecl pcre_int2char_utf8(unsigned int, unsigned char*);

//-----------------------------------------
// DLL 初期化関数
//-----------------------------------------
bool Migemo::InitDllImp()
{
	// staticにしてはいけないらしい
	const ImportTable table[] = {
		{ &p_migemo_open              ,"migemo_open"              },
		{ &p_migemo_close             ,"migemo_close"             },
		{ &p_migemo_query             ,"migemo_query"             },
		{ &p_migemo_release           ,"migemo_release"           },
		{ &p_migemo_set_operator      ,"migemo_set_operator"      },
		{ &p_migemo_get_operator      ,"migemo_get_operator"      },
		{ &p_migemo_setproc_char2int  ,"migemo_setproc_char2int"  },
		{ &p_migemo_setproc_int2char  ,"migemo_setproc_int2char"  },
		{ &p_migemo_load              ,"migemo_load"              },
		{ &p_migemo_is_enable         ,"migemo_is_enable"         },
		{ nullptr, 0                                                 }
	};
	
	if (!RegisterEntries(table)) {
		return false;
	}

	p_migemo_open_s             = (Proc_migemo_open_s)				p_migemo_open;
	p_migemo_close_s            = (Proc_migemo_close_s)				p_migemo_close;
	p_migemo_query_s            = (Proc_migemo_query_s)				p_migemo_query;
	p_migemo_release_s          = (Proc_migemo_release_s)			p_migemo_release;
	p_migemo_set_operator_s     = (Proc_migemo_set_operator_s)		p_migemo_set_operator;
	p_migemo_get_operator_s     = (Proc_migemo_get_operator_s)		p_migemo_get_operator;
	p_migemo_setproc_char2int_s = (Proc_migemo_setproc_char2int_s)	p_migemo_setproc_char2int;
	p_migemo_setproc_int2char_s = (Proc_migemo_setproc_int2char_s)	p_migemo_setproc_int2char;
	p_migemo_load_s             = (Proc_migemo_load_s)				p_migemo_load;
	p_migemo_is_enable_s        = (Proc_migemo_is_enable_s)			p_migemo_is_enable;

	// ver 1.3 以降は stdcall
	DWORD dwVersionMS, dwVersionLS;
	GetAppVersionInfo(GetInstance(), VS_VERSION_INFO, &dwVersionMS, &dwVersionLS);
	
	DWORD dwver103 = (1 << 16) | 3;
	bStdcall = (dwVersionMS >= dwver103);
	bUtf8 = false;

	if (!p_migemo_open(NULL)) {
		return false;
	}
	
	return true;
}

int Migemo::DeInitDll(void)
{
	migemo_close();
	return 0;
}

LPCTSTR Migemo::GetDllNameImp(int nIndex)
{
	if (nIndex == 0) {
		TCHAR* szDll;
		static TCHAR szDllName[_MAX_PATH];
		szDll = GetDllShareData().common.helper.szMigemoDll;

		if (szDll[0] == _T('\0')) {
			GetInidir(szDllName, _T("migemo.dll"));
			return fexist(szDllName) ? szDllName : _T("migemo.dll");
		}else {
			if (_IS_REL_PATH(szDll)) {
				GetInidirOrExedir(szDllName , szDll);	// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
				szDll = szDllName;
			}
			return szDll;
		}
		//return "migemo.dll";
	}else {
		return NULL;
	}
}

long Migemo::migemo_open(char* dict)
{	

	if (!IsAvailable())
		return 0;
	if (bStdcall) {
		migemo = (*p_migemo_open_s)(NULL);
	}else {
		migemo = (*p_migemo_open)(NULL);
	}
	
	if (!migemo)
		return 0;
	
	//if (!migemo_load(MIGEMO_DICTID_MIGEMO, path2))
	//	return 0;
	
	return 1;
}

void Migemo::migemo_close()
{
	if (!IsAvailable() || !migemo) {
		return;
	}
	
	if (bStdcall) {
		(*p_migemo_close_s)(migemo);
	}else {
		(*p_migemo_close)(migemo);
	}
}

unsigned char* Migemo::migemo_query(unsigned char* query)
{
	if (!IsAvailable() || !migemo) {
		return NULL;
	}
	
	if (bStdcall) {
		return (*p_migemo_query_s)(migemo, query);
	}else {
		return (*p_migemo_query)(migemo, query);
	}
}

std::wstring Migemo::migemo_query_w(const wchar_t* query)
{
	if (bUtf8) {
		NativeW cnvStr;
		NativeA utf8Str;
		cnvStr.SetString(query);
		Utf8::UnicodeToUTF8(cnvStr, utf8Str._GetMemory());
		unsigned char* ret;
		ret = migemo_query((unsigned char*)utf8Str.GetStringPtr());
		utf8Str.SetString((const char*)ret);
		Utf8::UTF8ToUnicode(*(utf8Str._GetMemory()), &cnvStr);
		migemo_release(ret);
		return cnvStr.GetStringPtr();
	}
	unsigned char* ret = migemo_query((unsigned char*)to_achar(query));
	std::wstring retVal = to_wchar((const char*)ret);
	migemo_release(ret);
	return retVal;
}

void Migemo::migemo_release(unsigned char* str)
{
	if (!IsAvailable() || !migemo) {
		return;
	}
	
	if (bStdcall) {
		(*p_migemo_release_s)(migemo, str);
	}else {
		(*p_migemo_release)(migemo, str);
	}

}
int Migemo::migemo_set_operator(int index, unsigned char* op)
{
	if (!IsAvailable() || !migemo) {
		return 0;
	}
	
	if (bStdcall) {
		return (*p_migemo_set_operator_s)(migemo, index, op);
	}else {
		return (*p_migemo_set_operator)(migemo, index, op);
	}
}
const unsigned char* Migemo::migemo_get_operator(int index)
{
	if (!IsAvailable() || !migemo) {
		return NULL;
	}
	
	if (bStdcall) {
		return (*p_migemo_get_operator_s)(migemo, index);
	}else {
		return (*p_migemo_get_operator)(migemo, index);
	}
}

void Migemo::migemo_setproc_char2int(MIGEMO_PROC_CHAR2INT proc)
{
	if (!IsAvailable() || !migemo) {
		return ;
	}
	
	if (bStdcall) {
		(*p_migemo_setproc_char2int_s)(migemo, proc);
	}else {
		(*p_migemo_setproc_char2int)(migemo, proc);
	}
}

void Migemo::migemo_setproc_int2char(MIGEMO_PROC_INT2CHAR proc)
{
	if (!IsAvailable() || !migemo) {
		return;
	}
	
	if (bStdcall) {
		(*p_migemo_setproc_int2char_s)(migemo, proc);
	}else {
		(*p_migemo_setproc_int2char)(migemo, proc);
	}
}

int Migemo::migemo_load_a(int dict_id, const char* dict_file)
{
	if (!IsAvailable() || !migemo) {
		return 0;
	}
	
	if (bStdcall) {
		return (*p_migemo_load_s)(migemo, dict_id, dict_file);
	}else {
		return (*p_migemo_load)(migemo, dict_id, dict_file);
	}
}

int Migemo::migemo_load_w(int dict_id, const wchar_t* dict_file)
{
	char szBuf[_MAX_PATH];
	wcstombs2(szBuf, dict_file, _countof(szBuf));
	return migemo_load_a(dict_id, szBuf);
}

int Migemo::migemo_is_enable()
{
	if (!IsAvailable() || !migemo) {
		return 0;
	}
	
	if (bStdcall) {
		return (*p_migemo_is_enable_s)(migemo);
	}else {
		return (*p_migemo_is_enable)(migemo);
	}
}

int Migemo::migemo_load_all()
{
	if (!migemo_is_enable()) {

		TCHAR* szDict = GetDllShareData().common.helper.szMigemoDict;
		TCHAR path[MAX_PATH];
		//char path2[MAX_PATH];
		TCHAR* ppath;
		
		if (szDict[0] == _T('\0')) {
			GetInidirOrExedir(path, _T("dict"));
		}else {
			if (_IS_REL_PATH(szDict)) {
				GetInidirOrExedir(path, szDict);
			}else {
				_tcscpy(path, szDict);
			}
		}
		ppath = &path[_tcslen(path)];
		*(ppath++) = _T('\\');
		// ver1.3 utf8対応
		_tcscpy(ppath, _T("utf-8\\migemo-dict"));
		if (fexist(path)) {
			_tcscpy(ppath, _T("utf-8\\"));
			ppath = &path[_tcslen(path)];
			bUtf8 = true;
		}else {
			_tcscpy(ppath, _T("cp932\\migemo-dict"));
			if (fexist(path)) {
				_tcscpy(ppath, _T("cp932\\"));
				ppath = &path[_tcslen(path)];
			}
			bUtf8 = false;
		}
		_tcscpy(ppath, _T("migemo-dict"));

		migemo_load_t(MIGEMO_DICTID_MIGEMO, path);
		_tcscpy(ppath, _T("han2zen.dat"));
		migemo_load_t(MIGEMO_DICTID_HAN2ZEN, path);
		_tcscpy(ppath, _T("hira2kata.dat"));
		migemo_load_t(MIGEMO_DICTID_HIRA2KATA, path);
		_tcscpy(ppath, _T("roma2hira.dat"));
		migemo_load_t(MIGEMO_DICTID_ROMA2HIRA, path);
		_tcscpy(ppath, _T("zen2han.dat"));
		migemo_load_t(MIGEMO_DICTID_ZEN2HAN, path);

		// 辞書登録後でないとmigemo内臓のものに変更されてしまう
		if (bUtf8) {
			migemo_setproc_char2int(pcre_char2int_utf8);
			migemo_setproc_int2char(pcre_int2char_utf8);
		}else {
			migemo_setproc_char2int(pcre_char2int_sjis);
			migemo_setproc_int2char(pcre_int2char);	// 2009.04.30 miau
		}
	}
	return 1;
}


Migemo::~Migemo()
{
}


int __cdecl pcre_char2int_sjis(const unsigned char* in, unsigned int* out)
{
	if (_IS_SJIS_1(in[0]) && _IS_SJIS_2(in[1])) {
		if (out) {
			*out = (in[0] << 8) | in[1];
		}
		return 2;
	}
	if (out) {
		*out = *in;
	}
    return 1;
}


// C/Migemo ソース中の rxgen.c:default_int2char を元に作成
static int __cdecl pcre_int2char(unsigned int in, unsigned char* out)
{
    // outは最低でも16バイトはある、という仮定を置く
    if (in >= 0x100) {
		if (out) {
		    out[0] = (unsigned char)((in >> 8) & 0xFF);
		    out[1] = (unsigned char)(in & 0xFF);
		}
		return 2;
    }else {
		int len = 0;
		switch (in) {
	    case '\\':
	    case '.': case '*': case '^': case '$': case '/':
	    case '[': case ']': 
	    case '|': case '(': case ')':
	    case '+': case '?': case '{': case '}':
	    case ':': case '-': case '&':
			if (out) {
			    out[len] = '\\';
			}
			++len;
	    default:
			if (out) {
			    out[len] = (unsigned char)(in & 0xFF);
			}
			++len;
			break;
		}
		return len;
    }
}


int __cdecl pcre_char2int_utf8(const unsigned char* in, unsigned int* out)
{
	if (0x80 & in[0]) {
		if ((0xe0 & in[0]) == 0xc0 && (0xc0 & in[1]) == 0x80) {
			if (out) {
				*out = ((0x1f & in[0]) << 6)
					| (0x3f & in[1]);
			}
			return 2;
		}else if ((0xf0 & in[0]) == 0xe0 && (0xc0 & in[1]) == 0x80
			&& (0xc0 & in[2]) == 0x80
		) {
			if (out) {
				*out = ((0x1f & in[0]) << 12)
					| ((0x3f & in[1]) << 6)
					| (0x3f & in[2]);
			}
			return 3;
		}else if ((0xf8 & in[0]) == 0xf0  && (0xc0 & in[1]) == 0x80
			&& (0xc0 & in[2]) == 0x80 && (0xc0 & in[3]) == 0x80
		) {
			if (out) {
				*out = ((0x1f & in[0]) << 18)
					| ((0x3f & in[1]) << 12)
					| ((0x3f & in[2]) << 6)
					| (0x3f & in[3]);
			}
			return 4;
		}
	}
	if (out) {
		*out = *in;
	}
    return 1;
}

int __cdecl pcre_int2char_utf8(unsigned int in, unsigned char* out)
{
	int len = 0;
	if (in < 0x80) {
		switch (in) {
		case '\\':
		case '.': case '*': case '^': case '$': case '/':
		case '[': case ']': 
		case '|': case '(': case ')':
		case '+': case '?': case '{': case '}':
		case ':': case '-': case '&':
			if (out) {
				out[len] = '\\';
			}
			++len;
		default:
			if (out) {
				out[len] = (unsigned char)(in & 0xFF);
			}
			++len;
			break;
		}
	}else if (in < 0x800) {
		if (out) {
			out[0] = static_cast<unsigned char>((in & 0x07c0) >> 6) | 0xc0;
			out[1] = static_cast<unsigned char>(in & 0x003f) | 0x80;
		}
		len = 2;
	}else if (in < 0x10000) {
		if (out) {
			out[0] = static_cast<unsigned char>((in & 0xf000) >> 12) | 0xe0;
			out[1] = static_cast<unsigned char>((in & 0x0fc0) >> 6)  | 0x80;
			out[2] = static_cast<unsigned char>(in & 0x003f) | 0x80;
		}
		len = 3;
	}else {
		if (out) {
			out[0] = static_cast<unsigned char>((in & 0x001c0000) >> 18) | 0xf0;
			out[1] = static_cast<unsigned char>((in & 0x0003f000) >> 12) | 0x80;
			out[2] = static_cast<unsigned char>((in & 0x00000fc0) >> 6)  | 0x80;
			out[3] = static_cast<unsigned char>(in & 0x0000003f) | 0x80;
		}
		len = 4;
	}
	return len;
}

