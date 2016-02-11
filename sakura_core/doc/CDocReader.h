/*
	Copyright (C) 2008, kobake

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

class DocLineMgr;

// as decorator
class DocReader {
public:
	DocReader(const DocLineMgr& pcDocLineMgr) : m_pDocLineMgr(&pcDocLineMgr) { }

	wchar_t* GetAllData(int* pnDataLen);	// 全行データを返す
	const wchar_t* GetLineStr(LogicInt , LogicInt*);
	const wchar_t* GetLineStrWithoutEOL(LogicInt , int*); // 2003.06.22 Moca
	const wchar_t* GetFirstLinrStr(int*);	// 順アクセスモード：先頭行を得る
	const wchar_t* GetNextLinrStr(int*);	// 順アクセスモード：次の行を得る

private:
	const DocLineMgr* m_pDocLineMgr;
};

