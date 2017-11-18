#pragma once

#include "_main/global.h"
#include "env/DllSharedData.h"

// �ŋߎg�������X�g

class Recent {
public:
	virtual ~Recent() {}

	// �C���X�^���X�Ǘ�
	virtual void Terminate() = 0;

	// �A�C�e��
	virtual const TCHAR*	GetItemText(size_t nIndex) const = 0;
	virtual size_t			GetArrayCount() const = 0;
	virtual size_t			GetItemCount() const = 0;
	virtual void			DeleteAllItem() = 0;
	virtual bool			DeleteItemsNoFavorite() = 0;
	virtual bool			DeleteItem(size_t nIndex) = 0;	// �A�C�e�����N���A
	virtual bool			AppendItemText(const TCHAR* pszText) = 0;
	virtual bool			EditItemText(size_t nIndex, const TCHAR* pszText) = 0;

	int FindItemByText(const TCHAR* pszText) const {
		size_t n = GetItemCount();
		for (size_t i=0; i<n; ++i) {
			if (_tcscmp(GetItemText(i), pszText) == 0) {
				return (int)i;
			}
		}
		return -1;
	}

	// ���C�ɓ���
	virtual bool	SetFavorite(size_t nIndex, bool bFavorite = true) = 0;		// ���C�ɓ���ɐݒ�
	virtual bool	IsFavorite(size_t nIndex) const = 0;						// ���C�ɓ��肩���ׂ�

	// ���̑�
	virtual size_t	GetViewCount() const = 0;
	virtual bool	UpdateView() = 0;

	// ���L�������A�N�Z�X
	DllSharedData*	GetShareData() {
		return &GetDllShareData();
	}
};

#include "RecentImp.h"

