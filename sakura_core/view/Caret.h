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

#define _CARETMARGINRATE 20
class TextArea;
class EditView;
class EditDoc;
class TextMetrics;
class Caret;
class EditWnd;

class CaretUnderLine {
public:
	CaretUnderLine(EditView* pEditView)
		:
		m_pEditView(pEditView)
	{
		m_nLockCounter = 0;
		m_nUnderLineLockCounter = 0;
	}
	// �\����\����؂�ւ����Ȃ��悤�ɂ���
	void Lock() {
		++m_nLockCounter;
	}
	// �\����\����؂�ւ�����悤�ɂ���
	void UnLock() {
		--m_nLockCounter;
		if (m_nLockCounter < 0) {
			m_nLockCounter = 0;
		}
	}
	void UnderLineLock() {
		++m_nUnderLineLockCounter;
	}
	// �\����\����؂�ւ�����悤�ɂ���
	void UnderLineUnLock() {
		--m_nUnderLineLockCounter;
		if (m_nUnderLineLockCounter < 0) {
			m_nUnderLineLockCounter = 0;
		}
	}
	void CaretUnderLineON(bool, bool);	// �J�[�\���s�A���_�[���C����ON
	void CaretUnderLineOFF(bool, bool = true, bool = false);	// �J�[�\���s�A���_�[���C����OFF
	void SetUnderLineDoNotOFF(bool flag) { if (!m_nLockCounter)m_bUnderLineDoNotOFF = flag; }
	void SetVertLineDoNotOFF(bool flag) { if (!m_nLockCounter)m_bVertLineDoNotOFF = flag; }
	inline bool GetUnderLineDoNotOFF()const { return m_bUnderLineDoNotOFF; }
	inline bool GetVertLineDoNotOFF()const { return m_bVertLineDoNotOFF; }
private:
	// ���b�N�J�E���^�B0�̂Ƃ��́A���b�N����Ă��Ȃ��BUnLock���Ă΂ꂷ���Ă����ɂ͂Ȃ�Ȃ�
	int m_nLockCounter;
	int m_nUnderLineLockCounter;
	EditView* m_pEditView;
	bool m_bUnderLineDoNotOFF;
	bool m_bVertLineDoNotOFF;
};


class Caret {
public:
	Caret(EditView* pEditView, const EditDoc* pEditDoc);
	
	virtual
	~Caret();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �O���ˑ�                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int GetHankakuDx() const;
	int GetHankakuDy() const;
	int GetHankakuHeight() const;

	// �h�L�������g�̃C���X�^���X�����߂�
	const EditDoc* GetDocument() const { return m_pEditDoc; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �����⏕                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	POINT CalcCaretDrawPos(const LayoutPoint& ptCaretPos) const;


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                   �������E�I�������Ȃ�                      //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �L�����b�g�̍쐬�B2006.12.07 ryoji
	void CreateEditCaret(
		COLORREF crCaret,
		COLORREF crBack,
		int nWidth,
		int nHeight
	);
	
	// �L�����b�g��j������i�����I�ɂ��j���j
	void DestroyCaret() {
		::DestroyCaret();
		m_sizeCaret.cx = 0;
	}

	// �R�s�[
	void CopyCaretStatus(Caret* pDestCaret) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ړ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �ݒ�
	LayoutInt MoveCursorToClientPoint(const POINT& ptClientPos, bool = false, LayoutPoint* = NULL);		// �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ�
	LayoutInt Cursor_UPDOWN(LayoutInt nMoveLines, bool bSelect);		// �J�[�\���㉺�ړ�����
	LayoutInt MoveCursor(												// �s���w��ɂ��J�[�\���ړ�
		LayoutPoint		ptWk_CaretPos,									// [in] �ړ��惌�C�A�E�g�ʒu
		bool			bScroll,										// [in] true: ��ʈʒu�����L��  false: ��ʈʒu��������
		int				nCaretMarginRate	= _CARETMARGINRATE,			// [in] �c�X�N���[���J�n�ʒu�����߂�l
		bool			bUnderlineDoNotOFF	= false,					// [in] �A���_�[���C�����������Ȃ�
		bool			bVertLineDoNotOFF	= false						// [in] �J�[�\���ʒu�c�����������Ȃ�
	);
	LayoutInt MoveCursorFastMode(
		const LogicPoint&	pptWk_CaretPosLogic							// [in] �ړ��惍�W�b�N�ʒu
	);
	LayoutInt MoveCursorProperly(LayoutPoint ptNewXY, bool, bool = false, LayoutPoint* = NULL, int = _CARETMARGINRATE, int = 0);	// �s���w��ɂ��J�[�\���ړ��i���W�����t���j

	//$ �݌v�v�z�I�ɔ���
	void SetCaretLayoutPos(const LayoutPoint& pt) { m_ptCaretPos_Layout = pt; }	// �L�����b�g�ʒu(���C�A�E�g)��ݒ�
	void SetCaretLogicPos(const LogicPoint pt) { m_ptCaretPos_Logic = pt; }		// �L�����b�g�ʒu(���W�b�N)��ݒ�

	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �T�C�Y�ύX                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void SetCaretSize(int nW, int nH) { m_sizeCaret.Set(nW, nH); }						// �L�����b�g�T�C�Y��ݒ�

	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �v�Z                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �v�Z
	bool GetAdjustCursorPos(LayoutPoint* pptPosXY); // �������J�[�\���ʒu���Z�o����

	void ClearCaretPosInfoCache();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �\��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �`��H
	void ShowEditCaret();    // �L�����b�g�̕\���E�X�V
	void ShowCaretPosInfo(); // �L�����b�g�̍s���ʒu��\������

	// API�Ăяo��
	void ShowCaret_(HWND hwnd);
	void HideCaret_(HWND hwnd);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �擾                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	LayoutPoint GetCaretLayoutPos() const	{ return m_ptCaretPos_Layout; }	// �L�����b�g�ʒu(���C�A�E�g)���擾
	Size GetCaretSize() const				{ return m_sizeCaret; }			// �L�����b�g�T�C�Y���擾�B�����m�ɂ͍����͈Ⴄ�炵�� (���̔����̂��Ƃ�����H)
	bool ExistCaretFocus() const			{ return m_sizeCaret.cx>0; }	// �L�����b�g�̃t�H�[�J�X�����邩�B�������l�Ŕ��肵�Ă�炵���B
	LogicPoint GetCaretLogicPos() const		{ return m_ptCaretPos_Logic; }	// �L�����b�g�ʒu(���W�b�N)���擾


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  ��p�x�C���^�[�t�F�[�X                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool GetCaretShowFlag() const { return m_bCaretShowFlag; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	// �Q��
	EditView*				m_pEditView;
	const EditDoc*			m_pEditDoc;

	// �L�����b�g�ʒu
	LayoutPoint	m_ptCaretPos_Layout;	// �r���[����[����̃J�[�\���ʒu�B���C�A�E�g�P�ʁB
	LogicPoint	m_ptCaretPos_Logic;		// �J�[�\���ʒu�B���W�b�N�P�ʁB�f�[�^�������P�ʁB

	// �J�[�\���ʒu�v�Z�L���b�V��
	LayoutInt m_nOffsetCache;
	LayoutInt m_nLineNoCache;
	LogicInt  m_nLogicOffsetCache;
	LogicInt  m_nLineLogicNoCache;
	LayoutInt m_nLineNo50Cache;
	LayoutInt m_nOffset50Cache;
	LogicInt  m_nLogicOffset50Cache;
	int m_nLineLogicModCache;

	
public:
	LayoutInt		m_nCaretPosX_Prev;	// ���O��X���W�L���p�B���C�A�E�g�P�ʁB���̃\�[�X�̉����ɏڍא���������܂��B

	// �L�����b�g������
private:
	Size			m_sizeCaret;		// �L�����b�g�̃T�C�Y�B�s�N�Z���P�ʁB
	COLORREF		m_crCaret;			// �L�����b�g�̐F				// 2006.12.07 ryoji
	HBITMAP			m_hbmpCaret;		// �L�����b�g�̃r�b�g�}�b�v		// 2006.11.28 ryoji
	bool			m_bCaretShowFlag;

	// �A���_�[���C��
public:
	mutable CaretUnderLine m_underLine;
	
	bool			m_bClearStatus;
};


/*!	@brief Caret::m_nCaretPosX_Prev
	���O��X���W�L���p

	�t���[�J�[�\�����[�h�łȂ��ꍇ�ɃJ�[�\�����㉺�Ɉړ��������ꍇ
	�J�[�\���ʒu���Z���s�ł͍s���ɃJ�[�\�����ړ����邪�C
	����Ɉړ��𑱂����ꍇ�ɒ����s�ňړ��N�_��X�ʒu�𕜌��ł���悤��
	���邽�߂̕ϐ��D
	
	@par �g����
	�ǂݏo����EditView::Cursor_UPDOWN()�݂̂ōs���D
	�J�[�\���㉺�ړ��ȊO�ŃJ�[�\���ړ����s�����ꍇ�ɂ�
	������m_nCaretPosX�̒l��ݒ肷��D�������Ȃ���
	���̒���̃J�[�\���㉺�ړ��ňړ��O��X���W�ɖ߂��Ă��܂��D

	�r���[���[����̃J�[�\�����ʒu(�O�J�n)
	
	@date 2004.04.09 genta �������ǉ�
*/

