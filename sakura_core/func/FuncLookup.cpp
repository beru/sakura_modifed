/*!	@file
	@brief �\���p�����񓙂̎擾

	�@�\���C�@�\���ށC�@�\�ԍ��Ȃǂ̕ϊ��D�ݒ��ʂł̕\���p�������p�ӂ���D
*/

#include "StdAfx.h"
#include "func/FuncLookup.h"
#include "plugin/JackManager.h"

// �I�t�Z�b�g�l
const size_t LUOFFSET_MACRO = 0;
const size_t LUOFFSET_CUSTMENU = 1;
const size_t LUOFFSET_PLUGIN = 2;

/*!	@brief ���ޒ��̈ʒu�ɑΉ�����@�\�ԍ���Ԃ��D

	@param category [in] ���ޔԍ� (0-)
	@param position [in] ���ޒ���index (0-)
	@param bGetUnavailable [in] ���o�^�}�N���ł��@�\�ԍ���Ԃ�

	@retval �@�\�ԍ�
*/
EFunctionCode FuncLookup::Pos2FuncCode(int category, int position, bool bGetUnavailable) const
{
	if (category < 0 || position < 0) {
		return F_DISABLE;
	}
	
	if (category < (int)nsFuncCode::nFuncKindNum) {
		if (position < nsFuncCode::pnFuncListNumArr[category]) {
			return nsFuncCode::ppnFuncListArr[category][position];
		}
	}else if (category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO) {
		// �L�[���蓖�ă}�N��
		if (position < MAX_CUSTMACRO) {
			if (bGetUnavailable || pMacroRec[position].IsEnabled()) {
				return (EFunctionCode)(F_USERMACRO_0 + position);
			}
		}
	}else if (category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU) {
		// �J�X�^�����j���[
		if (position == 0) {
			return F_MENU_RBUTTON;
		}else if (position < MAX_CUSTOM_MENU) {
			return (EFunctionCode)(F_CUSTMENU_BASE + position);
		}
	}else if (category == nsFuncCode::nFuncKindNum + LUOFFSET_PLUGIN) {
		// �v���O�C��
		return JackManager::getInstance().GetCommandCode(position);
	}
	return F_DISABLE;
}

/*!	@brief ���ޒ��̈ʒu�ɑΉ�����@�\���̂�Ԃ��D

	@retval true �w�肳�ꂽ�@�\�ԍ��͒�`����Ă���
	@retval false �w�肳�ꂽ�@�\�ԍ��͖���`
*/
bool FuncLookup::Pos2FuncName(
	int		category,	// [in]  ���ޔԍ� (0-)
	int		position,	// [in]  ���ޒ���index (0-)
	wchar_t*	ptr,		// [out] ��������i�[����o�b�t�@�̐擪
	int		bufsize		// [in]  ��������i�[����o�b�t�@�̃T�C�Y
	) const
{
	int funccode = Pos2FuncCode(category, position);
	return Funccode2Name(funccode, ptr, bufsize);
}

/*!	@brief �@�\�ԍ��ɑΉ�����@�\���̂�Ԃ��D

	@param funccode [in] �@�\�ԍ�
	@param ptr [out] ��������i�[����o�b�t�@�̐擪
	@param bufsize [in] ��������i�[����o�b�t�@�̃T�C�Y
	
	@retval true �w�肳�ꂽ�@�\�ԍ��͒�`����Ă���
	@retval false �w�肳�ꂽ�@�\�ԍ��͖���`
*/
bool FuncLookup::Funccode2Name(int funccode, wchar_t* ptr, int bufsize) const
{
	LPCWSTR pszStr = NULL;

	if (F_USERMACRO_0 <= funccode && funccode < F_USERMACRO_0 + MAX_CUSTMACRO) {
		int position = funccode - F_USERMACRO_0;
		if (pMacroRec[position].IsEnabled()) {
			const TCHAR* p = pMacroRec[position].GetTitle();
			_tcstowcs(ptr, p, bufsize - 1);
			ptr[bufsize - 1] = LTEXT('\0');
		}else {
			_snwprintf(ptr, bufsize, LSW(STR_ERR_DLGFUNCLKUP03), position);
			ptr[bufsize - 1] = LTEXT('\0');
		}
		return true;
	}else if (funccode == F_MENU_RBUTTON) {
		Custmenu2Name(0, ptr, bufsize);
		ptr[bufsize-1] = LTEXT('\0');
		return true;
	}else if (F_CUSTMENU_1 <= funccode && funccode < F_CUSTMENU_BASE + MAX_CUSTOM_MENU) {	// MAX_CUSTMACRO->MAX_CUSTOM_MENU	2010/3/14 Uchi
		Custmenu2Name(funccode - F_CUSTMENU_BASE, ptr, bufsize);
		ptr[bufsize-1] = LTEXT('\0');
		return true;
	}else if (F_MENU_FIRST <= funccode && funccode < F_MENU_NOT_USED_FIRST) {
		if ((pszStr = LSW(funccode))[0] != L'\0') {
			wcsncpy(ptr, pszStr, bufsize);
			ptr[bufsize-1] = LTEXT('\0');
			return true;	// ��`���ꂽ�R�}���h
		}
	}else if (F_PLUGCOMMAND_FIRST <= funccode && funccode < F_PLUGCOMMAND_LAST) {
		if (JackManager::getInstance().GetCommandName(funccode, ptr, bufsize) > 0) {
			return true;	// �v���O�C���R�}���h
		}
	}

	// ����`�R�}���h
	if ((pszStr = LSW(funccode))[0] != L'\0') {
		wcsncpy(ptr, pszStr, bufsize);
		ptr[bufsize-1] = LTEXT('\0');
		return false;
	}
	return false;
}

/*!	@brief �@�\���ޔԍ��ɑΉ�����@�\���̂�Ԃ��D

	@param category [in] �@�\���ޔԍ�
	
	@return NULL ���ޖ��́D�擾�Ɏ��s������NULL�D
*/
const TCHAR* FuncLookup::Category2Name(int category) const
{
	if (category < 0) {
		return NULL;
	}
	if (category < (int)nsFuncCode::nFuncKindNum) {
		return LS(nsFuncCode::ppszFuncKind[category]);
	}else if (category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO) {
		return LS(STR_ERR_DLGFUNCLKUP01);
	}else if (category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU) {
		return LS(STR_ERR_DLGFUNCLKUP02);
	}else if (category == nsFuncCode::nFuncKindNum + LUOFFSET_PLUGIN) {
		return LS(STR_ERR_DLGFUNCLKUP19);
	}
	return NULL;
}

/*!	@brief ComboBox�ɗ��p�\�ȋ@�\���ވꗗ��o�^����

	@param hComboBox [in(out)] �f�[�^��ݒ肷��R���{�{�b�N�X
*/
void FuncLookup::SetCategory2Combo(HWND hComboBox) const
{
	// �R���{�{�b�N�X������������
	Combo_ResetContent(hComboBox);

	// �Œ�@�\���X�g
	for (size_t i=0; i<nsFuncCode::nFuncKindNum; ++i) {
		Combo_AddString(hComboBox, LS(nsFuncCode::ppszFuncKind[i]));
	}

	// ���[�U�}�N��
	Combo_AddString(hComboBox, LS(STR_ERR_DLGFUNCLKUP01));
	// �J�X�^�����j���[
	Combo_AddString(hComboBox, LS(STR_ERR_DLGFUNCLKUP02));
	// �v���O�C��
	Combo_AddString(hComboBox, LS(STR_ERR_DLGFUNCLKUP19));
}

/*!	@brief �w�肳�ꂽ���ނɑ�����@�\���X�g��ListBox�ɓo�^����D
	
	@param hListBox [in(out)] �l��ݒ肷�郊�X�g�{�b�N�X
	@param category [in] �@�\���ޔԍ�
*/
void FuncLookup::SetListItem(HWND hListBox, size_t category) const
{
	wchar_t pszLabel[256];
	// ���X�g������������
	List_ResetContent(hListBox);
	size_t n = GetItemCount(category);
	for (size_t i=0; i<n; ++i) {
		if (Pos2FuncCode(category, i) == F_DISABLE) {
			continue;
		}
		Pos2FuncName(category, i, pszLabel, _countof(pszLabel));
		List_AddString(hListBox, pszLabel);
	}
}

/*!
	�w�蕪�ޒ��̋@�\�����擾����D
	
	@param category [in] �@�\���ޔԍ�
*/
size_t FuncLookup::GetItemCount(size_t category) const
{
	if (category < nsFuncCode::nFuncKindNum) {
		return nsFuncCode::pnFuncListNumArr[category];
	}else if (category == nsFuncCode::nFuncKindNum + LUOFFSET_MACRO) {
		// �}�N��
		return MAX_CUSTMACRO;
	}else if (category == nsFuncCode::nFuncKindNum + LUOFFSET_CUSTMENU) {
		// �J�X�^�����j���[
		return MAX_CUSTOM_MENU;
	}else if (category == nsFuncCode::nFuncKindNum + LUOFFSET_PLUGIN) {
		// �v���O�C���R�}���h
		return JackManager::getInstance().GetCommandCount();
	}
	return 0;
}

/*!	@brief �ԍ��ɑΉ�����J�X�^�����j���[���̂�Ԃ��D

	@param index [in] �J�X�^�����j���[�ԍ�
	
	@return NULL ���ޖ��́D�擾�Ɏ��s������NULL�D
*/
const wchar_t* FuncLookup::Custmenu2Name(int index, wchar_t buf[], int bufSize) const
{
	if (index < 0 || CUSTMENU_INDEX_FOR_TABWND < index) {
		return NULL;
	}
	// ���ʐݒ�Ŗ��̂�ݒ肵�Ă���΂����Ԃ�
	if (pCommon->customMenu.szCustMenuNameArr[index][0] != '\0') {
		wcscpyn(buf, pCommon->customMenu.szCustMenuNameArr[index], bufSize);
		return pCommon->customMenu.szCustMenuNameArr[index];
	}

	// ���ʐݒ�Ŗ��ݒ�̏ꍇ�A���\�[�X�̃f�t�H���g����Ԃ�
	if (index == 0) {
		wcscpyn(buf, LSW(STR_CUSTMENU_RIGHT_CLICK), bufSize);
		return buf;
	}else if (index == CUSTMENU_INDEX_FOR_TABWND) {
		wcscpyn(buf, LSW(STR_CUSTMENU_TAB), bufSize);
		return buf;
	}else {
		swprintf(buf, LSW(STR_CUSTMENU_CUSTOM), index);
		return buf;
	}

	return NULL;
}

