// EditView�N���X��grep�֘A�R�}���h�����n�֐��Q

#include "StdAfx.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "_main/ControlTray.h"
#include "charset/charcode.h"
#include "EditApp.h"
#include "GrepAgent.h"
#include "sakura_rc.h"

/*!
	�R�}���h�R�[�h�̕ϊ�(grep mode��)
*/
void EditView::TranslateCommand_grep(
	EFunctionCode&	nCommand,
	bool&			bRedraw,
	LPARAM&			lparam1,
	LPARAM&			lparam2,
	LPARAM&			lparam3,
	LPARAM&			lparam4
	)
{
	if (!EditApp::getInstance().pGrepAgent->bGrepMode)
		return;

	if (nCommand == F_WCHAR) {
		if (WCODE::IsLineDelimiter((wchar_t)lparam1, GetDllShareData().common.edit.bEnableExtEol)
			&& GetDllShareData().common.search.bGTJW_Return
		) {
			nCommand = F_TAGJUMP;
			lparam1 = GetKeyState_Control() ? 1 : 0;
		}
	}
}

