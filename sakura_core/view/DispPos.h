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
		m_nDx(nDx),
		m_nDy(nDy)
	{
		m_ptDrawOrigin.x = 0;
		m_ptDrawOrigin.y = 0;
		m_ptDrawLayout.x = LayoutInt(0);
		m_ptDrawLayout.y = LayoutInt(0);
		m_nLineRef = LayoutInt(0);
		// �L���b�V��
		m_pLayoutRef = EditDoc::GetInstance(0)->m_layoutMgr.GetTopLayout();
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �`��ʒu                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �Œ�l
	void InitDrawPos(const POINT& pt) {
		m_ptDrawOrigin = pt;
		m_ptDrawLayout.x = m_ptDrawLayout.y = LayoutInt(0);
	}

	// �擾
	Point GetDrawPos() const {
		return Point(
			m_ptDrawOrigin.x + (Int)m_ptDrawLayout.x * m_nDx,
			m_ptDrawOrigin.y + (Int)m_ptDrawLayout.y * m_nDy
		);
	}

	// �i��
	void ForwardDrawCol (int nColOffset) { m_ptDrawLayout.x += nColOffset; }
	void ForwardDrawLine(int nOffsetLine) { m_ptDrawLayout.y += nOffsetLine; }

	// ���Z�b�g
	void ResetDrawCol() { m_ptDrawLayout.x = LayoutInt(0); }

	// �擾
	LayoutInt GetDrawCol() const { return m_ptDrawLayout.x; }
	LayoutInt GetDrawLine() const { return m_ptDrawLayout.y; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �e�L�X�g�Q�ƈʒu                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �ύX
	void SetLayoutLineRef(LayoutInt nOffsetLine) {
		m_nLineRef = nOffsetLine;
		// �L���b�V���X�V
		m_pLayoutRef = EditDoc::GetInstance(0)->m_layoutMgr.SearchLineByLayoutY(m_nLineRef);
	}
	void ForwardLayoutLineRef(int nOffsetLine);

	// �擾
	LayoutInt		GetLayoutLineRef() const { return m_nLineRef; }
	const Layout*	GetLayoutRef() const { return m_pLayoutRef; }

private:
	// �Œ�v�f
	int				m_nDx;			// ���p�����̕����Ԋu�B�Œ�B
	int				m_nDy;			// ���p�����̍s�Ԋu�B�Œ�B
	POINT			m_ptDrawOrigin;	// �`��ʒu��B�P�ʂ̓s�N�Z���B�Œ�B

	// �`��ʒu
	LayoutPoint		m_ptDrawLayout; // �`��ʒu�B���΃��C�A�E�g�P�ʁB

	// �e�L�X�g�Q�ƈʒu
	LayoutInt		m_nLineRef;		// ��΃��C�A�E�g�P�ʁB

	// �L���b�V��############
	const Layout*	m_pLayoutRef;
};

