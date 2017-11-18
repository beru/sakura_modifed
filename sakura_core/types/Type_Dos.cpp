#include "StdAfx.h"
#include "types/Type.h"

// MS-DOS�o�b�`�t�@�C��
void CType_Dos::InitTypeConfigImp(TypeConfig& type)
{
	// ���O�Ɗg���q
	_tcscpy(type.szTypeName, _T("MS-DOS�o�b�`�t�@�C��"));
	_tcscpy(type.szTypeExts, _T("bat"));

	// �ݒ�
	type.lineComment.CopyTo(0, L"REM ", -1);		// �s�R�����g�f���~�^
	type.eDefaultOutline = OutlineType::Text;		// �A�E�g���C����͕��@
	type.nKeywordSetIdx[0] = 7;					// �L�[���[�h�Z�b�g
}

const wchar_t* g_ppszKeywordsBAT[] = {
	L"PATH",
	L"PROMPT",
	L"TEMP",
	L"TMP",
	L"TZ",
	L"CONFIG",
	L"COMSPEC",
	L"DIRCMD",
	L"COPYCMD",
	L"winbootdir",
	L"windir",
	L"DIR",
	L"CALL",
	L"CHCP",
	L"RENAME",
	L"REN",
	L"ERASE",
	L"DEL",
	L"TYPE",
	L"REM",
	L"COPY",
	L"PAUSE",
	L"DATE",
	L"TIME",
	L"VER",
	L"VOL",
	L"CD",
	L"CHDIR",
	L"MD",
	L"MKDIR",
	L"RD",
	L"RMDIR",
	L"BREAK",
	L"VERIFY",
	L"SET",
	L"EXIT",
	L"CTTY",
	L"ECHO",
	L"@ECHO",	// Oct. 31, 2000 JEPRO '@' �������\�ɂ����̂Œǉ�
	L"LOCK",
	L"UNLOCK",
	L"GOTO",
	L"SHIFT",
	L"IF",
	L"FOR",
	L"DO",		// Nov. 2, 2000 JEPRO �ǉ�
	L"IN",		// Nov. 2, 2000 JEPRO �ǉ�
	L"ELSE",	// Nov. 2, 2000 JEPRO �ǉ� Win2000�Ŏg����
	L"CLS",
	L"TRUENAME",
	L"LOADHIGH",
	L"LH",
	L"LFNFOR",
	L"ON",
	L"OFF",
	L"NOT",
	L"ERRORLEVEL",
	L"EXIST",
	L"NUL",
	L"CON",
	L"AUX",
	L"COM1",
	L"COM2",
	L"COM3",
	L"COM4",
	L"PRN",
	L"LPT1",
	L"LPT2",
	L"LPT3",
	L"CLOCK",
	L"CLOCK$",
	L"CONFIG$"
};
size_t g_nKeywordsBAT = _countof(g_ppszKeywordsBAT);

