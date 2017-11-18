/*!	@file
	@brief PPA Library Handler

	PPA.DLL�𗘗p���邽�߂̃C���^�[�t�F�[�X
*/

#include "StdAfx.h"
#include "PPA.h"
#include "view/EditView.h"
#include "func/Funccode.h"
#include "Macro.h"
#include "macro/SMacroMgr.h"// 2002/2/10 aroka
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "_os/OleTypes.h"

#define NEVER_USED_PARAM(p) ((void)p)

// 2003.06.01 Moca
#define omGet (0)
#define omSet (1)

// 2007.07.26 genta
PPA::PpaExecInfo* PPA::curInstance = NULL;
bool PPA::bIsRunning = false;

PPA::PPA()
{
}

PPA::~PPA()
{
}

// @date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
bool PPA::Execute(EditView& editView, int flags)
{
	// PPA�̑��d�N���֎~ 2008.10.22 syat
	if (PPA::bIsRunning) {
		MYMESSAGEBOX(editView.GetHwnd(), MB_OK, LS(STR_ERR_DLGPPA7), LS(STR_ERR_DLGPPA1));
		fnAbort();
		PPA::bIsRunning = false;
		return false;
	}
	PPA::bIsRunning = true;

	PpaExecInfo info;
	info.pEditView = &editView;
	info.pShareData = &GetDllShareData();
	info.bError = false;			// 2003.06.01 Moca
	info.memDebug.SetString("");	// 2003.06.01 Moca
	info.commandflags = flags | FA_FROMMACRO;	// 2007.07.22 genta
	
	// ���s�O�ɃC���X�^���X��Ҕ�����
	PpaExecInfo* old_instance = curInstance;
	curInstance = &info;
	fnExecute();
	
	// �}�N�����s������͂����ɖ߂��Ă���
	curInstance = old_instance;

	// PPA�̑��d�N���֎~ 2008.10.22 syat
	PPA::bIsRunning = false;
	return !info.bError;
}

LPCTSTR PPA::GetDllNameImp(int nIndex)
{
	return _T("PPA.DLL");
}

/*!
	DLL�̏�����

	�֐��̃A�h���X���擾���ă����o�ɕۊǂ���D

	@retval 0 ����
	@retval 1 �A�h���X�擾�Ɏ��s
*/
bool PPA::InitDllImp()
{
	// PPA.DLL�������Ă���֐�������

	// pr. 15, 2002 genta const��t����
	// �A�h���X�̓���ꏊ�̓I�u�W�F�N�g�Ɉˑ�����̂�
	// tatic�z��ɂ͂ł��Ȃ��B
	const ImportTable table[] = {
		{ &fnExecute,		"Execute" },
		{ &fnSetDeclare,	"SetDeclare" },
		{ &fnSetSource,	"SetSource" },
		{ &fnSetDefProc,	"SetDefProc" },
		{ &fnSetDefine,	"SetDefine" },
		{ &fnSetIntFunc,	"SetIntFunc" },
		{ &fnSetStrFunc,	"SetStrFunc" },
		{ &fnSetProc,		"SetProc" },
		{ &fnSetErrProc,	"SetErrProc" },
		{ &fnAbort,		"ppaAbort" },
		{ &fnGetVersion,	"GetVersion" },
		{ &fnDeleteVar,	"DeleteVar" },
		{ &fnGetArgInt,	"GetArgInt" },
		{ &fnGetArgStr,	"GetArgStr" },
		{ &fnGetArgBStr,	"GetArgBStr" },
		{ &fnGetIntVar,	"GetIntVar" },
		{ &fnGetStrVar,	"GetStrVar" },
		{ &fnGetBStrVar,	"GetBStrVar" },
		{ &fnSetIntVar,	"SetIntVar" },
		{ &fnSetStrVar,	"SetStrVar" },
		{ &fnAddIntObj,	"AddIntObj" },
		{ &fnAddStrObj,	"AddStrObj" },
		{ &fnAddIntVar,	"AddIntVar" },
		{ &fnAddStrVar,	"AddStrVar" },
		{ &fnSetIntObj,	"SetIntObj" },
		{ &fnSetStrObj,	"SetStrObj" },

#if PPADLL_VER >= 120
		{ &fnAddRealVar,	"AddRealVar" },
		{ &fnSetRealObj,	"SetRealObj" },
		{ &fnAddRealObj,	"AddRealObj" },
		{ &fnGetRealVar,	"GetRealVar" },
		{ &fnSetRealVar,	"SetRealVar" },
		{ &fnSetRealFunc,	"SetRealFunc" },
		{ &fnGetArgReal,	"GetArgReal" },
#endif

#if PPADLL_VER >= 123
		{ &fnIsRunning, "IsRunning" },
		{ &fnSetFinishProc, "SetFinishProc"}, // 2003.06.23 Moca
#endif

		{ NULL, 0 }
	};

	// pr. 15, 2002 genta
	// DllImp�̋��ʊ֐�������
	if (! RegisterEntries(table)) {
		return false;
	}

	SetIntFunc((void *)PPA::stdIntFunc);	// 2003.02.24 Moca
	SetStrFunc((void *)PPA::stdStrFunc);
	SetProc((void *)PPA::stdProc);

	// 2003.06.01 Moca �G���[���b�Z�[�W��ǉ�
	SetErrProc((void *)PPA::stdError);
	SetStrObj((void *)PPA::stdStrObj);	// UserErrorMes�p
#if PPADLL_VER >= 123
	SetFinishProc((void *)PPA::stdFinishProc);
#endif

	SetDefine("sakura-editor");	// 2003.06.01 Moca SAKURA�G�f�B�^�p�Ǝ��֐�������
	AddStrObj("UserErrorMes", "", FALSE, 2); // 2003.06.01 �f�o�b�O�p������ϐ���p��

	// Jun. 16, 2003 genta �ꎞ��ƃG���A
	char buf[1024];
	// �R�}���h�ɒu���������Ȃ��֐� �� PPA�����ł͎g���Ȃ��B�B�B
	for (int i=0; SMacroMgr::macroFuncInfoArr[i].pszFuncName; ++i) {
		// 2003.06.08 Moca �������[���[�N�̏C��
		// 2003.06.16 genta �o�b�t�@���O����^����悤��
		// �֐��o�^�p��������쐬����
		GetDeclarations(SMacroMgr::macroFuncInfoArr[i], buf);
		SetDefProc(buf);
	}

	// �R�}���h�ɒu����������֐� �� PPA�����ł��g����B
	for (int i=0; SMacroMgr::macroFuncInfoCommandArr[i].pszFuncName; ++i) {
		// 2003.06.08 Moca �������[���[�N�̏C��
		// 2003.06.16 genta �o�b�t�@���O����^����悤��
		// �֐��o�^�p��������쐬����
		GetDeclarations(SMacroMgr::macroFuncInfoCommandArr[i], buf);
		SetDefProc(buf);
	}
	return true; 
}

/*! PPA�Ɋ֐���o�^���邽�߂̕�������쐬����

	@param macroFuncInfo [in]	�}�N���f�[�^
	@param szBuffer [out]		�������������������o�b�t�@�ւ̃|�C���^

	@note �o�b�t�@�T�C�Y�� 9 + 3 + ���\�b�h���̒��� + 13 * 4 + 9 + 5 �͍Œ�K�v

	@date 2003.06.01 Moca
				�X�^�e�B�b�N�����o�ɕύX
				macroFuncInfo.pszData�����������Ȃ��悤�ɕύX

	@date 2003.06.16 genta ���ʂ�new/delete������邽�߃o�b�t�@���O����^����悤��
*/
char* PPA::GetDeclarations( const MacroFuncInfo& macroFuncInfo, char* szBuffer )
{
	char szType[20];	// procedure/function�p�o�b�t�@
	char szReturn[20];	// �߂�l�^�p�o�b�t�@
	if (macroFuncInfo.varResult == VT_EMPTY) {
		strcpy(szType, "procedure");
		szReturn[0] = '\0';
	}else {
		strcpy(szType, "function");
		if (macroFuncInfo.varResult == VT_BSTR) {
			strcpy(szReturn, ": string");
		}else if (macroFuncInfo.varResult == VT_I4) {
			strcpy(szReturn, ": Integer");
		}else {
			szReturn[0] = '\0';
		}
	}
	
	char szArguments[8][20]; // �����p�o�b�t�@
	int i;
	for (i=0; i<8; ++i) {
		VARTYPE type = VT_EMPTY;
		if (i < 4) {
			type = macroFuncInfo.varArguments[i];
		}else {
			if (macroFuncInfo.pData && i < macroFuncInfo.pData->nArgMinSize) {
				type = macroFuncInfo.pData->pVarArgEx[i - 4];
			}
		}
		if (type == VT_EMPTY) {
			break;
		}
		if (type == VT_BSTR) {
			strcpy(szArguments[i], "s0: string");
			szArguments[i][1] = '0' + (char)i;
		}else if (type == VT_I4) {
			strcpy(szArguments[i], "i0: Integer");
			szArguments[i][1] = '0' + (char)i;
		}else {
			strcpy(szArguments[i], "u0: Unknown");
		}
	}
	if (i > 0) {	// �������������Ƃ�
		char szArgument[8*20];
		// 2002.12.06 Moca �����s�������Cstrcat��VC6Pro�ł��܂������Ȃ��������߁Cstrcpy�ɂ��Ă݂��瓮����
		strcpy(szArgument, szArguments[0]);
		for (int j=1; j<i; ++j) {
			strcat(szArgument, "; ");
			strcat(szArgument, szArguments[j]);
		}
		auto_sprintf( szBuffer, "%hs S_%ls(%hs)%hs; index %d;",
			szType,
			macroFuncInfo.pszFuncName,
			szArgument,
			szReturn,
			macroFuncInfo.nFuncID
		);
	}else {
		auto_sprintf( szBuffer, "%hs S_%ls%hs; index %d;",
			szType,
			macroFuncInfo.pszFuncName,
			szReturn,
			macroFuncInfo.nFuncID
		);
	}
	// Jun. 01, 2003 Moca / Jun. 16, 2003 genta
	return szBuffer;
}

/*! ���[�U�[��`������^�I�u�W�F�N�g
	���݂́A�f�o�b�O�p�������ݒ肷��ׂ̂�
*/
void __stdcall PPA::stdStrObj(
	const char* ObjName,
	int index,
	BYTE GS_Mode,
	int* Err_CD,
	char** Value
	)
{
	NEVER_USED_PARAM(ObjName);
	*Err_CD = 0;
	switch (index) {
	case 2:
		switch (GS_Mode) {
		case omGet:
			*Value = curInstance->memDebug.GetStringPtr();
			break;
		case omSet:
			curInstance->memDebug.SetString(*Value);
			break;
		}
		break;
	default:
		*Err_CD = -1;
	}
}


/*! ���[�U�[��`�֐��̃G���[���b�Z�[�W�̍쐬

	stdProc, stdIntFunc, stdStrFunc ���G���[�R�[�h��Ԃ����ꍇ�APPA����Ăяo�����B
	�ُ�I��/�s���������̃G���[���b�Z�[�W��Ǝ��Ɏw�肷��B
	@author Moca
	@param Err_CD IN  0�ȊO�e�R�[���o�b�N�֐����ݒ肵���l
			 1�ȏ� FuncID + 1
			 0     PPA�̃G���[
			-1�ȉ� ���̑����[�U��`�G���[
	@param Err_Mes IN �G���[���b�Z�[�W

	@date 2003.06.01 Moca
*/
void __stdcall PPA::stdError(int Err_CD, const char* Err_Mes)
{
	if (curInstance->bError) {
		return;
	}
	curInstance->bError = true; // �֐����Ŋ֐����Ăԏꍇ���A2��\�������̂�h��

	TCHAR szMes[2048]; // 2048����Α���邩��
	const TCHAR* pszErr;
	pszErr = szMes;
	if (0 < Err_CD) {
		int i, FuncID;
		FuncID = Err_CD - 1;
		char szFuncDec[1024];
		szFuncDec[0] = '\0';
		for (i=0; SMacroMgr::macroFuncInfoCommandArr[i].nFuncID!=-1; ++i) {
			if (SMacroMgr::macroFuncInfoCommandArr[i].nFuncID == FuncID) {
				GetDeclarations(SMacroMgr::macroFuncInfoCommandArr[i], szFuncDec);
				break;
			}
		}
		if (SMacroMgr::macroFuncInfoArr[i].nFuncID != -1) {
			for (i=0; SMacroMgr::macroFuncInfoArr[i].nFuncID!=-1; ++i) {
				if (SMacroMgr::macroFuncInfoArr[i].nFuncID == FuncID) {
					GetDeclarations(SMacroMgr::macroFuncInfoArr[i], szFuncDec);
					break;
				}
			}
		}
		if (szFuncDec[0] != '\0') {
			auto_sprintf( szMes, LS(STR_ERR_DLGPPA2), szFuncDec );
		}else {
			auto_sprintf( szMes, LS(STR_ERR_DLGPPA3), FuncID );
		}
	}else {
		// 2007.07.26 genta : �l�X�g���s�����ꍇ��PPA���s���ȃ|�C���^��n���\�����l���D
		// ���ۂɂ͕s���ȃG���[�͑S��PPA.DLL�����Ńg���b�v�����悤�����O�̂��߁D
		if (IsBadStringPtrA(Err_Mes, 256)) {
			pszErr = LS(STR_ERR_DLGPPA6);
		}else {
			switch (Err_CD) {
			case 0:
				if ('\0' == Err_Mes[0]) {
					pszErr = LS(STR_ERR_DLGPPA4);
				}else {
					pszErr = to_tchar(Err_Mes);
				}
				break;
			default:
				auto_sprintf( szMes, LS(STR_ERR_DLGPPA5), Err_CD, to_tchar(Err_Mes) );
			}
		}
	}
	if (curInstance->memDebug.GetStringLength() == 0) {
		MYMESSAGEBOX(curInstance->pEditView->GetHwnd(), MB_OK, LS(STR_ERR_DLGPPA7), _T("%ts"), pszErr);
	}else {
		MYMESSAGEBOX(curInstance->pEditView->GetHwnd(), MB_OK, LS(STR_ERR_DLGPPA7), _T("%ts\n%hs"), pszErr, curInstance->memDebug.GetStringPtr());
	}
}

//----------------------------------------------------------------------
/** �v���V�[�W�����scallback

	@date 2007.07.20 genta index�ƈꏏ�Ƀt���O��n��
*/
void __stdcall PPA::stdProc(
	const char*	funcName,
	const int	index,
	const char*	args[],
	const int	numArgs,
	int*		err_CD
	)
{
	NEVER_USED_PARAM(funcName);
	EFunctionCode eIndex = (EFunctionCode)index;

	*err_CD = 0;

	// Argument��wchar_t[]�ɕϊ� -> tmpArguments
	wchar_t** tmpArguments2 = new wchar_t*[numArgs];
	int* tmpArgLengths = new int[numArgs];
	for (int i=0; i<numArgs; ++i) {
		if (args[i]) {
			tmpArguments2[i] = mbstowcs_new(args[i]);
			tmpArgLengths[i] = wcslen(tmpArguments2[i]);
		}else {
			tmpArguments2[i] = NULL;
			tmpArgLengths[i] = 0;
		}
	}
	const wchar_t** tmpArguments = (const wchar_t**)tmpArguments2;

	// ����
	bool bRet = Macro::HandleCommand(
		*curInstance->pEditView,
		(EFunctionCode)(eIndex | curInstance->commandflags),
		tmpArguments,
		tmpArgLengths,
		numArgs
	);
	if (!bRet) {
		*err_CD = eIndex + 1;
	}

	// tmpArguments�����
	for (int i=0; i<numArgs; ++i) {
		if (tmpArguments2[i]) {
			wchar_t* p = const_cast<wchar_t*>(tmpArguments2[i]);
			delete[] p;
		}
	}
	delete[] tmpArguments2;
	delete[] tmpArgLengths;
}

//----------------------------------------------------------------------
/*!
	�����l��Ԃ��֐�����������

	PPA����Ăт������
	@author Moca
	@date 2003.02.24 Moca
*/
void __stdcall PPA::stdIntFunc(
	const char* FuncName,
	const int index,
	const char* Argument[],
	const int ArgSize,
	int* Err_CD,
	int* ResultValue
	)
{
	NEVER_USED_PARAM(FuncName);
	VARIANT Ret;
	::VariantInit(&Ret);

	*ResultValue = 0;
	*Err_CD = 0;
	if (false != CallHandleFunction(index, Argument, ArgSize, &Ret)) {
		switch (Ret.vt) {
		case VT_I4:
			*ResultValue = Ret.lVal;
			break;
		case VT_INT:
			*ResultValue = Ret.intVal;
			break;
		case VT_UINT:
			*ResultValue = Ret.uintVal;
			break;
		default:
			*Err_CD = -2; // 2003.06.01 Moca �l�ύX
		}
		::VariantClear(&Ret);
		return;
	}
	*Err_CD = index + 1; // 2003.06.01 Moca
	::VariantClear(&Ret);
	return;
}

//----------------------------------------------------------------------
/*!
	�������Ԃ��֐�����������

	PPA����Ăт������
	@date 2003.02.24 Moca CallHandleFunction�Ή�
*/
void __stdcall PPA::stdStrFunc(
	const char* FuncName,
	const int index,
	const char* Argument[],
	const int ArgSize,
	int* Err_CD,
	char** ResultValue
	)
{
	NEVER_USED_PARAM(FuncName);

	VARIANT Ret;
	::VariantInit(&Ret);
	*Err_CD = 0;
	if (false != CallHandleFunction(index, Argument, ArgSize, &Ret)) {
		if (Ret.vt == VT_BSTR) {
			int len;
			char* buf;
			Wrap(&Ret.bstrVal)->Get(&buf, &len);
			curInstance->memRet.SetString(buf, len); // Mar. 9, 2003 genta
			delete[] buf;
			*ResultValue = curInstance->memRet.GetStringPtr();
			::VariantClear(&Ret);
			return;
		}
	}
	::VariantClear(&Ret);
	*Err_CD = index + 1;
	*ResultValue = const_cast<char*>("");
	return;
}

/*!
	�����^�ϊ�

	������ŗ^����ꂽ������VARIANT/BSTR�ɕϊ�����Macro::HandleFunction()���Ăт���
	@author Moca
*/
bool PPA::CallHandleFunction(
	const int index,
	const char* args[],
	int argSize,
	VARIANT* result
	)
{
	int argCnt;
	const int maxArgSize = 8;
	VARIANT vtArg[maxArgSize];
	
	const MacroFuncInfo* mfi = SMacroMgr::GetFuncInfoByID(index);
	for (int i=0; i<maxArgSize && i<argSize; ++i) {
		::VariantInit(&vtArg[i]);
	}
	argCnt = 0;
	for (int i=0, ArgCnt=0; i<maxArgSize && i<argSize; ++i) {
		VARTYPE type = VT_EMPTY;
		if (i < 4) {
			type = mfi->varArguments[i];
		}else {
			if (mfi->pData && i < mfi->pData->nArgMinSize) {
				type = mfi->pData->pVarArgEx[i - 4];
			}
		}
		if (type == VT_EMPTY) {
			break;
		}
		switch (type) {
		case VT_I4:
		{
			vtArg[i].vt = VT_I4;
			vtArg[i].lVal = atoi(args[i]);
			break;
		}
		case VT_BSTR:
		{
			SysString s(args[i], lstrlenA(args[i]));
			Wrap(&vtArg[i])->Receive(s);
			break;
		}
		default:
			for (int i=0; i<maxArgSize && i<argSize; ++i) {
				::VariantClear(&vtArg[i]);
			}
			return false;
		}
		++ArgCnt;
	}

	if (index >= F_FUNCTION_FIRST) {
		bool Ret = Macro::HandleFunction(
			*curInstance->pEditView,
			(EFunctionCode)index,
			vtArg,
			argCnt,
			*result
			);
		for (int i=0; i<maxArgSize && i<argSize; ++i) {
			::VariantClear(&vtArg[i]);
		}
		return Ret;
	}else {
		for (int i=0; i<maxArgSize && i<argSize; ++i) {
			::VariantClear(&vtArg[i]);
		}
		return false;
	}
}

#if PPADLL_VER >= 123

/*!
	PPA�}�N���̎��s�I�����ɌĂ΂��
	
	@date 2003.06.01 Moca
*/
void __stdcall PPA::stdFinishProc()
{
	// 2007.07.26 genta : �I�������͕s�v
}

#endif

