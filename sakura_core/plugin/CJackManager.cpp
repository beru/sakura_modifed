/*!	@file
	@brief �W���b�N�Ǘ��N���X

*/
/*
	Copyright (C) 2009, syat

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
#include "CJackManager.h"
#include "CPropertyManager.h"
#include "typeprop/CPropTypes.h"

// �R���X�g���N�^
JackManager::JackManager()
{
	// �W���b�N��`�ꗗ
	// �Y������EJack�̒l�Ɠ����ł��邱�ƁB
	struct tagJackEntry {
		EJack id;
		const WCHAR* name;
	} jackNames[] = {
		{ PP_COMMAND				, PP_COMMAND_STR		},
//		{ PP_INSTALL				, L"Install"			},
//		{ PP_UNINSTALL				, L"Uninstall"			},
//		{ PP_APP_START				, L"AppStart"			},
//		{ PP_APP_END				, L"AppEnd"				},
		{ PP_EDITOR_START			, L"EditorStart"		},
		{ PP_EDITOR_END				, L"EditorEnd"			},
		{ PP_DOCUMENT_OPEN			, L"DocumentOpen"		},
		{ PP_DOCUMENT_CLOSE			, L"DocumentClose"		},
		{ PP_DOCUMENT_BEFORE_SAVE	, L"DocumentBeforeSave"	},
		{ PP_DOCUMENT_AFTER_SAVE	, L"DocumentAfterSave"	},
		{ PP_OUTLINE				, L"Outline"			},
		{ PP_SMARTINDENT			, L"SmartIndent"		},
		{ PP_COMPLEMENT				, L"Complement"			},
		{ PP_COMPLEMENTGLOBAL		, L"ComplementGlobal"	},
		{ PP_MACRO					, L"Macro"				},
	};

	m_pShareData = &GetDllShareData();

	m_Jacks.reserve(PP_BUILTIN_JACK_COUNT);
	for (int i=0; i<PP_BUILTIN_JACK_COUNT; ++i) {
		assert(i == jackNames[i].id);

		JackDef jack;
		jack.ppId = jackNames[i].id;
		jack.szName = jackNames[i].name;

		m_Jacks.push_back(jack);
	}
	
}

// �W���b�N��`�ꗗ��Ԃ�
std::vector<JackDef> JackManager::GetJackDef() const
{
	return m_Jacks;
}

// �v���O���W���b�N�Ɋ֘A�t����
ERegisterPlugResult JackManager::RegisterPlug(
	wstring pszJack,
	Plug* plug
	)
{
	EJack ppId = GetJackFromName(pszJack);
	if (ppId == PP_NONE) {
		return PPMGR_INVALID_NAME;
	}

	// �@�\ID�̏����ɂȂ�悤�Ƀv���O��o�^����
	Plug::Array& plugs = m_Jacks[ppId].plugs;
	int plugid = plug->GetFunctionCode();
	if (plugs.empty()  ||  (*(plugs.end() - 1))->GetFunctionCode() < plugid) {
		plugs.push_back(plug);
	}else {
		for (size_t index=0; index<plugs.size(); ++index) {
			if (plugid < plugs[index]->GetFunctionCode()) {
				plugs.insert(plugs.begin() + index, plug);
				break;
			}
		}
	}

	switch (ppId) {
	case PP_OUTLINE:					// �A�E�g���C����͕��@��ǉ�
		{
			int nMethod = Plug::GetOutlineType(plug->GetFunctionCode());	// 2011/8/20 syat �v���O�������̂���GetOutlineType�d�l�ύX// 2010/5/1 Uchi �֐���
			CPropTypesScreen::AddOutlineMethod(nMethod, plug->m_sLabel.c_str());
		}
		break;
	case PP_SMARTINDENT:				// �X�}�[�g�C���f���g���@��ǉ�
		{
			int nMethod = Plug::GetSmartIndentType(plug->GetFunctionCode());	// 2011/8/20 syat �v���O�������̂���GetOutlineType�d�l�ύX// 2010/5/1 Uchi �֐���
			CPropTypesScreen::AddSIndentMethod(nMethod, plug->m_sLabel.c_str());
		}
		break;
	case PP_COMPLEMENT:
		{
			int nMethod = Plug::GetPluginFunctionCode(plug->m_cPlugin.m_id, 0);
			CPropTypesSupport::AddHokanMethod(nMethod, plug->m_sLabel.c_str());
		}
		break;
	}
	return PPMGR_REG_OK;
}

// �v���O�̊֘A�t������������
bool JackManager::UnRegisterPlug(
	wstring pszJack,
	Plug* plug
	)
{
	EJack ppId = GetJackFromName(pszJack);

	switch (ppId) {
	case PP_OUTLINE:					// �A�E�g���C����͕��@��ǉ�
		{
			int nMethod = Plug::GetOutlineType(plug->GetFunctionCode());
			CPropTypesScreen::RemoveOutlineMethod(nMethod, plug->m_sLabel.c_str());
		}
		break;
	case PP_SMARTINDENT:				// �X�}�[�g�C���f���g���@��ǉ�
		{
			int nMethod = Plug::GetSmartIndentType(plug->GetFunctionCode());
			CPropTypesScreen::RemoveSIndentMethod(nMethod, plug->m_sLabel.c_str());
		}
		break;
	case PP_COMPLEMENT:
		{
			int nMethod = Plug::GetPluginFunctionCode(plug->m_cPlugin.m_id, 0);
			CPropTypesSupport::RemoveHokanMethod(nMethod, plug->m_sLabel.c_str());
		}
		break;
	}

	auto& plugs = m_Jacks[ppId].plugs;
	for (size_t index=0; index<plugs.size(); ++index) {
		if (plugs[index] == plug) {
			plugs.erase(plugs.begin() + index);
			break;
		}
	}

	return true;
}

// �W���b�N�����W���b�N�ԍ��ɕϊ�����
EJack JackManager::GetJackFromName(wstring sName)
{
	const WCHAR* szName = sName.c_str();
	const size_t jackSize = m_Jacks.size();
	for (size_t i=0; i<jackSize; ++i) {
		auto& jack = m_Jacks[i];
		if (wcscmp(jack.szName, szName) == 0) {
			return jack.ppId;
		}
	}
	// ������Ȃ�
	return PP_NONE;
}

// ���p�\�ȃv���O����������
bool JackManager::GetUsablePlug(
	EJack			jack,		// [in] �W���b�N�ԍ�
	PlugId			plugId,		// [in] �v���OID
	Plug::Array*	plugs		// [out] ���p�\�v���O�̃��X�g
	)
{
	auto& jackPlugs = m_Jacks[jack].plugs;
	for (auto it=jackPlugs.begin(); it!=jackPlugs.end(); ++it) {
		if (plugId == 0 || plugId == (*it)->GetFunctionCode()) {
			plugs->push_back(*it);
		}
	}
	return true;
}

// �v���O�C���R�}���h�̋@�\�ԍ���Ԃ�
EFunctionCode JackManager::GetCommandCode(int index) const
{
	Plug::Array commands = m_Jacks[PP_COMMAND].plugs;

	if ((unsigned int)index < commands.size()) {
		return (commands[index])->GetFunctionCode();
	}else {
		return F_INVALID;
	}
}

// �v���O�C���R�}���h�̖��O��Ԃ�
int JackManager::GetCommandName(
	int funccode,
	WCHAR* buf,
	int size
	) const
{
	auto& plugs = m_Jacks[PP_COMMAND].plugs;
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		if (((Plug*)(*it))->GetFunctionCode() == funccode) {
			wcsncpy(buf, ((Plug*)(*it))->m_sLabel.c_str(), size);
			buf[size-1] = L'\0';
			return 1;
		}
	}
	return -1;
}

// �v���O�C���R�}���h�̐���Ԃ�
int JackManager::GetCommandCount() const
{
	return m_Jacks[PP_COMMAND].plugs.size();
}

// ID�ɍ��v����R�}���h�v���O��Ԃ�
Plug* JackManager::GetCommandById(int id) const
{
	const Plug::Array& plugs = GetPlugs(PP_COMMAND);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		if ((*it)->GetFunctionCode() == id) {
			return (*it);
		}
	}
	assert_warning(false);	// ID�ɍ��v����v���O���o�^����Ă��Ȃ�
	return NULL;
}

// �v���O��Ԃ�
const Plug::Array& JackManager::GetPlugs(EJack jack) const
{
	return m_Jacks[jack].plugs;	
}

