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
		ptDrawLayout.x = 0;
		ptDrawLayout.y = 0;
		nLineRef = 0;
		// �L���b�V��
		pLayoutRef = EditDoc::GetInstance(0)->layoutMgr.GetTopLayout();
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �`��ʒu                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �Œ�l
	void InitDrawPos(const POINT& pt) {
		ptDrawOrigin = pt;
		ptDrawLayout.x = ptDrawLayout.y = 0;
	}

	// �擾
	Point GetDrawPos() const {
		return Point(
			ptDrawOrigin.x + ptDrawLayout.x * nDx,
			ptDrawOrigin.y + ptDrawLayout.y * nDy
		);
	}

	// �i��
	void ForwardDrawCol (int nColOffset) { ptDrawLayout.x += nColOffset; }
	void ForwardDrawLine(int nOffsetLine) { ptDrawLayout.y += nOffsetLine; }

	// ���Z�b�g
	void ResetDrawCol() { ptDrawLayout.x = 0; }

	// �擾
	int GetDrawCol() const { return ptDrawLayout.x; }
	int GetDrawLine() const { return ptDrawLayout.y; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     �e�L�X�g�Q�ƈʒu                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �ύX
	void SetLayoutLineRef(int nOffsetLine) {
		nLineRef = nOffsetLine;
		// �L���b�V���X�V
		pLayoutRef = EditDoc::GetInstance(0)->layoutMgr.SearchLineByLayoutY(nLineRef);
	}
	void ForwardLayoutLineRef(int nOffsetLine);

	// �擾
	int				GetLayoutLineRef() const { return nLineRef; }
	const Layout*	GetLayoutRef() const { return pLayoutRef; }

private:
	// �Œ�v�f
	int		nDx;			// ���p�����̕����Ԋu�B�Œ�B
	int		nDy;			// ���p�����̍s�Ԋu�B�Œ�B
	POINT	ptDrawOrigin;	// �`��ʒu��B�P�ʂ̓s�N�Z���B�Œ�B

	// �`��ʒu
	Point	ptDrawLayout;	// �`��ʒu�B���΃��C�A�E�g�P�ʁB

	// �e�L�X�g�Q�ƈʒu
	int				nLineRef;		// ��΃��C�A�E�g�P�ʁB

	// �L���b�V��############
	const Layout*	pLayoutRef;
};

