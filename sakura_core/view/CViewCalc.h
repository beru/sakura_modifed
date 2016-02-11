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

/*
	X�l�̒P�ʕϊ��֐��Q�B
*/

class Layout;
class DocLine;
class EditView;

class ViewCalc {
protected:
	// �O���ˑ�
	LayoutInt GetTabSpace() const;

public:
	ViewCalc(const EditView* pOwner) : m_pOwner(pOwner) { }
	virtual ~ViewCalc() {}

	// �P�ʕϊ�: ���C�A�E�g�����W�b�N
	LogicInt  LineColumnToIndex (const Layout*  pcLayout,  LayoutInt nColumn) const;		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1		// @@@ 2002.09.28 YAZAKI
	LogicInt  LineColumnToIndex (const DocLine* pcDocLine, LayoutInt nColumn) const;		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1		// @@@ 2002.09.28 YAZAKI
	LogicInt  LineColumnToIndex2(const Layout*  pcLayout,  LayoutInt nColumn, LayoutInt* pnLineAllColLen) const;	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0		// @@@ 2002.09.28 YAZAKI

	// �P�ʕϊ�: ���W�b�N�����C�A�E�g
	LayoutInt LineIndexToColumn (const Layout*  pcLayout,  LogicInt nIndex) const;		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�	// @@@ 2002.09.28 YAZAKI
	LayoutInt LineIndexToColumn (const DocLine* pcLayout,  LogicInt nIndex) const;		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�	// @@@ 2002.09.28 YAZAKI

private:
	const EditView* m_pOwner;
};

