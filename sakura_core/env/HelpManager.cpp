#include "StdAfx.h"
#include "DllSharedData.h"

#include "HelpManager.h"
#include "env/DocTypeManager.h"


/*!	外部Winヘルプが設定されているか確認。
*/
bool HelpManager::ExtWinHelpIsSet(const TypeConfig* type)
{
	if (pShareData->common.helper.szExtHelp[0] != L'\0') {
		return true;	// 共通設定に設定されている
	}
	if (type && type->szExtHelp[0] != L'\0') {
		return true;	// タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
const TCHAR* HelpManager::GetExtWinHelp(const TypeConfig* type)
{
	if (type && type->szExtHelp[0] != _T('\0')) {
		return type->szExtHelp;
	}
	
	return pShareData->common.helper.szExtHelp;
}

/*!	外部HTMLヘルプが設定されているか確認。
*/
bool HelpManager::ExtHTMLHelpIsSet(const TypeConfig* type)
{
	if (pShareData->common.helper.szExtHtmlHelp[0] != L'\0') {
		return true;	// 共通設定に設定されている
	}
	if (type && type->szExtHtmlHelp[0] != L'\0') {
		return true;	// タイプ別設定に設定されている。
	}
	return false;
}

/*!	設定されている外部Winヘルプのファイル名を返す。
	タイプ別設定にファイル名が設定されていれば、そのファイル名を返します。
	そうでなければ、共通設定のファイル名を返します。
*/
const TCHAR* HelpManager::GetExtHTMLHelp(const TypeConfig* type)
{
	if (type && type->szExtHtmlHelp[0] != _T('\0')) {
		return type->szExtHtmlHelp;
	}
	
	return pShareData->common.helper.szExtHtmlHelp;
}

/*!	ビューアを複数起動しないがONかを返す。
*/
bool HelpManager::HTMLHelpIsSingle(const TypeConfig* type)
{
	if (type && type->szExtHtmlHelp[0] != L'\0') {
		return type->bHtmlHelpIsSingle;
	}

	return pShareData->common.helper.bHtmlHelpIsSingle;
}

