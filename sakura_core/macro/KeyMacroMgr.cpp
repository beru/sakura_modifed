/*!	@file
	@brief �L�[�{�[�h�}�N��

	@author Norio Nakatani

	@date 20011229 aroka �o�O�C���A�R�����g�ǉ�
	YAZAKI �g�ւ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka
	Copyright (C) 2002, YAZAKI, aroka, genta
	Copyright (C) 2004, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdio.h>
#include <string.h>
#include "KeyMacroMgr.h"
#include "Macro.h"
#include "macro/SMacroMgr.h"// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/Memory.h"
#include "MacroFactory.h"
#include "io/TextStream.h"

KeyMacroMgr::KeyMacroMgr()
{
	pTop = nullptr;
	pBot = nullptr;
//	nKeyMacroDataArrNum = 0;	2002.2.2 YAZAKI
	// Apr. 29, 2002 genta
	// nReady��MacroManagerBase��
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
//	nKeyMacroDataArrNum = 0;	2002.2.2 YAZAKI
	pTop = nullptr;
	pBot = nullptr;
	return;

}

/*! �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�
	�@�\�ԍ��ƁA�����ЂƂ�ǉ��ŁB
	@date 2002.2.2 YAZAKI pEditView���n���悤�ɂ����B
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
//	nKeyMacroDataArrNum++;	2002.2.2 YAZAKI
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
	
	@date 2007.07.20 genta flags�ǉ��DMacro::Exec()��
		FA_FROMMACRO���܂߂��l��n���D
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

	WCHAR	szFuncName[100];
	WCHAR	szFuncNameJapanese[256];
	EFunctionCode	nFuncID;
	size_t	i;
	int		nBgn, nEnd;
	Macro* macro = nullptr;

	// Jun. 16, 2002 genta
	nReady = true;	// �G���[�������false�ɂȂ�
	std::tstring MACRO_ERROR_TITLE_string = LS(STR_ERR_DLGKEYMACMGR2);
	const TCHAR* MACRO_ERROR_TITLE = MACRO_ERROR_TITLE_string.c_str();

	int line = 1;	// �G���[���ɍs�ԍ���ʒm���邽�߁D1�n�܂�D
	for (; in.Good(); ++line) {
		std::wstring strLine = in.ReadLineW();
		const WCHAR* szLine = strLine.c_str(); // '\0'�I�[��������擾
		using namespace WCODE;

		size_t nLineLen = strLine.length();
		// ��s����󔒂��X�L�b�v
		for (i=0; i<nLineLen; ++i) {
			if (szLine[i] != SPACE && szLine[i] != TAB) {
				break;
			}
		}
		nBgn = i;
		// Jun. 16, 2002 genta ��s�𖳎�����
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
		//@@@ 2002.2.2 YAZAKI �}�N����SMacroMgr�ɓ���
		nFuncID = SMacroMgr::GetFuncInfoByName(hInstance, szFuncName, szFuncNameJapanese);
		if (nFuncID != -1) {
			macro = new Macro(nFuncID);
			// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N�p�ɒǉ�
			int nArgs;
			const MacroFuncInfo* mInfo= SMacroMgr::GetFuncInfoByID(nFuncID);
			int nArgSizeMax = _countof(mInfo->varArguments);
			if (mInfo->pData) {
				nArgSizeMax = mInfo->pData->nArgMaxSize;
			}
			for (nArgs=0; szLine[i]; ++nArgs) {
				// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
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
				//@@@ 2002.2.2 YAZAKI PPA.DLL�}�N���ɂ��킹�Ďd�l�ύX�B�������''�ň͂ށB
				// Jun. 16, 2002 genta double quotation�����e����
				if (LTEXT('\'') == szLine[i] || LTEXT('\"') == szLine[i]) {	// '�Ŏn�܂����當���񂾂悫���ƁB
					// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
					// Jun. 27, 2002 genta �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
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
					WCHAR cQuote = szLine[i];
					++i;
					nBgn = nEnd = i;	// nBgn�͈����̐擪�̕���
					// Jun. 16, 2002 genta
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
					// Jun. 16, 2002 genta
					if (!nReady) {
						break;
					}

					NativeW memWork;
					memWork.SetString(strLine.c_str() + nBgn, nEnd - nBgn);
					// 2014.01.28 �u"\\'"�v�̂悤�ȏꍇ�̕s����C��
					memWork.Replace( L"\\\\", L"\\\1" ); // �ꎞ�u��(�ŏ��ɕK�v)
					memWork.Replace(LTEXT("\\\'"), LTEXT("\'"));

					// Jun. 16, 2002 genta double quotation���G�X�P�[�v����
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
					// Jun. 16, 2002 genta �v���g�^�C�v�`�F�b�N
					// Jun. 27, 2002 genta �]���Ȉ����𖳎�����悤�CVT_EMPTY�����e����D
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
					// Jun. 16, 2002 genta
					// �����̒���quotation�͓����Ă��Ȃ���
					//memWork.Replace(L"\\\'", L"\'");
					//memWork.Replace(L"\\\\", L"\\");
					macro->AddIntParam( _wtoi(memWork.GetStringPtr()) );	//	�����𐔎��Ƃ��Ēǉ�
				// Jun. 16, 2002 genta
				}else if (szLine[i] == LTEXT(')')) {
					// ��������
					break;
				}else {
					// Parse Error:���@�G���[���ۂ��B
					// Jun. 16, 2002 genta
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
			// Jun. 16, 2002 genta
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
			// Jun. 16, 2002 genta
			nReady = false;
			break;
		}
	}
	in.Close();

	// Jun. 16, 2002 genta
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

// From Here Apr. 29, 2002 genta
/*!
	Factory

	@param ext [in] �I�u�W�F�N�g�����̔���Ɏg���g���q(������)

	@date 2004-01-31 genta RegisterExt�̔p�~�̂���RegisterCreator�ɒu������
		���̂��߁C�߂����I�u�W�F�N�g�������s��Ȃ����߂Ɋg���q�`�F�b�N�͕K�{�D
*/
MacroManagerBase* KeyMacroMgr::Creator(EditView& view, const TCHAR* ext)
{
	if (_tcscmp(ext, _T("mac")) == 0) {
		return new KeyMacroMgr;
	}
	return nullptr;
}

/*!	CKeyMacroManager�̓o�^

	@date 2004.01.31 genta RegisterExt�̔p�~�̂���RegisterCreator�ɒu������
*/
void KeyMacroMgr::Declare(void)
{
	// ��Ɏ��s
	MacroFactory::getInstance().RegisterCreator(Creator);
}
// To Here Apr. 29, 2002 genta

