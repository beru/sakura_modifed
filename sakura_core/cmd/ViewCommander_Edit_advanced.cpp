#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "uiparts/WaitCursor.h"
#include "mem/MemoryIterator.h"
#include "_os/OsVersionInfo.h"

// ViewCommander�N���X�̃R�}���h(�ҏW�n ���x�ȑ���(���P��/�s����))�֐��Q

using namespace std;

#ifndef FID_RECONVERT_VERSION
#define FID_RECONVERT_VERSION 0x10000000
#endif
#ifndef SCS_CAP_SETRECONVERTSTRING
#define SCS_CAP_SETRECONVERTSTRING 0x00000004
#define SCS_QUERYRECONVERTSTRING 0x00020000
#define SCS_SETRECONVERTSTRING 0x00010000
#endif

// �C���f���g ver1
void ViewCommander::Command_Indent(wchar_t wcChar, IndentType eIndent)
{
	using namespace WCODE;

	auto& selInfo = view.GetSelectionInfo();
#if 1	// ���������c���ΑI�𕝃[�����ő�ɂ���i�]���݊������j�B�����Ă� Command_Indent() ver0 ���K�؂ɓ��삷��悤�ɕύX���ꂽ�̂ŁA�폜���Ă����ɕs�s���ɂ͂Ȃ�Ȃ��B
	// SPACEorTAB�C�����f���g�ŋ�`�I�������[���̎��͑I��͈͂��ő�ɂ���
	if (eIndent != IndentType::None
		&& selInfo.IsBoxSelecting()
		&& GetSelect().GetFrom().x == GetSelect().GetTo().x
	) {
		GetSelect().SetToX((int)GetDocument().layoutMgr.GetMaxLineKetas());
		view.RedrawAll();
		return;
	}
#endif
	Command_Indent(&wcChar, 1, eIndent);
	return;
}


// �C���f���g ver0
/*
	�I�����ꂽ�e�s�͈̔͂̒��O�ɁA�^����ꂽ������(pData)��}������B
	@param eIndent �C���f���g�̎��
*/
void ViewCommander::Command_Indent(
	const wchar_t* const pData,
	const size_t nDataLen,
	IndentType eIndent
	)
{
	if (nDataLen == 0) {
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	Range selectOld;		// �͈͑I��
	Point ptInserted;		// �}����̑}���ʒu
	const struct IsIndentCharSpaceTab {
		IsIndentCharSpaceTab() {}
		bool operator()(const wchar_t ch) const
		{ return ch == WCODE::SPACE || ch == WCODE::TAB; }
	} IsIndentChar;
	struct SoftTabData {
		SoftTabData(size_t nTab) : szTab(NULL), nTab(nTab) {}
		~SoftTabData() { delete[] szTab; }
		operator const wchar_t* ()
		{
			if (!szTab) {
				szTab = new wchar_t[nTab];
				wmemset(szTab, WCODE::SPACE, nTab);
			}
			return szTab;
		}
		size_t Len(size_t nCol) { return nTab - (nCol % nTab); }
		wchar_t* szTab;
		size_t nTab;
	} stabData(layoutMgr.GetTabSpace());

	const bool bSoftTab = (eIndent == IndentType::Tab && view.pTypeData->bInsSpace);
	GetDocument().docEditor.SetModified(true, true);

	auto& caret = GetCaret();
	auto& selInfo = view.GetSelectionInfo();

	if (!selInfo.IsTextSelected()) {			// �e�L�X�g���I������Ă��邩
		if (eIndent != IndentType::None && !bSoftTab) {
			// ����`�I���ł͂Ȃ��̂� Command_WCHAR ����Ăі߂������悤�Ȃ��Ƃ͂Ȃ�
			Command_WCHAR(pData[0]);	// 1��������
		}else {
			// ����`�I���ł͂Ȃ��̂ł����֗���͎̂��ۂɂ̓\�t�g�^�u�̂Ƃ�����
			if (bSoftTab && !view.IsInsMode()) {
				DelCharForOverwrite(pData, nDataLen);
			}
			view.InsertData_CEditView(
				caret.GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(caret.GetCaretLayoutPos().x),
				&ptInserted,
				true
			);
			caret.MoveCursor(ptInserted, true);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
		return;
	}
	const bool bDrawSwitchOld = view.SetDrawSwitch(false);
	// ��`�͈͑I�𒆂�
	if (selInfo.IsBoxSelecting()) {

		// 2�_��Ίp�Ƃ����`�����߂�
		Range rcSel;
		TwoPointToRange(
			&rcSel,
			GetSelect().GetFrom(),	// �͈͑I���J�n
			GetSelect().GetTo()		// �͈͑I���I��
		);
		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(false);

		/*
			�����𒼑O�ɑ}�����ꂽ�������A����ɂ�茳�̈ʒu����ǂꂾ�����ɂ��ꂽ���B
			����ɏ]����`�I��͈͂����ɂ��炷�B
		*/
		int minOffset = -1;
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
		WaitCursor waitCursor(view.GetHwnd(), 1000 < rcSel.GetTo().y - rcSel.GetFrom().y);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}
		for (bool insertionWasDone=false; ; alignFullWidthChar=false) {
			minOffset = -1;
			for (int nLineNum=rcSel.GetFrom().y; nLineNum<=rcSel.GetTo().y; ++nLineNum) {
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
				// ���ꂪ�Ȃ���EOF�s���܂ދ�`�I�𒆂̕�������͂ŗ�����
				int nIdxFrom, nIdxTo;
				int xLayoutFrom, xLayoutTo;
				bool reachEndOfLayout = false;
				if (pLayout) {
					// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
					const struct {
						int keta;
						int* outLogicX;
						int* outLayoutX;
					} sortedKetas[] = {
						{ rcSel.GetFrom().x, &nIdxFrom, &xLayoutFrom },
						{ rcSel.GetTo().x, &nIdxTo, &xLayoutTo },
						{ -1, 0, 0 }
					};
					MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
					for (int i=0; 0<=sortedKetas[i].keta; ++i) {
						for (; !it.end(); it.addDelta()) {
							if (sortedKetas[i].keta == it.getColumn()) {
								break;
							}
							it.scanNext();
							if (sortedKetas[i].keta < (int)(it.getColumn() + it.getColumnDelta())) {
								break;
							}
						}
						*sortedKetas[i].outLogicX = (int)it.getIndex();
						*sortedKetas[i].outLayoutX = (int)it.getColumn();
					}
					reachEndOfLayout = it.end();
				}else {
					nIdxFrom = nIdxTo = 0;
					xLayoutFrom = xLayoutTo = 0;
					reachEndOfLayout = true;
				}
				const bool emptyLine = ! pLayout || pLayout->GetLengthWithoutEOL() == 0;
				const bool selectionIsOutOfLine = reachEndOfLayout && (
					(pLayout && pLayout->GetLayoutEol() != EolType::None) ? xLayoutFrom == xLayoutTo : xLayoutTo < rcSel.GetFrom().x
				);

				// ���͕����̑}���ʒu
				const Point ptInsert(selectionIsOutOfLine ? rcSel.GetFrom().x : xLayoutFrom, nLineNum);

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
						minOffset = 0;
						continue;
					}
				}

				// ���݈ʒu�Ƀf�[�^��}��
				view.InsertData_CEditView(
					ptInsert,
					!bSoftTab? pData: stabData,
					!bSoftTab? nDataLen: stabData.Len(ptInsert.x),
					&ptInserted,
					false
				);
				insertionWasDone = true;
				minOffset = t_min(
					(0 <= minOffset) ? minOffset : (int)layoutMgr.GetMaxLineKetas(),
					(ptInsert.x <= ptInserted.x) ? (int)(ptInserted.x - ptInsert.x) : t_max(0, (int)layoutMgr.GetMaxLineKetas() - (int)ptInsert.x)
				);

				caret.MoveCursor(ptInserted, false);
				caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

				if (hwndProgress) {
					int newPos = ::MulDiv(nLineNum, 100, rcSel.GetTo().y);
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
			rcSel.GetFrom().x = t_min((int)rcSel.GetFrom().x + minOffset, (int)layoutMgr.GetMaxLineKetas());
			rcSel.GetTo().x = t_min((int)rcSel.GetTo().x + minOffset, (int)layoutMgr.GetMaxLineKetas());
		}

		// �J�[�\�����ړ�
		caret.MoveCursor(rcSel.GetFrom(), true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

		if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
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
			view.DeleteData(false);
			view.InsertData_CEditView(
				caret.GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(caret.GetCaretLayoutPos().x),
				&ptInserted,
				false
			);
			caret.MoveCursor(ptInserted, true);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
	}else {	// �ʏ�I��(�����s)
		selectOld.SetFrom(Point(0, GetSelect().GetFrom().y));
		selectOld.SetTo  (Point(0, GetSelect().GetTo().y ));
		if (GetSelect().GetTo().x > 0) {
			selectOld.GetTo().y++;
		}

		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(false);

		WaitCursor waitCursor(
			view.GetHwnd(),
			1000 < selectOld.GetTo().y - selectOld.GetFrom().y
		);
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}

		for (int i=selectOld.GetFrom().y; i<selectOld.GetTo().y; ++i) {
			size_t nLineCountPrev = layoutMgr.GetLineCount();
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(i);
			if (!pLayout ||						// �e�L�X�g������EOL�̍s�͖���
				pLayout->GetLogicOffset() > 0 ||				// �܂�Ԃ��s�͖���
				pLayout->GetLengthWithoutEOL() == 0
			) {	// ���s�݂̂̍s�͖�������B
				continue;
			}

			// �J�[�\�����ړ�
			caret.MoveCursor(Point(0, i), false);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

			// ���݈ʒu�Ƀf�[�^��}��
			view.InsertData_CEditView(
				Point(0, i),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(0),
				&ptInserted,
				false
			);
			// �J�[�\�����ړ�
			caret.MoveCursor(ptInserted, false);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

			if (nLineCountPrev != layoutMgr.GetLineCount()) {
				// �s�����ω�����!!
				selectOld.GetTo().y += (int)layoutMgr.GetLineCount() - (int)nLineCountPrev;
			}
			if (hwndProgress) {
				int newPos = ::MulDiv(i, 100, selectOld.GetTo().GetY());
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

		caret.MoveCursor(GetSelect().GetTo(), true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}
	}
	// �ĕ`��
	view.SetDrawSwitch(bDrawSwitchOld);
	view.RedrawAll();
	return;
}


// �t�C���f���g
void ViewCommander::Command_Unindent(wchar_t wcChar)
{
	// �I������Ă��Ȃ��ꍇ�ɋt�C���f���g�����ꍇ��
	// ���Ӄ��b�Z�[�W���o��
	auto& selInfo = view.GetSelectionInfo();
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
		Command_Indent(wcChar, eIndent);
		view.SendStatusMessage(LS(STR_ERR_UNINDENT1));
		return;
	}

	// ��`�͈͑I�𒆂�
	if (selInfo.IsBoxSelecting()) {
		ErrorBeep();
//**********************************************
// ���^�t�C���f���g�ɂ��ẮA�ۗ��Ƃ���
//**********************************************
	}else {
		GetDocument().docEditor.SetModified(true, true);

		Range selectOld;	// �͈͑I��
		selectOld.SetFrom(Point(0, GetSelect().GetFrom().y));
		selectOld.SetTo  (Point(0, GetSelect().GetTo().y ));
		if (GetSelect().GetTo().x > 0) {
			selectOld.GetTo().y++;
		}

		// ���݂̑I��͈͂��I����Ԃɖ߂�
		selInfo.DisableSelectArea(false);

		WaitCursor waitCursor(view.GetHwnd(), 1000 < selectOld.GetTo().GetY() - selectOld.GetFrom().GetY());
		HWND hwndProgress = NULL;
		int nProgressPos = 0;
		if (waitCursor.IsEnable()) {
			hwndProgress = view.StartProgress();
		}

		auto& caret = GetCaret();
		auto& layoutMgr = GetDocument().layoutMgr;
		size_t nDelLen;
		for (int i = selectOld.GetFrom().y; i < selectOld.GetTo().y; ++i) {
			size_t nLineCountPrev = layoutMgr.GetLineCount();

			const Layout*	pLayout;
			size_t nLineLen;
			const wchar_t*	pLine = layoutMgr.GetLineStr(i, &nLineLen, &pLayout);
			if (!pLayout || pLayout->GetLogicOffset() > 0) { // �܂�Ԃ��ȍ~�̍s�̓C���f���g�������s��Ȃ�
				continue;
			}

			if (wcChar == WCODE::TAB) {
				if (pLine[0] == wcChar) {
					nDelLen = 1;
				}else {
					// ����锼�p�X�y�[�X�� (1�`�^�u����) -> nDelLen
					size_t i;
					size_t nTabSpaces = layoutMgr.GetTabSpace();
					for (i=0; i<nLineLen; ++i) {
						if (pLine[i] != WCODE::SPACE) {
							break;
						}
						// LayoutMgr�̒l���g��
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
				nDelLen = 1;
			}

			// �J�[�\�����ړ�
			caret.MoveCursor(Point(0, i), false);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
			
			// �w��ʒu�̎w�蒷�f�[�^�폜
			view.DeleteData2(
				Point(0, i),
				nDelLen,
				nullptr
			);
			if (nLineCountPrev != layoutMgr.GetLineCount()) {
				// �s�����ω�����!!
				selectOld.GetTo().y += (int)layoutMgr.GetLineCount() - (int)nLineCountPrev;
			}
			if (hwndProgress) {
				int newPos = ::MulDiv(i, 100, selectOld.GetTo().GetY());
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

		caret.MoveCursor(GetSelect().GetTo(), true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}
	}

	// �ĕ`��
	view.RedrawAll();
}


// from ViewCommander_New.cpp
/*! TRIM Step1
	��I�����̓J�����g�s��I������ view.ConvSelectedArea �� ConvMemory ��
*/
void ViewCommander::Command_Trim(
	bool bLeft	//  [in] false: �ETRIM / ����ȊO: ��TRIM
	)
{
	bool bBeDisableSelectArea = false;
	ViewSelect& viewSelect = view.GetSelectionInfo();

	if (!viewSelect.IsTextSelected()) {	// ��I�����͍s�I���ɕύX
		viewSelect.select.SetFrom(
			Point(
				0,
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		viewSelect.select.SetTo(
			Point(
				(int)GetDocument().layoutMgr.GetMaxLineKetas(),
				GetCaret().GetCaretLayoutPos().GetY()
			)
		);
		bBeDisableSelectArea = true;
	}

	view.ConvSelectedArea(bLeft ? F_LTRIM : F_RTRIM);

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
int64_t CStringRef_comp(
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
		return (int64_t)c1.GetLength() - (int64_t)c2.GetLength();
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
*/
void ViewCommander::Command_Sort(bool bAsc)	// bAsc:true=����, false=�~��
{
	Range rangeA;
	Range selectOld;

	size_t nColumnFrom, nColumnTo;
	int	nCF(0), nCT(0);
	int	nCaretPosYOLD;
	bool bBeginBoxSelectOld;
	size_t nLineLen;
	std::vector<SortData*> sta;

	auto& selInfo = view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {			// �e�L�X�g���I������Ă��邩
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	if (selInfo.IsBoxSelecting()) {
		rangeA = selInfo.select;
		if (selInfo.select.GetFrom().x == selInfo.select.GetTo().x) {
			selInfo.select.SetToX((int)layoutMgr.GetMaxLineKetas());
		}
		if (selInfo.select.GetFrom().x<selInfo.select.GetTo().x) {
			nCF = selInfo.select.GetFrom().x;
			nCT = selInfo.select.GetTo().x;
		}else {
			nCF = selInfo.select.GetTo().x;
			nCT = selInfo.select.GetFrom().x;
		}
	}
	bBeginBoxSelectOld = selInfo.IsBoxSelecting();
	auto& caret = GetCaret();
	nCaretPosYOLD = caret.GetCaretLayoutPos().GetY();
	layoutMgr.LayoutToLogic(
		selInfo.select,
		&selectOld
	);

	if (bBeginBoxSelectOld) {
		selectOld.GetTo().y++;
	}else {
		// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
		// ���̍s���I��͈͂ɉ�����
		if (selectOld.GetTo().x > 0) {
			const DocLine* pDocLine = GetDocument().docLineMgr.GetLine(selectOld.GetTo().y);
			if (pDocLine && EolType::None != pDocLine->GetEol()) {
				selectOld.GetTo().y++;
			}
		}
	}
	selectOld.SetFromX(0);
	selectOld.SetToX(0);

	// �s�I������ĂȂ�
	if (selectOld.IsLineOne()) {
		return;
	}
	
	sta.reserve(selectOld.GetTo().y - selectOld.GetFrom().y);
	for (int i=selectOld.GetFrom().y; i<selectOld.GetTo().y; ++i) {
		const DocLine* pDocLine = GetDocument().docLineMgr.GetLine(i);
		const NativeW& memLine = pDocLine->_GetDocLineDataWithEOL();
		const wchar_t* pLine = memLine.GetStringPtr(&nLineLen);
		size_t nLineLenWithoutEOL = pDocLine->GetLengthWithoutEOL();
		if (!pLine) {
			continue;
		}
		SortData* pst = new SortData;
		if (bBeginBoxSelectOld) {
			nColumnFrom = view.LineColumnToIndex(pDocLine, nCF);
			nColumnTo = view.LineColumnToIndex(pDocLine, nCT);
			if (nColumnTo < nLineLenWithoutEOL) {	// BOX�I��͈͂̉E�[���s���Ɏ��܂��Ă���ꍇ
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
		size_t nlen = sta[sta.size() - 1]->pMemLine->GetStringLength();
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
	repData.resize(sta.size());
	int opeSeq = GetDocument().docEditor.opeBuf.GetNextSeq();
	size_t sz = sta.size();
	for (size_t i=0; i<sz; ++i) {
		repData[i].nSeq = opeSeq;
		repData[i].memLine.SetString(sta[i]->pMemLine->GetStringPtr(), sta[i]->pMemLine->GetStringLength());
		if (pStrLast == sta[i]->pMemLine->GetStringPtr()) {
			// ���ŏI�s�ɉ��s���Ȃ��̂ł���
			Eol cWork = GetDocument().docEditor.GetNewLineCode();
			repData[i].memLine.AppendString(cWork.GetValue2(), cWork.GetLen());
		}
	}
	if (pStrLast) {
		// �ŏI�s�̉��s���폜
		LineData& lastData = repData[repData.size() - 1];
		size_t nLen = lastData.memLine.GetStringLength();
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		while (0 <nLen && WCODE::IsLineDelimiter(lastData.memLine[nLen-1], bExtEol)) {
			--nLen;
		}
		lastData.memLine._SetStringLength(nLen);
	}
	// swap�ō폜
	{
		std::vector<SortData*> temp;
		temp.swap(sta);
	}

	Range selectOld_Layout;
	layoutMgr.LogicToLayout(selectOld, &selectOld_Layout);
	view.ReplaceData_CEditView3(
		selectOld_Layout,
		nullptr,
		&repData,
		false,
		view.bDoing_UndoRedo ? nullptr : GetOpeBlk(),
		opeSeq,
		nullptr
	);

	// �I���G���A�̕���
	if (bBeginBoxSelectOld) {
		selInfo.SetBoxSelect(bBeginBoxSelectOld);
		selInfo.select = rangeA;
	}else {
		selInfo.select = selectOld_Layout;
	}
	if (nCaretPosYOLD == selInfo.select.GetFrom().y || selInfo.IsBoxSelecting()) {
		caret.MoveCursor(selInfo.select.GetFrom(), true);
	}else {
		caret.MoveCursor(selInfo.select.GetTo(), true);
	}
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
	if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}
	view.RedrawAll();
}


// from ViewCommander_New.cpp
/*! @brief �����s�̃}�[�W

	�A�����镨���s�œ��e������̕���1�s�ɂ܂Ƃ߂܂��D
	
	��`�I�����͂Ȃɂ����s���܂���D
	
	@note ���s�R�[�h���܂ރf�[�^���r���Ă���̂ŁA
	�t�@�C���̍ŏI�s�̓\�[�g�ΏۊO�ɂ��Ă��܂�
*/
void ViewCommander::Command_Merge(void)
{
	int	nCaretPosYOLD;
	size_t	nLineLen;

	auto& selInfo = view.GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {			// �e�L�X�g���I������Ă��邩
		return;
	}
	if (selInfo.IsBoxSelecting()) {
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	nCaretPosYOLD = caret.GetCaretLayoutPos().GetY();
	Range sSelectOld; // �͈͑I��
	layoutMgr.LayoutToLogic(
		selInfo.select,
		&sSelectOld
	);

	// �J�[�\���ʒu���s������Ȃ� �� �I��͈͂̏I�[�ɉ��s�R�[�h������ꍇ��
	// ���̍s���I��͈͂ɉ�����
	if (sSelectOld.GetTo().x > 0) {
#if 0
		const Layout* pLayout = layoutMgr.SearchLineByLayoutY(selInfo.select.GetTo().y);
		if (pLayout && EolType::None != pLayout->GetLayoutEol()) {
			selectOld.GetToPointer()->y++;
			//selectOld.GetTo().y++;
		}
#else
		// �\�[�g�Ǝd�l�����킹��
		const DocLine* pDocLine = GetDocument().docLineMgr.GetLine(sSelectOld.GetTo().y);
		if (pDocLine && EolType::None != pDocLine->GetEol()) {
			sSelectOld.GetTo().y++;
		}
#endif
	}

	sSelectOld.SetFromX(0);
	sSelectOld.SetToX(0);

	// �s�I������ĂȂ�
	if (sSelectOld.IsLineOne()) {
		return;
	}

	size_t j = GetDocument().docLineMgr.GetLineCount();
	size_t nMergeLayoutLines = layoutMgr.GetLineCount();

	Range selectOld_Layout;
	layoutMgr.LogicToLayout(sSelectOld, &selectOld_Layout);

	// NUL�Ή��C��
	std::vector<StringRef> lineArr;
	const wchar_t* pLinew = NULL;
	size_t nLineLenw = 0;
	bool bMerge = false;
	lineArr.reserve(sSelectOld.GetTo().y - sSelectOld.GetFrom().y);
	for (int i=sSelectOld.GetFrom().y; i<sSelectOld.GetTo().y; ++i) {
		const wchar_t* pLine = GetDocument().docLineMgr.GetLine(i)->GetDocLineStrWithEOL(&nLineLen);
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
		size_t nSize = lineArr.size();
		repData.resize(nSize);
		int opeSeq = GetDocument().docEditor.opeBuf.GetNextSeq();
		for (size_t idx=0; idx<nSize; ++idx) {
			repData[idx].nSeq = opeSeq;
			repData[idx].memLine.SetString(lineArr[idx].GetPtr(), lineArr[idx].GetLength());
		}
		view.ReplaceData_CEditView3(
			selectOld_Layout,
			nullptr,
			&repData,
			false,
			view.bDoing_UndoRedo ? nullptr : GetOpeBlk(),
			opeSeq,
			nullptr
		);
	}else {
		// ���ύX�Ȃ�ύX���Ȃ�
	}

	ASSERT_GE(j, GetDocument().docLineMgr.GetLineCount());
	ASSERT_GE(nMergeLayoutLines, layoutMgr.GetLineCount());
	j -= GetDocument().docLineMgr.GetLineCount();
	nMergeLayoutLines -= layoutMgr.GetLineCount();

	// �I���G���A�̕���
	selInfo.select = selectOld_Layout;
	selInfo.select.GetTo().y -= (int)nMergeLayoutLines;

	if (nCaretPosYOLD == selInfo.select.GetFrom().y) {
		caret.MoveCursor(selInfo.select.GetFrom(), true);
	}else {
		caret.MoveCursor(selInfo.select.GetTo(), true);
	}
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX();
	if (!view.bDoing_UndoRedo) {	// Undo, Redo�̎��s����
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
			)
		);
	}
	view.RedrawAll();

	if (j) {
		TopOkMessage(view.GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW7), j);
	}else {
		InfoMessage(view.GetHwnd(), LS(STR_ERR_DLGEDITVWCMDNW8));
	}
}


// from ViewCommander_New.cpp
/* ���j���[����̍ĕϊ��Ή� */
void ViewCommander::Command_Reconvert(void)
{
	const int ATRECONVERTSTRING_SET = 1;

	// �T�C�Y���擾
	LRESULT nSize = view.SetReconvertStruct(NULL, UNICODE_BOOL);
	if (nSize == 0)  // �T�C�Y�O�̎��͉������Ȃ�
		return ;

	bool bUseUnicodeATOK = false;
	// �o�[�W�����`�F�b�N
	if (!OsSupportReconvert()) {
		
		// MSIME���ǂ���
		HWND hWnd = ImmGetDefaultIMEWnd(view.GetHwnd());
		if (SendMessage(hWnd, view.uWM_MSIME_RECONVERTREQUEST, FID_RECONVERT_VERSION, 0)) {
			SendMessage(hWnd, view.uWM_MSIME_RECONVERTREQUEST, 0, (LPARAM)view.GetHwnd());
			return ;
		}

		// ATOK���g���邩�ǂ���
		TCHAR sz[256];
		ImmGetDescription(GetKeyboardLayout(0), sz, _countof(sz)); // �����̎擾
		if ((_tcsncmp(sz, _T("ATOK"),4) == 0) && view.AT_ImmSetReconvertString) {
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
		nSize = view.SetReconvertStruct(NULL, UNICODE_BOOL || bUseUnicodeATOK);
		if (nSize == 0) { // �T�C�Y�O�̎��͉������Ȃ�
			return;
		}
	}

	// IME�̃R���e�L�X�g�擾
	HIMC hIMC = ::ImmGetContext(view.GetHwnd());
	
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
	view.SetReconvertStruct(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	// �ϊ��͈͂̒���
	if (bUseUnicodeATOK) {
		(*view.AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else {
		::ImmSetCompositionString(hIMC, SCS_QUERYRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	// ���������ϊ��͈͂�I������
	view.SetSelectionFromReonvert(pReconv, UNICODE_BOOL || bUseUnicodeATOK);
	
	// �ĕϊ����s
	if (bUseUnicodeATOK) {
		(*view.AT_ImmSetReconvertString)(hIMC, ATRECONVERTSTRING_SET, pReconv, pReconv->dwSize);
	}else {
		::ImmSetCompositionString(hIMC, SCS_SETRECONVERTSTRING, pReconv, pReconv->dwSize, NULL, 0);
	}

	// �̈���
	::HeapFree(GetProcessHeap(), 0, (LPVOID)pReconv);
	::ImmReleaseContext(view.GetHwnd(), hIMC);
}

