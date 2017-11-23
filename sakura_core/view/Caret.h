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
	CaretUnderLine(EditView& editView)
		:
		editView(editView)
	{
		nLockCounter = 0;
		nUnderLineLockCounter = 0;
	}
	// �\����\����؂�ւ����Ȃ��悤�ɂ���
	void Lock() {
		++nLockCounter;
	}
	// �\����\����؂�ւ�����悤�ɂ���
	void UnLock() {
		--nLockCounter;
		if (nLockCounter < 0) {
			nLockCounter = 0;
		}
	}
	void UnderLineLock() {
		++nUnderLineLockCounter;
	}
	// �\����\����؂�ւ�����悤�ɂ���
	void UnderLineUnLock() {
		--nUnderLineLockCounter;
		if (nUnderLineLockCounter < 0) {
			nUnderLineLockCounter = 0;
		}
	}
	void CaretUnderLineON(bool, bool);	// �J�[�\���s�A���_�[���C����ON
	void CaretUnderLineOFF(bool, bool = true, bool = false);	// �J�[�\���s�A���_�[���C����OFF
	void SetUnderLineDoNotOFF(bool flag) { if (!nLockCounter) bUnderLineDoNotOFF = flag; }
	void SetVertLineDoNotOFF(bool flag) { if (!nLockCounter) bVertLineDoNotOFF = flag; }
	inline bool GetUnderLineDoNotOFF()const { return bUnderLineDoNotOFF; }
	inline bool GetVertLineDoNotOFF()const { return bVertLineDoNotOFF; }
private:
	// ���b�N�J�E���^�B0�̂Ƃ��́A���b�N����Ă��Ȃ��BUnLock���Ă΂ꂷ���Ă����ɂ͂Ȃ�Ȃ�
	int nLockCounter;
	int nUnderLineLockCounter;
	EditView& editView;
	bool bUnderLineDoNotOFF;
	bool bVertLineDoNotOFF;
};


class Caret {
public:
	Caret(EditView& editView, const EditDoc& editDoc);
	
	virtual
	~Caret();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �O���ˑ�                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int GetHankakuDx() const;
	int GetHankakuDy() const;
	int GetHankakuHeight() const;

	// �h�L�������g�̃C���X�^���X�����߂�
	const EditDoc& GetDocument() const { return editDoc; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �����⏕                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	POINT CalcCaretDrawPos(const Point& ptCaretPos) const;


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
		sizeCaret.cx = 0;
	}

	// �R�s�[
	void CopyCaretStatus(Caret* pDestCaret) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ړ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// �ݒ�
	int MoveCursorToClientPoint(const POINT& ptClientPos, bool = false, Point* = nullptr);		// �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ�
	int Cursor_UPDOWN(int nMoveLines, bool bSelect);		// �J�[�\���㉺�ړ�����
	int MoveCursor(												// �s���w��ɂ��J�[�\���ړ�
		Point	ptWk_CaretPos,									// [in] �ړ��惌�C�A�E�g�ʒu
		bool	bScroll,										// [in] true: ��ʈʒu�����L��  false: ��ʈʒu��������
		int		nCaretMarginRate	= _CARETMARGINRATE,			// [in] �c�X�N���[���J�n�ʒu�����߂�l
		bool	bUnderlineDoNotOFF	= false,					// [in] �A���_�[���C�����������Ȃ�
		bool	bVertLineDoNotOFF	= false						// [in] �J�[�\���ʒu�c�����������Ȃ�
	);
	int MoveCursorFastMode(
		const Point&	pptWk_CaretPosLogic							// [in] �ړ��惍�W�b�N�ʒu
	);
	int MoveCursorProperly(Point ptNewXY, bool, bool = false, Point* = nullptr, int = _CARETMARGINRATE, int = 0);	// �s���w��ɂ��J�[�\���ړ��i���W�����t���j

	//$ �݌v�v�z�I�ɔ���
	void SetCaretLayoutPos(const Point& pt) { ptCaretPos_Layout = pt; }	// �L�����b�g�ʒu(���C�A�E�g)��ݒ�
	void SetCaretLogicPos(const Point& pt) { ptCaretPos_Logic = pt; }		// �L�����b�g�ʒu(���W�b�N)��ݒ�

	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �T�C�Y�ύX                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void SetCaretSize(int nW, int nH) { sizeCaret.Set(nW, nH); }						// �L�����b�g�T�C�Y��ݒ�

	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �v�Z                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �v�Z
	bool GetAdjustCursorPos(Point* pptPosXY); // �������J�[�\���ʒu���Z�o����

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

	Point GetCaretLayoutPos() const	{ return ptCaretPos_Layout; }	// �L�����b�g�ʒu(���C�A�E�g)���擾
	Size GetCaretSize() const		{ return sizeCaret; }			// �L�����b�g�T�C�Y���擾�B�����m�ɂ͍����͈Ⴄ�炵�� (���̔����̂��Ƃ�����H)
	bool ExistCaretFocus() const	{ return sizeCaret.cx > 0; }	// �L�����b�g�̃t�H�[�J�X�����邩�B�������l�Ŕ��肵�Ă�炵���B
	Point GetCaretLogicPos() const	{ return ptCaretPos_Logic; }	// �L�����b�g�ʒu(���W�b�N)���擾


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                  ��p�x�C���^�[�t�F�[�X                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool GetCaretShowFlag() const { return bCaretShowFlag; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	// �Q��
	EditView&		editView;
	const EditDoc&	editDoc;

	// �L�����b�g�ʒu
	Point	ptCaretPos_Layout;		// �r���[����[����̃J�[�\���ʒu�B���C�A�E�g�P�ʁB
	Point	ptCaretPos_Logic;		// �J�[�\���ʒu�B���W�b�N�P�ʁB�f�[�^�������P�ʁB

	// �J�[�\���ʒu�v�Z�L���b�V��
	int nOffsetCache;
	int nLineNoCache;
	int nLogicOffsetCache;
	int nLineLogicNoCache;
	int nLineNo50Cache;
	int nOffset50Cache;
	int nLogicOffset50Cache;
	int nLineLogicModCache;
	
public:
	int	nCaretPosX_Prev;	// ���O��X���W�L���p�B���C�A�E�g�P�ʁB���̃\�[�X�̉����ɏڍא���������܂��B

	// �L�����b�g������
private:
	Size		sizeCaret;		// �L�����b�g�̃T�C�Y�B�s�N�Z���P�ʁB
	COLORREF	crCaret;			// �L�����b�g�̐F				// 2006.12.07 ryoji
	HBITMAP		hbmpCaret;		// �L�����b�g�̃r�b�g�}�b�v		// 2006.11.28 ryoji
	bool		bCaretShowFlag;

	// �A���_�[���C��
public:
	mutable CaretUnderLine underLine;
	
	bool	bClearStatus;
};


/*!	@brief Caret::nCaretPosX_Prev
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
*/

