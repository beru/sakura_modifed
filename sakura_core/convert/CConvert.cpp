#include "StdAfx.h"
#include "CConvert.h"
#include "func/Funccode.h"
#include "CEol.h"
#include "charset/charcode.h"
#include "charset/CCodeMediator.h"
#include "charset/CCodeFactory.h"
#include "charset/CShiftJis.h"
#include "charset/CJis.h"
#include "charset/CEuc.h"
#include "charset/CUnicodeBe.h"
#include "charset/CUtf8.h"
#include "charset/CUtf7.h"
#include "CConvert_ToLower.h"
#include "CConvert_ToUpper.h"
#include "CConvert_ToHankaku.h"
#include "CConvert_TabToSpace.h"
#include "CConvert_SpaceToTab.h"
#include "CConvert_ZenkataToHankata.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "CConvert_HankataToZenkata.h"
#include "CConvert_HankataToZenhira.h"
#include "CConvert_ToZenhira.h"
#include "CConvert_ToZenkata.h"
#include "CConvert_Trim.h"

#include "window/CEditWnd.h"

// 機能種別によるバッファの変換
void ConvertMediator::ConvMemory(
	NativeW* pMemory,
	EFunctionCode nFuncCode,
	int nTabWidth,
	int nStartColumn
	)
{
	// コード変換はできるだけANSI版のsakuraと互換の結果が得られるように実装する	// 2009.03.26 ryoji
	// xxx2SJIS:
	//   1. バッファの内容がANSI版相当になるよう Unicode→SJIS 変換する
	//   2. xxx→SJIS 変換後にバッファ内容をUNICODE版相当に戻す（SJIS→Unicode）のと等価な結果を得るために xxx→Unicode 変換する
	// SJIS2xxx:
	//   1. バッファ内容をANSI版相当に変換（Unicode→SJIS）後に SJIS→xxx 変換するのと等価な結果を得るために Unicode→xxx 変換する
	//   2. バッファ内容をUNICODE版相当に戻すために SJIS→Unicode 変換する

	switch (nFuncCode) {
	// コード変換(xxx2SJIS)
	case F_CODECNV_AUTO2SJIS:
	case F_CODECNV_EMAIL:
	case F_CODECNV_EUC2SJIS:
	case F_CODECNV_UNICODE2SJIS:
	case F_CODECNV_UNICODEBE2SJIS:
	case F_CODECNV_UTF82SJIS:
	case F_CODECNV_UTF72SJIS:
		ShiftJis::UnicodeToSJIS(*pMemory, pMemory->_GetMemory());
		break;
	// コード変換(SJIS2xxx)
	case F_CODECNV_SJIS2JIS:		Jis::UnicodeToJIS(*pMemory, pMemory->_GetMemory());			break;
	case F_CODECNV_SJIS2EUC:		Euc::UnicodeToEUC(*pMemory, pMemory->_GetMemory());			break;
	case F_CODECNV_SJIS2UTF8:		Utf8::UnicodeToUTF8(*pMemory, pMemory->_GetMemory());		break;
	case F_CODECNV_SJIS2UTF7:		Utf7::UnicodeToUTF7(*pMemory, pMemory->_GetMemory());		break;
	}

	ECodeType ecode = CODE_NONE;
	if (nFuncCode == F_CODECNV_AUTO2SJIS) {
		CodeMediator ccode(EditWnd::getInstance()->GetDocument()->m_docType.GetDocumentAttribute().m_encoding);
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
	bool bExtEol = GetDllShareData().m_common.m_edit.m_bEnableExtEol;

	switch (nFuncCode) {
	// 文字種変換、整形
	case F_TOLOWER:					Converter_ToLower().CallConvert(pMemory);			break;	// 小文字
	case F_TOUPPER:					Converter_ToUpper().CallConvert(pMemory);			break;	// 大文字
	case F_TOHANKAKU:				Converter_ToHankaku().CallConvert(pMemory);			break;	// 全角→半角
	case F_TOHANKATA:				Converter_ZenkataToHankata().CallConvert(pMemory);	break;	// 全角カタカナ→半角カタカナ
	case F_TOZENEI:					Converter_HaneisuToZeneisu().CallConvert(pMemory);	break;	// 半角英数→全角英数
	case F_TOHANEI:					Converter_ZeneisuToHaneisu().CallConvert(pMemory);	break;	// 全角英数→半角英数
	case F_TOZENKAKUKATA:			Converter_ToZenkata().CallConvert(pMemory);			break;	// 半角＋全ひら→全角・カタカナ
	case F_TOZENKAKUHIRA:			Converter_ToZenhira().CallConvert(pMemory);			break;	// 半角＋全カタ→全角・ひらがな
	case F_HANKATATOZENKATA:		Converter_HankataToZenkata().CallConvert(pMemory);	break;	// 半角カタカナ→全角カタカナ
	case F_HANKATATOZENHIRA:		Converter_HankataToZenhira().CallConvert(pMemory);	break;	// 半角カタカナ→全角ひらがな
	// 文字種変換、整形
	case F_TABTOSPACE:				Converter_TabToSpace(nTabWidth, nStartColumn, bExtEol).CallConvert(pMemory);break;	// TAB→空白
	case F_SPACETOTAB:				Converter_SpaceToTab(nTabWidth, nStartColumn, bExtEol).CallConvert(pMemory);break;	// 空白→TAB
	case F_LTRIM:					Converter_Trim(true, bExtEol).CallConvert(pMemory);		break;	// 2001.12.03 hor
	case F_RTRIM:					Converter_Trim(false, bExtEol).CallConvert(pMemory);	break;	// 2001.12.03 hor
	// コード変換(xxx2SJIS)
	// 2014.02.10 Moca F_CODECNV_AUTO2SJIS追加。自動判別でSJIS, Latin1, CESU8になった場合をサポート
	case F_CODECNV_AUTO2SJIS:
		{
			int nFlag = true;
			std::unique_ptr<CodeBase> pcCode( CodeFactory::CreateCodeBase(ecode, nFlag) );
			pcCode->CodeToUnicode(*(pMemory->_GetMemory()), pMemory);
		}
		break;
	case F_CODECNV_EMAIL:			Jis::JISToUnicode(*(pMemory->_GetMemory()), pMemory, true);	break;
	case F_CODECNV_EUC2SJIS:		Euc::EUCToUnicode(*(pMemory->_GetMemory()), pMemory);		break;
	case F_CODECNV_UNICODE2SJIS:	/* 無変換 */										break;
	case F_CODECNV_UNICODEBE2SJIS:	UnicodeBe::UnicodeBEToUnicode(*(pMemory->_GetMemory()), pMemory);	break;
	case F_CODECNV_UTF82SJIS:		Utf8::UTF8ToUnicode(*(pMemory->_GetMemory()), pMemory);		break;
	case F_CODECNV_UTF72SJIS:		Utf7::UTF7ToUnicode(*(pMemory->_GetMemory()), pMemory);		break;
	// コード変換(SJIS2xxx)
	case F_CODECNV_SJIS2JIS:
	case F_CODECNV_SJIS2EUC:
	case F_CODECNV_SJIS2UTF8:
	case F_CODECNV_SJIS2UTF7:
		ShiftJis::SJISToUnicode(*(pMemory->_GetMemory()), pMemory);
		break;
	}

	return;
}

