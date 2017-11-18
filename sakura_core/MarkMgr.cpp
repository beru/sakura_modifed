#include "StdAfx.h"
#include "_main/global.h"
#include "MarkMgr.h"

//-----------------------------------
// MarkMgr
//-----------------------------------
/*!
	@brief �ۊǂ���ő匏�����w�肷��B

	���݂�菬�����l��ݒ肵���Ƃ��͗]���ȗv�f�͍폜�����B

	@param max �ݒ肷��ő匏��
*/
void MarkMgr::SetMax(int max)
{
	nMaxitem = max;
	Expire();	//	�w�肵�����ɗv�f�����炷
}

/*!
	@brief ���݈ʒu�̗v�f���L�����ǂ����̔���

	@retval true	�L��
	@retval false	����
*/
bool MarkMgr::CheckCurrent(void) const
{
	if (nCurpos < (int)Count()) {
		return markChain[nCurpos].IsValid();
	}

	return false;
}

/*!
	@brief ���݈ʒu�̑O�ɗL���ȗv�f�����邩�ǂ����𒲂ׂ�

	@retval true	�L��
	@retval false	����
*/
bool MarkMgr::CheckPrev(void) const
{
	for (int i=nCurpos-1; i>=0; --i) {
		if (markChain[i].IsValid()) {
			return true;
		}
	}
	return false;
}

/*!
	@brief ���݈ʒu�̌�ɗL���ȗv�f�����邩�ǂ����𒲂ׂ�

	@retval true	�L��
	@retval false	����
*/
bool MarkMgr::CheckNext(void) const
{
	for (int i=nCurpos+1; i<(int)Count(); ++i) {
		if (markChain[i].IsValid()) {
			return true;
		}
	}
	return false;
}

/*!
	@brief ���݈ʒu��O�̗L���Ȉʒu�܂Ői�߂�

	@retval true	����I���B���݈ʒu��1�O�̗L���ȗv�f�Ɉړ������B
	@retval false	�L���ȗv�f��������Ȃ������B���݈ʒu�͈ړ����Ă��Ȃ��B
*/
bool MarkMgr::PrevValid(void)
{
	for (int i=nCurpos-1; i>=0; --i) {
		if (markChain[i].IsValid()) {
			nCurpos = i;
			return true;
		}
	}
	return false;
}

/*!
	@brief ���݈ʒu����̗L���Ȉʒu�܂Ői�߂�

	@retval true	����I���B���݈ʒu��1��̗L���ȗv�f�Ɉړ������B
	@retval false	�L���ȗv�f��������Ȃ������B���݈ʒu�͈ړ����Ă��Ȃ��B
*/
bool MarkMgr::NextValid(void)
{
	for (int i=nCurpos+1; i<(int)Count(); ++i) {
		if (markChain[i].IsValid()) {
			nCurpos = i;
			return true;
		}
	}
	return false;
}

//	From Here Apr. 1, 2001 genta
/*!
	���݂̃f�[�^��S�ď������A���݈ʒu�̃|�C���^�����Z�b�g����B

	@par history
	Apr. 1, 2001 genta �V�K�ǉ�
*/
void MarkMgr::Flush(void)
{
	markChain.erase(markChain.begin(), markChain.end());
	nCurpos = 0;
}

//	To Here

//-----------------------------------
// AutoMarkMgr
//-----------------------------------

/*!
	���݈ʒu�ɗv�f��ǉ�����D���݈ʒu�����͑S�č폜����B
	�v�f�ԍ����傫�������V�����f�[�^�B

	@param m �ǉ�����v�f
*/
void AutoMarkMgr::Add(const Mark& m)
{
	// ���݈ʒu���r���̎�
	if (nCurpos < (int)markChain.size()) {
		// ���݈ʒu�܂ŗv�f���폜
		markChain.erase(markChain.begin() + nCurpos, markChain.end());
	}

	// �v�f�̒ǉ�
	markChain.push_back(m);
	++nCurpos;

	// �K�萔�𒴂��Ă��܂��Ƃ��̑Ή�
	Expire();
}

/*!
	�v�f�����ő�l�𒴂��Ă���ꍇ�ɗv�f�����͈͓��Ɏ��܂�悤�A
	�Â���(�ԍ��̎Ⴂ��)����폜����B
*/
void AutoMarkMgr::Expire(void)
{
	int range = markChain.size() - GetMax();

	if (range <= 0) {
		return;
	}

	// �ő�l�𒴂��Ă���ꍇ
	markChain.erase(markChain.begin(), markChain.begin() + range);
	nCurpos -= range;
	if (nCurpos < 0) {
		nCurpos = 0;
	}
}

