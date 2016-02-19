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

#include "StdAfx.h"
#include "DocEditor.h"
#include "EditDoc.h"
#include "doc/logic/DocLine.h"
#include "doc/logic/DocLineMgr.h"
#include "env/DllSharedData.h"
#include "_main/AppMode.h"
#include "Eol.h"
#include "window/EditWnd.h"
#include "debug/RunningTimer.h"

DocEditor::DocEditor(EditDoc* pDoc)
	:
	m_pDocRef(pDoc),
	m_newLineCode(EolType::CRLF),	// New Line Type
	m_pOpeBlk(NULL),
	m_bInsMode(true),			// Oct. 2, 2005 genta
	m_bIsDocModified(false)	// �ύX�t���O // Jan. 22, 2002 genta �^�ύX
{
	// Oct. 2, 2005 genta �}�����[�h
	this->SetInsMode(GetDllShareData().m_common.general.m_bIsINSMode);
}


/*! �ύX�t���O�̐ݒ�

	@param flag [in] �ݒ肷��l�Dtrue: �ύX�L�� / false: �ύX����
	@param redraw [in] true: �^�C�g���̍ĕ`����s�� / false: �s��Ȃ�
	
	@author genta
	@date 2002.01.22 �V�K�쐬
*/
void DocEditor::SetModified(bool flag, bool redraw)
{
	if (m_bIsDocModified == flag)	// �ύX���Ȃ���Ή������Ȃ�
		return;

	m_bIsDocModified = flag;
	if (redraw)
		m_pDocRef->m_pEditWnd->UpdateCaption();
}

void DocEditor::OnBeforeLoad(LoadInfo* loadInfo)
{
	// �r���[�̃e�L�X�g�I������
	GetListeningDoc()->m_pEditWnd->Views_DisableSelectArea(true);
}

void DocEditor::OnAfterLoad(const LoadInfo& loadInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	// May 12, 2000 genta
	// �ҏW�p���s�R�[�h�̐ݒ�
	{
		const TypeConfig& type = pDoc->m_docType.GetDocumentAttribute();
		if (pDoc->m_docFile.GetCodeSet() == type.m_encoding.m_eDefaultCodetype) {
			SetNewLineCode(type.m_encoding.m_eDefaultEoltype);	// 2011.01.24 ryoji �f�t�H���gEOL
		}else {
			SetNewLineCode(EolType::CRLF);
		}
		DocLine* pFirstlineinfo = pDoc->m_docLineMgr.GetLine(LogicInt(0));
		if (pFirstlineinfo) {
			EolType t = pFirstlineinfo->GetEol();
			if (t != EolType::None && t != EolType::Unknown) {
				SetNewLineCode(t);
			}
		}
	}

	// Nov. 20, 2000 genta
	// IME��Ԃ̐ݒ�
	this->SetImeMode(pDoc->m_docType.GetDocumentAttribute().m_nImeState);

	// �J�����g�f�B���N�g���̕ύX
	::SetCurrentDirectory(pDoc->m_docFile.GetFilePathClass().GetDirPath().c_str());
	AppMode::getInstance()->SetViewMode(loadInfo.bViewMode);		// �r���[���[�h	##�������A�A������
}

void DocEditor::OnAfterSave(const SaveInfo& saveInfo)
{
	EditDoc* pDoc = GetListeningDoc();

	this->SetModified(false, false);	// Jan. 22, 2002 genta �֐��� �X�V�t���O�̃N���A

	// ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm
	this->m_opeBuf.SetNoModified();

	// �J�����g�f�B���N�g���̕ύX
	::SetCurrentDirectory(pDoc->m_docFile.GetFilePathClass().GetDirPath().c_str());
}

// From Here Nov. 20, 2000 genta
/*!	IME��Ԃ̐ݒ�
	
	@param mode [in] IME�̃��[�h
	
	@date Nov 20, 2000 genta
*/
void DocEditor::SetImeMode(int mode)
{
	HWND hwnd = m_pDocRef->m_pEditWnd->GetActiveView().GetHwnd();
	HIMC hIme = ImmGetContext(hwnd); //######���v�H // 2013.06.04 EditWnd����View�ɕύX

	// �ŉ��ʃr�b�g��IME���g��On/Off����
	if ((mode & 3) == 2) {
		ImmSetOpenStatus(hIme, FALSE);
	}
	if ((mode >> 2) > 0) {
		DWORD conv, sent;
		ImmGetConversionStatus(hIme, &conv, &sent);

		switch (mode >> 2) {
		case 1:	// FullShape
			conv |= IME_CMODE_FULLSHAPE;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 2:	// FullShape & Hiragana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
			conv &= ~(IME_CMODE_KATAKANA | IME_CMODE_NOCONVERSION);
			break;
		case 3:	// FullShape & Katakana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE | IME_CMODE_KATAKANA;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 4: // Non-Conversion
			conv |= IME_CMODE_NOCONVERSION;
			break;
		}
		ImmSetConversionStatus(hIme, conv, sent);
	}
	if ((mode & 3) == 1) {
		ImmSetOpenStatus(hIme, TRUE);
	}
	ImmReleaseContext(hwnd, hIme); //######���v�H
}
// To Here Nov. 20, 2000 genta

/*!
	�����ɍs��ǉ�

	@version 1.5

	@param pData    [in] �ǉ����镶����ւ̃|�C���^
	@param nDataLen [in] ������̒����B�����P�ʁB
	@param eol     [in] �s���R�[�h

*/
void DocEditAgent::AddLineStrX(const wchar_t* pData, int nDataLen)
{
	// �`�F�[���K�p
	DocLine* pDocLine = m_pDocLineMgr->AddNewLine();

	// �C���X�^���X�ݒ�
	pDocLine->SetDocLineString(pData, nDataLen);
}

