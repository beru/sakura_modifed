/*!	@file
	@brief WSH Handler
*/
#pragma once

#include <ActivScp.h>
// ��Microsoft Platform SDK ���
#include "macro/IfObj.h"

/* 2009.10.29 syat �C���^�t�F�[�X�I�u�W�F�N�g������WSHIfObj.h�ɕ���
template <class Base>
class ImplementsIUnknown: public Base

class InterfaceObject: public ImplementsIUnknown<IDispatch>
 */
typedef void (*ScriptErrorHandler)(BSTR Description, BSTR Source, void *Data);

class WSHClient : IWSHClient {
public:
	// �^��`
	typedef std::vector<IfObj*> List;      // ���L���Ă���C���^�t�F�[�X�I�u�W�F�N�g�̃��X�g
	typedef List::const_iterator ListIter;	// ���̃C�e���[�^

	// �R���X�g���N�^�E�f�X�g���N�^
	WSHClient(const wchar_t* AEngine, ScriptErrorHandler AErrorHandler, void* AData);
	~WSHClient();

	// �t�B�[���h�E�A�N�Z�T
	ScriptErrorHandler onError;
	void* data;
	bool isValid; ///< true�̏ꍇ�X�N���v�g�G���W�����g�p�\�Bfalse�ɂȂ�ꍇ�� ScriptErrorHandler�ɃG���[���e���ʒm����Ă���B
	virtual /*override*/ void* GetData() const { return this->data; }
	const List& GetInterfaceObjects() {	return this->ifObjArr; }

	// ����
	void AddInterfaceObject(IfObj* obj);
	bool Execute(const wchar_t* AScript);
	void Error(BSTR Description, BSTR Source); ///< ScriptErrorHandler���Ăяo���B
	void Error(const wchar_t* Description);          ///< ScriptErrorHandler���Ăяo���B

private:
	IActiveScript* engine;
	List ifObjArr;
};

