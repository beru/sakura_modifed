#include "stdafx.h"
#include "ControlResizer.h"

void ControlResizer::Init(HWND hWnd)
{
	hWnd_ = hWnd;
	RECT rect;
	GetClientRect(hWnd, &rect);
	initialWidth_ = rect.right - rect.left;
	initialHeight_ = rect.bottom - rect.top;
}

void ControlResizer::Add(int id, int anchorStyle)
{
	ControlSetting s;
	s.id = id;
	s.anchorStyle = anchorStyle;
	// http://stackoverflow.com/questions/18034975/how-do-i-find-position-of-a-win32-control-window-relative-to-its-parent-window
	::GetWindowRect(::GetDlgItem(hWnd_, id), &s.initialRect);
	MapWindowPoints(HWND_DESKTOP, hWnd_, (LPPOINT) &s.initialRect, 2);
	settings_.push_back(s);
}

void ControlResizer::DefereWindowPos(int width, int height)
{
	HDWP hdwp = BeginDeferWindowPos(settings_.size());
	for (size_t i=0; i<settings_.size(); ++i) {
		const ControlSetting& s = settings_[i];

		int x = s.initialRect.left;
		int y = s.initialRect.top;
		int w = s.initialRect.right - s.initialRect.left;
		int h = s.initialRect.bottom - s.initialRect.top;

		int style = s.anchorStyle;

		int diffW = width - initialWidth_;
		if (style & AnchorStyle::Left) {
			if (style & AnchorStyle::Right) {
				// left & right
				w += diffW;
			}else {
				// left only
			}
		}else if (style & AnchorStyle::Right) {
			// right only
			x += diffW;
		}

		int diffH = height - initialHeight_;
		if (style & AnchorStyle::Top) {
			if (style & AnchorStyle::Bottom) {
				// top & bottom
				h += diffH;
			}else {
				// top only
			}
		}else if (style & AnchorStyle::Bottom) {
			// bottom only
			y += diffH;
		}
		HWND ctrlHWND = ::GetDlgItem(hWnd_, s.id);
		DeferWindowPos(hdwp, ctrlHWND, 0, x, y, w, h, SWP_NOZORDER);
	}
//	DeferWindowPos(hdwp, GetDlgItem(IDC_LIST), 0, 5, 7, width-10, height-150, SWP_NOZORDER);
	EndDeferWindowPos(hdwp);
}

