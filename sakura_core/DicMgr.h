#pragma once

#include <Windows.h>
#include "util/container.h"
#include "_main/global.h"

class DicMgr {
public:
	DicMgr();
	~DicMgr();

	static
	BOOL Search(const wchar_t*, const size_t, NativeW**, NativeW**, const TCHAR*, int*);
	
	static
	int HokanSearch(const wchar_t* , bool, vector_ex<std::wstring>&, int, const TCHAR*);

protected:
	
};


