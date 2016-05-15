/*!	@file
	@brief �L�[�{�[�h�}�N��

	Macro�̃C���X�^���X�ЂƂ��A1�R�}���h�ɂȂ�B

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, �S

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

#include <Windows.h>
#include <ObjIdl.h>  // VARIANT��
#include "func/Funccode.h"

class TextOutputStream;
class EditView;

enum class MacroParamType {
	Null,
	Int,
	Str,
};

struct MacroParam {
	WCHAR*			pData;
	MacroParam*		pNext;
	int				nDataLen;
	MacroParamType type;

	MacroParam():pData(NULL), pNext(NULL), nDataLen(0), type(MacroParamType::Null){}
	MacroParam( const MacroParam& obj ){
		if (obj.pData) {
			pData = new WCHAR[obj.nDataLen + 1];
		}else {
			pData = NULL;
		}
		pNext = NULL;
		nDataLen = obj.nDataLen;
		type = obj.type;
	}
	~MacroParam(){
		Clear();
	}
	void Clear(){
		delete[] pData;
		pData = NULL;
		nDataLen = 0;
		type = MacroParamType::Null;
	}
	void SetStringParam( const WCHAR* szParam, int nLength = -1 );
	void SetStringParam( const ACHAR* lParam ){ SetStringParam(to_wchar(lParam)); }
	void SetIntParam( const int nParam );
};
/*! @brief �L�[�{�[�h�}�N����1�R�}���h

	���������X�g�\���ɂ��āA�����ł����Ă�悤�ɂ��Ă݂܂����B
	�X�^�b�N�ɂ���̂��ʗ�Ȃ̂�������܂���i�悭�킩��܂���j�B
	
	����A����\���������Ă�����Ȃ��悤�ɂ��悤�Ǝv�����̂ł����A���܂��܂����B
	
	���āA���̃N���X�͎��̂悤�ȑO��œ��삵�Ă���B

	@li �����̃��X�g���ApParamTop����̃��X�g�\���ŕێ��B
	@li ������V���ɒǉ�����ɂ́AAddParam()���g�p����B
	  AddParam�ɂǂ�Ȓl���n����Ă��悢�悤�ɏ�������R�g�B
	  �n���ꂽ�l�����l�Ȃ̂��A������ւ̃|�C���^�Ȃ̂��́AnFuncID�i�@�\ ID�j�ɂ���āA���̃N���X���Ŕ��ʂ��A��낵����邱�ƁB
	@li �����́AMacro�����ł͂��ׂĕ�����ŕێ����邱�Ɓi���l97�́A"97"�Ƃ��ĕێ��j�i���܂̂Ƃ���j
*/
class Macro {
public:
	/*
	||  Constructors
	*/
	Macro(EFunctionCode nFuncID);	// �@�\ID���w�肵�ď�����
	~Macro();
	void ClearMacroParam();

	void SetNext(Macro* pNext) { pNext = pNext; }
	Macro* GetNext() { return pNext; }
	// 2007.07.20 genta : flags�ǉ�
	bool Exec(EditView& editView, int flags) const; // 2007.09.30 kobake const�ǉ�
	void Save(HINSTANCE hInstance, TextOutputStream& out) const; // 2007.09.30 kobake const�ǉ�
	
	void AddLParam(const LPARAM* lParam, const EditView& editView );	//@@@ 2002.2.2 YAZAKI pEditView���n��
	void AddStringParam( const WCHAR* szParam, int nLength = -1 );
	void AddStringParam(const ACHAR* lParam) { return AddStringParam(to_wchar(lParam)); }
	void AddIntParam( const int nParam );
	int GetParamCount() const;

	static bool HandleCommand(EditView& view, EFunctionCode index, const WCHAR* arguments[], const int argLengths[], const int argSize);
	static bool HandleFunction(EditView& view, EFunctionCode index, const VARIANT* argumentss, const int argSize, VARIANT& result);
	// 2009.10.29 syat HandleCommand��HandleFunction�̈������������낦��
#if 0
	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID(HINSTANCE , int , char* , char*);	// �@�\ID���֐����C�@�\�����{��
	static int GetFuncInfoByName(HINSTANCE , const char* , char*);	// �֐������@�\ID�C�@�\�����{��
	static bool CanFuncIsKeyMacro(int);	// �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ�
#endif

protected:
	static WCHAR* GetParamAt(MacroParam*, int);

	/*
	||  �����w���p�֐�
	*/
	EFunctionCode	nFuncID;		// �@�\ID
	MacroParam*		pParamTop;	// �p�����[�^
	MacroParam*		pParamBot;
	Macro*			pNext;		// ���̃}�N���ւ̃|�C���^
};

