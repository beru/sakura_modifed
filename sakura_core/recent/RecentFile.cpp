#include "StdAfx.h"
#include "recent/RecentFile.h"
#include <string.h>
#include "env/DllSharedData.h"


/*
	�A�C�e���̔�r�v�f���擾����B

	@note	�擾��̃|�C���^�̓��[�U�Ǘ��̍\���̂ɃL���X�g���ĎQ�Ƃ��Ă��������B
*/
const TCHAR* RecentFile::GetItemText(size_t nIndex) const
{
	return GetItem(nIndex)->szPath;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentFile::RecentFile()
{
	Create(
		GetShareData()->history.fiMRUArr,
		&GetShareData()->history.nMRUArrNum,
		GetShareData()->history.bMRUArrFavorite,
		MAX_MRU,
		&(GetShareData()->common.general.nMRUArrNum_MAX)
	);
}

bool RecentFile::DataToReceiveType(const EditInfo** dst, const EditInfo* src) const
{
	*dst = src;
	return true;
}

bool RecentFile::TextToDataType(EditInfo* dst, LPCTSTR pszText) const
{
	if (_countof(dst->szPath) < auto_strlen(pszText) + 1) {
		return false;
	}
	_tcscpy_s(dst->szPath, pszText);
	return true;
}

int RecentFile::CompareItem(const EditInfo* p1, const EditInfo* p2) const
{
	return _tcsicmp(p1->szPath, p2->szPath);
}

void RecentFile::CopyItem(EditInfo* dst, const EditInfo* src) const
{
	memcpy_raw(dst, src, sizeof(*dst));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   �ŗL�C���^�[�t�F�[�X                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int RecentFile::FindItemByPath(const TCHAR* pszPath) const
{
	size_t n = GetItemCount();
	for (size_t i=0; i<n; ++i) {
		if (_tcsicmp(GetItem(i)->szPath, pszPath) == 0) {
			return (int)i;
		}
	}
	return -1;
}

