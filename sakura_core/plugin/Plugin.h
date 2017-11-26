/*!	@file
	@brief �v���O�C����{�N���X
*/
#pragma once

#include <algorithm>
#include "macro/WSHIfObj.h"
#include "DataProfile.h"
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
#define PII_OPTION					L"Option"		// �I�v�V������`���


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
		id(id),
		sJack(sJack),
		sHandler(sHandler),
		sLabel(sLabel),
		plugin(plugin)
	{
	}
	// �f�X�g���N�^
public:
	virtual ~Plug() {}

	// ����
public:
	bool Invoke(EditView& view, WSHIfObj::List& params);	// �v���O�����s����

	// ����
public:
	EFunctionCode GetFunctionCode() const;

	// �⏕�֐�
public:
	// Plug Function�ԍ��̌v�Z(�N���X�O�ł��g����o�[�W����)
	static inline EFunctionCode GetPluginFunctionCode(PluginId nPluginId, PlugId nPlugId) {
		return static_cast<EFunctionCode>((nPluginId%20 * 100) + (nPluginId/20 * 50) + nPlugId + F_PLUGCOMMAND_FIRST);
	}

	// PluginId�ԍ��̌v�Z(�N���X�O�ł��g����o�[�W����)
	static inline PluginId GetPluginId(EFunctionCode nFunctionCode) {
		if (nFunctionCode >= F_PLUGCOMMAND_FIRST && nFunctionCode < F_PLUGCOMMAND_LAST) {
			return PluginId((nFunctionCode - F_PLUGCOMMAND_FIRST)/100 + (nFunctionCode%100/50 * 20));
		}
		return PluginId(-1);
	}

	// PluginNo�ԍ��̌v�Z(�N���X�O�ł��g����o�[�W����)
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
	static OutlineType GetOutlineType(EFunctionCode nFunctionCode) {
		return static_cast<OutlineType>(nFunctionCode);
	}

	static SmartIndentType GetSmartIndentType(EFunctionCode nFunctionCode) {
		return static_cast<SmartIndentType>(nFunctionCode);
	}

	// �����o�ϐ�
public:
	const PlugId id;				// �v���OID
	const wstring sJack;			// �֘A�t����W���b�N��
	const wstring sHandler;			// �n���h��������i�֐����j
	const wstring sLabel;			// ���x��������
	wstring sIcon;					// �A�C�R���̃t�@�C���p�X
	Plugin& plugin;					// �e�v���O�C��
};

// �I�v�V������`
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
		Plugin& parent,
		const wstring& sLabel,
		const wstring& sSection,
		const wstring& sKey,
		const wstring& sType,
		const wstring& sSelects,
		const wstring& sDefaultVal,
		int index
		)
		:
		parent(parent),
		sLabel(sLabel),
		sSection(sSection),
		sKey(sKey),
		sType(sType),
		sSelects(sSelects),
		sDefaultVal(sDefaultVal),
		index(index)
	{
		// �������ϊ�
		std::transform(this->sType.begin(), this->sType.end(), this->sType.begin(), tolower);
	}

	// �f�X�g���N�^
public:
	~PluginOption() {}

	// ����
public:
	wstring	GetLabel(void)	{ return sLabel; }
	void	GetKey(wstring* sectin, wstring* key)	{ 
		*sectin = sSection; 
		*key = sKey;
	}
	wstring	GetType(void)	{ return sType; }
	int 	GetIndex(void)	{ return index; }
	std::vector<wstring> GetSelects() {
		return (wstring_split(sSelects, L'|'));
	}
	wstring	GetDefaultVal() { return sDefaultVal; }

protected:
	Plugin&		parent;
	wstring		sLabel;
	wstring		sSection;
	wstring		sKey;
	wstring		sType;
	wstring		sSelects;		// �I�����
	wstring		sDefaultVal;
	int 		index; 
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
	virtual int AddCommand(const wchar_t* handler, const wchar_t* label, const wchar_t* icon, bool doRegister);	// �R�}���h��ǉ�����
	int 	GetCommandCount()	{ return nCommandCount; }			// �R�}���h����Ԃ�

protected:
	bool ReadPluginDefCommon(DataProfile& profile, DataProfile* profileMlang);	// �v���O�C����`�t�@�C����Common�Z�N�V������ǂݍ���
	bool ReadPluginDefPlug(DataProfile& profile, DataProfile* profileMlang);	// �v���O�C����`�t�@�C����Plug�Z�N�V������ǂݍ���
	bool ReadPluginDefCommand(DataProfile& profile, DataProfile* profileMlang);	// �v���O�C����`�t�@�C����Command�Z�N�V������ǂݍ���
	bool ReadPluginDefOption(DataProfile& profile, DataProfile* profileMlang);	// �v���O�C����`�t�@�C����Option�Z�N�V������ǂݍ���
	bool ReadPluginDefString(DataProfile& profile, DataProfile* profileMlang);	// �v���O�C����`�t�@�C����String�Z�N�V������ǂݍ���

	// Plug�C���X�^���X�̍쐬�BReadPluginDefPlug/Command ����Ă΂��B
	virtual Plug* CreatePlug(
		Plugin& plugin,
		PlugId id,
		const wstring& sJack,
		const wstring& sHandler,
		const wstring& sLabel
	) {
		return new Plug(plugin, id, sJack, sHandler, sLabel);
	}

//	void NormalizeExtList(const wstring& sExtList, wstring& sOut);	// �J���}��؂�g���q���X�g�𐳋K������

	// ����
public:
	tstring GetFilePath(const tstring& sFileName) const;					// �v���O�C���t�H���_��̑��΃p�X���t���p�X�ɕϊ�
	tstring GetPluginDefPath() const { return GetFilePath(PII_FILENAME); }	// �v���O�C����`�t�@�C���̃p�X
	tstring GetOptionPath() const { return sOptionDir + PII_OPTFILEEXT; }	// �I�v�V�����t�@�C���̃p�X
	tstring GetFolderName() const;	// �v���O�C���̃t�H���_�����擾
	virtual Plug::Array GetPlugs() const = 0;								// �v���O�̈ꗗ

	// �����o�ϐ�
public:
	PluginId id;				// �v���O�C���ԍ��i�G�f�B�^���ӂ�0�`MAX_PLUGIN-1�̔ԍ��j
	wstring sId;				// �v���O�C��ID
	wstring sName;				// �v���O�C���a��
	wstring sDescription;		// �v���O�C���ɂ��Ă̊ȒP�ȋL�q
	wstring sAuthor;			// ���
	wstring sVersion;			// �o�[�W����
	wstring sUrl;				// �z�zURL
	tstring sBaseDir;
	tstring sOptionDir;
	tstring sLangName;		// ���ꖼ
	PluginOption::Array options;		// �I�v�V����
	std::vector<std::wstring> aStrings;	// ������
private:
	bool bLoaded;
protected:
	Plug::Array plugs;
	int nCommandCount;

	// �������
public:
	virtual bool InvokePlug(EditView& view, Plug& plug, WSHIfObj::List& param) = 0;			// �v���O�����s����
	virtual bool ReadPluginDef(DataProfile& profile, DataProfile* profileMlang) = 0;		// �v���O�C����`�t�@�C����ǂݍ���
	virtual bool ReadPluginOption(DataProfile& profile) = 0;									// �I�v�V�����t�@�C����ǂݍ���
};

