#pragma once

#include "RecentImp.h"
struct EditNode;

// EditNode(�E�B���h�E���X�g)�̗������Ǘ� (RECENT_FOR_EDITNODE)
class RecentEditNode : public RecentImp<EditNode> {
public:
	// ����
	RecentEditNode();

	// �I�[�o�[���C�h
	int				CompareItem(const EditNode* p1, const EditNode* p2) const;
	void			CopyItem(EditNode* dst, const EditNode* src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(const EditNode** dst, const EditNode* src) const;
	bool			TextToDataType(EditNode* dst, LPCTSTR pszText) const;
	// �ŗL�C���^�[�t�F�[�X
	int FindItemByHwnd(HWND hwnd) const;
	void DeleteItemByHwnd(HWND hwnd);
};

