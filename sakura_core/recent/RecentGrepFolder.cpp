#include "StdAfx.h"
#include <string.h>
#include "RecentGrepFolder.h"
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentGrepFolder::RecentGrepFolder()
{
	Create(
		GetShareData()->searchKeywords.grepFolders.dataPtr(),
		&GetShareData()->searchKeywords.grepFolders._GetSizeRef(),
		nullptr,
		MAX_GREPFOLDER,
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
const TCHAR* RecentGrepFolder::GetItemText(size_t nIndex) const
{
	return *GetItem(nIndex);
}

bool RecentGrepFolder::DataToReceiveType(LPCTSTR* dst, const GrepFolderString* src) const
{
	*dst = *src;
	return true;
}

bool RecentGrepFolder::TextToDataType(GrepFolderString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, pszText);
	return true;
}

int RecentGrepFolder::CompareItem(const GrepFolderString* p1, LPCTSTR p2) const
{
	return _tcsicmp(*p1, p2);
}

void RecentGrepFolder::CopyItem(GrepFolderString* dst, LPCTSTR src) const
{
	_tcscpy(*dst, src);
}

