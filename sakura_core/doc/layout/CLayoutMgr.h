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
#include "doc/CDocListener.h"
#include "_main/global.h"// 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "types/CType.h"
#include "CLayoutExInfo.h"
#include "view/colors/EColorIndexType.h"
#include "COpe.h"
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
enum EKinsokuType {
	KINSOKU_TYPE_NONE = 0,			// �Ȃ�
	KINSOKU_TYPE_WORDWRAP,			// �p�����[�h���b�v��
	KINSOKU_TYPE_KINSOKU_HEAD,		// �s���֑���
	KINSOKU_TYPE_KINSOKU_TAIL,		// �s���֑���
	KINSOKU_TYPE_KINSOKU_KUTO,		// ��Ǔ_�Ԃ牺����
};

struct LayoutReplaceArg {
	LayoutRange	sDelRange;		// [in]�폜�͈́B���C�A�E�g�P�ʁB
	OpeLineData*	pcmemDeleted;	// [out]�폜���ꂽ�f�[�^
	OpeLineData*	pInsData;		// [in/out]�}������f�[�^
	LayoutInt		nAddLineNum;	// [out] �ĕ`��q���g ���C�A�E�g�s�̑���
	LayoutInt		nModLineFrom;	// [out] �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sFrom(���C�A�E�g�s�̑�����0�̂Ƃ��g��)
	LayoutInt		nModLineTo;		// [out] �ĕ`��q���g �ύX���ꂽ���C�A�E�g�sTo(���C�A�E�g�s�̑�����0�̂Ƃ��g��)
	LayoutPoint	ptLayoutNew;	// [out]�}�����ꂽ�����̎��̈ʒu�̈ʒu(���C�A�E�g���ʒu, ���C�A�E�g�s)
	int				nDelSeq;		// [in]�폜�s��Ope�V�[�P���X
	int				nInsSeq;		// [out]�}���s�̌��̃V�[�P���X
};

// �ҏW���̃e�L�X�g�ő啝�Z�o�p		// 2009.08.28 nasukoji
struct CalTextWidthArg {
	LayoutPoint	ptLayout;		// �ҏW�J�n�ʒu
	LayoutInt		nDelLines;		// �폜�Ɋ֌W����s�� - 1�i�����̎��폜�Ȃ��j
	LayoutInt		nAllLinesOld;	// �ҏW�O�̃e�L�X�g�s��
	BOOL			bInsData;		// �ǉ������񂠂�
};

class LogicPointEx: public LogicPoint {
public:
	LayoutInt ext;
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
	typedef LayoutInt (LayoutMgr::*CalcIndentProc)(Layout*);

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
	KetaXInt GetTabSpaceKetas() const { return m_nTabSpace; }
	LayoutInt GetTabSpace() const { return m_nTabSpace; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          �Q�ƌn                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//2007.10.09 kobake �֐����ύX: Search �� SearchLineByLayoutY
	LayoutInt		GetLineCount() const { return m_nLines; }	// �S�����s����Ԃ�
	const wchar_t*	GetLineStr(LayoutInt nLine, LogicInt* pnLineLen) const;	// �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ�
	const wchar_t*	GetLineStr(LayoutInt nLine, LogicInt* pnLineLen, const Layout** ppcLayoutDes) const;	// �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ�

	// �擪�Ɩ���
	Layout*		GetTopLayout()		{ return m_pLayoutTop; }
	Layout*		GetBottomLayout()	{ return m_pLayoutBot; }
	const Layout*	GetTopLayout() const { return m_pLayoutTop; }
	const Layout*	GetBottomLayout() const { return m_pLayoutBot; }

	// ���C�A�E�g��T��
	const Layout*	SearchLineByLayoutY(LayoutInt nLineLayout) const;	// �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(Layout)�ւ̃|�C���^��Ԃ�
	Layout*		SearchLineByLayoutY(LayoutInt nLineLayout) { return const_cast<Layout*>(static_cast<const LayoutMgr*>(this)->SearchLineByLayoutY(nLineLayout)); }

	// ���[�h��T��
	bool			WhereCurrentWord(LayoutInt , LogicInt , LayoutRange* pSelect, NativeW*, NativeW*);	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�

	// ����
	bool			IsEndOfLine(const LayoutPoint& ptLinePos);	// �w��ʒu���s��(���s�����̒��O)�����ׂ�	//@@@ 2002.04.18 MIK

	/*! ����TAB�ʒu�܂ł̕�
		@param pos [in] ���݂̈ʒu
		@return ����TAB�ʒu�܂ł̕������D1�`TAB��
	 */
	LayoutInt GetActualTabSpace(LayoutInt pos) const { return m_nTabSpace - pos % m_nTabSpace; }

	// Aug. 14, 2005 genta
	// Sep. 07, 2007 kobake �֐����ύX GetMaxLineSize��GetMaxLineKetas
	LayoutInt GetMaxLineKetas(void) const { return m_nMaxLineKetas; }

	// 2005.11.21 Moca ���p���̐F���������������珜��
	bool ChangeLayoutParam(LayoutInt nTabSize, LayoutInt nMaxLineKetas);

	// Jul. 29, 2006 genta
	void GetEndLayoutPos(LayoutPoint* ptLayoutEnd);

	LayoutInt GetMaxTextWidth(void) const { return m_nTextWidth; }		// 2009.08.28 nasukoji	�e�L�X�g�ő啝��Ԃ�


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	int PrevOrNextWord(LayoutInt, LogicInt, LayoutPoint* pptLayoutNew, bool, bool bStopsBothEnds);	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
public:
	int PrevWord(LayoutInt nLineNum, LogicInt nIdx, LayoutPoint* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, true, bStopsBothEnds); }	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
	int NextWord(LayoutInt nLineNum, LogicInt nIdx, LayoutPoint* pptLayoutNew, bool bStopsBothEnds) { return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, false, bStopsBothEnds); }	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�

	int SearchWord(LayoutInt nLine, LogicInt nIdx, eSearchDirection eSearchDirection, LayoutRange* pMatchRange, const SearchStringPattern&);	// �P�ꌟ��

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �P�ʂ̕ϊ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// ���W�b�N�����C�A�E�g
	void LogicToLayoutEx(const LogicPointEx& ptLogicEx, LayoutPoint* pptLayout, LayoutInt nLineHint = LayoutInt(0)) {
		LogicToLayout(ptLogicEx, pptLayout, nLineHint);
		pptLayout->x += ptLogicEx.ext;
	}
	void LogicToLayout(const LogicPoint& ptLogic, LayoutPoint* pptLayout, LayoutInt nLineHint = LayoutInt(0));
	void LogicToLayout(const LogicRange& rangeLogic, LayoutRange* prangeLayout) {
		LogicToLayout(rangeLogic.GetFrom(), prangeLayout->GetFromPointer());
		LogicToLayout(rangeLogic.GetTo(), prangeLayout->GetToPointer());
	}
	
	// ���C�A�E�g�����W�b�N�ϊ�
	void LayoutToLogicEx(const LayoutPoint& ptLayout, LogicPointEx* pptLogicEx) const;
	void LayoutToLogic(const LayoutPoint& ptLayout, LogicPoint* pptLogic) const;
	void LayoutToLogic(const LayoutRange& rangeLayout, LogicRange* prangeLogic) const {
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
		LayoutInt		nTabSpace,
		LayoutInt		nMaxLineKetas
	);

	// ������u��
	void ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
	);

	BOOL CalculateTextWidth(bool bCalLineLen = true, LayoutInt nStart = LayoutInt(-1), LayoutInt nEnd = LayoutInt(-1));	// �e�L�X�g�ő啝���Z�o����		// 2009.08.28 nasukoji
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
	LayoutInt DoLayout_Range(Layout* , LogicInt, LogicPoint, EColorIndexType, LayoutColorInfo*, const CalTextWidthArg*, LayoutInt*);	// �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	void CalculateTextWidth_Range(const CalTextWidthArg* pctwArg);	// �e�L�X�g���ҏW���ꂽ��ő啝���Z�o����	// 2009.08.28 nasukoji
	Layout* DeleteLayoutAsLogical(Layout*, LayoutInt, LogicInt , LogicInt, LogicPoint, LayoutInt*);	// �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜
	void ShiftLogicalLineNum(Layout* , LogicInt);	// �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g����

	// ���i
	struct LayoutWork {
		// �����[�v������
		EKinsokuType	eKinsokuType;
		LogicInt		nPos;
		LogicInt		nBgn;
		StringRef		cLineStr;
		LogicInt		nWordBgn;
		LogicInt		nWordLen;
		LayoutInt		nPosX;
		LayoutInt		nIndent;
		Layout*		pLayoutCalculated;

		// ���[�v�O
		DocLine*		pcDocLine;
		Layout*		pLayout;
		ColorStrategy*	pcColorStrategy;
		EColorIndexType	colorPrev;
		LayoutExInfo	exInfoPrev;
		LogicInt		nCurLine;

		// ���[�v�O (DoLayout�̂�)
//		LogicInt		nLineNum;

		// ���[�v�O (DoLayout_Range�̂�)
		bool			bNeedChangeCOMMENTMODE;
		LayoutInt		nModifyLayoutLinesNew;
		
		// ���[�v�O (DoLayout_Range����)
		LayoutInt*		pnExtInsLineNum;
		LogicPoint		ptDelLogicalFrom;

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
	bool _ExistKinsokuKuto(wchar_t wc) const { return m_pszKinsokuKuto_1.exist(wc); }
	bool _ExistKinsokuHead(wchar_t wc) const { return m_pszKinsokuHead_1.exist(wc); }
	bool IsKinsokuHead(wchar_t wc);	// �s���֑��������`�F�b�N����	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail(wchar_t wc);	// �s���֑��������`�F�b�N����	//@@@ 2002.04.08 MIK
	bool IsKinsokuKuto(wchar_t wc);	// ��Ǔ_�������`�F�b�N����	//@@@ 2002.04.17 MIK
	// 2005-08-20 D.S.Koba �֑��֘A�����̊֐���
	/*! ��Ǔ_�Ԃ牺���̏����ʒu��
		@date 2005-08-20 D.S.Koba
		@date Sep. 3, 2005 genta �œK��
	*/
	bool IsKinsokuPosKuto(LayoutInt nRest, LayoutInt nCharChars) const {
		return nRest < nCharChars;
	}
	bool IsKinsokuPosHead(LayoutInt, LayoutInt, LayoutInt);	// �s���֑��̏����ʒu��
	bool IsKinsokuPosTail(LayoutInt, LayoutInt, LayoutInt);	// �s���֑��̏����ʒu��
private:
	// Oct. 1, 2002 genta �C���f���g���v�Z�֐��Q
	LayoutInt getIndentOffset_Normal(Layout* pLayoutPrev);
	LayoutInt getIndentOffset_Tx2x(Layout* pLayoutPrev);
	LayoutInt getIndentOffset_LeftSpace(Layout* pLayoutPrev);

protected:
	/*
	|| �����w���p�n
	*/
	//@@@ 2002.09.23 YAZAKI
	// 2009.08.28 nasukoji	nPosX�����ǉ�
	Layout* CreateLayout(DocLine* pCDocLine, LogicPoint ptLogicPos, LogicInt nLength, EColorIndexType nTypePrev, LayoutInt nIndent, LayoutInt nPosX, LayoutColorInfo*);
	Layout* InsertLineNext(Layout*, Layout*);
	void AddLineBottom(Layout*);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �����o�ϐ�                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	DocLineMgr*			m_pcDocLineMgr;	// �s�o�b�t�@�Ǘ��}�l�[�W��

protected:
	// 2002.10.07 YAZAKI add m_nLineTypeBot
	// 2007.09.07 kobake �ϐ����ύX: m_nMaxLineSize��m_nMaxLineKetas
	// 2007.10.08 kobake �ϐ����ύX: getIndentOffset��m_getIndentOffset

	// �Q��
	EditDoc*		m_pcEditDoc;

	// ���f�[�^
	Layout*				m_pLayoutTop;
	Layout*				m_pLayoutBot;

	// �^�C�v�ʐݒ�
	const TypeConfig*		m_pTypeConfig;
	LayoutInt				m_nMaxLineKetas;
	LayoutInt				m_nTabSpace;
	vector_ex<wchar_t>		m_pszKinsokuHead_1;			// �s���֑�����	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuTail_1;			// �s���֑�����	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuKuto_1;			// ��Ǔ_�Ԃ炳������	//@@@ 2002.04.17 MIK
	CalcIndentProc			m_getIndentOffset;			// Oct. 1, 2002 genta �C���f���g���v�Z�֐���ێ�

	// �t���O��
	EColorIndexType			m_nLineTypeBot;				// �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V����������
	LayoutExInfo			m_cLayoutExInfoBot;
	LayoutInt				m_nLines;					// �S���C�A�E�g�s��

	mutable LayoutInt		m_nPrevReferLine;
	mutable Layout*		m_pLayoutPrevRefer;
	
	// EOF�J�[�\���ʒu���L������(_DoLayout/DoLayout_Range�Ŗ����ɂ���)	//2006.10.01 Moca
	LayoutInt				m_nEOFLine;		// EOF�s��
	LayoutInt				m_nEOFColumn;	// EOF���ʒu

	// �e�L�X�g�ő啝���L���i�܂�Ԃ��ʒu�Z�o�Ɏg�p�j	// 2009.08.28 nasukoji
	LayoutInt				m_nTextWidth;				// �e�L�X�g�ő啝�̋L��
	LayoutInt				m_nTextWidthMaxLine;		// �ő啝�̃��C�A�E�g�s

private:
	DISALLOW_COPY_AND_ASSIGN(LayoutMgr);
};

