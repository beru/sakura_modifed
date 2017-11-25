/*!	@file
	@brief �W���b�N�Ǘ��N���X
*/
#pragma once

#include "plugin/Plugin.h"
#include <list>

#define PP_COMMAND_STR	L"Command"

// �W���b�N�i���v���O�C���\�ӏ��j
enum EJack {
	PP_NONE			= -1,
	PP_COMMAND		= 0,
//	PP_INSTALL,
//	PP_UNINSTALL,
//	PP_APP_START,	// ����G�f�B�^���ƂɃv���O�C���Ǘ����Ă��邽��
//	PP_APP_END,		// �A�v�����x���̃C�x���g�͈����ɂ���
	PP_EDITOR_START,
	PP_EDITOR_END,
	PP_DOCUMENT_OPEN,
	PP_DOCUMENT_CLOSE,
	PP_DOCUMENT_BEFORE_SAVE,
	PP_DOCUMENT_AFTER_SAVE,
	PP_OUTLINE,
	PP_SMARTINDENT,
	PP_COMPLEMENT,
	PP_COMPLEMENTGLOBAL,
	PP_MACRO,

	// ���W���b�N��ǉ�����Ƃ��͂��̍s�̏�ɁB
	PP_BUILTIN_JACK_COUNT	// �g�ݍ��݃W���b�N��
};

// �W���b�N��`�\����
struct JackDef {
	EJack			ppId;
	const wchar_t*	szName;
	Plug::Array	plugs;	// �W���b�N�Ɋ֘A�t����ꂽ�v���O
};

// �v���O�o�^����
enum ERegisterPlugResult {
	PPMGR_REG_OK,				// �v���O�C���o�^����
	PPMGR_INVALID_NAME,			// �W���b�N�����s��
	PPMGR_CONFLICT				// �w�肵���W���b�N�͕ʂ̃v���O�C�����ڑ����Ă���
};

// �W���b�N�Ǘ��N���X
class JackManager : public TSingleton<JackManager> {
	friend class TSingleton<JackManager>;
	JackManager();

	typedef std::wstring wstring;

	// ����
public:
	ERegisterPlugResult RegisterPlug(wstring pszJack, Plug* plug);	// �v���O���W���b�N�Ɋ֘A�t����
	bool UnRegisterPlug(wstring pszJack, Plug* plug);	// �v���O�̊֘A�t������������
	bool GetUsablePlug(EJack jack, PlugId plugId, Plug::Array* plugs);	// ���p�\�ȃv���O����������
private:
	EJack GetJackFromName(wstring sName);	// �W���b�N�����W���b�N�ԍ��ɕϊ�����

	// ����
public:
	std::vector<JackDef> GetJackDef() const;	// �W���b�N��`�ꗗ��Ԃ�
	EFunctionCode GetCommandCode(int index) const;		// �v���O�C���R�}���h�̋@�\�R�[�h��Ԃ�
	int GetCommandName(int funccode, wchar_t* buf, int size) const;	// �v���O�C���R�}���h�̖��O��Ԃ�
	size_t GetCommandCount() const;	// �v���O�C���R�}���h�̐���Ԃ�
	Plug* GetCommandById(int id) const;	// ID�ɍ��v����R�}���h�v���O��Ԃ�
	const Plug::Array& GetPlugs(EJack jack) const;	// �v���O��Ԃ�

	// �����o�ϐ�
private:
	DllSharedData* pShareData;
	std::vector<JackDef> jacks;	// �W���b�N��`�̈ꗗ
};

