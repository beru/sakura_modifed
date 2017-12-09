// �e�L�X�g�̃��C�A�E�g���Ǘ�

#pragma once

#include <Windows.h>
#include <vector>
#include "doc/DocListener.h"
#include "_main/global.h"
#include "basis/SakuraBasis.h"
#include "types/Type.h"
#include "LayoutExInfo.h"
#include "view/colors/EColorIndexType.h"
#include "Ope.h"
#include "util/container.h"
#include "util/design_template.h"

class Bregexp;
class Layout;
class DocLineMgr;
class DocLine;
class Memory;
class EditDoc;
class SearchStringPattern;
class ColorStrategy;

// ���C�A�E�g���֑̋��^�C�v
enum class KinsokuType {
	None = 0,	// �Ȃ�
	WordWrap,	// �p�����[�h���b�v��
	Head,		// �s���֑���
	Tail,		// �s���֑���
	Kuto,		// ��Ǔ_�Ԃ牺����
};

struct LayoutReplaceArg {
	Range			delRange;		// [in]�폜�͈́B���C�A�E�g�P�ʁB
	OpeLineData*	pMemDeleted;	// [out]�폜���ꂽ�f�[�^
	OpeLineData*	pInsData;		// [in/out]�}������f�[�^
	size_t	nAddLineNum;	// [out] �ĕ`��q���g ���C�A�E�g�s�̑���
	int		nModLineFrom;	// [out] �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��)
	int		nModLineTo;		// [out] �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sTo(���C�A�E�g�s�̑�����0�̂Ƃ��g��)
	Point	ptLayoutNew;	// [out]�}�����ꂽ�����̎��̈ʒu�̈ʒu(���C�A�E�g���ʒu, ���C�A�E�g�s)
	int				nDelSeq;		// [in]�폜�s��Ope�V�[�P���X
	int				nInsSeq;		// [out]�}���s�̌��̃V�[�P���X
};

// �ҏW���̃e�L�X�g�ő啝�Z�o�p
struct CalTextWidthArg {
	Point ptLayout;		// �ҏW�J�n�ʒu
	int	nDelLines;		// �폜�Ɋ֌W����s�� - 1�i�����̎��폜�Ȃ��j
	size_t nAllLinesOld;	// �ҏW�O�̃e�L�X�g�s��
	bool bInsData;		// �ǉ������񂠂�
};

class PointEx: public Point {
public:
	size_t ext;
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!	@brief �e�L�X�g�̃��C�A�E�g���Ǘ� */
class LayoutMgr : public ProgressSubject {
private:
	typedef size_t (LayoutMgr::*CalcIndentProc)(Layout*);

public:
	// �����Ɣj��
	LayoutMgr();
	~LayoutMgr();
	void Create(EditDoc*, DocLineMgr*);
	void Init();
	void _Empty();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �R���t�B�O                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// �^�u���̎擾
	size_t GetTabSpaceKetas() const { return nTabSpace; }
	size_t GetTabSpace() const { return nTabSpace; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          �Q�ƌn                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:	
	size_t			GetLineCount() const { return nLines; }	// �S�����s����Ԃ�
	const wchar_t*	GetLineStr(size_t nLine, size_t* pnLineLen) const;	// �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ�
	const wchar_t*	GetLineStr(size_t nLine, size_t* pnLineLen, const Layout** ppcLayoutDes) const;	// �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ�

	// �擪�Ɩ���
	Layout*			GetTopLayout()		{ return pLayoutTop; }
	Layout*			GetBottomLayout()	{ return pLayoutBot; }
	const Layout*	GetTopLayout() const { return pLayoutTop; }
	const Layout*	GetBottomLayout() const { return pLayoutBot; }

	// ���C�A�E�g��T��
	const Layout*	SearchLineByLayoutY(size_t nLineLayout) const;	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	Layout*			SearchLineByLayoutY(size_t nLineLayout) { return const_cast<Layout*>(static_cast<const LayoutMgr*>(this)->SearchLineByLayoutY(nLineLayout)); }

	// ���[�h��T��
	bool			WhereCurrentWord(size_t , size_t , Range* pSelect, NativeW*, NativeW*);	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�

	// ����
	bool			IsEndOfLine(const Point& ptLinePos);	// �w��ʒu���s��(���s�����̒��O)�����ׂ�

	/*! ����TAB�ʒu�܂ł̕�
		@param pos [in] ���݂̈ʒu
		@return ����TAB�ʒu�܂ł̕������D1�`TAB��
	 */
	size_t GetActualTabSpace(size_t pos) const { return nTabSpace - pos % nTabSpace; }

	size_t GetMaxLineKetas(void) const { return nMaxLineKetas; }

	bool ChangeLayoutParam(size_t nTabSize, size_t nMaxLineKetas);

	void GetEndLayoutPos(Point* ptLayoutEnd);

	size_t GetMaxTextWidth(void) const { return nTextWidth; }		// �e�L�X�g�ő啝��Ԃ�


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	bool PrevOrNextWord(size_t, size_t, Point* pptLayoutNew, bool, bool bStopsBothEnds);	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
public:
	bool PrevWord(size_t nLineNum, size_t nIdx, Point* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, true, bStopsBothEnds); }	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
	bool NextWord(size_t nLineNum, size_t nIdx, Point* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, false, bStopsBothEnds); }	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�

	bool SearchWord(size_t nLine, size_t nIdx, SearchDirection SearchDirection, Range* pMatchRange, const SearchStringPattern&);	// �P�ꌟ��

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �P�ʂ̕ϊ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ���W�b�N�����C�A�E�g
	Point LogicToLayoutEx(const PointEx& ptLogicEx, int nLineHint = 0) {
		Point ptLayout = LogicToLayout(ptLogicEx, nLineHint);
		ptLayout.x += (int)ptLogicEx.ext;
		return ptLayout;
	}
	Point LogicToLayout(const Point& ptLogic, int nLineHint = 0);
	void LogicToLayout(const Range& rangeLogic, Range* prangeLayout) {
		prangeLayout->SetFrom(LogicToLayout(rangeLogic.GetFrom()));
		prangeLayout->SetTo(LogicToLayout(rangeLogic.GetTo()));
	}
	
	// ���C�A�E�g�����W�b�N�ϊ�
	PointEx LayoutToLogicEx(const Point& ptLayout) const;
	Point LayoutToLogic(const Point& ptLayout) const;
	void LayoutToLogic(const Range& rangeLayout, Range* prangeLogic) const {
		prangeLogic->SetFrom(LayoutToLogic(rangeLayout.GetFrom()));
		prangeLogic->SetTo(LayoutToLogic(rangeLayout.GetTo()));
	}
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �f�o�b�O                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void DUMP();	// �e�X�g�p�Ƀ��C�A�E�g�����_���v
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �ҏW�Ƃ�                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*
	|| �X�V�n
	*/
	/* ���C�A�E�g���̕ύX */
	void SetLayoutInfo(
		bool			bDoLayout,
		const TypeConfig&	refType,
		size_t			nTabSpace,
		size_t			nMaxLineKetas
	);

	// ������u��
	void ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
	);

	BOOL CalculateTextWidth(bool bCalLineLen = true, int nStart = -1, int nEnd = -1);	// �e�L�X�g�ő啝���Z�o����
	void ClearLayoutLineWidth(void);				// �e�s�̃��C�A�E�g�s���̋L�����N���A����

protected:
	/*
	||  �Q�ƌn
	*/
	const char* GetFirstLinrStr(int*);	// ���A�N�Z�X���[�h�F�擪�s�𓾂�
	const char* GetNextLinrStr(int*);	// ���A�N�Z�X���[�h�F���̍s�𓾂�

	/*
	|| �X�V�n
	*/
public:
	void _DoLayout();	// ���݂̐܂�Ԃ��������ɍ��킹�đS�f�[�^�̃��C�A�E�g�����Đ������܂�
protected:
	int DoLayout_Range(Layout* , int, Point, EColorIndexType, LayoutColorInfo*, const CalTextWidthArg&);	// �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	void CalculateTextWidth_Range(const CalTextWidthArg& ctwArg);	// �e�L�X�g���ҏW���ꂽ��ő啝���Z�o����
	Layout* DeleteLayoutAsLogical(Layout*, int, int , int, Point, size_t*);	// �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜
	void ShiftLogicalLineNum(Layout* , int);	// �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g����

	// ���i
	struct LayoutWork {
		// �����[�v������
		KinsokuType	eKinsokuType;
		size_t		nPos;
		size_t		nBgn;
		StringRef	lineStr;
		size_t		nWordBgn;
		size_t		nWordLen;
		int			nPosX;
		int			nIndent;
		Layout*		pLayoutCalculated;

		// ���[�v�O
		DocLine*		pDocLine;
		Layout*			pLayout;
		ColorStrategy*	pColorStrategy;
		EColorIndexType	colorPrev;
		LayoutExInfo	exInfoPrev;
		int				nCurLine;

		// ���[�v�O (DoLayout�̂�)
//		int		nLineNum;

		// ���[�v�O (DoLayout_Range�̂�)
		bool			bNeedChangeCOMMENTMODE;
		int				nModifyLayoutLinesNew;
		
		// ���[�v�O (DoLayout_Range����)
		Point			ptDelLogicalFrom;

		// �֐�
		Layout* _CreateLayout(LayoutMgr* mgr);
	};
	// �֐��|�C���^
	typedef void (LayoutMgr::*PF_OnLine)(LayoutWork*);
	// DoLayout�p
	bool _DoKinsokuSkip(LayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoWordWrap(LayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoKutoBurasage(LayoutWork* pWork);
	void _DoGyotoKinsoku(LayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoGyomatsuKinsoku(LayoutWork* pWork, PF_OnLine pfOnLine);
	bool _DoTab(LayoutWork* pWork, PF_OnLine pfOnLine);
	void _MakeOneLine(LayoutWork* pWork, PF_OnLine pfOnLine);
	// DoLayout�p�R�A
	void _OnLine1(LayoutWork* pWork);
	// DoLayout_Range�p�R�A
	void _OnLine2(LayoutWork* pWork);
	
private:
	bool _ExistKinsokuKuto(wchar_t wc) const { return pszKinsokuKuto_1.exist(wc); }
	bool _ExistKinsokuHead(wchar_t wc) const { return pszKinsokuHead_1.exist(wc); }
	bool IsKinsokuHead(wchar_t wc);	// �s���֑��������`�F�b�N����
	bool IsKinsokuTail(wchar_t wc);	// �s���֑��������`�F�b�N����
	bool IsKinsokuKuto(wchar_t wc);	// ��Ǔ_�������`�F�b�N����
	/*! ��Ǔ_�Ԃ牺���̏����ʒu�� */
	bool IsKinsokuPosKuto(size_t nRest, size_t nCharChars) const {
		return nRest < nCharChars;
	}
	bool IsKinsokuPosHead(size_t, size_t, size_t);	// �s���֑��̏����ʒu��
	bool IsKinsokuPosTail(size_t, size_t, size_t);	// �s���֑��̏����ʒu��
private:
	// �C���f���g���v�Z�֐��Q
	size_t getIndentOffset_Normal(Layout* pLayoutPrev);
	size_t getIndentOffset_Tx2x(Layout* pLayoutPrev);
	size_t getIndentOffset_LeftSpace(Layout* pLayoutPrev);

protected:
	/*
	|| �����w���p�n
	*/
	Layout* CreateLayout(DocLine* pDocLine, Point ptLogicPos, int nLength, EColorIndexType nTypePrev, int nIndent, int nPosX, LayoutColorInfo*);
	Layout* InsertLineNext(Layout*, Layout*);
	void AddLineBottom(Layout*);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	DocLineMgr*			pDocLineMgr;	// �s�o�b�t�@�Ǘ��}�l�[�W��

protected:
	// �Q��
	EditDoc*			pEditDoc;

	// ���f�[�^
	Layout*				pLayoutTop;
	Layout*				pLayoutBot;

	// �^�C�v�ʐݒ�
	const TypeConfig*		pTypeConfig;
	size_t					nMaxLineKetas;
	size_t					nTabSpace;
	vector_ex<wchar_t>		pszKinsokuHead_1;			// �s���֑�����
	vector_ex<wchar_t>		pszKinsokuTail_1;			// �s���֑�����
	vector_ex<wchar_t>		pszKinsokuKuto_1;			// ��Ǔ_�Ԃ炳������
	CalcIndentProc			getIndentOffset;			// �C���f���g���v�Z�֐���ێ�

	// �t���O��
	EColorIndexType			nLineTypeBot;				// �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V����������
	LayoutExInfo			layoutExInfoBot;
	size_t					nLines;					// �S���C�A�E�g�s��

	mutable int				nPrevReferLine;
	mutable Layout*			pLayoutPrevRefer;
	
	// EOF�J�[�\���ʒu���L������(_DoLayout/DoLayout_Range�Ŗ����ɂ���)
	int						nEOFLine;		// EOF�s��
	int						nEOFColumn;	// EOF���ʒu

	// �e�L�X�g�ő啝���L���i�܂�Ԃ��ʒu�Z�o�Ɏg�p�j
	size_t					nTextWidth;				// �e�L�X�g�ő啝�̋L��
	size_t					nTextWidthMaxLine;		// �ő啝�̃��C�A�E�g�s

private:
	DISALLOW_COPY_AND_ASSIGN(LayoutMgr);
};

