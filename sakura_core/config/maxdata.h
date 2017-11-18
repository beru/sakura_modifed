#pragma once

enum maxdata{
	MAX_EDITWINDOWS				= 256,
	MAX_SEARCHKEY				=  30,
	MAX_REPLACEKEY				=  30,
	MAX_GREPFILE				=  30,
	MAX_GREPFOLDER				=  30,
	MAX_TYPES					=  60,
	MAX_TYPES_EXTS				=  64,
	MAX_PrintSettingARR			=   8,

	MACRONAME_MAX				= 64,
	MAX_EXTCMDLEN				= 1024,
	MAX_EXTCMDMRUNUM			= 32,

	MAX_CMDLEN					= 1024,
	MAX_CMDARR					= 32,
	MAX_REGEX_KEYWORD			= 100,
	MAX_REGEX_KEYWORDLEN		= 1000,
	MAX_REGEX_KEYWORDLISTLEN	= MAX_REGEX_KEYWORD * 100 + 1,

	MAX_KEYHELP_FILE			= 20,

	MAX_MARKLINES_LEN			= 1023,
	MAX_DOCTYPE_LEN				= 7,
	MAX_TRANSFORM_FILENAME		= 16,

	MAX_CUSTMACRO				= 50,
	MAX_CUSTMACRO_ICO			= 50,	// �A�C�R���ɐ�p�ʒu�����蓖�ĂĂ��鐔

	MAX_TAGJUMPNUM				= 100,	// �^�u�W�����v���ő�l
	MAX_TAGJUMP_KEYWORD			= 30,	// �^�O�W�����v�p�L�[���[�h�ő�o�^��
	MAX_KEYWORDSET_PER_TYPE		= 10,	// �^�C�v�ʐݒ薈�̃L�[���[�h�Z�b�g��
	MAX_VERTLINES = 10,	// �w�茅�c��

	// MRU���X�g�Ɋ֌W����maxdata
	MAX_MRU						=  36,	//  A-Z ��36�ɂȂ�̂�
	MAX_OPENFOLDER				=  36,

	MAX_PLUGIN					= 40,	// �o�^�ł���v���O�C���̐�
	MAX_PLUG_CMD				= 50,	// �o�^�ł���v���O�C�� �R�}���h�̐�+1(1 origin��)
	MAX_PLUG_OPTION				= 100,	// �o�^�ł���v���O�C���I�v�V�����̐�
	MAX_PLUGIN_ID				= 63+1,	// �v���O�C��ID�̍ő咷��
	MAX_PLUGIN_NAME				= 63+1,	// �v���O�C�����̍ő咷��
	MAX_PLUG_STRING				= 100,	// �o�^�ł���v���O�C��������̐�

	// MainMenu
	MAX_MAINMENU				= 500,	// �o�^�ł��郁�C�����j���[�̐�
	MAX_MAINMENU_TOP			= 20,	// �o�^�ł��郁�C�����j���[�̐�
	MAX_MAIN_MENU_NAME_LEN		= 40,	// ���C�����j���[��������
};

