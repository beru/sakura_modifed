#include "StdAfx.h"
#include "DllSharedData.h"

#include "HelpManager.h"
#include "env/DocTypeManager.h"


/*!	�O��Win�w���v���ݒ肳��Ă��邩�m�F�B
*/
bool HelpManager::ExtWinHelpIsSet(const TypeConfig* type)
{
	if (pShareData->common.helper.szExtHelp[0] != L'\0') {
		return true;	// ���ʐݒ�ɐݒ肳��Ă���
	}
	if (type && type->szExtHelp[0] != L'\0') {
		return true;	// �^�C�v�ʐݒ�ɐݒ肳��Ă���B
	}
	return false;
}

/*!	�ݒ肳��Ă���O��Win�w���v�̃t�@�C������Ԃ��B
	�^�C�v�ʐݒ�Ƀt�@�C�������ݒ肳��Ă���΁A���̃t�@�C������Ԃ��܂��B
	�����łȂ���΁A���ʐݒ�̃t�@�C������Ԃ��܂��B
*/
const TCHAR* HelpManager::GetExtWinHelp(const TypeConfig* type)
{
	if (type && type->szExtHelp[0] != _T('\0')) {
		return type->szExtHelp;
	}
	
	return pShareData->common.helper.szExtHelp;
}

/*!	�O��HTML�w���v���ݒ肳��Ă��邩�m�F�B
*/
bool HelpManager::ExtHTMLHelpIsSet(const TypeConfig* type)
{
	if (pShareData->common.helper.szExtHtmlHelp[0] != L'\0') {
		return true;	// ���ʐݒ�ɐݒ肳��Ă���
	}
	if (type && type->szExtHtmlHelp[0] != L'\0') {
		return true;	// �^�C�v�ʐݒ�ɐݒ肳��Ă���B
	}
	return false;
}

/*!	�ݒ肳��Ă���O��Win�w���v�̃t�@�C������Ԃ��B
	�^�C�v�ʐݒ�Ƀt�@�C�������ݒ肳��Ă���΁A���̃t�@�C������Ԃ��܂��B
	�����łȂ���΁A���ʐݒ�̃t�@�C������Ԃ��܂��B
*/
const TCHAR* HelpManager::GetExtHTMLHelp(const TypeConfig* type)
{
	if (type && type->szExtHtmlHelp[0] != _T('\0')) {
		return type->szExtHtmlHelp;
	}
	
	return pShareData->common.helper.szExtHtmlHelp;
}

/*!	�r���[�A�𕡐��N�����Ȃ���ON����Ԃ��B
*/
bool HelpManager::HTMLHelpIsSingle(const TypeConfig* type)
{
	if (type && type->szExtHtmlHelp[0] != L'\0') {
		return type->bHtmlHelpIsSingle;
	}

	return pShareData->common.helper.bHtmlHelpIsSingle;
}

