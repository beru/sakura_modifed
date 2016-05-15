/*
	Copyright (C) 2008, kobake

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

#include "doc/EditDoc.h"
#include "doc/layout/LayoutMgr.h"
#include "doc/layout/Layout.h"

struct DispPos {
public:
	DispPos(int nDx, int nDy)
		:
		nDx(nDx),
		nDy(nDy)
	{
		ptDrawOrigin.x = 0;
		ptDrawOrigin.y = 0;
		ptDrawLayout.x = LayoutInt(0);
		ptDrawLayout.y = LayoutInt(0);
		nLineRef = LayoutInt(0);
		// �L���b�V��
		pLayoutRef = EditDoc::GetInstance(0)->layoutMgr.GetTopLayout();
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �`��ʒu                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �Œ�l
	void InitDrawPos(const POINT& pt) {
		ptDrawOrigin = pt;
		ptDrawLayout.x = ptDrawLayout.y = LayoutInt(0);
	}

	// �擾
	Point GetDrawPos() const {
		return Point(
			ptDrawOrigin.x + (Int)ptDrawLayout.x * nDx,
			ptDrawOrigin.y + (Int)ptDrawLayout.y * nDy
		);
	}

	// �i��
	void ForwardDrawCol (int nColOffset) { ptDrawLayout.x += nColOffset; }
	void ForwardDrawLine(int nOffsetLine) { ptDrawLayout.y += nOffsetLine; }

	// ���Z�b�g
	void ResetDrawCol() { ptDrawLayout.x = LayoutInt(0); }

	// �擾
	LayoutInt GetDrawCol() const { return ptDrawLayout.x; }
	LayoutInt GetDrawLine() const { return ptDrawLayout.y; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �e�L�X�g�Q�ƈʒu                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �ύX
	void SetLayoutLineRef(LayoutInt nOffsetLine) {
		nLineRef = nOffsetLine;
		// �L���b�V���X�V
		pLayoutRef = EditDoc::GetInstance(0)->layoutMgr.SearchLineByLayoutY(nLineRef);
	}
	void ForwardLayoutLineRef(int nOffsetLine);

	// �擾
	LayoutInt		GetLayoutLineRef() const { return nLineRef; }
	const Layout*	GetLayoutRef() const { return pLayoutRef; }

private:
	// �Œ�v�f
	int				nDx;			// ���p�����̕����Ԋu�B�Œ�B
	int				nDy;			// ���p�����̍s�Ԋu�B�Œ�B
	POINT			ptDrawOrigin;	// �`��ʒu��B�P�ʂ̓s�N�Z���B�Œ�B

	// �`��ʒu
	LayoutPoint		ptDrawLayout;	// �`��ʒu�B���΃��C�A�E�g�P�ʁB

	// �e�L�X�g�Q�ƈʒu
	LayoutInt		nLineRef;		// ��΃��C�A�E�g�P�ʁB

	// �L���b�V��############
	const Layout*	pLayoutRef;
};

