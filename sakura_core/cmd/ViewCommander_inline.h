/*
	Copyright (C) 2013, novice

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "window/EditWnd.h"
#include "OpeBlk.h"

// �O���ˑ�
inline EditDoc& ViewCommander::GetDocument()
{
	return *view.pEditDoc;
}

inline EditWnd& ViewCommander::GetEditWindow()
{
	return view.editWnd;
}

inline HWND ViewCommander::GetMainWindow()
{
	return ::GetParent(view.hwndParent);
}

inline OpeBlk* ViewCommander::GetOpeBlk()
{
	return GetDocument().docEditor.pOpeBlk;
}

inline void ViewCommander::SetOpeBlk(OpeBlk* p)
{
	auto& editor = GetDocument().docEditor;
	editor.pOpeBlk = p;
	editor.nOpeBlkRedawCount = 0;
}

inline Range& ViewCommander::GetSelect()
{
	return view.GetSelectionInfo().select;
}

inline Caret& ViewCommander::GetCaret()
{
	return view.GetCaret();
}

