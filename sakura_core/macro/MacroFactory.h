/*!	@file
	@brief �}�N����ʊǗ�
*/

#pragma once

#include <map>
#include <list>
#include <string>
#include "util/design_template.h"

class MacroManagerBase;

/*!
	@brief �}�N��Handler�����N���X
	
	@par ������
	MacroManagerBase::Declare() �ɂ��CMacroEngine��Creater�̓o�^
	RegisterEngine() �y�� �Ή��g���q�̓o�^ RegisterExt() ���Ăяo�����D
	
	@par �Ăяo��
	MacroFactory::Create()���g���q�������ɂ��ČĂяo���ƑΉ�����
	�}�N���G���W�����Ԃ����D����ꂽEngine�ɑ΂���LoadKeyMacro()�y��
	ExecKeyMacro() ���Ăяo�����ƂŃ}�N���̓ǂݍ��݁E���s���s����D

	Singleton
*/
class MacroFactory : public TSingleton<MacroFactory> {
	friend class TSingleton<MacroFactory>;
	MacroFactory();

public:
	typedef MacroManagerBase* (*Creator)(class EditView& view, const TCHAR*);

	bool RegisterCreator(Creator);
	// Jan. 31, 2004 genta
	// �o�C�i���T�C�Y�팸�̂���mMacroExts���폜
	//bool RegisterExt(const char*, Creator);
	bool Unregister(Creator);

	MacroManagerBase* Create(class EditView& view, const TCHAR* ext);

private:
	std::tstring Ext2Key(const TCHAR* ext);

	// Jan. 31, 2004 genta
	// �o�C�i���T�C�Y�팸�̂��ߊg���q�ێ��pmap���폜
	// typedef std::map<std::string, Creator> MacroTypeRep;
	typedef std::list<Creator> MacroEngineRep;

	// Jan. 31, 2004 genta
	// �o�C�i���T�C�Y�팸�̂���
	//MacroTypeRep mMacroExts;	// �g���q�Ή��\
	/*!
		Creator���X�g
		@date 2002.08.25 genta �ǉ�
	*/
	MacroEngineRep mMacroCreators;

};

