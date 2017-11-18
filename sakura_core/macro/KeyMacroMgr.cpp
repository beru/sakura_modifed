/*!	@file
	@brief �L�[�{�[�h�}�N��
*/

#include "StdAfx.h"
#include <stdio.h>
#include <string.h>
#include "KeyMacroMgr.h"
#include "Macro.h"
#include "macro/SMacroMgr.h"
#include "charset/charcode.h"
#include "mem/Memory.h"
#include "MacroFactory.h"
#include "io/TextStream.h"

KeyMacroMgr::KeyMacroMgr()
{
	pTop = nullptr;
	pBot = nullptr;
	return;
}

KeyMacroMgr::~KeyMacroMgr()
{
	// �L�[�}�N���̃o�b�t�@���N���A����
	ClearAll();
	return;
}


// �L�[�}�N���̃o�b�t�@���N���A����
void KeyMacroMgr::ClearAll(void)
{
	Macro* p = pTop;
	Macro* del_p;
	while (p) {
		del_p = p;
		p = p->GetNext();
		delete del_p;
	}
	pTop = nullptr;
	pBot = nullptr;
	return;

}

/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
	�@�\�ԍ��ƁA�����ЂƂ�ǉ��ŁB
*/
void KeyMacroMgr::Append(
	EFunctionCode	nFuncID,
	const LPARAM*	lParams,
	EditView&		editView
	)
{
	auto macro = new Macro(nFuncID);
	macro->AddLParam(lParams, editView);
	Append(macro);
}

/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
	Macro���w�肵�Ēǉ������
*/
void KeyMacroMgr::Append(Macro* macro)
{
	if (pTop) {
		pBot->SetNext(macro);
		pBot = macro;
	}else {
		pTop = macro;
		pBot = pTop;
	}
	return;
}



/*! �L�[�{�[�h�}�N���̕ۑ�
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
bool KeyMacroMgr::SaveKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath) const
{
	TextOutputStream out(pszPath);
	if (!out) {
		return false;
	}

	// �ŏ��̃R�����g
	out.WriteF(LSW(STR_ERR_DLGKEYMACMGR1));

	// �}�N�����e
	Macro* p = pTop;
	while (p) {
		p->Save(hInstance, out);
		p = p->GetNext();
	}

	out.Close();
	return true;
}



/** �L�[�{�[�h�}�N���̎��s
	Macro�ɈϏ��B
*/
bool KeyMacroMgr::ExecKeyMacro(EditView& editView, int flags) const
{
	Macro* p = pTop;
	int macroflag = flags | FA_FROMMACRO;
	bool bRet = true;
	while (p) {
		if (!p->Exec(editView, macroflag)) {
			bRet = false;
			break;
		}
		p = p->GetNext();
	}
	return bRet;
}

/*! �L�[�{�[�h�}�N���̓ǂݍ���
	�G���[���b�Z�[�W�͏o���܂���B�Ăяo�����ł悫�ɂ͂�����Ă��������B
*/
bool KeyMacroMgr::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	// �L�[�}�N���̃o�b�t�@���N���A����
	ClearAll();

	TextInputStream in(pszPath);
	if (!in) {
		nReady = false;
		return false;
	}

	wchar_t szFuncName[100];
	wchar_t szFuncNameJapanese[256];
	EFunctionCode	nFuncID;
	size_t i;
	size_t nBgn;
	size_t nEnd;
	Macro* macro = nullptr;

	nReady = true;	// �G���[�������false�ɂȂ�
	std::tstring MACRO_ERROR_TITLE_string = LS(STR_ERR_DLGKEYMACMGR2);
	const TCHAR* MACRO_ERROR_TITLE = MACRO_ERROR_TITLE_string.c_str();

	int line = 1;	// �G���[���ɍs�ԍ���ʒm���邽�߁D1�n�܂�D
	for (; in.Good(); ++line) {
		std::wstring strLine = in.ReadLineW();
		const wchar_t* szLine = strLine.c_str(); // '\0'�I�[��������擾
		using namespace WCODE;

		size_t nLineLen = strLine.length();
		// ��s����󔒂��X�L�b�v
		for (i=0; i<nLineLen; ++i) {
			if (szLine[i] != SPACE && szLine[i] != TAB) {
				break;
			}
		}
		nBgn = i;
		// ��s�𖳎�����
		if (nBgn == nLineLen || szLine[nBgn] == LTEXT('\0')) {
			continue;
		}
		// �R�����g�s�̌��o
		//# �p�t�H�[�}���X�F'/'�̂Ƃ������Q�����ڂ��e�X�g
		if (szLine[nBgn] == LTEXT('/') && nBgn + 1 < nLineLen && szLine[nBgn + 1] == LTEXT('/')) {
			continue;
		}

		// �֐����̎擾
		szFuncName[0]='\0';// ������
		for (; i<nLineLen; ++i) {
			//# �o�b�t�@�I�[�o�[�����`�F�b�N
			if (szLine[i] == LTEXT('(') && (i - nBgn)< _countof(szFuncName)) {
				auto_memcpy(szFuncName, &szLine[nBgn], i - nBgn);
				szFuncName[i - nBgn] = L'\0';
				++i;
				nBgn = i;
				break;
			}
		}
		// �֐�����S_���t���Ă�����

		// �֐������@�\ID�C�@�\�����{��
		nFuncID = SMacroMgr::GetFuncInfoByName(hInstance, szFuncName, szFuncNameJapanese);
		if (nFuncID != -1) {
			macro = new Macro(nFuncID);
			// �v���g�^�C�v�`�F�b�N�p�ɒǉ�
			int nArgs;
			const MacroFuncInfo* mInfo= SMacroMgr::GetFuncInfoByID(nFuncID);
			int nArgSizeMax = _countof(mInfo->varArguments);
			if (mInfo->pData) {
				nArgSizeMax = mInfo->pData->nArgMaxSize;
			}
			for (nArgs=0; szLine[i]; ++nArgs) {
				// �v���g�^�C�v�`�F�b�N
				if (nArgs >= nArgSizeMax) {
					::MYMESSAGEBOX(
						NULL,
						MB_OK | MB_ICONSTOP | MB_TOPMOST,
						MACRO_ERROR_TITLE,
						LS(STR_ERR_DLGKEYMACMGR3),
						line,
						i + 1
					);
					nReady = false;
				}
				VARTYPE type = VT_EMPTY;
				if (nArgs < 4) {
					type = mInfo->varArguments[nArgs];
				}else {
					if (mInfo->pData && nArgs < mInfo->pData->nArgMinSize){
						type = mInfo->pData->pVarArgEx[nArgs - 4];
					}
				}

				// Skip Space
				while (szLine[i] == LTEXT(' ') || szLine[i] == LTEXT('\t')) {
					++i;
				}
				// PPA.DLL�}�N���ɂ��킹�Ďd�l�ύX�B�������''�ň͂ށB
				// double quotation�����e����
				if (LTEXT('\'') == szLine[i] || LTEXT('\"') == szLine[i]) {	// '�Ŏn�܂����當���񂾂悫���ƁB
					// �v���g�^�C�v�`�F�b�N
					// �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
					if (type != VT_BSTR && 
						type != VT_EMPTY
					) {
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							LS(STR_ERR_DLGKEYMACMGR4),
							line,
							i + 1,
							szFuncName,
							nArgs + 1
						);
						nReady = false;
						break;
					}
					wchar_t cQuote = szLine[i];
					++i;
					nBgn = nEnd = i;	// nBgn�͈����̐擪�̕���
					// �s���̌��o�̂��߁C���[�v�񐔂�1���₵��
					for (; i<=nLineLen; ++i) {		// �Ō�̕���+1�܂ŃX�L����
						if (szLine[i] == LTEXT('\\')) {	// �G�X�P�[�v�̃X�L�b�v
							++i;
							continue;
						}
						if (szLine[i] == cQuote) {	// �n�܂�Ɠ���quotation�ŏI���B
							nEnd = i;	// nEnd�͏I���̎��̕����i'�j
							break;
						}
						if (i == nLineLen) {	//	�s���ɗ��Ă��܂���
							::MYMESSAGEBOX(
								NULL,
								MB_OK | MB_ICONSTOP | MB_TOPMOST,
								MACRO_ERROR_TITLE,
								LS(STR_ERR_DLGKEYMACMGR5),
								line,
								szFuncName,
								nArgs + 1,
								cQuote
							);
							nReady = false;
							nEnd = i - 1;	// nEnd�͏I���̎��̕����i'�j
							break;
						}
					}
					if (!nReady) {
						break;
					}

					NativeW memWork;
					memWork.SetString(strLine.c_str() + nBgn, nEnd - nBgn);
					memWork.Replace( L"\\\\", L"\\\1" ); // �ꎞ�u��(�ŏ��ɕK�v)
					memWork.Replace(LTEXT("\\\'"), LTEXT("\'"));

					// double quotation���G�X�P�[�v����
					memWork.Replace(LTEXT("\\\""), LTEXT("\""));
					memWork.Replace( L"\\r", L"\r" );
					memWork.Replace( L"\\n", L"\n" );
					memWork.Replace( L"\\t", L"\t" );
					{
						// \uXXXX �u��
						size_t nLen = memWork.GetStringLength();
						size_t nBegin = 0;
						const wchar_t* p = memWork.GetStringPtr();
						NativeW memTemp;
						for (size_t n=0; n<nLen; ++n) {
							if (n + 1 < nLen && p[n] == L'\\' && p[n+1] == L'u') {
								size_t k;
								for (k = n + 2;
									k < nLen
									&& k < n + 2 + 4
									&& (WCODE::Is09(p[k])
										|| (L'a' <= p[k] && p[k] <= L'f')
										|| (L'A' <= p[k] && p[k] <= L'F'));
									++k
								) {
								}
								memTemp.AppendString( p + nBegin, n - nBegin );
								nBegin = k;
								if (0 < k - n - 2) {
									wchar_t hex[5];
									wcsncpy( hex, &p[n+2], k - n - 2 );
									hex[k - n - 2] = L'\0';
									wchar_t* pEnd = NULL;
									wchar_t c = static_cast<wchar_t>(wcstol(hex, &pEnd, 16));
									memTemp.AppendString( &c, 1 );
								}
								n = k - 1;
							}
						}
						if (nBegin != 0) {
							if (0 < nLen - nBegin) {
								memTemp.AppendString( p + nBegin, nLen - nBegin );
							}
							memWork.swap( memTemp );
						}
					}
					memWork.Replace( L"\\\1", L"\\" ); // �ꎞ�u����\�ɖ߂�(�Ō�łȂ��Ƃ����Ȃ�)
					macro->AddStringParam( memWork.GetStringPtr(), memWork.GetStringLength() );	//	�����𕶎���Ƃ��Ēǉ�
				}else if (Is09(szLine[i]) || szLine[i] == L'-') {	// �����Ŏn�܂����琔����(-�L�����܂�)�B
					// �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
					if (type != VT_I4 &&
						type != VT_EMPTY
					) {
						::MYMESSAGEBOX(
							NULL,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							MACRO_ERROR_TITLE,
							LS(STR_ERR_DLGKEYMACMGR6),
							line,
							i + 1,
							szFuncName,
							nArgs + 1
						);
						nReady = false;
						break;
					}
					nBgn = nEnd = i;	// nBgn�͈����̐擪�̕���
					// �s���̌��o�̂��߁C���[�v�񐔂�1���₵��
					for (i=nBgn+1; i<=nLineLen; ++i) {		// �Ō�̕���+1�܂ŃX�L����
						if (Is09(szLine[i])) {	// �܂����l
//							++i;
							continue;
						}else {
							nEnd = i;	// �����̍Ō�̕���
							--i;
							break;
						}
					}

					NativeW memWork;
					memWork.SetString(strLine.c_str() + nBgn, nEnd - nBgn);
					// �����̒���quotation�͓����Ă��Ȃ���
					//memWork.Replace(L"\\\'", L"\'");
					//memWork.Replace(L"\\\\", L"\\");
					macro->AddIntParam( _wtoi(memWork.GetStringPtr()) );	//	�����𐔎��Ƃ��Ēǉ�
				}else if (szLine[i] == LTEXT(')')) {
					// ��������
					break;
				}else {
					// Parse Error:���@�G���[���ۂ��B
					nBgn = nEnd = i;
					::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
						LS(STR_ERR_DLGKEYMACMGR7), line, i + 1);
					nReady = false;
					break;
				}
				for (; i<nLineLen; ++i) {		// �Ō�̕����܂ŃX�L����
					if (szLine[i] == LTEXT(')') || szLine[i] == LTEXT(',')) {	// ,��������)��ǂݔ�΂�
						++i;
						break;
					}
				}
				if (szLine[i-1] == LTEXT(')')) {
					break;
				}
			}
			if (!nReady) {
				// �ǂ����ŃG���[���������炵��
				delete macro;
				break;
			}
			// �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
			Append(macro);
		}else {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
				LS(STR_ERR_DLGKEYMACMGR8), line, szFuncName);
			nReady = false;
			break;
		}
	}
	in.Close();

	// �}�N�����ɃG���[����������ُ�I���ł���悤�ɂ���D
	return nReady;
}

// �L�[�{�[�h�}�N���𕶎��񂩂�ǂݍ���
bool KeyMacroMgr::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	// �ꎞ�t�@�C�������쐬
	TCHAR szTempDir[_MAX_PATH];
	TCHAR szTempFile[_MAX_PATH];
	if (::GetTempPath(_MAX_PATH, szTempDir) == 0) return FALSE;
	if (::GetTempFileName(szTempDir, _T("mac"), 0, szTempFile) == 0) return FALSE;
	// �ꎞ�t�@�C���ɏ�������
	TextOutputStream out = TextOutputStream(szTempFile);
	out.WriteString(to_wchar(pszCode));
	out.Close();

	// �}�N���ǂݍ���
	bool bRet = LoadKeyMacro(hInstance, szTempFile);

	::DeleteFile(szTempFile);			// �ꎞ�t�@�C���폜

	return bRet;
}

/*!
	Factory
	@param ext [in] �I�u�W�F�N�g�����̔���Ɏg���g���q(������)
*/
MacroManagerBase* KeyMacroMgr::Creator(EditView& view, const TCHAR* ext)
{
	if (_tcscmp(ext, _T("mac")) == 0) {
		return new KeyMacroMgr;
	}
	return nullptr;
}

/*!	CKeyMacroManager�̓o�^ */
void KeyMacroMgr::Declare(void)
{
	// ��Ɏ��s
	MacroFactory::getInstance().RegisterCreator(Creator);
}

