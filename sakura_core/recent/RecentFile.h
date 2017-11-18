#pragma once

#include "RecentImp.h"
#include "EditInfo.h" //EditInfo

// EditInfo�̗������Ǘ� (RECENT_FOR_FILE)
class RecentFile : public RecentImp<EditInfo> {
public:
	// ����
	RecentFile();

	// �I�[�o�[���C�h
	int				CompareItem(const EditInfo* p1, const EditInfo* p2) const;
	void			CopyItem(EditInfo* dst, const EditInfo* src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(const EditInfo** dst, const EditInfo* src) const;
	bool			TextToDataType(EditInfo* dst, LPCTSTR pszText) const;
	// �ŗL�C���^�[�t�F�[�X
	int FindItemByPath(const TCHAR* pszPath) const;
};

