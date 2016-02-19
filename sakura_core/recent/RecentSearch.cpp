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

#include "StdAfx.h"
#include "RecentSearch.h"
#include "config/maxdata.h"
#include "env/DllSharedData.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentSearch::RecentSearch()
{
	Create(
		GetShareData()->m_searchKeywords.searchKeys.dataPtr(),
		&GetShareData()->m_searchKeywords.searchKeys._GetSizeRef(),
		NULL,
		MAX_SEARCHKEY,
		NULL
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �I�[�o�[���C�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	�A�C�e���̔�r�v�f���擾����B

	@note	�擾��̃|�C���^�̓��[�U�Ǘ��̍\���̂ɃL���X�g���ĎQ�Ƃ��Ă��������B
*/
const TCHAR* RecentSearch::GetItemText(int nIndex) const
{
	return to_tchar(*GetItem(nIndex));
}

bool RecentSearch::DataToReceiveType(LPCWSTR* dst, const SearchString* src) const
{
	*dst = *src;
	return true;
}

bool RecentSearch::TextToDataType(SearchString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, to_wchar(pszText));
	return true;
}

int RecentSearch::CompareItem(const SearchString* p1, LPCWSTR p2) const
{
	return wcscmp(*p1, p2);
}

void RecentSearch::CopyItem(SearchString* dst, LPCWSTR src) const
{
	wcscpy(*dst, src);
}

