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

#include "doc/CDocListener.h"
#include "_os/CClipboard.h"
#include "COpeBuf.h"

class EditDoc;
class DocLineMgr;

class DocEditor : public DocListenerEx {
public:
	DocEditor(EditDoc* pDoc);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �C�x���g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// ���[�h�O��
	void OnBeforeLoad(LoadInfo* pLoadInfo);
	void OnAfterLoad(const LoadInfo& loadInfo);

	// �Z�[�u�O��
	void OnAfterSave(const SaveInfo& saveInfo);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// Jan. 22, 2002 genta Modified Flag�̐ݒ�
	void SetModified(bool flag, bool redraw);
	//! �t�@�C�����C�������ǂ���
	bool IsModified() const { return m_bIsDocModified; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// Nov. 20, 2000 genta
	void SetImeMode(int mode);	// IME��Ԃ̐ݒ�

	// May 15, 2000 genta
	Eol  GetNewLineCode() const { return m_newLineCode; }
	void  SetNewLineCode(const Eol& t) { m_newLineCode = t; }

	// Oct. 2, 2005 genta �}�����[�h�̐ݒ�
	bool IsInsMode() const { return m_bInsMode; }
	void SetInsMode(bool mode) { m_bInsMode = mode; }

	//! Undo(���ɖ߂�)�\�ȏ�Ԃ��H
	bool IsEnableUndo(void) const {
		return m_opeBuf.IsEnableUndo();
	}

	//! Redo(��蒼��)�\�ȏ�Ԃ��H
	bool IsEnableRedo(void) const {
		return m_opeBuf.IsEnableRedo();
	}

	//! �N���b�v�{�[�h����\��t���\���H
	bool IsEnablePaste(void) const {
		return Clipboard::HasValidData();
	}

public:
	EditDoc*		m_pDocRef;
	Eol 			m_newLineCode;				//!< Enter�������ɑ}��������s�R�[�h���
	OpeBuf			m_opeBuf;					//!< �A���h�D�o�b�t�@
	OpeBlk*			m_pOpeBlk;					//!< ����u���b�N
	int				m_nOpeBlkRedawCount;		//!< OpeBlk�̍ĕ`���Ώې�
	bool			m_bInsMode;					//!< �}���E�㏑�����[�h Oct. 2, 2005 genta
	bool			m_bIsDocModified;
};


class DocEditAgent {
public:
	DocEditAgent(DocLineMgr* pDocLineMgr) : m_pDocLineMgr(pDocLineMgr) { }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// May 15, 2000 genta
	void AddLineStrX(const wchar_t*, int);	// �����ɍs��ǉ� Ver1.5

private:
	DocLineMgr* m_pDocLineMgr;
};

