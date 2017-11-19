/*
	�h�L�������g��ʂ̊Ǘ�
*/
#pragma once

#include "types/Type.h" // TypeConfig
#include "env/DocTypeManager.h"

class DocType {
public:
	// �����Ɣj��
	DocType(EditDoc& doc);
	
	// ���b�N�@�\
	void LockDocumentType() { nSettingTypeLocked = true; }
	void UnlockDocumentType() { nSettingTypeLocked = false; }
	bool GetDocumentLockState() { return nSettingTypeLocked; }
	
	// ������ʂ̐ݒ�Ǝ擾
	void SetDocumentType(TypeConfigNum type, bool force, bool bTypeOnly = false);	// ������ʂ̐ݒ�
	void SetDocumentTypeIdx(int id = -1, bool force = false);
	TypeConfigNum GetDocumentType() const {					// ������ʂ̎擾
		return nSettingType;
	}
	const TypeConfig& GetDocumentAttribute() const {		// ������ʂ̏ڍ׏��
		return typeConfig;
	}
	TypeConfig& GetDocumentAttributeWrite() {				// ������ʂ̏ڍ׏��
		return typeConfig;
	}

	// �g���@�\
	void SetDocumentIcon();						// �A�C�R���̐ݒ�

private:
	EditDoc&		doc;
	TypeConfigNum	nSettingType;
	TypeConfig		typeConfig;
	bool			nSettingTypeLocked;		// ������ʂ̈ꎞ�ݒ���
};

