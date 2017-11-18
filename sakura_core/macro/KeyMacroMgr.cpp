/*!	@file
	@brief キーボードマクロ
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
	// キーマクロのバッファをクリアする
	ClearAll();
	return;
}


// キーマクロのバッファをクリアする
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

/*! キーマクロのバッファにデータ追加
	機能番号と、引数ひとつを追加版。
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

/*! キーマクロのバッファにデータ追加
	Macroを指定して追加する版
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



/*! キーボードマクロの保存
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
bool KeyMacroMgr::SaveKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath) const
{
	TextOutputStream out(pszPath);
	if (!out) {
		return false;
	}

	// 最初のコメント
	out.WriteF(LSW(STR_ERR_DLGKEYMACMGR1));

	// マクロ内容
	Macro* p = pTop;
	while (p) {
		p->Save(hInstance, out);
		p = p->GetNext();
	}

	out.Close();
	return true;
}



/** キーボードマクロの実行
	Macroに委譲。
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

/*! キーボードマクロの読み込み
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
bool KeyMacroMgr::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	// キーマクロのバッファをクリアする
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

	nReady = true;	// エラーがあればfalseになる
	std::tstring MACRO_ERROR_TITLE_string = LS(STR_ERR_DLGKEYMACMGR2);
	const TCHAR* MACRO_ERROR_TITLE = MACRO_ERROR_TITLE_string.c_str();

	int line = 1;	// エラー時に行番号を通知するため．1始まり．
	for (; in.Good(); ++line) {
		std::wstring strLine = in.ReadLineW();
		const wchar_t* szLine = strLine.c_str(); // '\0'終端文字列を取得
		using namespace WCODE;

		size_t nLineLen = strLine.length();
		// 先行する空白をスキップ
		for (i=0; i<nLineLen; ++i) {
			if (szLine[i] != SPACE && szLine[i] != TAB) {
				break;
			}
		}
		nBgn = i;
		// 空行を無視する
		if (nBgn == nLineLen || szLine[nBgn] == LTEXT('\0')) {
			continue;
		}
		// コメント行の検出
		//# パフォーマンス：'/'のときだけ２文字目をテスト
		if (szLine[nBgn] == LTEXT('/') && nBgn + 1 < nLineLen && szLine[nBgn + 1] == LTEXT('/')) {
			continue;
		}

		// 関数名の取得
		szFuncName[0]='\0';// 初期化
		for (; i<nLineLen; ++i) {
			//# バッファオーバーランチェック
			if (szLine[i] == LTEXT('(') && (i - nBgn)< _countof(szFuncName)) {
				auto_memcpy(szFuncName, &szLine[nBgn], i - nBgn);
				szFuncName[i - nBgn] = L'\0';
				++i;
				nBgn = i;
				break;
			}
		}
		// 関数名にS_が付いていたら

		// 関数名→機能ID，機能名日本語
		nFuncID = SMacroMgr::GetFuncInfoByName(hInstance, szFuncName, szFuncNameJapanese);
		if (nFuncID != -1) {
			macro = new Macro(nFuncID);
			// プロトタイプチェック用に追加
			int nArgs;
			const MacroFuncInfo* mInfo= SMacroMgr::GetFuncInfoByID(nFuncID);
			int nArgSizeMax = _countof(mInfo->varArguments);
			if (mInfo->pData) {
				nArgSizeMax = mInfo->pData->nArgMaxSize;
			}
			for (nArgs=0; szLine[i]; ++nArgs) {
				// プロトタイプチェック
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
				// PPA.DLLマクロにあわせて仕様変更。文字列は''で囲む。
				// double quotationも許容する
				if (LTEXT('\'') == szLine[i] || LTEXT('\"') == szLine[i]) {	// 'で始まったら文字列だよきっと。
					// プロトタイプチェック
					// 余分な引数を無視するよう，VT_EMPTYを許容する．
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
					nBgn = nEnd = i;	// nBgnは引数の先頭の文字
					// 行末の検出のため，ループ回数を1増やした
					for (; i<=nLineLen; ++i) {		// 最後の文字+1までスキャン
						if (szLine[i] == LTEXT('\\')) {	// エスケープのスキップ
							++i;
							continue;
						}
						if (szLine[i] == cQuote) {	// 始まりと同じquotationで終了。
							nEnd = i;	// nEndは終わりの次の文字（'）
							break;
						}
						if (i == nLineLen) {	//	行末に来てしまった
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
							nEnd = i - 1;	// nEndは終わりの次の文字（'）
							break;
						}
					}
					if (!nReady) {
						break;
					}

					NativeW memWork;
					memWork.SetString(strLine.c_str() + nBgn, nEnd - nBgn);
					memWork.Replace( L"\\\\", L"\\\1" ); // 一時置換(最初に必要)
					memWork.Replace(LTEXT("\\\'"), LTEXT("\'"));

					// double quotationもエスケープ解除
					memWork.Replace(LTEXT("\\\""), LTEXT("\""));
					memWork.Replace( L"\\r", L"\r" );
					memWork.Replace( L"\\n", L"\n" );
					memWork.Replace( L"\\t", L"\t" );
					{
						// \uXXXX 置換
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
					memWork.Replace( L"\\\1", L"\\" ); // 一時置換を\に戻す(最後でないといけない)
					macro->AddStringParam( memWork.GetStringPtr(), memWork.GetStringLength() );	//	引数を文字列として追加
				}else if (Is09(szLine[i]) || szLine[i] == L'-') {	// 数字で始まったら数字列だ(-記号も含む)。
					// 余分な引数を無視するよう，VT_EMPTYを許容する．
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
					nBgn = nEnd = i;	// nBgnは引数の先頭の文字
					// 行末の検出のため，ループ回数を1増やした
					for (i=nBgn+1; i<=nLineLen; ++i) {		// 最後の文字+1までスキャン
						if (Is09(szLine[i])) {	// まだ数値
//							++i;
							continue;
						}else {
							nEnd = i;	// 数字の最後の文字
							--i;
							break;
						}
					}

					NativeW memWork;
					memWork.SetString(strLine.c_str() + nBgn, nEnd - nBgn);
					// 数字の中にquotationは入っていないよ
					//memWork.Replace(L"\\\'", L"\'");
					//memWork.Replace(L"\\\\", L"\\");
					macro->AddIntParam( _wtoi(memWork.GetStringPtr()) );	//	引数を数字として追加
				}else if (szLine[i] == LTEXT(')')) {
					// 引数無し
					break;
				}else {
					// Parse Error:文法エラーっぽい。
					nBgn = nEnd = i;
					::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
						LS(STR_ERR_DLGKEYMACMGR7), line, i + 1);
					nReady = false;
					break;
				}
				for (; i<nLineLen; ++i) {		// 最後の文字までスキャン
					if (szLine[i] == LTEXT(')') || szLine[i] == LTEXT(',')) {	// ,もしくは)を読み飛ばす
						++i;
						break;
					}
				}
				if (szLine[i-1] == LTEXT(')')) {
					break;
				}
			}
			if (!nReady) {
				// どこかでエラーがあったらしい
				delete macro;
				break;
			}
			// キーマクロのバッファにデータ追加
			Append(macro);
		}else {
			::MYMESSAGEBOX(NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, MACRO_ERROR_TITLE,
				LS(STR_ERR_DLGKEYMACMGR8), line, szFuncName);
			nReady = false;
			break;
		}
	}
	in.Close();

	// マクロ中にエラーがあったら異常終了できるようにする．
	return nReady;
}

// キーボードマクロを文字列から読み込み
bool KeyMacroMgr::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	// 一時ファイル名を作成
	TCHAR szTempDir[_MAX_PATH];
	TCHAR szTempFile[_MAX_PATH];
	if (::GetTempPath(_MAX_PATH, szTempDir) == 0) return FALSE;
	if (::GetTempFileName(szTempDir, _T("mac"), 0, szTempFile) == 0) return FALSE;
	// 一時ファイルに書き込む
	TextOutputStream out = TextOutputStream(szTempFile);
	out.WriteString(to_wchar(pszCode));
	out.Close();

	// マクロ読み込み
	bool bRet = LoadKeyMacro(hInstance, szTempFile);

	::DeleteFile(szTempFile);			// 一時ファイル削除

	return bRet;
}

/*!
	Factory
	@param ext [in] オブジェクト生成の判定に使う拡張子(小文字)
*/
MacroManagerBase* KeyMacroMgr::Creator(EditView& view, const TCHAR* ext)
{
	if (_tcscmp(ext, _T("mac")) == 0) {
		return new KeyMacroMgr;
	}
	return nullptr;
}

/*!	CKeyMacroManagerの登録 */
void KeyMacroMgr::Declare(void)
{
	// 常に実行
	MacroFactory::getInstance().RegisterCreator(Creator);
}

