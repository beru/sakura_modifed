/*!	@file
	@brief �v���O�C����{�N���X

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
#pragma once

#include <algorithm>
#include "macro/CWSHIfObj.h"
#include "CDataProfile.h"
#include "util/string_ex.h"

// �v���O�C���̊Ǘ��ԍ�index
typedef int PluginId;
// �v���O�̊Ǘ��ԍ� �v���O�C���̃R�}���h�v���O���ƂɈ�ӁB�ق���0
typedef int PlugId;

// �v���O�C����`�t�@�C����
#define PII_FILENAME				_T("plugin.def")
#define PII_L10NDIR					_T("local")
#define PII_L10NFILEBASE			_T("plugin_")
#define PII_L10NFILEEXT				_T(".def")
// �I�v�V�����t�@�C���g���q�i�I�v�V�����t�@�C�����ʃt�H���_���{�g���q�j
#define PII_OPTFILEEXT				_T(".ini")

// �v���O�C����`�t�@�C���E�L�[������
#define	PII_PLUGIN					L"Plugin"		// ���ʏ��
#define	PII_PLUGIN_ID				L"Id"			// ID�F�v���O�C��ID
#define	PII_PLUGIN_NAME				L"Name"			// ���O�F�v���O�C����
#define	PII_PLUGIN_DESCRIPTION		L"Description"	// �����F�Ȍ��Ȑ���
#define	PII_PLUGIN_PLUGTYPE			L"Type"			// ��ʁFwsh / dll
#define	PII_PLUGIN_AUTHOR			L"Author"		// ��ҁF���쌠�Җ�
#define	PII_PLUGIN_VERSION			L"Version"		// �o�[�W�����F�v���O�C���̃o�[�W����
#define	PII_PLUGIN_URL				L"Url"			// �z�zURL�F�z�z��URL

#define PII_PLUG					L"Plug"			// �v���O���
#define PII_STRING					L"String"		// ��������

#define PII_COMMAND					L"Command"		// �R�}���h���
#define PII_OPTION					L"Option"		// �I�v�V������`���	// 2010/3/24 Uchi


class Plugin;

// �v���O�i�v���O�C�����̏����P�ʁj�N���X
class Plug {
	// �^��`
protected:
	typedef std::wstring wstring;
public:
	/*!
	  Plug::Array��std::vector�Ȃ̂ŁA�v�f�̒ǉ��폜�iinsert/erase�j�������
	  �C�e���[�^�������ɂȂ邱�Ƃ�����B���̂��ߕϐ��Ɋi�[�����C�e���[�^��
	  insert/erase�̑������Ɏw�肷��ƁAVC2005�Ńr���h�G���[���o��B
	  ������begin/end����̑��Έʒu�w���A�C���f�b�N�X�w����g�����ƁB
	*/
	typedef std::vector<Plug*> Array;			// �v���O�̃��X�g
	typedef Array::const_iterator ArrayIter;	// ���̃C�e���[�^

	// �R���X�g���N�^
public:
	Plug(Plugin& plugin, PlugId id, const wstring& sJack, const wstring& sHandler, const wstring& sLabel)
		:
		m_id(id),
		m_sJack(sJack),
		m_sHandler(sHandler),
		m_sLabel(sLabel),
		m_plugin(plugin)
	{
	}
	// �f�X�g���N�^
public:
	virtual ~Plug() {}

	// ����
public:
	bool Invoke(EditView* view, WSHIfObj::List& params);	// �v���O�����s����

	// ����
public:
	EFunctionCode GetFunctionCode() const;

	// �⏕�֐�
public:
	// Plug Function�ԍ��̌v�Z(�N���X�O�ł��g����o�[�W����)
	// 2010/4/19 Uchi
	// 2011/8/20 syat �֐��R�[�h�̊��蓖�Ē���
	static inline EFunctionCode GetPluginFunctionCode(PluginId nPluginId, PlugId nPlugId) {
		return static_cast<EFunctionCode>((nPluginId%20 * 100) + (nPluginId/20 * 50) + nPlugId + F_PLUGCOMMAND_FIRST);
	}

	// PluginId�ԍ��̌v�Z(�N���X�O�ł��g����o�[�W����)
	// 2010/4/19 Uchi
	// 2011/8/20 syat �֐��R�[�h�̊��蓖�Ē���
	static inline PluginId GetPluginId(EFunctionCode nFunctionCode) {
		if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
			return PluginId((nFunctionCode - F_PLUGCOMMAND_FIRST)/100 + (nFunctionCode%100/50 * 20));
		}
		return PluginId(-1);
	}

	// PluginNo�ԍ��̌v�Z(�N���X�O�ł��g����o�[�W����)
	// 2010/6/24 Uchi
	// 2011/8/20 syat �֐��R�[�h�̊��蓖�Ē���
	static inline PlugId GetPlugId(EFunctionCode nFunctionCode) {
		if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
			return PlugId(nFunctionCode%100 - (nFunctionCode%100/50 * 50));
		}
		return PlugId(-1);
	}

	/* PluginId, PlugId �� �֐��R�[�h�̃}�b�s���O *****************************
	 *   PluginId �c �v���O�C���̔ԍ� 0�`39
	 *     PlugId �c �v���O�C�����̃v���O�̔ԍ� 0�`49
	 *
	 *   �֐��R�[�h 20000�`21999   ()����(PluginId, PlugId)��\��
	 *   +------------+------------+----+------------+
	 *   |20000(0,0)  |20100(1,0)  |    |21900(19,0) |
	 *   |  :         |  :         | �c |  :         |
	 *   |20049(0,49) |20149(1,49) |    |21949(19,49)| 
	 *   +------------+------------+----+------------+
	 *   |20050(20,0) |20150(21,0) |    |21950(39,0) |
	 *   |  :         |  :         | �c |  :         |
	 *   |20099(20,49)|20199(21,49)|    |21999(39,49)| 
	 *   +------------+------------+----+------------+
	 *   ��������Ȃ���΁A22000�`23999�𕥂��o���ĐH���Ԃ�
	 *************************************************************************/
	static EOutlineType GetOutlineType(EFunctionCode nFunctionCode) {
		return static_cast<EOutlineType>(nFunctionCode);
	}

	static SmartIndentType GetSmartIndentType(EFunctionCode nFunctionCode) {
		return static_cast<SmartIndentType>(nFunctionCode);
	}

	// �����o�ϐ�
public:
	const PlugId m_id;					// �v���OID
	const wstring m_sJack;				// �֘A�t����W���b�N��
	const wstring m_sHandler;			// �n���h��������i�֐����j
	const wstring m_sLabel;				// ���x��������
	wstring m_sIcon;					// �A�C�R���̃t�@�C���p�X
	Plugin& m_plugin;					// �e�v���O�C��
};

// �I�v�V������`	// 2010/3/24 Uchi
std::vector<std::wstring> wstring_split(std::wstring, wchar_t);

class PluginOption {
	// �^��`
protected:
	typedef std::wstring wstring;
public:
	typedef std::vector<PluginOption*> Array;	// �I�v�V�����̃��X�g
	typedef Array::const_iterator ArrayIter;	// ���̃C�e���[�^

	// �R���X�g���N�^
public:
	PluginOption(
		Plugin* parent,
		const wstring& sLabel,
		const wstring& sSection,
		const wstring& sKey,
		const wstring& sType,
		const wstring& sSelects,
		const wstring& sDefaultVal,
		int index
		)
	{
		m_parent		= parent;
		m_sLabel		= sLabel;
		m_sSection	= sSection;
		m_sKey		= sKey;
		// �������ϊ�
		m_sType		= sType;
		std::transform(m_sType.begin(), m_sType.end(), m_sType.begin(), tolower);
		m_sSelects	= sSelects;
		m_sDefaultVal = sDefaultVal;
		m_index		= index;
	}

	// �f�X�g���N�^
public:
	~PluginOption() {}

	// ����
public:
	wstring	GetLabel(void)	{ return m_sLabel; }
	void	GetKey(wstring* sectin, wstring* key)	{ 
		*sectin = m_sSection; 
		*key = m_sKey;
	}
	wstring	GetType(void)	{ return m_sType; }
	int 	GetIndex(void)	{ return m_index; }
	std::vector<wstring> GetSelects() {
		return (wstring_split(m_sSelects, L'|'));
	}
	wstring	GetDefaultVal() { return m_sDefaultVal; }

protected:
	Plugin*	m_parent;
	wstring		m_sLabel;
	wstring		m_sSection;
	wstring		m_sKey;
	wstring		m_sType;
	wstring		m_sSelects;		// �I�����
	wstring		m_sDefaultVal;
	int 		m_index; 
};


// �v���O�C���N���X

class Plugin {
	// �^��`
protected:
	typedef std::wstring wstring;
	typedef std::string string;

public:
	typedef std::list<Plugin*> List;		// �v���O�C���̃��X�g
	typedef List::const_iterator ListIter;	// ���̃C�e���[�^

	// �R���X�g���N�^
public:
	Plugin(const tstring& sBaseDir);

	// �f�X�g���N�^
public:
	virtual ~Plugin(void);

	// ����
public:
	virtual int AddCommand(const WCHAR* handler, const WCHAR* label, const WCHAR* icon, bool doRegister);	// �R�}���h��ǉ�����
	int 	GetCommandCount()	{ return m_nCommandCount; }			// �R�}���h����Ԃ�	2010/7/4 Uchi

protected:
	bool ReadPluginDefCommon(DataProfile *cProfile, DataProfile *cProfileMlang);	// �v���O�C����`�t�@�C����Common�Z�N�V������ǂݍ���
	bool ReadPluginDefPlug(DataProfile *cProfile, DataProfile *cProfileMlang);	// �v���O�C����`�t�@�C����Plug�Z�N�V������ǂݍ���
	bool ReadPluginDefCommand(DataProfile *cProfile, DataProfile *cProfileMlang);	// �v���O�C����`�t�@�C����Command�Z�N�V������ǂݍ���
	bool ReadPluginDefOption(DataProfile *cProfile, DataProfile *cProfileMlang);	// �v���O�C����`�t�@�C����Option�Z�N�V������ǂݍ���	// 2010/3/24 Uchi
	bool ReadPluginDefString(DataProfile *cProfile, DataProfile *cProfileMlang);	// �v���O�C����`�t�@�C����String�Z�N�V������ǂݍ���

	// Plug�C���X�^���X�̍쐬�BReadPluginDefPlug/Command ����Ă΂��B
	virtual Plug* CreatePlug(Plugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel) {
		return new Plug(plugin, id, sJack, sHandler, sLabel);
	}

//	void NormalizeExtList(const wstring& sExtList, wstring& sOut);	// �J���}��؂�g���q���X�g�𐳋K������

	// ����
public:
	tstring GetFilePath(const tstring& sFileName) const;					// �v���O�C���t�H���_��̑��΃p�X���t���p�X�ɕϊ�
	tstring GetPluginDefPath() const { return GetFilePath(PII_FILENAME); }	// �v���O�C����`�t�@�C���̃p�X
	tstring GetOptionPath() const { return m_sOptionDir + PII_OPTFILEEXT; }	// �I�v�V�����t�@�C���̃p�X
	tstring GetFolderName() const;	// �v���O�C���̃t�H���_�����擾
	virtual Plug::Array GetPlugs() const = 0;								// �v���O�̈ꗗ

	// �����o�ϐ�
public:
	PluginId m_id;				// �v���O�C���ԍ��i�G�f�B�^���ӂ�0�`MAX_PLUGIN-1�̔ԍ��j
	wstring m_sId;				// �v���O�C��ID
	wstring m_sName;			// �v���O�C���a��
	wstring m_sDescription;		// �v���O�C���ɂ��Ă̊ȒP�ȋL�q
	wstring m_sAuthor;			// ���
	wstring m_sVersion;			// �o�[�W����
	wstring m_sUrl;				// �z�zURL
	tstring m_sBaseDir;
	tstring m_sOptionDir;
	tstring m_sLangName;		// ���ꖼ
	PluginOption::Array m_options;		// �I�v�V����	// 2010/3/24 Uchi
	std::vector<std::wstring> m_aStrings;	// ������
private:
	bool m_bLoaded;
protected:
	Plug::Array m_plugs;
	int m_nCommandCount;

	// �������
public:
	virtual bool InvokePlug(EditView* view, Plug& plug, WSHIfObj::List& param) = 0;			// �v���O�����s����
	virtual bool ReadPluginDef(DataProfile* cProfile, DataProfile* cProfileMlang) = 0;		// �v���O�C����`�t�@�C����ǂݍ���
	virtual bool ReadPluginOption(DataProfile* cProfile) = 0;									// �I�v�V�����t�@�C����ǂݍ���
};

