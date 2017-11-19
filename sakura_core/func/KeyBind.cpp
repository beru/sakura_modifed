/*!	@file
	@brief �L�[���蓖�ĂɊւ���N���X
*/
#include "StdAfx.h"
#include "func/KeyBind.h"
#include "env/ShareData.h"
#include "macro/SMacroMgr.h"

// KeyData�Ƃقړ���
struct KEYDATAINIT {
	short nKeyCode;			// Key Code (0 for non-keybord button)
	union {
		const TCHAR* pszKeyName;	// Key Name (for display)
		UINT nKeyNameId;			// String Resource Id (0x0000 - 0xFFFF)
	};
	EFunctionCode	nFuncCodeArr[8];	// Key Function Number
//					nFuncCodeArr[0]	//                      Key
//					nFuncCodeArr[1]	// Shift +              Key
//					nFuncCodeArr[2]	//         Ctrl +       Key
//					nFuncCodeArr[3]	// Shift + Ctrl +       Key
//					nFuncCodeArr[4]	//                Alt + Key
//					nFuncCodeArr[5]	// Shift +        Alt + Key
//					nFuncCodeArr[6]	//         Ctrl + Alt + Key
//					nFuncCodeArr[7]	// Shift + Ctrl + Alt + Key
};

// �����⏕
// KeyData�z��Ƀf�[�^���Z�b�g
static void SetKeyNameArrVal(
	DllSharedData&		shareData,
	int					nIdx,
	const KEYDATAINIT*	pKeydata
);

KeyBind::KeyBind()
{
}

KeyBind::~KeyBind()
{
}

/*! Windows �A�N�Z�����[�^�̍쐬 */
HACCEL KeyBind::CreateAccerelator(
	int			nKeyNameArrNum,
	KeyData*	pKeyNameArr
	)
{
	// �@�\�����蓖�Ă��Ă���L�[�̐����J�E���g -> nAccelArrNum
	int nAccelArrNum = 0;
	for (int i=0; i<nKeyNameArrNum; ++i) {
		auto& keyName = pKeyNameArr[i];
		if (keyName.nKeyCode != 0) {
			for (int j=0; j<8; ++j) {
				if (GetFuncCodeAt(keyName, j) != 0) {
					++nAccelArrNum;
				}
			}
		}
	}

	if (nAccelArrNum <= 0) {
		// �@�\���蓖�Ă��[��
		return NULL;
	}
	std::vector<ACCEL> accels(nAccelArrNum);
	ACCEL* pAccelArr = &accels[0];
	int k = 0;
	for (int i=0; i<nKeyNameArrNum; ++i) {
		auto& keyName = pKeyNameArr[i];
		if (keyName.nKeyCode != 0) {
			for (int j=0; j<8; ++j) {
				if (GetFuncCodeAt(keyName, j) != 0) {
					auto& accel = pAccelArr[k];
					accel.fVirt = FNOINVERT | FVIRTKEY;
					accel.fVirt |= (j & _SHIFT) ? FSHIFT   : 0;
					accel.fVirt |= (j & _CTRL ) ? FCONTROL : 0;
					accel.fVirt |= (j & _ALT  ) ? FALT     : 0;
					accel.key = keyName.nKeyCode;
					accel.cmd = keyName.nKeyCode | (((WORD)j)<<8) ;

					++k;
				}
			}
		}
	}
	HACCEL hAccel = ::CreateAcceleratorTable(pAccelArr, nAccelArrNum);
	return hAccel;
}


/*! �A�N���Z���[�^���ʎq�ɑΉ�����R�}���h���ʎq��Ԃ��D
	�Ή�����A�N���Z���[�^���ʎq���Ȃ��ꍇ�܂��͋@�\�����蓖�Ă̏ꍇ��0��Ԃ��D
*/
EFunctionCode KeyBind::GetFuncCode(
	WORD		nAccelCmd,
	int			nKeyNameArrNum,
	KeyData*	pKeyNameArr,
	bool		bGetDefFuncCode // = true
	)
{
	int nCmd = (int)LOBYTE(nAccelCmd);
	int nSts = (int)HIBYTE(nAccelCmd);
	if (nCmd == 0) { // mouse command
		for (int i=0; i<nKeyNameArrNum; ++i) {
			auto& keyName = pKeyNameArr[i];
			if (nCmd == keyName.nKeyCode) {
				return GetFuncCodeAt(keyName, nSts, bGetDefFuncCode);
			}
		}
	}else {
		DllSharedData& shareData = GetDllShareData();
		return GetFuncCodeAt(pKeyNameArr[shareData.common.keyBind.keyToKeyNameArr[nCmd]], nSts, bGetDefFuncCode);
	}
	return F_DEFAULT;
}


/*!
	@return �@�\�����蓖�Ă��Ă���L�[�X�g���[�N�̐�
*/
int KeyBind::CreateKeyBindList(
	HINSTANCE		hInstance,		// [in] �C���X�^���X�n���h��
	int				nKeyNameArrNum,	// [in]
	KeyData*		pKeyNameArr,	// [out]
	NativeW&		memList,		//
	FuncLookup*		pFuncLookup,	// [in] �@�\�ԍ������O�̑Ή������
	bool			bGetDefFuncCode // [in] ON:�f�t�H���g�@�\���蓖�Ă��g��/OFF:�g��Ȃ� �f�t�H���g:true
	)
{
	wchar_t	szStr[256];
	wchar_t	szFuncName[256];
	wchar_t	szFuncNameJapanese[256];

	int nValidKeys = 0;
	memList.SetString(LTEXT(""));
	const wchar_t* pszSHIFT = LTEXT("Shift+");
	const wchar_t* pszCTRL  = LTEXT("Ctrl+");
	const wchar_t* pszALT   = LTEXT("Alt+");
	const wchar_t* pszTAB   = LTEXT("\t");
	const wchar_t* pszCR    = LTEXT("\r\n");	// \r=0x0d=CR��ǉ�

	memList.AppendString(LSW(STR_ERR_DLGKEYBIND1));
	memList.AppendString(pszCR);
	memList.AppendStringLiteral(LTEXT("-----\t-----\t-----\t-----\t-----"));
	memList.AppendString(pszCR);

	for (int j=0; j<8; ++j) {
		for (int i=0; i<nKeyNameArrNum; ++i) {
			int iFunc = GetFuncCodeAt(pKeyNameArr[i], j, bGetDefFuncCode);

			if (iFunc != 0) {
				++nValidKeys;
				if (j & _SHIFT) {
					memList.AppendString(pszSHIFT);
				}
				if (j & _CTRL) {
					memList.AppendString(pszCTRL);
				}
				if (j & _ALT) {
					memList.AppendString(pszALT);
				}
				memList.AppendString(to_wchar(pKeyNameArr[i].szKeyName));
				// Oct. 31, 2001 genta 
				if (!pFuncLookup->Funccode2Name(
					iFunc,
					szFuncNameJapanese, 255
					)
				) {
					auto_strcpy(szFuncNameJapanese, LSW(STR_ERR_DLGKEYBIND2));
				}
				szFuncName[0] = LTEXT('\0'); /*"---unknown()--"*/

//				// �@�\�����{��
//				::LoadString(
//					hInstance,
//					pKeyNameArr[i].nFuncCodeArr[j],
//					 szFuncNameJapanese, 255
//				);
				memList.AppendString(pszTAB);
				memList.AppendString(szFuncNameJapanese);

				// �@�\ID���֐����C�@�\�����{��
				SMacroMgr::GetFuncInfoByID(
					hInstance,
					iFunc,
					szFuncName,
					szFuncNameJapanese
				);

				// �֐���
				memList.AppendString(pszTAB);
				memList.AppendString(szFuncName);

				// �@�\�ԍ�
				memList.AppendString(pszTAB);
				auto_sprintf(szStr, LTEXT("%d"), iFunc);
				memList.AppendString(szStr);

				// �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ�
				memList.AppendString(pszTAB);
				if (SMacroMgr::CanFuncIsKeyMacro(iFunc)) {
					memList.AppendStringLiteral(LTEXT("��"));
				}else {
					memList.AppendStringLiteral(LTEXT("�~"));
				}

				memList.AppendString(pszCR);
			}
		}
	}
	return nValidKeys;
}

/** �@�\�ɑΉ�����L�[���̃T�[�`(�⏕�֐�)

	�^����ꂽ�V�t�g��Ԃɑ΂��āC�w�肳�ꂽ�͈͂̃L�[�G���A����
	���Y�@�\�ɑΉ�����L�[�����邩�𒲂ׁC����������
	�Ή�����L�[��������Z�b�g����D
	
	�֐�����o��Ƃ��ɂ͌����J�n�ʒu(nKeyNameArrBegin)��
	���ɏ�������index��ݒ肷��D

	@param[in,out] nKeyNameArrBegin �����J�nINDEX (�I�����ɂ͎���̊J�nINDEX�ɏ�����������)
	@param[in] nKeyNameArrBegin �����I��INDEX + 1
	@param[in] pKeyNameArr �L�[�z��
	@param[in] nShiftState �V�t�g���
	@param[out] memList �L�[������ݒ��
	@param[in]	nFuncId �����Ώۋ@�\ID
	@param[in]	bGetDefFuncCode �W���@�\���擾���邩�ǂ���
*/
bool KeyBind::GetKeyStrSub(
	int&		nKeyNameArrBegin,
	int			nKeyNameArrEnd,
	KeyData*	pKeyNameArr,
	int			nShiftState,
	NativeT&	memList,
	int			nFuncId,
	bool		bGetDefFuncCode // = true
	)
{
	static const TCHAR*	pszSHIFT = _T("Shift+");
	static const TCHAR*	pszCTRL  = _T("Ctrl+");
	static const TCHAR*	pszALT   = _T("Alt+");

	int i;
	for (i=nKeyNameArrBegin; i<nKeyNameArrEnd; ++i) {
		auto& keyName = pKeyNameArr[i];
		if (nFuncId == GetFuncCodeAt(keyName, nShiftState, bGetDefFuncCode)) {
			if (nShiftState & _SHIFT) {
				memList.AppendString(pszSHIFT);
			}
			if (nShiftState & _CTRL) {
				memList.AppendString(pszCTRL);
			}
			if (nShiftState & _ALT) {
				memList.AppendString(pszALT);
			}
			memList.AppendString(keyName.szKeyName);
			nKeyNameArrBegin = i + 1;
			return true;
		}
	}
	nKeyNameArrBegin = i;
	return false;
}


/** �@�\�ɑΉ�����L�[���̎擾 */
int KeyBind::GetKeyStr(
	HINSTANCE	hInstance,
	int			nKeyNameArrNum,
	KeyData*	pKeyNameArr,
	NativeT&	memList,
	int			nFuncId,
	bool		bGetDefFuncCode // = true
	)
{
	memList.SetString(_T(""));

	// ��ɃL�[�����𒲍�����
	for (int j=0; j<8; ++j) {
		for (int i=(int)MouseFunctionType::KeyBegin; i<nKeyNameArrNum; /* 1�������Ă͂����Ȃ� */) {
			if (GetKeyStrSub(i, nKeyNameArrNum, pKeyNameArr, j, memList, nFuncId, bGetDefFuncCode)) {
				return 1;
			}
		}
	}

	// ��Ƀ}�E�X�����𒲍�����
	for (int j=0; j<8; ++j) {
		for (int i=0; i<(int)MouseFunctionType::KeyBegin; /* 1�������Ă͂����Ȃ� */) {
			if (GetKeyStrSub(i, nKeyNameArrNum, pKeyNameArr, j, memList, nFuncId, bGetDefFuncCode)) {
				return 1;
			}
		}
	}
	return 0;
}


/** �@�\�ɑΉ�����L�[���̎擾(����) */
int KeyBind::GetKeyStrList(
	HINSTANCE	hInstance,
	int			nKeyNameArrNum,
	KeyData*	pKeyNameArr,
	NativeT***	pppMemList,
	int			nFuncId,
	bool		bGetDefFuncCode // = true
	)
{
	int nAssignedKeysNum = 0;
	if (nFuncId == 0) {
		return 0;
	}
	for (int j=0; j<8; ++j) {
		for (int i=0; i<nKeyNameArrNum; ++i) {
			if (nFuncId == GetFuncCodeAt(pKeyNameArr[i], j, bGetDefFuncCode)) {
				++nAssignedKeysNum;
			}
		}
	}
	if (nAssignedKeysNum == 0) {
		return 0;
	}
	(*pppMemList) = new NativeT*[nAssignedKeysNum + 1];
	int i;
	for (i=0; i<nAssignedKeysNum; ++i) {
		(*pppMemList)[i] = new NativeT;
	}
	(*pppMemList)[i] = NULL;
	
	nAssignedKeysNum = 0;
	for (int j=0; j<8; ++j) {
		for (int i=0; i<nKeyNameArrNum; /* 1�������Ă͂����Ȃ� */) {
			if (GetKeyStrSub(
				i,
				nKeyNameArrNum,
				pKeyNameArr,
				j,
				*((*pppMemList)[nAssignedKeysNum]),
				nFuncId,
				bGetDefFuncCode
				)
			) {
				++nAssignedKeysNum;
			}
		}
	}
	return nAssignedKeysNum;
}


/*! �A�N�Z�X�L�[�t���̕�����̍쐬
	@param name ���x��
	@param sKey �A�N�Z�X�L�[
	@return �A�N�Z�X�L�[�t���̕�����
*/
TCHAR* KeyBind::MakeMenuLabel(const TCHAR* sName, const TCHAR* sKey)
{
	const int MAX_LABEL_CCH = _MAX_PATH * 2 + 30;
	static TCHAR sLabel[MAX_LABEL_CCH];
	const TCHAR* p;

	if (!sKey || sKey[0] == L'\0') {
		return const_cast<TCHAR*>(sName);
	}else {
		if (!GetDllShareData().common.mainMenu.bMainMenuKeyParentheses
			&& (
				((p = auto_strchr(sName, sKey[0])))
				|| ((p = auto_strchr(sName, _totlower(sKey[0]))))
			)
		) {
			// �������A�g�p���Ă��镶�����A�N�Z�X�L�[��
			auto_strcpy_s(sLabel, _countof(sLabel), sName);
			sLabel[p-sName] = _T('&');
			auto_strcpy_s(sLabel + (p-sName) + 1, _countof(sLabel), p);
		}else if (
			(p = auto_strchr(sName, _T('(')))
			&& (p = auto_strchr(p, sKey[0]))
		) {
			// (�t���̌�ɃA�N�Z�X�L�[
			auto_strcpy_s(sLabel, _countof(sLabel), sName);
			sLabel[p-sName] = _T('&');
			auto_strcpy_s(sLabel + (p-sName) + 1, _countof(sLabel), p);
		}else if (_tcscmp(sName + _tcslen(sName) - 3, _T("...")) == 0) {
			// ����...
			auto_strcpy_s(sLabel, _countof(sLabel), sName);
			sLabel[_tcslen(sName) - 3] = '\0';						// ������...�����
			auto_strcat_s(sLabel, _countof(sLabel), _T("(&"));
			auto_strcat_s(sLabel, _countof(sLabel), sKey);
			auto_strcat_s(sLabel, _countof(sLabel), _T(")..."));
		}else {
			auto_sprintf_s(sLabel, _countof(sLabel), _T("%ts(&%ts)"), sName, sKey);
		}
		return sLabel;
	}
}

/*! ���j���[���x���̍쐬 */
TCHAR* KeyBind::GetMenuLabel(
	HINSTANCE		hInstance,
	int				nKeyNameArrNum,
	KeyData*		pKeyNameArr,
	int				nFuncId,
	TCHAR*      	pszLabel,   // [in,out] �o�b�t�@��256�ȏ�Ɖ���
	const TCHAR*	pszKey,
	bool			bKeyStr,
	int				nLabelSize,
	bool			bGetDefFuncCode // = true
	)
{
	const unsigned int LABEL_MAX = nLabelSize;
	if (pszLabel[0] == _T('\0')) {
		_tcsncpy(pszLabel, LS(nFuncId), LABEL_MAX - 1);
		pszLabel[LABEL_MAX - 1] = _T('\0');
	}
	if (pszLabel[0] == _T('\0')) {
		_tcscpy(pszLabel, _T("-- undefined name --"));
	}
	// �A�N�Z�X�L�[�̒ǉ�	2010/5/17 Uchi
	_tcsncpy_s(pszLabel, LABEL_MAX, MakeMenuLabel(pszLabel, pszKey), _TRUNCATE);

	// �@�\�ɑΉ�����L�[����ǉ����邩
	if (bKeyStr) {
		NativeT memAccessKey;
		// [�t�@�C��/�t�H���_/�E�B���h�E�ꗗ�ȊO]����[�A�N�Z�X�L�[������Ƃ��̂�]�ɕt������悤�ɕύX
		// �@�\�ɑΉ�����L�[���̎擾
		if (GetKeyStr(hInstance, nKeyNameArrNum, pKeyNameArr, memAccessKey, nFuncId, bGetDefFuncCode)) {
			// �o�b�t�@������Ȃ��Ƃ��͓���Ȃ�
			if (_tcslen(pszLabel) + memAccessKey.GetStringLength() + 1 < LABEL_MAX) {
				_tcscat(pszLabel, _T("\t"));
				_tcscat(pszLabel, memAccessKey.GetStringPtr());
			}
		}
	}
	return pszLabel;
}


/*! �L�[�̃f�t�H���g�@�\���擾����

	@param nKeyCode [in] �L�[�R�[�h
	@param nState [in] Shift,Ctrl,Alt�L�[���

	@return �@�\�ԍ�
*/
EFunctionCode KeyBind::GetDefFuncCode(int nKeyCode, int nState)
{
	auto& csTabBar = GetDllShareData().common.tabBar;
	EFunctionCode nDefFuncCode = F_DEFAULT;
	if (nKeyCode == VK_F4) {
		if (nState == _CTRL) {
			nDefFuncCode = F_FILECLOSE;	// ����(����)
			if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
				nDefFuncCode = F_WINCLOSE;	// ����
			}
		}else if (nState == _ALT) {
			nDefFuncCode = F_WINCLOSE;	// ����
			if (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin) {
				if (!csTabBar.bTab_CloseOneWin) {
					nDefFuncCode = F_GROUPCLOSE;	// �O���[�v�����
				}
			}
		}
	}
	return nDefFuncCode;
}


/*! ����̃L�[��񂩂�@�\�R�[�h���擾����

	@param keyData [in] �L�[���
	@param nState [in] Shift,Ctrl,Alt�L�[���
	@param bGetDefFuncCode [in] �f�t�H���g�@�\���擾���邩�ǂ���

	@return �@�\�ԍ�
*/
EFunctionCode KeyBind::GetFuncCodeAt(KeyData& keyData, int nState, bool bGetDefFuncCode)
{
	if (keyData.nFuncCodeArr[nState] != 0) {
		return keyData.nFuncCodeArr[nState];
	}
	if (bGetDefFuncCode) {
		return GetDefFuncCode(keyData.nKeyCode, nState);
	}
	return F_DEFAULT;
}


#define _SQL_RUN	F_PLSQL_COMPILE_ON_SQLPLUS
#define _COPYWITHLINENUM	F_COPYLINESWITHLINENUMBER
	static const KEYDATAINIT	KeyDataInit[] = {
	// note: key binding
	// note 2: ���Ԃ�2�i�ŉ���3�r�b�g[Alt][Ctrl][Shift]�̑g�����̏�(�����2���������l)
	//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)

		// �}�E�X�{�^��
		//keycode,			keyname,							�Ȃ�,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VKEX_DBL_CLICK,	(LPCTSTR)STR_KEY_BIND_DBL_CLICK,	{ F_SELECTWORD,		F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD }, }, //Feb. 19, 2001 JEPRO Alt�ƉE�N���b�N�̑g�����͌����Ȃ��̂ŉE�N���b�N���j���[�̃L�[���蓖�Ă��͂�����
		{ VKEX_R_CLICK,		(LPCTSTR)STR_KEY_BIND_R_CLICK,		{ F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_0,					F_0,				F_0,					F_0 }, },
		{ VKEX_MDL_CLICK,	(LPCTSTR)STR_KEY_BIND_MID_CLICK,	{ F_AUTOSCROLL,		F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, }, // novice 2004/10/11 �}�E�X���{�^���Ή�
		{ VKEX_LSD_CLICK,	(LPCTSTR)STR_KEY_BIND_LSD_CLICK,	{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, }, // novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
		{ VKEX_RSD_CLICK,	(LPCTSTR)STR_KEY_BIND_RSD_CLICK,	{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VKEX_TRI_CLICK,	(LPCTSTR)STR_KEY_BIND_TRI_CLICK,	{ F_SELECTLINE,		F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE }, },
		{ VKEX_QUA_CLICK,	(LPCTSTR)STR_KEY_BIND_QUA_CLICK,	{ F_SELECTALL,		F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL }, },
		{ VKEX_WHEEL_UP,	(LPCTSTR)STR_KEY_BIND_WHEEL_UP,		{ F_WHEELUP,		F_WHEELUP,			F_SETFONTSIZEUP,		F_WHEELUP,			F_WHEELUP,				F_WHEELUP,			F_WHEELUP,				F_WHEELUP }, },
		{ VKEX_WHEEL_DOWN,	(LPCTSTR)STR_KEY_BIND_WHEEL_DOWN,	{ F_WHEELDOWN,		F_WHEELDOWN,		F_SETFONTSIZEDOWN,		F_WHEELDOWN,		F_WHEELDOWN,			F_WHEELDOWN,		F_WHEELDOWN,			F_WHEELDOWN }, },
		{ VKEX_WHEEL_LEFT,	(LPCTSTR)STR_KEY_BIND_WHEEL_LEFT,	{ F_WHEELLEFT,		F_WHEELLEFT,		F_WHEELLEFT,			F_WHEELLEFT,		F_WHEELLEFT,			F_WHEELLEFT,		F_WHEELLEFT,			F_WHEELLEFT }, },
		{ VKEX_WHEEL_RIGHT,	(LPCTSTR)STR_KEY_BIND_WHEEL_RIGHT,	{ F_WHEELRIGHT,		F_WHEELRIGHT,		F_WHEELRIGHT,			F_WHEELRIGHT,		F_WHEELRIGHT,			F_WHEELRIGHT,		F_WHEELRIGHT,			F_WHEELRIGHT }, },

		// �t�@���N�V�����L�[
		//keycode,	keyname,			�Ȃ�,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_F1,	_T("F1"),			{ F_EXTHTMLHELP,	F_MENU_ALLFUNC,		F_EXTHELP1,				F_ABOUT,			F_HELP_CONTENTS,		F_HELP_SEARCH,		F_0,					F_0 }, },
		{ VK_F2,	_T("F2"),			{ F_BOOKMARK_NEXT,	F_BOOKMARK_PREV,	F_BOOKMARK_SET,			F_BOOKMARK_RESET,	F_BOOKMARK_VIEW,		F_0,				F_0,					F_0 }, },
		{ VK_F3,	_T("F3"),			{ F_SEARCH_NEXT,	F_SEARCH_PREV,		F_SEARCH_CLEARMARK,		F_JUMP_SRCHSTARTPOS,F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F4,	_T("F4"),			{ F_SPLIT_V,		F_SPLIT_H,			F_0,					F_FILECLOSE_OPEN,	F_0,					F_EXITALLEDITORS,	F_EXITALL,				F_0 }, },
		{ VK_F5,	_T("F5"),			{ F_REDRAW,			F_0,				F_EXECMD_DIALOG,		F_0,				F_UUDECODE,				F_0,				F_TABTOSPACE,			F_SPACETOTAB }, },
		{ VK_F6,	_T("F6"),			{ F_BEGIN_SEL,		F_BEGIN_BOX,		F_TOLOWER,				F_0,				F_BASE64DECODE,			F_0,				F_0,					F_0 }, },
		{ VK_F7,	_T("F7"),			{ F_CUT,			F_0,				F_TOUPPER,				F_0,				F_CODECNV_UTF72SJIS,	F_CODECNV_SJIS2UTF7,F_FILE_REOPEN_UTF7,		F_0 }, },
		{ VK_F8,	_T("F8"),			{ F_COPY,			F_COPY_CRLF,		F_TOHANKAKU,			F_0,				F_CODECNV_UTF82SJIS,	F_CODECNV_SJIS2UTF8,F_FILE_REOPEN_UTF8,		F_0 }, },
		{ VK_F9,	_T("F9"),			{ F_PASTE,			F_PASTEBOX,			F_TOZENKAKUKATA,		F_0,				F_CODECNV_UNICODE2SJIS,	F_0,				F_FILE_REOPEN_UNICODE,	F_0 }, },
		{ VK_F10,	_T("F10"),			{ _SQL_RUN,			F_DUPLICATELINE,	F_TOZENKAKUHIRA,		F_0,				F_CODECNV_EUC2SJIS,		F_CODECNV_SJIS2EUC,	F_FILE_REOPEN_EUC,		F_0 }, },
		{ VK_F11,	_T("F11"),			{ F_OUTLINE,		F_ACTIVATE_SQLPLUS,	F_HANKATATOZENKATA,		F_0,				F_CODECNV_EMAIL,		F_CODECNV_SJIS2JIS,	F_FILE_REOPEN_JIS,		F_0 }, },
		{ VK_F12,	_T("F12"),			{ F_TAGJUMP,		F_TAGJUMPBACK,		F_HANKATATOZENHIRA,		F_0,				F_CODECNV_AUTO2SJIS,	F_0,				F_FILE_REOPEN_SJIS,		F_0 }, },
		{ VK_F13,	_T("F13"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F14,	_T("F14"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F15,	_T("F15"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F16,	_T("F16"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F17,	_T("F17"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F18,	_T("F18"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F19,	_T("F19"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F20,	_T("F20"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F21,	_T("F21"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F22,	_T("F22"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F23,	_T("F23"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F24,	_T("F24"),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },

		// ����L�[
		//keycode,	keyname,			�Ȃ�,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_TAB,	_T("Tab"),			{ F_INDENT_TAB,		F_UNINDENT_TAB,		F_NEXTWINDOW,			F_PREVWINDOW,		F_0,					F_0,				F_0,					F_0 }, },
		{ VK_RETURN,_T("Enter"),		{ F_0,				F_0,				F_COMPARE,				F_0,				F_PROPERTY_FILE,		F_0,				F_0,					F_0 }, },
		{ VK_ESCAPE,_T("Esc"),			{ F_CANCEL_MODE,	F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_BACK,	_T("BkSp"),			{ F_DELETE_BACK,	F_0,				F_WordDeleteToStart,	F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_INSERT,_T("Ins"),			{ F_CHGMOD_INS,		F_PASTE,			F_COPY,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_DELETE,_T("Del"),			{ F_DELETE,			F_CUT,				F_WordDeleteToEnd,		F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_HOME,	_T("Home"),			{ F_GOLINETOP,		F_GOLINETOP_SEL,	F_GOFILETOP,			F_GOFILETOP_SEL,	F_GOLINETOP_BOX,		F_0,				F_GOFILETOP_BOX,		F_0 }, },
		{ VK_END,	_T("End(Help)"),	{ F_GOLINEEND,		F_GOLINEEND_SEL,	F_GOFILEEND,			F_GOFILEEND_SEL,	F_GOLINEEND_BOX,		F_0,				F_GOFILEEND_BOX,		F_0 }, },
		{ VK_LEFT,	_T("��"),			{ F_LEFT,			F_LEFT_SEL,			F_WORDLEFT,				F_WORDLEFT_SEL,		F_LEFT_BOX,				F_0,				F_WORDLEFT_BOX,			F_0 }, },
		{ VK_UP,	_T("��"),			{ F_UP,				F_UP_SEL,			F_WndScrollDown,		F_UP2_SEL,			F_UP_BOX,				F_0,				F_UP2_BOX,				F_MAXIMIZE_V },}, 
		{ VK_RIGHT,	_T("��"),			{ F_RIGHT,			F_RIGHT_SEL,		F_WORDRIGHT,			F_WORDRIGHT_SEL,	F_RIGHT_BOX,			F_0,				F_WORDRIGHT_BOX,		F_MAXIMIZE_H },}, 
		{ VK_DOWN,	_T("��"),			{ F_DOWN,			F_DOWN_SEL,			F_WndScrollUp,			F_DOWN2_SEL,		F_DOWN_BOX,				F_0,				F_DOWN2_BOX,			F_MINIMIZE_ALL },}, 
		{ VK_NEXT,	_T("PgDn(RollUp)"),	{ F_1PageDown,		F_1PageDown_Sel,	F_HalfPageDown,			F_HalfPageDown_Sel,	F_1PageDown_BOX,		F_0,				F_HalfPageDown_BOX,		F_0 }, },
		{ VK_PRIOR,	_T("PgUp(RollDn)"),	{ F_1PageUp,		F_1PageUp_Sel,		F_HalfPageUp,			F_HalfPageUp_Sel,	F_1PageUp_BOX,			F_0,				F_HalfPageUp_BOX,		F_0 }, },
		{ VK_SPACE,	_T("Space"),		{ F_INDENT_SPACE,	F_UNINDENT_SPACE,	F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 }, },

		// ����
		//keycode,	keyname,			�Ȃ�,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ '0',		_T("0"),			{ F_0,				F_0,				F_0,					F_0,				F_CUSTMENU_10,			F_CUSTMENU_20,		F_0,					F_0 }, },
		{ '1',		_T("1"),			{ F_0,				F_0,				F_SHOWTOOLBAR,			F_CUSTMENU_21,		F_CUSTMENU_1,			F_CUSTMENU_11,		F_0,					F_0 }, },
		{ '2',		_T("2"),			{ F_0,				F_0,				F_SHOWFUNCKEY,			F_CUSTMENU_22,		F_CUSTMENU_2,			F_CUSTMENU_12,		F_0,					F_0 }, },
		{ '3',		_T("3"),			{ F_0,				F_0,				F_SHOWSTATUSBAR,		F_CUSTMENU_23,		F_CUSTMENU_3,			F_CUSTMENU_13,		F_0,					F_0 }, },
		{ '4',		_T("4"),			{ F_0,				F_0,				F_TYPE_LIST,			F_CUSTMENU_24,		F_CUSTMENU_4,			F_CUSTMENU_14,		F_0,					F_0 }, },
		{ '5',		_T("5"),			{ F_0,				F_0,				F_OPTION_TYPE,			F_0,				F_CUSTMENU_5,			F_CUSTMENU_15,		F_0,					F_0 }, },
		{ '6',		_T("6"),			{ F_0,				F_0,				F_OPTION,				F_0,				F_CUSTMENU_6,			F_CUSTMENU_16,		F_0,					F_0 }, },
		{ '7',		_T("7"),			{ F_0,				F_0,				F_FONT,					F_0,				F_CUSTMENU_7,			F_CUSTMENU_17,		F_0,					F_0 }, },
		{ '8',		_T("8"),			{ F_0,				F_0,				F_0,					F_0,				F_CUSTMENU_8,			F_CUSTMENU_18,		F_0,					F_0 }, },
		{ '9',		_T("9"),			{ F_0,				F_0,				F_0,					F_0,				F_CUSTMENU_9,			F_CUSTMENU_19,		F_0,					F_0 }, },

		// �A���t�@�x�b�g
		//keycode,	keyname,			�Ȃ�,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 'A',		_T("A"),			{ F_0,				F_0,				F_SELECTALL,			F_0,				F_SORT_ASC,				F_0,				F_0,					F_0 }, },
		{ 'B',		_T("B"),			{ F_0,				F_0,				F_BROWSE,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'C',		_T("C"),			{ F_0,				F_0,				F_COPY,					F_OPEN_HfromtoC,	F_0,					F_0,				F_0,					F_0 }, },
		{ 'D',		_T("D"),			{ F_0,				F_0,				F_WordCut,				F_WordDelete,		F_SORT_DESC,			F_0,				F_0,					F_0 }, },
		{ 'E',		_T("E"),			{ F_0,				F_0,				F_CUT_LINE,				F_DELETE_LINE,		F_0,					F_0,				F_CASCADE,				F_0 }, },
		{ 'F',		_T("F"),			{ F_0,				F_0,				F_SEARCH_DIALOG,		F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'G',		_T("G"),			{ F_0,				F_0,				F_GREP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'H',		_T("H"),			{ F_0,				F_0,				F_CURLINECENTER,		F_OPEN_HfromtoC,	F_0,					F_0,				F_TILE_V,				F_0 }, },
		{ 'I',		_T("I"),			{ F_0,				F_0,				F_DUPLICATELINE,		F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'J',		_T("J"),			{ F_0,				F_0,				F_JUMP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'K',		_T("K"),			{ F_0,				F_0,				F_LineCutToEnd,			F_LineDeleteToEnd,	F_0,					F_0,				F_0,					F_0 }, },
		{ 'L',		_T("L"),			{ F_0,				F_0,				F_LOADKEYMACRO,			F_EXECKEYMACRO,		F_LTRIM,				F_0,				F_TOLOWER,				F_TOUPPER }, },
		{ 'M',		_T("M"),			{ F_0,				F_0,				F_SAVEKEYMACRO,			F_RECKEYMACRO,		F_MERGE,				F_0,				F_0,					F_0 }, },
		{ 'N',		_T("N"),			{ F_0,				F_0,				F_FILENEW,				F_0,				F_JUMPHIST_NEXT,		F_0,				F_0,					F_0 }, },
		{ 'O',		_T("O"),			{ F_0,				F_0,				F_FILEOPEN,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'P',		_T("P"),			{ F_0,				F_0,				F_PRINT,				F_PRINT_PREVIEW,	F_JUMPHIST_PREV,		F_0,				F_PRINT_PAGESETUP,		F_0 }, },
		{ 'Q',		_T("Q"),			{ F_0,				F_0,				F_CREATEKEYBINDLIST,	F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'R',		_T("R"),			{ F_0,				F_0,				F_REPLACE_DIALOG,		F_0,				F_RTRIM,				F_0,				F_0,					F_0 }, },
		{ 'S',		_T("S"),			{ F_0,				F_0,				F_FILESAVE,				F_FILESAVEAS_DIALOG,F_0,					F_0,				F_TMPWRAPSETTING,		F_0 }, },
		{ 'T',		_T("T"),			{ F_0,				F_0,				F_TAGJUMP,				F_TAGJUMPBACK,		F_0,					F_0,				F_TILE_H,				F_0 }, },
		{ 'U',		_T("U"),			{ F_0,				F_0,				F_LineCutToStart,		F_LineDeleteToStart,F_0,					F_0,				F_WRAPWINDOWWIDTH,		F_0 }, },
		{ 'V',		_T("V"),			{ F_0,				F_0,				F_PASTE,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'W',		_T("W"),			{ F_0,				F_0,				F_SELECTWORD,			F_0,				F_0,					F_0,				F_TMPWRAPWINDOW,		F_0 }, },
		{ 'X',		_T("X"),			{ F_0,				F_0,				F_CUT,					F_0,				F_0,					F_0,				F_TMPWRAPNOWRAP,		F_0 }, },
		{ 'Y',		_T("Y"),			{ F_0,				F_0,				F_REDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'Z',		_T("Z"),			{ F_0,				F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },

		// �L��
		//keycode,	keyname,			�Ȃ�,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 0x00bd,	_T("-"),			{ F_0,				F_0,				F_COPYFNAME,			F_SPLIT_V,			F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00de,	(LPCTSTR)STR_KEY_BIND_HAT_ENG_QT,		{ F_0,				F_0,				F_COPYTAG,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00dc,	_T("\\"),			{ F_0,				F_0,				F_COPYPATH,				F_SPLIT_H,			F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00c0,	(LPCTSTR)STR_KEY_BIND_AT_ENG_BQ,		{ F_0,				F_0,				F_COPYLINES,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00db,	_T("["),			{ F_0,				F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00bb,	_T(";"),			{ F_0,				F_0,				F_0,					F_SPLIT_VH,			F_INS_DATE,				F_0,				F_0,					F_0 }, },
		{ 0x00ba,	_T(":"),			{ F_0,				F_0,				_COPYWITHLINENUM,		F_0,				F_INS_TIME,				F_0,				F_0,					F_0 }, },
		{ 0x00dd,	_T("]"),			{ F_0,				F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00bc,	_T(","),			{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00be,	_T("."),			{ F_0,				F_0,				F_COPYLINESASPASSAGE,	F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00bf,	_T("/"),			{ F_0,				F_0,				F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00e2,	_T("_"),			{ F_0,				F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00df,	_T("_(PC-98)"),		{ F_0,				F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_APPS,	(LPCTSTR)STR_KEY_BIND_APLI,	{ F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON }, }
	};

const TCHAR* jpVKEXNames[] = {
	_T("�_�u���N���b�N"),
	_T("�E�N���b�N"),
	_T("���N���b�N"),
	_T("���T�C�h�N���b�N"),
	_T("�E�T�C�h�N���b�N"),
	_T("�g���v���N���b�N"),
	_T("�N�A�h���v���N���b�N"),
	_T("�z�C�[���A�b�v"),
	_T("�z�C�[���_�E��"),
	_T("�z�C�[����"),
	_T("�z�C�[���E")
};
const int jpVKEXNamesLen = _countof( jpVKEXNames );

/*!	@brief ���L������������/�L�[���蓖��

	�f�t�H���g�L�[���蓖�Ċ֘A�̏���������
*/
bool ShareData::InitKeyAssign(DllSharedData& shareData)
{
	/********************/
	/* ���ʐݒ�̋K��l */
	/********************/
	const int nKeyDataInitNum = _countof(KeyDataInit);
	const int KEYNAME_SIZE = _countof(shareData.common.keyBind.pKeyNameArr) -1;// �Ō�̂P�v�f�̓_�~�[�p�ɗ\��
	assert(!(nKeyDataInitNum > KEYNAME_SIZE));
//	if (nKeyDataInitNum > KEYNAME_SIZE) {
//		PleaseReportToAuthor(NULL, _T("�L�[�ݒ萔�ɑ΂���DLLSHARE::nKeyNameArr[]�̃T�C�Y���s�����Ă��܂�"));
//		return false;
//	}

	// �}�E�X�R�[�h�̌Œ�Əd���r��
	static const KEYDATAINIT dummy[] = {
		{ 0,		_T(""),				{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 } }
	};

	// �C���f�b�N�X�p�_�~�[�쐬
	SetKeyNameArrVal(shareData, KEYNAME_SIZE, &dummy[0]);
	// �C���f�b�N�X�쐬 �d�������ꍇ�͐擪�ɂ�����̂�D��
	for (size_t ii=0; ii<_countof(shareData.common.keyBind.keyToKeyNameArr); ++ii) {
		shareData.common.keyBind.keyToKeyNameArr[ii] = KEYNAME_SIZE;
	}
	for (int i=nKeyDataInitNum-1; i>=0; --i) {
		shareData.common.keyBind.keyToKeyNameArr[KeyDataInit[i].nKeyCode] = (BYTE)i;
	}
	for (int i=0; i<nKeyDataInitNum; ++i) {
		SetKeyNameArrVal(shareData, i, &KeyDataInit[i]);
	}
	shareData.common.keyBind.nKeyNameArrNum = nKeyDataInitNum;
	return true;
}

//	@brief ����I����̕�����X�V����
void ShareData::RefreshKeyAssignString(DllSharedData& shareData)
{
	const int nKeyDataInitNum = _countof(KeyDataInit);
	for (int i=0; i<nKeyDataInitNum; ++i) {
		KeyData* pKeydata = &shareData.common.keyBind.pKeyNameArr[i];
		if (KeyDataInit[i].nKeyNameId <= 0xFFFF) {
			_tcscpy(pKeydata->szKeyName, LS(KeyDataInit[i].nKeyNameId));
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// KeyData�z��Ƀf�[�^���Z�b�g
static void SetKeyNameArrVal(
	DllSharedData&		shareData,
	int					nIdx,
	const KEYDATAINIT*	pKeydataInit
	)
{
	KeyData* pKeydata = &shareData.common.keyBind.pKeyNameArr[nIdx];
	pKeydata->nKeyCode = pKeydataInit->nKeyCode;
	if (0xFFFF < pKeydataInit->nKeyNameId) {
		_tcscpy(pKeydata->szKeyName, pKeydataInit->pszKeyName);
	}
	assert( sizeof(pKeydata->nFuncCodeArr) == sizeof(pKeydataInit->nFuncCodeArr) );
	memcpy_raw(pKeydata->nFuncCodeArr, pKeydataInit->nFuncCodeArr, sizeof(pKeydataInit->nFuncCodeArr));
}

