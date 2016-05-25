/*!	@file
	@brief DicMgr�N���X��`

	@author Norio Nakatani
	@date	1998/11/05 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "util/container.h"
#include "_main/global.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class DicMgr {
public:
	/*
	||  Constructors
	*/
	DicMgr();
	~DicMgr();

	/*
	||  Attributes & Operations
	*/
//	BOOL Open(char*);
	static
	BOOL Search(const wchar_t*, const size_t, NativeW**, NativeW**, const TCHAR*, int*);	// 2006.04.10 fon (const int, CMemory**, int*)������ǉ�
	
	static
	int HokanSearch(const wchar_t* , bool, vector_ex<std::wstring>&, int, const TCHAR*);
//	BOOL Close(char*);


protected:
	/*
	||  �����w���p�֐�
	*/
};


