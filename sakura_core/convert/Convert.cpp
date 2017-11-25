#include "StdAfx.h"
#include "Convert.h"
#include "func/Funccode.h"
#include "Eol.h"
#include "charset/charcode.h"
#include "charset/CodeMediator.h"
#include "charset/CodeFactory.h"
#include "charset/ShiftJis.h"
#include "charset/Jis.h"
#include "charset/Euc.h"
#include "charset/UnicodeBe.h"
#include "charset/Utf8.h"
#include "charset/Utf7.h"
#include "Convert_ToLower.h"
#include "Convert_ToUpper.h"
#include "Convert_ToHankaku.h"
#include "Convert_TabToSpace.h"
#include "Convert_SpaceToTab.h"
#include "Convert_ZenkataToHankata.h"
#include "Convert_ZeneisuToHaneisu.h"
#include "Convert_HaneisuToZeneisu.h"
#include "Convert_HankataToZenkata.h"
#include "Convert_HankataToZenhira.h"
#include "Convert_ToZenhira.h"
#include "Convert_ToZenkata.h"
#include "Convert_Trim.h"

#include "window/EditWnd.h"

// �@�\��ʂɂ��o�b�t�@�̕ϊ�
void ConvertMediator::ConvMemory(
	NativeW* pMemory,
	EFunctionCode nFuncCode,
	int nTabWidth,
	int nStartColumn
	)
{
	// �R�[�h�ϊ��͂ł��邾��ANSI�ł�sakura�ƌ݊��̌��ʂ�������悤�Ɏ�������
	// xxx2SJIS:
	//   1. �o�b�t�@�̓��e��ANSI�ő����ɂȂ�悤 Unicode��SJIS �ϊ�����
	//   2. xxx��SJIS �ϊ���Ƀo�b�t�@���e��UNICODE�ő����ɖ߂��iSJIS��Unicode�j�̂Ɠ����Ȍ��ʂ𓾂邽�߂� xxx��Unicode �ϊ�����
	// SJIS2xxx:
	//   1. �o�b�t�@���e��ANSI�ő����ɕϊ��iUnicode��SJIS�j��� SJIS��xxx �ϊ�����̂Ɠ����Ȍ��ʂ𓾂邽�߂� Unicode��xxx �ϊ�����
	//   2. �o�b�t�@���e��UNICODE�ő����ɖ߂����߂� SJIS��Unicode �ϊ�����

	switch (nFuncCode) {
	// �R�[�h�ϊ�(xxx2SJIS)
	case F_CODECNV_AUTO2SJIS:
	case F_CODECNV_EMAIL:
	case F_CODECNV_EUC2SJIS:
	case F_CODECNV_UNICODE2SJIS:
	case F_CODECNV_UNICODEBE2SJIS:
	case F_CODECNV_UTF82SJIS:
	case F_CODECNV_UTF72SJIS:
		ShiftJis::UnicodeToSJIS(*pMemory, pMemory->_GetMemory());
		break;
	// �R�[�h�ϊ�(SJIS2xxx)
	case F_CODECNV_SJIS2JIS:		Jis::UnicodeToJIS(*pMemory, pMemory->_GetMemory());			break;
	case F_CODECNV_SJIS2EUC:		Euc::UnicodeToEUC(*pMemory, pMemory->_GetMemory());			break;
	case F_CODECNV_SJIS2UTF8:		Utf8::UnicodeToUTF8(*pMemory, pMemory->_GetMemory());		break;
	case F_CODECNV_SJIS2UTF7:		Utf7::UnicodeToUTF7(*pMemory, pMemory->_GetMemory());		break;
	}

	EncodingType ecode = CODE_NONE;
	if (nFuncCode == F_CODECNV_AUTO2SJIS) {
		CodeMediator ccode(EditWnd::getInstance().GetDocument().docType.GetDocumentAttribute().encoding);
		ecode = ccode.CheckKanjiCode(
			reinterpret_cast<const char*>(pMemory->_GetMemory()->GetRawPtr()),
			pMemory->_GetMemory()->GetRawLength());
		switch (ecode) {
		case CODE_JIS:			nFuncCode = F_CODECNV_EMAIL;			break;
		case CODE_EUC:			nFuncCode = F_CODECNV_EUC2SJIS;			break;
		case CODE_UNICODE:		nFuncCode = F_CODECNV_UNICODE2SJIS;		break;
		case CODE_UNICODEBE:	nFuncCode = F_CODECNV_UNICODEBE2SJIS;	break;
		case CODE_UTF8:			nFuncCode = F_CODECNV_UTF82SJIS;		break;
		case CODE_UTF7:			nFuncCode = F_CODECNV_UTF72SJIS;		break;
		}
	}
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	switch (nFuncCode) {
	// ������ϊ��A���`
	case F_TOLOWER:					Converter_ToLower().CallConvert(pMemory);			break;	// ������
	case F_TOUPPER:					Converter_ToUpper().CallConvert(pMemory);			break;	// �啶��
	case F_TOHANKAKU:				Converter_ToHankaku().CallConvert(pMemory);			break;	// �S�p�����p
	case F_TOHANKATA:				Converter_ZenkataToHankata().CallConvert(pMemory);	break;	// �S�p�J�^�J�i�����p�J�^�J�i
	case F_TOZENEI:					Converter_HaneisuToZeneisu().CallConvert(pMemory);	break;	// ���p�p�����S�p�p��
	case F_TOHANEI:					Converter_ZeneisuToHaneisu().CallConvert(pMemory);	break;	// �S�p�p�������p�p��
	case F_TOZENKAKUKATA:			Converter_ToZenkata().CallConvert(pMemory);			break;	// ���p�{�S�Ђ灨�S�p�E�J�^�J�i
	case F_TOZENKAKUHIRA:			Converter_ToZenhira().CallConvert(pMemory);			break;	// ���p�{�S�J�^���S�p�E�Ђ炪��
	case F_HANKATATOZENKATA:		Converter_HankataToZenkata().CallConvert(pMemory);	break;	// ���p�J�^�J�i���S�p�J�^�J�i
	case F_HANKATATOZENHIRA:		Converter_HankataToZenhira().CallConvert(pMemory);	break;	// ���p�J�^�J�i���S�p�Ђ炪��
	// ������ϊ��A���`
	case F_TABTOSPACE:				Converter_TabToSpace(nTabWidth, nStartColumn, bExtEol).CallConvert(pMemory);break;	// TAB����
	case F_SPACETOTAB:				Converter_SpaceToTab(nTabWidth, nStartColumn, bExtEol).CallConvert(pMemory);break;	// �󔒁�TAB
	case F_LTRIM:					Converter_Trim(true, bExtEol).CallConvert(pMemory);		break;
	case F_RTRIM:					Converter_Trim(false, bExtEol).CallConvert(pMemory);	break;
	// �R�[�h�ϊ�(xxx2SJIS)
	// �������ʂ�SJIS, Latin1, CESU8�ɂȂ����ꍇ���T�|�[�g
	case F_CODECNV_AUTO2SJIS:
		{
			int nFlag = true;
			std::unique_ptr<CodeBase> pcCode( CodeFactory::CreateCodeBase(ecode, nFlag) );
			pcCode->CodeToUnicode(*(pMemory->_GetMemory()), pMemory);
		}
		break;
	case F_CODECNV_EMAIL:			Jis::JISToUnicode(*(pMemory->_GetMemory()), pMemory, true);	break;
	case F_CODECNV_EUC2SJIS:		Euc::EUCToUnicode(*(pMemory->_GetMemory()), pMemory);		break;
	case F_CODECNV_UNICODE2SJIS:	/* ���ϊ� */										break;
	case F_CODECNV_UNICODEBE2SJIS:	UnicodeBe::UnicodeBEToUnicode(*(pMemory->_GetMemory()), pMemory);	break;
	case F_CODECNV_UTF82SJIS:		Utf8::UTF8ToUnicode(*(pMemory->_GetMemory()), pMemory);		break;
	case F_CODECNV_UTF72SJIS:		Utf7::UTF7ToUnicode(*(pMemory->_GetMemory()), pMemory);		break;
	// �R�[�h�ϊ�(SJIS2xxx)
	case F_CODECNV_SJIS2JIS:
	case F_CODECNV_SJIS2EUC:
	case F_CODECNV_SJIS2UTF8:
	case F_CODECNV_SJIS2UTF7:
		ShiftJis::SJISToUnicode(*(pMemory->_GetMemory()), pMemory);
		break;
	}

	return;
}

