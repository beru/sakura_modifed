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
	bool Unregister(Creator);

	MacroManagerBase* Create(class EditView& view, const TCHAR* ext);

private:
	std::tstring Ext2Key(const TCHAR* ext);

	typedef std::list<Creator> MacroEngineRep;

	MacroEngineRep mMacroCreators;

};

