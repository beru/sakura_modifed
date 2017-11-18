#include "stdafx.h"
#include "RecentExceptMRU.h"
#include <string.h>
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentExceptMRU::RecentExceptMRU()
{
	auto& exceptMRU = GetShareData()->history.aExceptMRU;
	Create(
		exceptMRU.dataPtr(),
		&exceptMRU._GetSizeRef(),
		nullptr,
		MAX_MRU,
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
const TCHAR* RecentExceptMRU::GetItemText(size_t nIndex) const
{
	return *GetItem(nIndex);
}

bool RecentExceptMRU::DataToReceiveType(LPCTSTR* dst, const MetaPath* src) const
{
	*dst = *src;
	return true;
}

bool RecentExceptMRU::TextToDataType(MetaPath* dst, LPCTSTR pszText) const
{
	CopyItem(dst, pszText);
	return true;
}

int RecentExceptMRU::CompareItem(const MetaPath* p1, LPCTSTR p2) const
{
	return _tcsicmp(*p1, p2);
}

void RecentExceptMRU::CopyItem(MetaPath* dst, LPCTSTR src) const
{
	_tcscpy(*dst, src);
}

