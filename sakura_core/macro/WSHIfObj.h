/*!	@file
	@brief WSH�C���^�t�F�[�X�I�u�W�F�N�g��{�N���X
*/
#pragma once

#include <list>
#include <ActivScp.h>
#include "_os/OleTypes.h"
#include "macro/IfObj.h"
#include "macro/WSH.h" // CWSHClient::List, ListIter
#include "macro/SMacroMgr.h" // MacroFuncInfo, MacroFuncInfoArray
class EditView;

/* WSHIfObj - �v���O�C����}�N���Ɍ��J����I�u�W�F�N�g
 * �g�p��̒���:
 *   1. ������new�ŁB
 *      �Q�ƃJ�E���^�����̂ŁA�����ϐ��Ő�������ƃX�R�[�v�����ĉ�������Ƃ��Ƀq�[�v�G���[���o�܂��B
 *   2. ����������AddRef()�A�s�v�ɂȂ�����Release()���ĂԂ��ƁB
 *   3. �V����IfObj����鎞��WSHIfObj���p�����A�ȉ���4���I�[�o�[���C�h���邱�ƁB
 *      GetMacroCommandInfo, GetMacroFuncInfo, HandleCommand, HandleFunction
 */
class WSHIfObj : public IfObj {
public:
	// �^��`
	typedef std::list<WSHIfObj*> List;
	typedef List::const_iterator ListIter;

	// �R���X�g���N�^
	WSHIfObj(const wchar_t* name, bool isGlobal)
		:
		IfObj(name, isGlobal)
	{
	}

	virtual void ReadyMethods(EditView& view, int flags);

protected:
	// ����
	//	2007.07.20 genta : flags�ǉ�
	//  2009.09.05 syat CWSHManager����ړ�
	void ReadyCommands(MacroFuncInfo* info, int flags);
	HRESULT MacroCommand(int index, DISPPARAMS* arguments, VARIANT* result, void* data);

	// �������
	virtual bool HandleFunction(EditView& view, EFunctionCode index, const VARIANT* arguments, const int argSize, VARIANT& result) = 0;		// �֐�����������
	virtual bool HandleCommand(EditView& view, EFunctionCode index, const wchar_t* arguments[], const int argLengths[], const int argSize) = 0;	// �R�}���h����������
	virtual MacroFuncInfoArray GetMacroCommandInfo() const = 0;	// �R�}���h�����擾����
	virtual MacroFuncInfoArray GetMacroFuncInfo() const = 0;	// �֐������擾����

	EditView* pView;
};

