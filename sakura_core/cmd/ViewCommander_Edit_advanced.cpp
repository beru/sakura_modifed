/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�ҏW�n ���x�ȑ���(���P��/�s����))�֐��Q

	2012/12/17	ViewCommander.cpp,ViewCommander_New.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta, �݂�
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, genta, aroka, Azumaiya, minfu, MIK, oak, ���Ȃӂ�, Moca, ai
	Copyright (C) 2003, MIK, genta, �����, zenryaku, Moca, ryoji, naoh, KEITA, ���イ��
	Copyright (C) 2004, isearch, Moca, gis_dur, genta, crayonzen, fotomo, MIK, novice, �݂��΂�, Kazika
	Copyright (C) 2005, genta, novice, �����, MIK, Moca, D.S.Koba, aroka, ryoji, maru
	Copyright (C) 2006, genta, aroka, ryoji, �����, fon, yukihane, Moca
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, ryoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "uiparts/WaitCursor.h"
#include "mem/MemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "_os/OsVersionInfo.h"

using namespace std; // 2002/2/3 aroka to here

#ifndef FID_RECONVERT_VERSION  // 2002.04.10 minfu 
#define FID_RECONVERT_VERSION 0x10000000
#endif
#ifndef SCS_CAP_SETRECONVERTSTRING
#define SCS_CAP_SETRECONVERTSTRING 0x00000004
#define SCS_QUERYRECONVERTSTRING 0x00020000
#define SCS_SETRECONVERTSTRING 0x00010000
#endif

// �C���f���g ver1
void ViewCommander::Command_INDENT(wchar_t wcChar, IndentType eIndent)
{
	using namespace WCODE;

	auto& selInfo = m_view.GetSelectionInfo();
#if 1	// ���������c���ΑI�𕝃[�����ő�ɂ���i�]���݊������j�B�����Ă� Command_INDENT() ver0 ���K�؂ɓ��삷��悤�ɕύX���ꂽ�̂ŁA�폜���Ă����ɕs�s���ɂ͂Ȃ�Ȃ��B
	// From Here 2001.12.03 hor
	// SPACEorTAB�C�����f���g�ŋ�`�I�������[���̎��͑I��͈͂��ő�ɂ���
	// Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if (eIndent != IndentType::None
		&& selInfo.IsBoxSelecting()
		&& GetSelect().GetFrom().x == GetSelect().GetTo().x
	) {
		GetSelect().SetToX(GetDocument()->m_layoutMgr.GetMaxLineKetas());
		m_view.RedrawAll();
		return;
	}
	// To Here 2001.12.03 hor
#endif
	Command_INDENT(&wcChar, LogicInt(1), eIndent);
	return;
}


// �C���f���g ver0
/*
	�I�����ꂽ�e�s�͈̔͂̒��O�ɁA�^����ꂽ������(pData)��}������B
	@param eIndent �C���f���g�̎��
*/
void ViewCommander::Command_INDENT(
	const wchar_t* const pData,
	const LogicInt nDataLen,
	IndentType eIndent
	)
{
	if (nDataLen <= 0) {
		return;
	}
	auto& layoutMgr = GetDocument()->m_layoutMgr;
	LayoutRange selectOld;		// �͈͑I��
	LayoutPoint ptInserted;		// �}����̑}���ʒu
	const struct IsIndentCharSpaceTab {
		IsIndentCharSpaceTab() {}
		bool operator()(const wchar_t ch) const
		{ return ch == WCODE::SPACE || ch == WCODE::TAB; }
	} IsIndentChar;
	struct SoftTabData {
		SoftTabData(LayoutInt nTab) : m_szTab(NULL), m_nTab((Int)nTab) {}
		~SoftTabData() { delete []m_szTab; }
		operator const wchar_t* ()
		{
			if (!m_szTab) {
				m_szTab = new wchar_t[m_nTab];
				wmemset(m_szTab, WCODE::SPACE, m_nTab);
			}
			return m_szTab;
		}
		int Len(LayoutInt nCol) { return m_nTab - ((Int)nCol % m_nTab); }
		wchar_t* m_szTab;
		int m_nTab;
	} stabData(layoutMgr.GetTabSpace());

	const bool bSoftTab = (eIndent == IndentType::Tab && m_view.m_pTypeData->bInsSpace);
	GetDocument()->m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

	auto& caret = GetCaret();
	auto& selInfo = m_view.GetSelectionInfo();

	if (!selInfo.IsTextSelected()) {			// �e�L�X�g���I������Ă��邩
		if (eIndent != IndentType::None && !bSoftTab) {
			// ����`�I���ł͂Ȃ��̂� Command_WCHAR ����Ăі߂������悤�Ȃ��Ƃ͂Ȃ�
			Command_WCHAR(pData[0]);	// 1��������
		}else {
			// ����`�I���ł͂Ȃ��̂ł����֗���͎̂��ۂɂ̓\�t�g�^�u�̂Ƃ�����
			if (bSoftTab && !m_view.IsInsMode()) {
				DelCharForOverwrite(pData, nDataLen);
			}
			m_view.InsertData_CEditView(
				caret.GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(caret.GetCaretLayoutPos().GetX2()),
				&ptInserted,
				true
			);
			caret.MoveCursor(ptInserted, true);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}
		return;
	}
	const bool bDrawSwitchOld = m_view.SetDrawSwitch(false);	// 2002.01.25 hor
	// ��`�͈͑I�𒆂�
	if (selInfo.IsBoxSelecting()) {
// 2012.10.31 Moca �㏑�����[�h�̂Ƃ��̑I��͈͍폜����߂�
#if 0
		// From Here 2001.12.03 hor
		// �㏑���[�h�̂Ƃ��͑I��͈͍폜
		if (!m_view.IsInsMode() /* Oct. 2, 2005 genta */) {
			selectOld = GetSelect();
			m_view.DeleteData(false);
			GetSelect() = selectOld;
			selInfo.SetBoxSelect(true);
		}
		// To Here 2001.12.03 hor
#endif

		// 2�_��Ίp�Ƃ����`�����߂�
		LayoutRange rcSel;
		TwoPointToRange(
			&rcSel,
			GetSelect().GetFrom(),	// �͈͑I���J�n
			GetSelect().GetTo()		// �͈͑I���I��
		);
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(false/*true 2002.01.25 hor*/);

		/*
			�����𒼑O�ɑ}�����ꂽ�������A����ɂ�茳�̈ʒu����ǂꂾ�����ɂ��ꂽ���B
			����ɏ]����`�I��͈͂����ɂ��炷�B
		*/
		LayoutInt minOffset(-1);
		/*
			���S�p�����̍����̌������ɂ���
			(1) eIndent == IndentType::Tab �̂Ƃ�
				�I��͈͂��^�u���E�ɂ���Ƃ��Ƀ^�u����͂���ƁA�S�p�����̑O�����I��͈͂���
				�͂ݏo���Ă���s�Ƃ����łȂ��s�Ń^�u�̕����A1����ݒ肳�ꂽ�ő�܂łƑ傫���قȂ�A
				�ŏ��ɑI������Ă���������I��͈͓��ɂƂǂ߂Ă������Ƃ��ł��Ȃ��Ȃ�B
				�ŏ��͋�`�I��͈͓��ɂ��ꂢ�Ɏ��܂��Ă���s�ɂ̓^�u��}�������A������Ƃ����͂�
				�o���Ă���s�ɂ����^�u��}�����邱�ƂƂ��A����ł͂ǂ̍s�ɂ��^�u���}������Ȃ�
				�Ƃ킩�����Ƃ��͂�蒼���ă^�u��}������B
			(2) eIndent == IndentType::Space �̂Ƃ��i���]���݊��I�ȓ���j
				��1�őI�����Ă���ꍇ�̂ݑS�p�����̍���������������B
				�ŏ��͋�`�I��͈͓��ɂ��ꂢ�Ɏ��܂��Ă���s�ɂ̓X�y�[�X��}�������A������Ƃ����͂�
				�o���Ă���s�ɂ����X�y�[�X��}�����邱�ƂƂ��A����ł͂ǂ̍s�ɂ��X�y�[�X���}������Ȃ�
				�Ƃ킩�����Ƃ��͂�蒼���ăX�y�[�X��}������B
		*/
		bool alignFullWidthChar = (eIndent == IndentType::Tab) && ((rcSel.GetFrom().x % layoutMgr.GetTabSpace()) == 0);
#if 1	// ���������c���ΑI��1��SPACE�C���f���g�őS�p�����𑵂���@�\(2)���ǉ������B
		alignFullWidthChar = alignFullWidthChar || (eIndent == IndentType::Space && rcSel.GetTo().x - rcSel.GetFrom().x == 1);
#endif
		WaitCursor waitCursor(m_view.GetHwnd(), 1000 < rcSel.GetTo().y - rcSel.GetFrom().y);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}
		for (bool insertionWasDone=false; ; alignFullWidthChar=false) {
			minOffset = LayoutInt(-1);
			for (LayoutInt nLineNum=rcSel.GetFrom().y; nLineNum<=rcSel.GetTo().y; ++nLineNum) {
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
				// Nov. 6, 2002 genta NULL�`�F�b�N�ǉ�
				// ���ꂪ�Ȃ���EOF�s���܂ދ�`�I�𒆂̕�������͂ŗ�����
				LogicInt nIdxFrom, nIdxTo;
				LayoutInt xLayoutFrom, xLayoutTo;
				bool reachEndOfLayout = false;
				if (pLayout) {
					// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
					const struct {
						LayoutInt keta;
						LogicInt* outLogicX;
						LayoutInt* outLayoutX;
					} sortedKetas[] = {
						{ rcSel.GetFrom().x, &nIdxFrom, &xLayoutFrom },
						{ rcSel.GetTo().x, &nIdxTo, &xLayoutTo },
						{ LayoutInt(-1), 0, 0 }
					};
					MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
					for (int i=0; 0<=sortedKetas[i].keta; ++i) {
						for (; !it.end(); it.addDelta()) {
							if (sortedKetas[i].keta == it.getColumn()) {
								break;
							}
							it.scanNext();
							if (sortedKetas[i].keta < it.getColumn() + it.getColumnDelta()) {
								break;
							}
						}
						*sortedKetas[i].outLogicX = it.getIndex();
						*sortedKetas[i].outLayoutX = it.getColumn();
					}
					reachEndOfLayout = it.end();
				}else {
					nIdxFrom = nIdxTo = LogicInt(0);
					xLayoutFrom = xLayoutTo = LayoutInt(0);
					reachEndOfLayout = true;
				}
				const bool emptyLine = ! pLayout || pLayout->GetLengthWithoutEOL() == 0;
				const bool selectionIsOutOfLine = reachEndOfLayout && (
					(pLayout && pLayout->GetLayoutEol() != EolType::None) ? xLayoutFrom == xLayoutTo : xLayoutTo < rcSel.GetFrom().x
				);

				// ���͕����̑}���ʒu
				const LayoutPoint ptInsert(selectionIsOutOfLine ? rcSel.GetFrom().x : xLayoutFrom, nLineNum);

				// TAB��X�y�[�X�C���f���g�̎�
				if (eIndent != IndentType::None) {
					if (emptyLine || selectionIsOutOfLine) {
						continue; // �C���f���g�������C���f���g�Ώۂ����݂��Ȃ�����(���s�����̌����s)�ɑ}�����Ȃ��B
					}
					/*
						���͂��C���f���g�p�̕����̂Ƃ��A��������œ��͕�����}�����Ȃ����Ƃ�
						�C���f���g�𑵂��邱�Ƃ��ł���B
						http://sakura-editor.sourceforge.net/cgi-bin/cyclamen/cyclamen.cgi?log=dev&v=4103
					*/
					if (nIdxFrom == nIdxTo // ��`�I��͈͂̉E�[�܂łɔ͈͂̍��[�ɂ��镶���̖������܂܂�Ă��炸�A
						&& ! selectionIsOutOfLine && pLayout && IsIndentChar(pLayout->GetPtr()[nIdxFrom]) // ���́A�����̊܂܂�Ă��Ȃ��������C���f���g�����ł���A
						&& rcSel.GetFrom().x < rcSel.GetTo().x // ��0��`�I���ł͂Ȃ�(<<�݊����ƃC���f���g�����}���̎g������̂��߂ɏ��O����)�Ƃ��B
					) {
						continue;
					}
 					// �S�p�����̍����̌�����
					if (alignFullWidthChar
						&& (ptInsert.x == rcSel.GetFrom().x || (pLayout && IsIndentChar(pLayout->GetPtr()[nIdxFrom])))
					) {	// �����̍������͈͂ɂ҂�������܂��Ă���
						minOffset = LayoutInt(0);
						continue;
					}
				}

				// ���݈ʒu�Ƀf�[�^��}��
				m_view.InsertData_CEditView(
					ptInsert,
					!bSoftTab? pData: stabData,
					!bSoftTab? nDataLen: stabData.Len(ptInsert.x),
					&ptInserted,
					false
				);
				insertionWasDone = true;
				minOffset = t_min(
					0 <= minOffset ? minOffset : layoutMgr.GetMaxLineKetas(),
					ptInsert.x <= ptInserted.x ? ptInserted.x - ptInsert.x : t_max(LayoutInt(0), layoutMgr.GetMaxLineKetas() - ptInsert.x)
				);

				caret.MoveCursor(ptInserted, false);
				caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

				if (hwndProgress) {
					int newPos = ::MulDiv((Int)nLineNum, 100, (Int)rcSel.GetTo().y);
					if (newPos != nProgressPos) {
						nProgressPos = newPos;
						Progress_SetPos(hwndProgress, newPos + 1);
						Progress_SetPos(hwndProgress, newPos);
					}
				}
			}
			if (insertionWasDone || !alignFullWidthChar) {
				break; // ���[�v�̕K�v�͂Ȃ��B(1.�����̑}�����s��ꂽ����B2.�����ł͂Ȃ��������̑}�����T���������ł͂Ȃ�����)
			}
		}

		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}

		// �}�����ꂽ�����̕������I��͈͂����ɂ��炵�ArcSel�ɃZ�b�g����B
		if (0 < minOffset) {
			rcSel.GetFromPointer()->x = t_min(rcSel.GetFrom().x + minOffset, layoutMgr.GetMaxLineKetas());
			rcSel.GetToPointer()->x = t_min(rcSel.GetTo().x + minOffset, layoutMgr.GetMaxLineKetas());
		}

		// �J�[�\�����ړ�
		caret.MoveCursor(rcSel.GetFrom(), true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

		if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redo�̎��s����
			// ����̒ǉ�
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					caret.GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
		GetSelect().SetFrom(rcSel.GetFrom());	// �͈͑I���J�n�ʒu
		GetSelect().SetTo(rcSel.GetTo());		// �͈͑I���I���ʒu
		selInfo.SetBoxSelect(true);
	}else if (GetSelect().IsLineOne()) {	// �ʏ�I��(1�s��)
		if (eIndent != IndentType::None && !bSoftTab) {
			// ����`�I���ł͂Ȃ��̂� Command_WCHAR ����Ăі߂������悤�Ȃ��Ƃ͂Ȃ�
			Command_WCHAR(pData[0]);	// 1��������
		}else {
			// ����`�I���ł͂Ȃ��̂ł����֗���͎̂��ۂɂ̓\�t�g�^�u�̂Ƃ�����
			m_view.DeleteData(false);
			m_view.InsertData_CEditView(
				caret.GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(caret.GetCaretLayoutPos().GetX2()),
				&ptInserted,
				false
			);
			caret.MoveCursor(ptInserted, true);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}
	}else {	// �ʏ�I��(�����s)
		selectOld.SetFrom(LayoutPoint(LayoutInt(0), GetSelect().GetFrom().y));
		selectOld.SetTo  (LayoutPoint(LayoutInt(0), GetSelect().GetTo().y ));
		if (GetSelect().GetTo().x > 0) {
			selectOld.GetToPointer()->y++;
		}

		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(false);

		WaitCursor waitCursor(
			m_view.GetHwnd(),
			1000 < selectOld.GetTo().GetY2() - selectOld.GetFrom().GetY2()
		);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}

		for (LayoutInt i=selectOld.GetFrom().GetY2(); i<selectOld.GetTo().GetY2(); ++i) {
			LayoutInt nLineCountPrev = layoutMgr.GetLineCount();
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(i);
			if (!pLayout ||						// �e�L�X�g������EOL�̍s�͖���
				pLayout->GetLogicOffset() > 0 ||				// �܂�Ԃ��s�͖���
				pLayout->GetLengthWithoutEOL() == 0
			) {	// ���s�݂̂̍s�͖�������B
				continue;
			}

			// �J�[�\�����ړ�
			caret.MoveCursor(LayoutPoint(LayoutInt(0), i), false);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

			// ���݈ʒu�Ƀf�[�^��}��
			m_view.InsertData_CEditView(
				LayoutPoint(LayoutInt(0), i),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(LayoutInt(0)),
				&ptInserted,
				false
			);
			// �J�[�\�����ړ�
			caret.MoveCursor(ptInserted, false);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

			if (nLineCountPrev != layoutMgr.GetLineCount()) {
				// �s�����ω�����!!
				selectOld.GetToPointer()->y += layoutMgr.GetLineCount() - nLineCountPrev;
			}
			if (hwndProgress) {
				int newPos = ::MulDiv((Int)i, 100, (Int)selectOld.GetTo().GetY());
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}

		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}

		GetSelect() = selectOld;

		// From Here 2001.12.03 hor
		caret.MoveCursor(GetSelect().GetTo(), true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redo�̎��s����
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}
		// To Here 2001.12.03 hor
	}
	// �ĕ`��
	m_view.SetDrawSwitch(bDrawSwitchOld);	// 2002.01.25 hor
	m_view.RedrawAll();			// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
	return;
}


// �t�C���f���g
void ViewCommander::Command_UNINDENT(wchar_t wcChar)
{
	// Aug. 9, 2003 genta
	// �I������Ă��Ȃ��ꍇ�ɋt�C���f���g�����ꍇ��
	// ���Ӄ��b�Z�[�W���o��
	auto& selInfo = m_view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		IndentType eIndent;
		switch (wcChar) {
		case WCODE::TAB:
			eIndent = IndentType::Tab;	// ��[SPACE�̑}��]�I�v�V������ ON �Ȃ�\�t�g�^�u�ɂ���iWiki BugReport/66�j
			break;
		case WCODE::SPACE:
			eIndent = IndentType::Space;
			break;
		default:
			eIndent = IndentType::None;
		}
		Command_INDENT(wcChar, eIndent);
		m_view.SendStatusMessage(LS(STR_ERR_UNINDENT1));
		return;
	}

	// ��`�͈͑I�𒆂�
	if (selInfo.IsBoxSelecting()) {
		ErrorBeep();
//**********************************************
// ���^�t�C���f���g�ɂ��ẮA�ۗ��Ƃ��� (1998.10.22)
//**********************************************
	}else {
		GetDocument()->m_docEditor.SetModified(true, true);	// Jan. 22, 2002 genta

		LayoutRange selectOld;	// �͈͑I��
		selectOld.SetFrom(LayoutPoint(LayoutInt(0), GetSelect().GetFrom().y));
		selectOld.SetTo  (LayoutPoint(LayoutInt(0), GetSelect().GetTo().y ));
		if (GetSelect().GetTo().x > 0) {
			selectOld.GetToPointer()->y++;
		}

		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(false);

		WaitCursor waitCursor(m_view.GetHwnd(), 1000 < selectOld.GetTo().GetY() - selectOld.GetFrom().GetY());
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = m_view.StartProgress();
		}

		auto& caret = GetCaret();
		auto& layoutMgr = GetDocument()->m_layoutMgr;
		LogicInt nDelLen;
		for (LayoutInt i = selectOld.GetFrom().GetY2(); i < selectOld.GetTo().GetY2(); ++i) {
			LayoutInt nLineCountPrev = layoutMgr.GetLineCount();

			const Layout*	pLayout;
			LogicInt		nLineLen;
			const wchar_t*	pLine = layoutMgr.GetLineStr(i, &nLineLen, &pLayout);
			if (!pLayout || pLayout->GetLogicOffset() > 0) { // �܂�Ԃ��ȍ~�̍s�̓C���f���g�������s��Ȃ�
				continue;
			}

			if (wcChar == WCODE::TAB) {
				if (pLine[0] == wcChar) {
					nDelLen = LogicInt(1);
				}else {
					// ����锼�p�X�y�[�X�� (1�`�^�u����) -> nDelLen
					LogicInt i;
					LogicInt nTabSpaces = LogicInt((Int)layoutMgr.GetTabSpace());
					for (i=LogicInt(0); i<nLineLen; ++i) {
						if (pLine[i] != WCODE::SPACE) {
							break;
						}
						// Sep. 23, 2002 genta LayoutMgr�̒l���g��
						if (i >= nTabSpaces) {
							break;
						}
					}
					if (i == 0) {
						continue;
					}
					nDelLen = i;
				}
			}else {
				if (pLine[0] != wcChar) {
					continue;
				}
				nDelLen = LogicInt(1);
			}

			// �J�[�\�����ړ�
			caret.MoveCursor(LayoutPoint(LayoutInt(0), i), false);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
			
			// �w��ʒu�̎w�蒷�f�[�^�폜
			m_view.DeleteData2(
				LayoutPoint(LayoutInt(0), i),
				nDelLen,	// 2001.12.03 hor
				NULL
			);
			if (nLineCountPrev != layoutMgr.GetLineCount()) {
				// �s�����ω�����!!
				selectOld.GetToPointer()->y += layoutMgr.GetLineCount() - nLineCountPrev;
			}
			if (hwndProgress) {
				int newPos = ::MulDiv((Int)i, 100, (Int)selectOld.GetTo().GetY());
				if (newPos != nProgressPos) {
					nProgressPos = newPos;
					Progress_SetPos(hwndProgress, newPos + 1);
					Progress_SetPos(hwndProgress, newPos);
				}
			}
		}
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
		GetSelect() = selectOld;	// �͈͑I��

		// From Here 2001.12.03 hor
		caret.MoveCursor(GetSelect().GetTo(), true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redo�̎��s����
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}
		// To Here 2001.12.03 hor
	}

	// �ĕ`��
	m_view.RedrawAll();	// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
}


// from ViewCommander_New.cpp
/*! TRIM Step1
	��I�����̓J�����g�s��I������ m_view.ConvSelectedArea �� ConvMemory ��
	@author hor
	@date 2001.12.03 hor �V�K�쐬
*/
void ViewCommander::Command_TRIM(
	bool bLeft	//  [in] FALSE: �ETRIM / ����ȊO: ��TRIM
	)
{
	bool bBeDisableSelectArea = false;
	ViewSelect& viewSelect = m_view.GetSelectionInfo();

	if (!viewSelect.IsTextSelected()) {	// ��I�����͍s�I���ɕύX
		viewSelect.m_select.SetFrom(
			LayoutPoint(
				LayoutInt(0),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		viewSelect.m_select.SetTo(
			LayoutPoint(
				GetDocument()->m_layoutMgr.GetMaxLineKetas(),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		bBeDisableSelectArea = true;
	}

	m_view.ConvSelectedArea(bLeft ? F_LTRIM : F_RTRIM);

	if (bBeDisableSelectArea) {
		viewSelect.DisableSelectArea(true);
	}
}


// from ViewCommander_New.cpp
// �����s�̃\�[�g�Ɏg���\����
struct SortData {
	const NativeW* pMemLine;
	StringRef sKey;
};

inline
int CNativeW_comp(
	const NativeW& lhs,
	const NativeW& rhs
	)
{
	// ��r���ɂ͏I�[NUL���܂߂Ȃ��Ƃ����Ȃ�
	return wmemcmp(
		lhs.GetStringPtr(),
		rhs.GetStringPtr(),
		t_min(lhs.GetStringLength() + 1, rhs.GetStringLength() + 1)
	);
}

// �����s�̃\�[�g�Ɏg���֐�(����)
bool SortByLineAsc (SortData* pst1, SortData* pst2) {return CNativeW_comp(*pst1->pMemLine, *pst2->pMemLine) < 0;}

// �����s�̃\�[�g�Ɏg���֐�(�~��)
bool SortByLineDesc(SortData* pst1, SortData* pst2) {return CNativeW_comp(*pst1->pMemLine, *pst2->pMemLine) > 0;}

inline
int CStringRef_comp(
	const StringRef& c1,
	const StringRef& c2
	)
{
	int ret = wmemcmp(
		c1.GetPtr(),
		c2.GetPtr(),
		t_min(c1.GetLength(), c2.GetLength())
	);
	if (ret == 0) {
		return c1.GetLength() - c2.GetLength();
	}
	return ret;
}

// �����s�̃\�[�g�Ɏg���֐�(����)
bool SortByKeyAsc(SortData* pst1, SortData* pst2)  {return CStringRef_comp(pst1->sKey, pst2->sKey) < 0 ;}

// �����s�̃\�[�g�Ɏg���֐�(�~��)
bool SortByKeyDesc(SortData* pst1, SortData* pst2) {return CStringRef_comp(pst1->sKey, pst2->sKey) > 0 ;}

/*!	@brief �����s�̃\�[�g

	��I�����͉������s���Ȃ��D��`�I�����́A���͈̔͂��L�[�ɂ��ĕ����s���\�[�g�D
	
	@note �Ƃ肠�������s�R�[�h���܂ރf�[�^���\�[�g���Ă���̂ŁA
	�t�@�C���̍ŏI�s�̓\�[�g�ΏۊO�ɂ��Ă��܂�
	@author hor
	@date 2001.12.03 hor �V�K�쐬
	@date 2001.12.21 hor �I��͈͂̒������W�b�N�����
	@date 2010.07.27 �s�\�[�g�ŃR�s�[�����炷/NUL��������r�ΏƂ�
	@date 2013.06.19 Moca ��`�I�����ŏI�s�ɉ��s���Ȃ��ꍇ�͕t��+�\�[�g��̍ŏI�s�̉��s���폜
*/
void ViewCommander::Command_SORT(bool bAsc)	// bAsc:true=����, false=�~��
{
	LayoutRange rangeA;
	LogicRange selectOld;

	int			nColumnFrom, nColumnTo;
	LayoutInt	nCF(0), nCT(0);
	LayoutInt	nCaretPosYOLD;
	bool		bBeginBoxSelectOld;
	LogicInt	nLineLen;
	std::vector<SortData*> sta;

	auto& selInfo = m_view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {			// �e�L�X�g���I������Ă��邩
		return;
	}
	auto& layoutMgr = GetDocument()->m_layoutMgr;
	if (selInfo.IsBoxSelecting()) {
		rangeA = selInfo.m_select;
		if (selInfo.m_select.GetFrom().x == selInfo.m_select.GetTo().x) {
			// Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
			selInfo.m_select.SetToX(layoutMgr.GetMaxLineKetas());
		}
		if (selInfo.m_select.GetFrom().x<selInfo.m_select.GetTo().x) {
			nCF = selInfo.m_select.GetFrom().GetX2();
			nCT = selInfo.m_select.GetTo().GetX2();
		}else {
			nCF = selInfo.m_select.GetTo().GetX2();
			nCT = selInfo.m_select.GetFrom().GetX2();
		}
	}
	bBeginBoxSelectOld = selInfo.IsBoxSelecting();
	nCaretPosYOLD = GetCaret().GetCaretLayoutPos().GetY();
	layoutMgr.LayoutToLogic(
		selInfo.m_select,
		&selectOld
	);

	if (bBeginBoxSelectOld) {
		selectOld.GetToPointer()->y++;
	}else {
		// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
		// ���̍s���I��͈͂ɉ�����
		if (selectOld.GetTo().x > 0) {
			// 2006.03.31 Moca nSelectLineToOld�́A�����s�Ȃ̂�Layout�n����DocLine�n�ɏC��
			const DocLine* pDocLine = GetDocument()->m_docLineMgr.GetLine(selectOld.GetTo().GetY2());
			if (pDocLine && EolType::None != pDocLine->GetEol()) {
				selectOld.GetToPointer()->y++;
			}
		}
	}
	selectOld.SetFromX(LogicInt(0));
	selectOld.SetToX(LogicInt(0));

	// �s�I������ĂȂ�
	if (selectOld.IsLineOne()) {
		return;
	}
	
	sta.reserve(selectOld.GetTo().GetY2() - selectOld.GetFrom().GetY2());
	for (LogicInt i=selectOld.GetFrom().GetY2(); i<selectOld.GetTo().y; ++i) {
		const DocLine* pDocLine = GetDocument()->m_docLineMgr.GetLine(i);
		const NativeW& memLine = pDocLine->_GetDocLineDataWithEOL();
		const wchar_t* pLine = memLine.GetStringPtr(&nLineLen);
		LogicInt nLineLenWithoutEOL = pDocLine->GetLengthWithoutEOL();
		if (!pLine) {
			continue;
		}
		SortData* pst = new SortData;
		if (bBeginBoxSelectOld) {
			nColumnFrom = m_view.LineColumnToIndex(pDocLine, nCF);
			nColumnTo   = m_view.LineColumnToIndex(pDocLine, nCT);
			if (nColumnTo < nLineLenWithoutEOL) {	// BOX�I��͈͂̉E�[���s���Ɏ��܂��Ă���ꍇ
				// 2006.03.31 genta std::string::assign���g���Ĉꎞ�ϐ��폜
				pst->sKey = StringRef(&pLine[nColumnFrom], nColumnTo - nColumnFrom);
			}else if (nColumnFrom < nLineLenWithoutEOL) {	// BOX�I��͈͂̉E�[���s�����E�ɂ͂ݏo���Ă���ꍇ
				pst->sKey = StringRef(&pLine[nColumnFrom], nLineLenWithoutEOL - nColumnFrom);
			}else {
				// �I��͈͂̍��[���͂ݏo���Ă��� == �f�[�^�Ȃ�
				pst->sKey = StringRef(L"", 0);
			}
		}
		pst->pMemLine = &memLine;
		sta.push_back(pst);
	}
	const wchar_t* pStrLast = NULL; // �Ō�̍s�ɉ��s���Ȃ���΂��̃|�C���^
	if (0 < sta.size()) {
		pStrLast = sta[sta.size() - 1]->pMemLine->GetStringPtr();
		int nlen = sta[sta.size() - 1]->pMemLine->GetStringLength();
		if (0 < nlen) {
			if (WCODE::IsLineDelimiter(pStrLast[nlen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
				pStrLast = NULL;
			}
		}
	}
	if (bBeginBoxSelectOld) {
		std::stable_sort(
			sta.begin(),
			sta.end(),
			(bBeginBoxSelectOld ? (bAsc ? SortByKeyAsc : SortByKeyDesc) : (bAsc ? SortByLineAsc : SortByLineDesc))
		);
	}
	OpeLineData repData;
	int j = (int)sta.size();
	repData.resize(sta.size());
	int opeSeq = GetDocument()->m_docEditor.m_opeBuf.GetNextSeq();
	for (int i=0; i<j; ++i) {
		repData[i].nSeq = opeSeq;
		repData[i].memLine.SetString(sta[i]->pMemLine->GetStringPtr(), sta[i]->pMemLine->GetStringLength());
		if (pStrLast == sta[i]->pMemLine->GetStringPtr()) {
			// ���ŏI�s�ɉ��s���Ȃ��̂ł���
			Eol cWork = GetDocument()->m_docEditor.GetNewLineCode();
			repData[i].memLine.AppendString(cWork.GetValue2(), cWork.GetLen());
		}
	}
	if (pStrLast) {
		// �ŏI�s�̉��s���폜
		LineData& lastData = repData[repData.size() - 1];
		int nLen = lastData.memLine.GetStringLength();
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		while (0 <nLen && WCODE::IsLineDelimiter(lastData.memLine[nLen-1], bExtEol)) {
			--nLen;
		}
		lastData.memLine._SetStringLength(nLen);
	}
	// 2010.08.22 Moca swap�ō폜
	{
		std::vector<SortData*> temp;
		temp.swap(sta);
	}

	LayoutRange selectOld_Layout;
	layoutMgr.LogicToLayout(selectOld, &selectOld_Layout);
	m_view.ReplaceData_CEditView3(
		selectOld_Layout,
		NULL,
		&repData,
		false,
		m_view.m_bDoing_UndoRedo ? NULL : GetOpeBlk(),
		opeSeq,
		NULL
	);

	// �I���G���A�̕���
	if (bBeginBoxSelectOld) {
		selInfo.SetBoxSelect(bBeginBoxSelectOld);
		selInfo.m_select = rangeA;
	}else {
		selInfo.m_select = selectOld_Layout;
	}
	if (nCaretPosYOLD == selInfo.m_select.GetFrom().y || selInfo.IsBoxSelecting()) {
		GetCaret().MoveCursor(selInfo.m_select.GetFrom(), true);
	}else {
		GetCaret().MoveCursor(selInfo.m_select.GetTo(), true);
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}
	m_view.RedrawAll();
}


// from ViewCommander_New.cpp
/*! @brief �����s�̃}�[�W

	�A�����镨���s�œ��e������̕���1�s�ɂ܂Ƃ߂܂��D
	
	��`�I�����͂Ȃɂ����s���܂���D
	
	@note ���s�R�[�h���܂ރf�[�^���r���Ă���̂ŁA
	�t�@�C���̍ŏI�s�̓\�[�g�ΏۊO�ɂ��Ă��܂�
	
	@author hor
	@date 2001.12.03 hor �V�K�쐬
	@date 2001.12.21 hor �I��͈͂̒������W�b�N�����
*/
void ViewCommander::Command_MERGE(void)
{
	LayoutInt	nCaretPosYOLD;
	LogicInt	nLineLen;
	LayoutInt	nMergeLayoutLines;

	auto& selInfo = m_view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {			// �e�L�X�g���I������Ă��邩
		return;
	}
	if (selInfo.IsBoxSelecting()) {
		return;
	}
	auto& layoutMgr = GetDocument()->m_layoutMgr;
	nCaretPosYOLD = GetCaret().GetCaretLayoutPos().GetY();
	LogicRange sSelectOld; // �͈͑I��
	layoutMgr.LayoutToLogic(
		selInfo.m_select,
		&sSelectOld
	);

	// 2001.12.21 hor
	// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
	// ���̍s���I��͈͂ɉ�����
	if (sSelectOld.GetTo().x > 0) {
#if 0
		const Layout* pLayout = layoutMgr.SearchLineByLayoutY(selInfo.m_select.GetTo().GetY2()); // 2007.10.09 kobake �P�ʍ��݃o�O�C��
		if (pLayout && EolType::None != pLayout->GetLayoutEol()) {
			selectOld.GetToPointer()->y++;
			//selectOld.GetTo().y++;
		}
#else
		// 2010.08.22 Moca �\�[�g�Ǝd�l�����킹��
		const DocLine* pDocLine = GetDocument()->m_docLineMgr.GetLine(sSelectOld.GetTo().GetY2());
		if (pDocLine && EolType::None != pDocLine->GetEol()) {
			sSelectOld.GetToPointer()->y++;
		}
#endif
	}

	sSelectOld.SetFromX(LogicInt(0));
	sSelectOld.SetToX(LogicInt(0));

	// �s�I������ĂȂ�
	if (sSelectOld.IsLineOne()) {
		return;
	}

	int j = GetDocument()->m_docLineMgr.GetLineCount();
	nMergeLayoutLines = layoutMgr.GetLineCount();

	LayoutRange selectOld_Layout;
	layoutMgr.LogicToLayout(sSelectOld, &selectOld_Layout);

	// 2010.08.22 NUL�Ή��C��
	std::vector<StringRef> lineArr;
	const wchar_t* pLinew = NULL;
	int nLineLenw = 0;
	bool bMerge = false;
	lineArr.reserve(sSelectOld.GetTo().y - sSelectOld.GetFrom().GetY2());
	for (LogicInt i=sSelectOld.GetFrom().GetY2(); i<sSelectOld.GetTo().y; ++i) {
		const wchar_t* pLine = GetDocument()->m_docLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
		if (!pLine) continue;
		if (!pLinew || nLineLen != nLineLenw || wmemcmp(pLine, pLinew, nLineLen)) {
			lineArr.emplace_back(pLine, nLineLen);
		}else {
			bMerge = true;
		}
		pLinew = pLine;
		nLineLenw = nLineLen;
	}
	if (bMerge) {
		OpeLineData repData;
		int nSize = (int)lineArr.size();
		repData.resize(nSize);
		int opeSeq = GetDocument()->m_docEditor.m_opeBuf.GetNextSeq();
		for (int idx=0; idx<nSize; ++idx) {
			repData[idx].nSeq = opeSeq;
			repData[idx].memLine.SetString(lineArr[idx].GetPtr(), lineArr[idx].GetLength());
		}
		m_view.ReplaceData_CEditView3(
			selectOld_Layout,
			NULL,
			&repData,
			false,
			m_view.m_bDoing_UndoRedo ? NULL : GetOpeBlk(),
			opeSeq,
			NULL
		);
	}else {
		// 2010.08.23 ���ύX�Ȃ�ύX���Ȃ�
	}

	j -= GetDocument()->m_docLineMgr.GetLineCount();
	nMergeLayoutLines -= layoutMgr.GetLineCount();

	// �I���G���A�̕���
	selInfo.m_select = selectOld_Layout;
	// 2010.08.22 ���W���݃o�O
	selInfo.m_select.GetToPointer()->y -= nMergeLayoutLines;

	if (nCaretPosYOLD == selInfo.m_select.GetFrom().y) {
		GetCaret().MoveCursor(selInfo.m_select.GetFrom(), true);
	}else {
		GetCaret().MoveCursor(selInfo.m_select.GetTo(), true);
	}
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();
	if (!m_view.m_bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}
	m_view.RedrawAll();

	if (j) {
		TopOkMessage(m_view.GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW7), j);
	}else {
		InfoMessage(m_view.GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW8));
	}
}


// from ViewCommander_New.cpp
/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09

	@date 2002.04.11 YAZAKI COsVersionInfo�̃J�v�Z���������܂��傤�B
	@date 2010.03.17 ATOK�p��SCS_SETRECONVERTSTRING => ATRECONVERTSTRING_SET�ɕύX
		2002.11.20 Stonee����̏��
*/
void ViewCommander::Command_Reconvert(void)
{
	const int ATRECONVERTSTRING_SET = 1;

	// �T�C�Y���擾
	int nSize = m_view.SetReconvertStruct(NULL, UNICODE_BOOL);
	if (nSize == 0)  // �T�C�Y�O�̎��͉������Ȃ�
		return ;

	bool bUseUnicodeATOK = false;
	// �o�[�W�����`�F�b�N
	if (!OsSupportReconvert()) {
		
		// MSIME���ǂ���
		HWND hWnd = ImmGetDefaultIMEWnd(m_view.GetHwnd());
		if (SendMessage(hWnd, m_view.m_uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)) {
			SendMessage(hWnd, m_view.m_uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)m_view.GetHwnd());
			return ;
		}

		// ATOK���g���邩�ǂ���
		TCHAR sz[256];
		ImmGetDescription(GetKeyboardLayout(0), sz, _countof(sz)); // �����̎擾
		if ((_tcsncmp(sz, _T("ATOK"),4) == 0) && m_view.m_AT_ImmSetReconvertString) {
			bUseUnicodeATOK = true;
		}else {
			// �Ή�IME�Ȃ�
			return;
		}
	}else {
		// ���݂�IME���Ή����Ă��邩�ǂ���
		// IME�̃v���p�e�B
		if (!(ImmGetProperty(GetKeyboardLayout(0), IGP_SETCOMPSTR) & SCS_CAP_SETRECONVERTSTRING)) {
			// �Ή�IME�Ȃ�
			return ;
		}
	}

	// �T�C�Y�擾������
	if (!UNICODE_BOOL && bUseUnicodeATOK) {
		nSize = m_view.SetReconvertStruct(NULL, UNICODE_BOOL || bUseUnicodeATOK);
		if (nSize == 0)  // �T�C�Y�O�̎��͉������Ȃ�
			return;
	}

	// IME�̃R���e�L�X�g�擾
	HIMC hIMC = ::ImmGetContext(m_view.GetHwnd());
	
	// �̈�m��
	PRECONVERTSTRING pReconv = (PRECONVERTSTRING)::HeapAlloc(
		GetProcessHeap(),
		HEAP_GENERATE_EXCEPTIONS,
		nSize
	);
	
	// �\���̐ݒ�
	// Size�̓o�b�t�@�m�ۑ����ݒ�
	pReconv->dwSize = nSize;
	pReconv->dwVersion = 0;
	m_view.SetReconvertStruct(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	// �ϊ��͈͂̒���
	if (bUseUnicodeATOK) {
		(*m_view.m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else {
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	// ���������ϊ��͈͂�I������
	m_view.SetSelectionFromReonvert(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	// �ĕϊ����s
	if (bUseUnicodeATOK) {
		(*m_view.m_AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else {
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	// �̈���
	::HeapFree(GetProcessHeap(), 0, (LPVOID)pReconv);
	::ImmReleaseContext(m_view.GetHwnd(), hIMC);
}

