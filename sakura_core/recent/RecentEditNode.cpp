#include "StdAfx.h"
#include "RecentEditNode.h"
#include <string.h>
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentEditNode::RecentEditNode()
{
	Create(
		GetShareData()->nodes.pEditArr,
		&GetShareData()->nodes.nEditArrNum,
		nullptr,
		MAX_EDITWINDOWS,
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
const TCHAR* RecentEditNode::GetItemText(size_t nIndex) const
{
	return _T("WIN"); // ���e�L�X�g���͖��� (GetWindowText���Ă����Ă��ǂ����ǁA���̊֐��͎��s����Ȃ��̂ŁA�Ӗ��͖���)
}

bool RecentEditNode::DataToReceiveType(const EditNode** dst, const EditNode* src) const
{
	*dst = src;
	return true;
}

bool RecentEditNode::TextToDataType(EditNode* dst, LPCTSTR pszText) const
{
	return false;
}

int RecentEditNode::CompareItem(const EditNode* p1, const EditNode* p2) const
{
	return p1->hWnd - p2->hWnd;
}

void RecentEditNode::CopyItem(EditNode* dst, const EditNode* src) const
{
	*dst = *src;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   �ŗL�C���^�[�t�F�[�X                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int RecentEditNode::FindItemByHwnd(HWND hwnd) const
{
	size_t n = GetItemCount();
	for (size_t i=0; i<n; ++i) {
		if (GetItem(i)->hWnd == hwnd) {
			return (int)i;
		}
	}
	return -1;
}

void RecentEditNode::DeleteItemByHwnd(HWND hwnd)
{
	int n = FindItemByHwnd(hwnd);
	if (n != -1) {
		DeleteItem(n);
	}else {
		DEBUG_TRACE(_T("DeleteItemByHwnd���s\n"));
	}
}

