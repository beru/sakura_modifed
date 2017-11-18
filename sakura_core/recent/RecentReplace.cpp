#include "StdAfx.h"
#include "RecentReplace.h"
#include <string.h>
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentReplace::RecentReplace()
{
	Create(
		GetShareData()->searchKeywords.replaceKeys.dataPtr(),
		&GetShareData()->searchKeywords.replaceKeys._GetSizeRef(),
		nullptr,
		MAX_REPLACEKEY,
		nullptr
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �I�[�o�[���C�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	�A�C�e���̔�r�v�f���擾����B

	@note	�擾��̃|�C���^�̓��[�U�Ǘ��̍\���̂ɃL���X�g���ĎQ�Ƃ��Ă��������B
*/
const TCHAR* RecentReplace::GetItemText(size_t nIndex) const
{
	return to_tchar(*GetItem(nIndex));
}

bool RecentReplace::DataToReceiveType(LPCWSTR* dst, const ReplaceString* src) const
{
	*dst = *src;
	return true;
}

bool RecentReplace::TextToDataType(ReplaceString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, to_wchar(pszText));
	return true;
}


int RecentReplace::CompareItem(const ReplaceString* p1, LPCWSTR p2) const
{
	return wcscmp(*p1, p2);
}

void RecentReplace::CopyItem(ReplaceString* dst, LPCWSTR src) const
{
	wcscpy(*dst, src);
}

