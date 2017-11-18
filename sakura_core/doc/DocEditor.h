#pragma once

#include "doc/DocListener.h"
#include "_os/Clipboard.h"
#include "OpeBuf.h"

class EditDoc;
class DocLineMgr;

class DocEditor : public DocListenerEx {
public:
	DocEditor(EditDoc& doc);

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
	// �t�@�C�����C�������ǂ���
	bool IsModified() const { return bIsDocModified; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// Nov. 20, 2000 genta
	void SetImeMode(int mode);	// IME��Ԃ̐ݒ�

	// May 15, 2000 genta
	Eol  GetNewLineCode() const { return newLineCode; }
	void  SetNewLineCode(const Eol& t) { newLineCode = t; }

	// Oct. 2, 2005 genta �}�����[�h�̐ݒ�
	bool IsInsMode() const { return bInsMode; }
	void SetInsMode(bool mode) { bInsMode = mode; }

	// Undo(���ɖ߂�)�\�ȏ�Ԃ��H
	bool IsEnableUndo(void) const {
		return opeBuf.IsEnableUndo();
	}

	// Redo(��蒼��)�\�ȏ�Ԃ��H
	bool IsEnableRedo(void) const {
		return opeBuf.IsEnableRedo();
	}

	// �N���b�v�{�[�h����\��t���\���H
	bool IsEnablePaste(void) const {
		return Clipboard::HasValidData();
	}

public:
	EditDoc&	doc;
	Eol 		newLineCode;			// Enter�������ɑ}��������s�R�[�h���
	OpeBuf		opeBuf;					// �A���h�D�o�b�t�@
	OpeBlk*		pOpeBlk;				// ����u���b�N
	int			nOpeBlkRedawCount;		// OpeBlk�̍ĕ`���Ώې�
	bool		bInsMode;				// �}���E�㏑�����[�h Oct. 2, 2005 genta
	bool		bIsDocModified;
};


class DocEditAgent {
public:
	DocEditAgent(DocLineMgr& docLineMgr) : docLineMgr(docLineMgr) { }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// May 15, 2000 genta
	void AddLineStrX(const wchar_t*, int);	// �����ɍs��ǉ� Ver1.5

private:
	DocLineMgr& docLineMgr;
};

