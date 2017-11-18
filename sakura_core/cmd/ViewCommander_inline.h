#pragma once

#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "window/EditWnd.h"
#include "OpeBlk.h"

// ŠO•”ˆË‘¶
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

