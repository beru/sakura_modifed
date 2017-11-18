// タイプ別設定インポート確認ダイアログ

class DlgTypeAscertain;

#pragma once

using std::wstring;
using std::tstring;

#include "dlg/Dialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ファイルタイプ一覧ダイアログ
*/
class DlgTypeAscertain : public Dialog {
public:
	// 型
	struct AscertainInfo {
		tstring	sImportFile;	// in インポートファイル名
		wstring	sTypeNameTo;	// in タイプ名（インポート先）
		wstring	sTypeNameFile;	// in タイプ名（ファイルから）
		int 	nColorType;		// out 文書種類(カラーコピー用)
		wstring	sColorFile;		// out 色設定ファイル名
		bool	bAddType;		// out タイプを追加する
	};

public:
	// Constructors
	DlgTypeAscertain();
	// モーダルダイアログの表示
	INT_PTR DoModal(HINSTANCE, HWND, AscertainInfo*);	// モーダルダイアログの表示

protected:
	// 実装ヘルパ関数
	BOOL OnBnClicked(int);
	void SetData();	// ダイアログデータの設定
	LPVOID GetHelpIdTable(void);

private:
	AscertainInfo* psi;			// インターフェイス
};

