// ファイルタイプ一覧ダイアログ

class DlgTypeList;

#pragma once

#include "dlg/Dialog.h"
using std::wstring;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief ファイルタイプ一覧ダイアログ
*/
class DlgTypeList : public Dialog {
public:
	// 型
	struct Result {
		TypeConfigNum	documentType;	// 文書種類
		bool			bTempChange;	// 旧PROP_TEMPCHANGE_FLAG
	};

public:
	// インターフェース
	INT_PTR DoModal(HINSTANCE, HWND, Result*);	// モーダルダイアログの表示

protected:
	// 実装ヘルパ関数
	BOOL OnLbnDblclk(int);
	BOOL OnBnClicked(int);
	BOOL OnActivate(WPARAM wParam, LPARAM lParam);
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
	void SetData();		// ダイアログデータの設定
	void SetData(int);	// ダイアログデータの設定
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	bool Import(void);			// 2010/4/12 Uchi
	bool Export(void);			// 2010/4/12 Uchi
	bool InitializeType(void);	// 2010/4/12 Uchi
	bool CopyType();
	bool UpType();
	bool DownType();
	bool AddType();
	bool DelType();
	bool AlertFileAssociation();	// 2011/8/20 syat

private:
	TypeConfigNum nSettingType;
	// 関連付け状態
	bool bRegistryChecked[MAX_TYPES];	// レジストリ確認 未／済
	bool bExtRMenu[MAX_TYPES];			// 右クリック登録 未／済
	bool bExtDblClick[MAX_TYPES];		// ダブルクリック 未／済
	bool bAlertFileAssociation;			// 関連付け警告の表示フラグ
	bool bEnableTempChange;				// 一時適用の有効化
};

