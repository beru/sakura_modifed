/*!	@file
	@brief �L�[�{�[�h�}�N��

	Macro�̃C���X�^���X�ЂƂ��A1�R�}���h�ɂȂ�B
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
	wchar_t*		pData;
	MacroParam*		pNext;
	size_t			nDataLen;
	MacroParamType type;

	MacroParam():pData(NULL), pNext(NULL), nDataLen(0), type(MacroParamType::Null){}
	MacroParam( const MacroParam& obj ){
		if (obj.pData) {
			pData = new wchar_t[obj.nDataLen + 1];
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
	void SetStringParam( const wchar_t* szParam, int nLength = -1 );
	void SetStringParam( const char* lParam ){ SetStringParam(to_wchar(lParam)); }
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

	void SetNext(Macro* pNext) { this->pNext = pNext; }
	Macro* GetNext() { return pNext; }
	bool Exec(EditView& editView, int flags) const;
	void Save(HINSTANCE hInstance, TextOutputStream& out) const;
	
	void AddLParam(const LPARAM* lParam, const EditView& editView );
	void AddStringParam( const wchar_t* szParam, int nLength = -1 );
	void AddStringParam(const char* lParam) { return AddStringParam(to_wchar(lParam)); }
	void AddIntParam( const int nParam );
	int GetParamCount() const;

	static bool HandleCommand(EditView& view, EFunctionCode index, const wchar_t* arguments[], const int argLengths[], const int argSize);
	static bool HandleFunction(EditView& view, EFunctionCode index, const VARIANT* argumentss, const int argSize, VARIANT& result);
#if 0
	/*
	||  Attributes & Operations
	*/
	static char* GetFuncInfoByID(HINSTANCE , int , char* , char*);	// �@�\ID���֐����C�@�\�����{��
	static int GetFuncInfoByName(HINSTANCE , const char* , char*);	// �֐������@�\ID�C�@�\�����{��
	static bool CanFuncIsKeyMacro(int);	// �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ�
#endif

protected:
	static wchar_t* GetParamAt(MacroParam*, int);

	/*
	||  �����w���p�֐�
	*/
	EFunctionCode	nFuncID;		// �@�\ID
	MacroParam*		pParamTop;	// �p�����[�^
	MacroParam*		pParamBot;
	Macro*			pNext;		// ���̃}�N���ւ̃|�C���^
};

