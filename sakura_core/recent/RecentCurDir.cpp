#include "StdAfx.h"
#include "RecentCurDir.h"
#include "config/maxdata.h"
#include "env/DllSharedData.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentCurDir::RecentCurDir()
{
	Create(
		GetShareData()->history.aCurDirs.dataPtr(),
		&GetShareData()->history.aCurDirs._GetSizeRef(),
		nullptr,
		MAX_CMDARR,
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
const TCHAR* RecentCurDir::GetItemText(size_t nIndex) const
{
	return *GetItem(nIndex);
}

bool RecentCurDir::DataToReceiveType(LPCTSTR* dst, const CurDirString* src) const
{
	*dst = *src;
	return true;
}

bool RecentCurDir::TextToDataType(CurDirString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, pszText);
	return true;
}

int RecentCurDir::CompareItem(const CurDirString* p1, LPCTSTR p2) const
{
	return _tcscmp(*p1, p2);
}

void RecentCurDir::CopyItem(CurDirString* dst, LPCTSTR src) const
{
	_tcscpy(*dst, src);
}

