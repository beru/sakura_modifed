//
/*!	@file
	@brief ���ݍs�̃}�[�N���Ǘ�����

	@author genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka

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

#include <vector>


/*!
	�s�}�[�N���Ǘ�����N���X�B
	�������z�֐����܂ނ̂ŁA���ۂɂ̓T�u�N���X������Ďg���B

	@par �ʏ푀��i���ʁj
	Add()�Œǉ��D�ꏊ�Ɩ��O��o�^�ł���D���삻�̂��̂̓J�X�^�}�C�Y�\�D
	[�ԍ�]�ŊY���ԍ��̗v�f���擾�ł���D

	@par ��������
	�ő�l�𒴂����ꍇ��protected�Ȋ֐��ŏ�������D�i�J�X�^�}�C�Y�\�j
	Add()�̏����̓T�u�N���X�ɔC����D

	@par ���݈ʒu�̊Ǘ�
	���݈ʒu��Manager���ŊǗ�����D

	�폜����̓T�u�N���X�ɂ܂�����

*/
class MarkMgr {
public:

	//	���ڂ̃N���X
	class Mark {
	public:
		//	constructor
		Mark(const Point& pt) : ptLogic(pt) { }

		Point GetPosition() const { return ptLogic; }
		void SetPosition(const Point& pt) { ptLogic = pt; }

		bool IsValid(void) const { return true; }

		bool operator == (Mark &r) const { return ptLogic.y == r.ptLogic.y; }
		bool operator != (Mark &r) const { return ptLogic.y != r.ptLogic.y; }

	private:
		Point ptLogic;
	};

	// GENERATE_FACTORY(Mark, CMarkFactory);	//	Mark�pFactory class

	//	�^�錾
	typedef std::vector<Mark> MarkChain;
	typedef std::vector<Mark>::const_iterator	MarkIterator;

	//	Interface
	//	constructor
	MarkMgr() : nCurpos(0), nMaxitem(10) {}
	// MarkMgr(const CDocLineMgr *p) : doc(p) {}

	size_t Count(void) const { return markChain.size(); }	//	���ڐ���Ԃ�
	int GetMax(void) const { return nMaxitem; }	//	�ő區�ڐ���Ԃ�
	void SetMax(int max);	//	�ő區�ڐ���ݒ�

	virtual void Add(const Mark& m) = 0;	//	�v�f�̒ǉ�

	//	Apr. 1, 2001 genta
	virtual void Flush(void);	//	�v�f�̑S����

	// �v�f�̎擾
	const Mark& GetCurrent(void) const { return markChain[nCurpos]; }

	// �L�����̊m�F
	bool  CheckCurrent(void) const;
	bool  CheckPrev(void) const;
	bool  CheckNext(void) const;

	// ���݈ʒu�̈ړ�
	bool NextValid(void);
	bool PrevValid(void);

	const Mark& operator[](int index) const { return markChain[index]; }

	//	�A���擾�C���^�[�t�F�[�X
//	MarkIterator CurrentPos(void) const { return (MarkIterator)markChain.begin() + nCurpos; }
//	MarkIterator Begin(void) const { return (MarkIterator)markChain.begin(); }
//	MarkIterator End(void) const { return (MarkIterator)markChain.end(); }

protected:
	virtual void Expire(void) = 0;

	// CMarkFactory factory;	//	Factory Class (�}�N���Ő��������j
	MarkChain markChain;	//	�}�[�N�f�[�^�{��
	int nCurpos;	//	���݈ʒu�i�ԍ��j

	int nMaxitem;	//	�ۊǉ\�A�C�e���̍ő吔
private:
	//MarkMgr(const MarkMgr&);	//	Copy�֎~

};

// ----------------------------------------------------
/*!
	@brief �ړ������̊Ǘ��N���X

	MarkMgr ���p�����A���삪�K�肳��Ă��Ȃ���������������B
*/
class AutoMarkMgr : public MarkMgr {
public:
	virtual void Add(const Mark& m);	// �v�f�̒ǉ�
	virtual void Expire(void);			// �v�f���̒���
};


