/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
	@date 1998/03/06 �V�K�쐬
	@date 1998/04/14 �f�[�^�̍폜������
	@date 1999/12/20 �f�[�^�̒u��������
	@date 2009/08/28 nasukoji	CalTextWidthArg��`�ǉ��ADoLayout_Range()�̈����ύX
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, Moca, genta, D.S.Koba
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>// 2002/2/10 aroka
#include <vector>
#include "doc/DocListener.h"
#include "_main/global.h"// 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "types/Type.h"
#include "LayoutExInfo.h"
#include "view/colors/EColorIndexType.h"
#include "Ope.h"
#include "util/container.h"
#include "util/design_template.h"

class Bregexp;// 2002/2/10 aroka
class Layout;// 2002/2/10 aroka
class DocLineMgr;// 2002/2/10 aroka
class DocLine;// 2002/2/10 aroka
class Memory;// 2002/2/10 aroka
class EditDoc;// 2003/07/20 genta
class SearchStringPattern;
class ColorStrategy;

// ���C�A�E�g���֑̋��^�C�v	//@@@ 2002.04.20 MIK
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
	int		nAddLineNum;	// [out] �ĕ`��q���g ���C�A�E�g�s�̑���
	int		nModLineFrom;	// [out] �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��)
	int		nModLineTo;		// [out] �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sTo(���C�A�E�g�s�̑�����0�̂Ƃ��g��)
	Point	ptLayoutNew;	// [out]�}�����ꂽ�����̎��̈ʒu�̈ʒu(���C�A�E�g���ʒu, ���C�A�E�g�s)
	int				nDelSeq;		// [in]�폜�s��Ope�V�[�P���X
	int				nInsSeq;		// [out]�}���s�̌��̃V�[�P���X
};

// �ҏW���̃e�L�X�g�ő啝�Z�o�p		// 2009.08.28 nasukoji
struct CalTextWidthArg {
	Point	ptLayout;		// �ҏW�J�n�ʒu
	int	nDelLines;		// �폜�Ɋ֌W����s�� - 1�i�����̎��폜�Ȃ��j
	int	nAllLinesOld;	// �ҏW�O�̃e�L�X�g�s��
	bool		bInsData;		// �ǉ������񂠂�
};

class PointEx: public Point {
public:
	size_t ext;
};

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@date 2005.11.21 Moca �F�������������o�[�ֈړ��D�s�v�ƂȂ��������������o�֐�����폜�D
*/
// 2007.10.15 XYLogicalToLayout��p�~�BLogicToLayout�ɓ����B
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
	//2007.10.09 kobake �֐����ύX: Search �� SearchLineByLayoutY
	size_t			GetLineCount() const { return nLines; }	// �S�����s����Ԃ�
	const wchar_t*	GetLineStr(size_t nLine, size_t* pnLineLen) const;	// �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ�
	const wchar_t*	GetLineStr(size_t nLine, size_t* pnLineLen, const Layout** ppcLayoutDes) const;	// �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ�

	// �擪�Ɩ���
	Layout*			GetTopLayout()		{ return pLayoutTop; }
	Layout*			GetBottomLayout()	{ return pLayoutBot; }
	const Layout*	GetTopLayout() const { return pLayoutTop; }
	const Layout*	GetBottomLayout() const { return pLayoutBot; }

	// ���C�A�E�g��T��
	const Layout*	SearchLineByLayoutY(int nLineLayout) const;	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	Layout*			SearchLineByLayoutY(int nLineLayout) { return const_cast<Layout*>(static_cast<const LayoutMgr*>(this)->SearchLineByLayoutY(nLineLayout)); }

	// ���[�h��T��
	bool			WhereCurrentWord(int , int , Range* pSelect, NativeW*, NativeW*);	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�

	// ����
	bool			IsEndOfLine(const Point& ptLinePos);	// �w��ʒu���s��(���s�����̒��O)�����ׂ�	//@@@ 2002.04.18 MIK

	/*! ����TAB�ʒu�܂ł̕�
		@param pos [in] ���݂̈ʒu
		@return ����TAB�ʒu�܂ł̕������D1�`TAB��
	 */
	size_t GetActualTabSpace(size_t pos) const { return nTabSpace - pos % nTabSpace; }

	// Aug. 14, 2005 genta
	// Sep. 07, 2007 kobake �֐����ύX GetMaxLineSize��GetMaxLineKetas
	size_t GetMaxLineKetas(void) const { return nMaxLineKetas; }

	// 2005.11.21 Moca ���p���̐F���������������珜��
	bool ChangeLayoutParam(size_t nTabSize, size_t nMaxLineKetas);

	// Jul. 29, 2006 genta
	void GetEndLayoutPos(Point* ptLayoutEnd);

	size_t GetMaxTextWidth(void) const { return nTextWidth; }		// 2009.08.28 nasukoji	�e�L�X�g�ő啝��Ԃ�


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	int PrevOrNextWord(int, int, Point* pptLayoutNew, bool, bool bStopsBothEnds);	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
public:
	int PrevWord(int nLineNum, int nIdx, Point* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, true, bStopsBothEnds); }	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
	int NextWord(int nLineNum, int nIdx, Point* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, false, bStopsBothEnds); }	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�

	int SearchWord(int nLine, int nIdx, SearchDirection SearchDirection, Range* pMatchRange, const SearchStringPattern&);	// �P�ꌟ��

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �P�ʂ̕ϊ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ���W�b�N�����C�A�E�g
	void LogicToLayoutEx(const PointEx& ptLogicEx, Point* pptLayout, int nLineHint = 0) {
		LogicToLayout(ptLogicEx, pptLayout, nLineHint);
		pptLayout->x += (int)ptLogicEx.ext;
	}
	void LogicToLayout(const Point& ptLogic, Point* pptLayout, int nLineHint = 0);
	void LogicToLayout(const Range& rangeLogic, Range* prangeLayout) {
		LogicToLayout(rangeLogic.GetFrom(), prangeLayout->GetFromPointer());
		LogicToLayout(rangeLogic.GetTo(), prangeLayout->GetToPointer());
	}
	
	// ���C�A�E�g�����W�b�N�ϊ�
	void LayoutToLogicEx(const Point& ptLayout, PointEx* pptLogicEx) const;
	void LayoutToLogic(const Point& ptLayout, Point* pptLogic) const;
	void LayoutToLogic(const Range& rangeLayout, Range* prangeLogic) const {
		LayoutToLogic(rangeLayout.GetFrom(), prangeLogic->GetFromPointer());
		LayoutToLogic(rangeLayout.GetTo(), prangeLogic->GetToPointer());
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
	/* ���C�A�E�g���̕ύX
		@date Jun. 01, 2001 JEPRO char* (�s�R�����g�f���~�^3�p)��1�ǉ�
		@date 2002.04.13 MIK �֑�,���s�������Ԃ牺����,��Ǔ_�Ԃ炳����ǉ�
		@date 2002/04/27 YAZAKI TypeConfig��n���悤�ɕύX�B
	*/
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

	BOOL CalculateTextWidth(bool bCalLineLen = true, int nStart = -1, int nEnd = -1);	// �e�L�X�g�ő啝���Z�o����		// 2009.08.28 nasukoji
	void ClearLayoutLineWidth(void);				// �e�s�̃��C�A�E�g�s���̋L�����N���A����		// 2009.08.28 nasukoji

protected:
	/*
	||  �Q�ƌn
	*/
	const char* GetFirstLinrStr(int*);	// ���A�N�Z�X���[�h�F�擪�s�𓾂�
	const char* GetNextLinrStr(int*);		// ���A�N�Z�X���[�h�F���̍s�𓾂�

	/*
	|| �X�V�n
	*/
	// 2005.11.21 Moca ���p���̐F���������������珜��
public:
	void _DoLayout();	// ���݂̐܂�Ԃ��������ɍ��킹�đS�f�[�^�̃��C�A�E�g�����Đ������܂�
protected:
	// 2005.11.21 Moca ���p���̐F���������������珜��
	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�����ǉ�
	int DoLayout_Range(Layout* , int, Point, EColorIndexType, LayoutColorInfo*, const CalTextWidthArg&, int*);	// �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	void CalculateTextWidth_Range(const CalTextWidthArg& ctwArg);	// �e�L�X�g���ҏW���ꂽ��ő啝���Z�o����	// 2009.08.28 nasukoji
	Layout* DeleteLayoutAsLogical(Layout*, int, int , int, Point, int*);	// �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜
	void ShiftLogicalLineNum(Layout* , int);	// �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g����

	// ���i
	struct LayoutWork {
		// �����[�v������
		KinsokuType	eKinsokuType;
		int			nPos;
		int			nBgn;
		StringRef	lineStr;
		int			nWordBgn;
		int			nWordLen;
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
		int*			pnExtInsLineNum;
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
	bool IsKinsokuHead(wchar_t wc);	// �s���֑��������`�F�b�N����	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail(wchar_t wc);	// �s���֑��������`�F�b�N����	//@@@ 2002.04.08 MIK
	bool IsKinsokuKuto(wchar_t wc);	// ��Ǔ_�������`�F�b�N����	//@@@ 2002.04.17 MIK
	// 2005-08-20 D.S.Koba �֑��֘A�����̊֐���
	/*! ��Ǔ_�Ԃ牺���̏����ʒu��
		@date 2005-08-20 D.S.Koba
		@date Sep. 3, 2005 genta �œK��
	*/
	bool IsKinsokuPosKuto(int nRest, int nCharChars) const {
		return nRest < nCharChars;
	}
	bool IsKinsokuPosHead(int, int, int);	// �s���֑��̏����ʒu��
	bool IsKinsokuPosTail(int, int, int);	// �s���֑��̏����ʒu��
private:
	// Oct. 1, 2002 genta �C���f���g���v�Z�֐��Q
	size_t getIndentOffset_Normal(Layout* pLayoutPrev);
	size_t getIndentOffset_Tx2x(Layout* pLayoutPrev);
	size_t getIndentOffset_LeftSpace(Layout* pLayoutPrev);

protected:
	/*
	|| �����w���p�n
	*/
	//@@@ 2002.09.23 YAZAKI
	// 2009.08.28 nasukoji	nPosX�����ǉ�
	Layout* CreateLayout(DocLine* pDocLine, Point ptLogicPos, int nLength, EColorIndexType nTypePrev, int nIndent, int nPosX, LayoutColorInfo*);
	Layout* InsertLineNext(Layout*, Layout*);
	void AddLineBottom(Layout*);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	DocLineMgr*			pDocLineMgr;	// �s�o�b�t�@�Ǘ��}�l�[�W��

protected:
	// 2002.10.07 YAZAKI add nLineTypeBot
	// 2007.09.07 kobake �ϐ����ύX: nMaxLineSize��nMaxLineKetas
	// 2007.10.08 kobake �ϐ����ύX: getIndentOffset��getIndentOffset

	// �Q��
	EditDoc*			pEditDoc;

	// ���f�[�^
	Layout*				pLayoutTop;
	Layout*				pLayoutBot;

	// �^�C�v�ʐݒ�
	const TypeConfig*		pTypeConfig;
	size_t					nMaxLineKetas;
	size_t					nTabSpace;
	vector_ex<wchar_t>		pszKinsokuHead_1;			// �s���֑�����	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		pszKinsokuTail_1;			// �s���֑�����	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		pszKinsokuKuto_1;			// ��Ǔ_�Ԃ炳������	//@@@ 2002.04.17 MIK
	CalcIndentProc			getIndentOffset;			// Oct. 1, 2002 genta �C���f���g���v�Z�֐���ێ�

	// �t���O��
	EColorIndexType			nLineTypeBot;				// �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V����������
	LayoutExInfo			layoutExInfoBot;
	size_t					nLines;					// �S���C�A�E�g�s��

	mutable int				nPrevReferLine;
	mutable Layout*			pLayoutPrevRefer;
	
	// EOF�J�[�\���ʒu���L������(_DoLayout/DoLayout_Range�Ŗ����ɂ���)	//2006.10.01 Moca
	int						nEOFLine;		// EOF�s��
	int						nEOFColumn;	// EOF���ʒu

	// �e�L�X�g�ő啝���L���i�܂�Ԃ��ʒu�Z�o�Ɏg�p�j	// 2009.08.28 nasukoji
	size_t					nTextWidth;				// �e�L�X�g�ő啝�̋L��
	size_t					nTextWidthMaxLine;		// �ő啝�̃��C�A�E�g�s

private:
	DISALLOW_COPY_AND_ASSIGN(LayoutMgr);
};

