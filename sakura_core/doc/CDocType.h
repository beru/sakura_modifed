/*
	�h�L�������g��ʂ̊Ǘ�

	2008.01�`03 kobake �쐬
*/
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

#include "types/CType.h" // TypeConfig
#include "env/CDocTypeManager.h"

class DocType {
public:
	// �����Ɣj��
	DocType(EditDoc* pcDoc);
	
	// ���b�N�@�\	// Nov. 29, 2000 genta �ݒ�̈ꎞ�ύX���Ɋg���q�ɂ�鋭���I�Ȑݒ�ύX�𖳌��ɂ���
	void LockDocumentType() { m_nSettingTypeLocked = true; }
	void UnlockDocumentType() { m_nSettingTypeLocked = false; }
	bool GetDocumentLockState() { return m_nSettingTypeLocked; }
	
	// ������ʂ̐ݒ�Ǝ擾		// Nov. 23, 2000 genta
	void SetDocumentType(TypeConfigNum type, bool force, bool bTypeOnly = false);	// ������ʂ̐ݒ�
	void SetDocumentTypeIdx(int id = -1, bool force = false);
	TypeConfigNum GetDocumentType() const {					// ������ʂ̎擾
		return m_nSettingType;
	}
	const TypeConfig& GetDocumentAttribute() const {		// ������ʂ̏ڍ׏��
		return m_typeConfig;
	}
	TypeConfig& GetDocumentAttributeWrite() {				// ������ʂ̏ڍ׏��
		return m_typeConfig;
	}

	// �g���@�\
	void SetDocumentIcon();						// �A�C�R���̐ݒ�	// Sep. 10, 2002 genta

private:
	EditDoc*		m_pDocRef;
	TypeConfigNum	m_nSettingType;
	TypeConfig		m_typeConfig;
	bool			m_nSettingTypeLocked;		// ������ʂ̈ꎞ�ݒ���
};

