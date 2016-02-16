#include "StdAfx.h"
#include "GrepEnumKeys.h"

int GrepEnumKeys::SetFileKeys(LPCTSTR lpKeys)
{
	const TCHAR* wildcard_delimiter = _T(" ;,");	// ���X�g�̋�؂�
	const TCHAR* wildcard_any = _T("*.*");			// �T�u�t�H���_�T���p
	int nWildCardLen = _tcslen(lpKeys);
	std::vector<TCHAR> wildCard( nWildCardLen + 1 );
	TCHAR* pWildCard = &wildCard[0];
	_tcscpy(pWildCard, lpKeys);
	ClearItems();
		
	int nPos = 0;
	TCHAR* token;
	while ((token = my_strtok<TCHAR>(pWildCard, nWildCardLen, &nPos, wildcard_delimiter))) {	// �g�[�N�����ɌJ��Ԃ��B
		// �t�B���^����ނ��ƂɐU�蕪����
		enum KeyFilterType {
			FILTER_SEARCH,
			FILTER_EXCEPT_FILE,
			FILTER_EXCEPT_FOLDER,
		};
		KeyFilterType keyType = FILTER_SEARCH;
		if (token[0] == _T('!')) {
			++token;
			keyType = FILTER_EXCEPT_FILE;
		}else if (token[0] == _T('#')) {
			++token;
			keyType = FILTER_EXCEPT_FOLDER;
		}
		// "����菜���č��ɋl�߂�
		TCHAR* p;
		TCHAR* q;
		p = q = token;
		while (*p) {
			if (*p != _T('"')) {
				if (p != q) {
					*q = *p;
				}
				++q;
			}
			++p;
		}
		*q = _T('\0');
			
		bool bRelPath = _IS_REL_PATH(token);
		int nValidStatus = ValidateKey(token);
		if (nValidStatus != 0) {
			return nValidStatus;
		}
		if (keyType == FILTER_SEARCH) {
			if (bRelPath) {
				push_back_unique(m_vecSearchFileKeys, token);
			}else {
//					push_back_unique(m_vecSearchAbsFileKeys, token);
//					push_back_unique(m_vecSearchFileKeys, token);
				return 2; // ��΃p�X�w��͕s��
			}
		}else if (keyType == FILTER_EXCEPT_FILE) {
			if (bRelPath) {
				push_back_unique(m_vecExceptFileKeys, token);
			}else {
				push_back_unique(m_vecExceptAbsFileKeys, token);
			}
		}else if (keyType == FILTER_EXCEPT_FOLDER) {
			if (bRelPath) {
				push_back_unique(m_vecExceptFolderKeys, token);
			}else {
				push_back_unique(m_vecExceptAbsFolderKeys, token);
			}
		}
	}
	if (m_vecSearchFileKeys.size() == 0) {
		push_back_unique(m_vecSearchFileKeys, wildcard_any);
	}
	if (m_vecSearchFolderKeys.size() == 0) {
		push_back_unique(m_vecSearchFolderKeys, wildcard_any);
	}
	return 0;
}

