/*!	@file
	@brief アウトライン解析ダイアログボックス

	@author Norio Nakatani
	@date 1998/06/23 新規作成
	@date 1998/12/04 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, hor
	Copyright (C) 2002, aroka, hor, YAZAKI, frozen
	Copyright (C) 2003, little YOSHI
	Copyright (C) 2005, genta
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <Windows.h>
#include "dlg/Dialog.h"
#include "doc/EditDoc.h"

class FuncInfo;
class FuncInfoArr; // 2002/2/10 aroka
class DataProfile;

#define OUTLINE_LAYOUT_FOREGROUND (0)
#define OUTLINE_LAYOUT_BACKGROUND (1)
#define OUTLINE_LAYOUT_FILECHANGED (2)

// ファイルツリー関連クラス
enum class FileTreeSettingFromType {
	Common,
	Type,
	File,
};

class FileTreeSetting {
public:
	std::vector<FileTreeItem>	items;		//!< ツリーアイテム
	bool		bProject;				//!< プロジェクトファイルモード
	SFilePath	m_szDefaultProjectIni;	//!< デフォルトiniファイル名
	SFilePath	m_szLoadProjectIni;		//!< 現在読み込んでいるiniファイル名
	FileTreeSettingFromType	m_eFileTreeSettingOrgType;
	FileTreeSettingFromType	m_eFileTreeSettingLoadType;
};


//!	アウトライン解析ダイアログボックス
class DlgFuncList : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgFuncList();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless(HINSTANCE, HWND, LPARAM, FuncInfoArr*, LayoutInt, LayoutInt, int, int, bool); // モードレスダイアログの表示
	void ChangeView(LPARAM);	// モードレス時：検索対象となるビューの変更
	bool IsDocking() { return m_eDockSide > DockSideType::Float; }
	DockSideType GetDockSide() { return m_eDockSide; }

protected:
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);	// 2007.11.07 ryoji 標準以外のメッセージを捕捉する

	CommonSetting_OutLine& CommonSet(void) { return m_pShareData->m_common.outline; }
	TypeConfig& TypeSet(void) { return type; }
	int& ProfDockSet() { return CommonSet().nOutlineDockSet; }
	bool& ProfDockSync() { return CommonSet().bOutlineDockSync; }
	bool& ProfDockDisp() { return (ProfDockSet() == 0)? CommonSet().bOutlineDockDisp: TypeSet().bOutlineDockDisp; }
	DockSideType& ProfDockSide() { return (ProfDockSet() == 0)? CommonSet().eOutlineDockSide: TypeSet().eOutlineDockSide; }
	int& ProfDockLeft() { return (ProfDockSet() == 0)? CommonSet().cxOutlineDockLeft: TypeSet().cxOutlineDockLeft; }
	int& ProfDockTop() { return (ProfDockSet() == 0)? CommonSet().cyOutlineDockTop: TypeSet().cyOutlineDockTop; }
	int& ProfDockRight() { return (ProfDockSet() == 0)? CommonSet().cxOutlineDockRight: TypeSet().cxOutlineDockRight; }
	int& ProfDockBottom() { return (ProfDockSet() == 0)? CommonSet().cyOutlineDockBottom: TypeSet().cyOutlineDockBottom; }
	void SetTypeConfig(TypeConfigNum, const TypeConfig&);

public:
	//! 現在の種別と同じなら
	bool CheckListType(int nOutLineType) const { return nOutLineType == m_nOutlineType; }
	void Redraw(int nOutLineType, int nListType, FuncInfoArr*, LayoutInt nCurLine, LayoutInt nCurCol);
	void Refresh(void);
	bool ChangeLayout(int nId);
	void OnOutlineNotify(WPARAM wParam, LPARAM lParam);
	void SyncColor(void);
	void SetWindowText(const TCHAR* szTitle);		// ダイアログタイトルの設定
	EFunctionCode GetFuncCodeRedraw(int outlineType);
	void LoadFileTreeSetting( FileTreeSetting&, SFilePath& );
	static void ReadFileTreeIni( DataProfile&, FileTreeSetting& );

protected:
	bool m_bInChangeLayout;

	FuncInfoArr*	m_pFuncInfoArr;	// 関数情報配列
	LayoutInt		m_nCurLine;			// 現在行
	LayoutInt		m_nCurCol;			// 現在桁
	int				m_nSortCol;			// ソートする列番号
	int				m_nSortColOld;		//!< ソートする列番号(OLD)
	bool			m_bSortDesc;		//!< 降順
	NativeW			m_memClipText;		// クリップボードコピー用テキスト
	bool			m_bLineNumIsCRLF;	// 行番号の表示 false=折り返し単位／true=改行単位
	int				m_nListType;		// 一覧の種類
public:
	int				m_nDocType;			// ドキュメントの種類
	int				m_nOutlineType;		// アウトライン解析の種別
	bool			m_bEditWndReady;	// エディタ画面の準備完了
protected:
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	BOOL OnNotify(WPARAM, LPARAM);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnMinMaxInfo(LPARAM lParam);
	BOOL OnDestroy(void); // 20060201 aroka
	BOOL OnCbnSelChange(HWND hwndCtl, int wID); // 2002/11/1 frozen
	BOOL OnContextMenu(WPARAM, LPARAM);
	void SetData();	// ダイアログデータの設定
	int GetData(void);	// ダイアログデータの取得

	/*
	||  実装ヘルパ関数
	*/
	BOOL OnJump( bool bCheckAutoClose = true, bool bFileJump = true );	//	bCheckAutoClose：「このダイアログを自動的に閉じる」をチェックするかどうか
	void SetTreeCpp(HWND);			// ツリーコントロールの初期化：C++メソッドツリー
	void SetTreeJava(HWND, bool);	// ツリーコントロールの初期化：Javaメソッドツリー
	void SetTree(bool tagjump = false, bool nolabel = false);		// ツリーコントロールの初期化：汎用品
	void SetTreeFile();				// ツリーコントロールの初期化：ファイルツリー
	void SetListVB( void );			// リストビューコントロールの初期化：VisualBasic		// Jul 10, 2003  little YOSHI
	void SetDocLineFuncList();

	void SetTreeFileSub( HTREEITEM, const TCHAR* );
	// 2002/11/1 frozen 
	void SortTree(HWND hWndTree, HTREEITEM htiParent);//!< ツリービューの項目をソートする（ソート基準はm_nSortTypeを使用）
#if 0
2002.04.01 YAZAKI SetTreeTxt()、SetTreeTxtNest()は廃止。GetTreeTextNextはもともと使用されていなかった。
	void SetTreeTxt(HWND);	// ツリーコントロールの初期化：テキストトピックツリー
	int SetTreeTxtNest(HWND, HTREEITEM, int, int, HTREEITEM*, int);
	void GetTreeTextNext(HWND, HTREEITEM, int);
#endif

	//	Apr. 23, 2005 genta リストビューのソートを関数として独立させた
	void SortListView(HWND hwndList, int sortcol);
	static int CALLBACK CompareFunc_Asc(LPARAM, LPARAM, LPARAM);
	static int CALLBACK CompareFunc_Desc(LPARAM, LPARAM, LPARAM);

	// 2001.12.03 hor
//	void SetTreeBookMark(HWND);		// ツリーコントロールの初期化：ブックマーク
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	void Key2Command(WORD);		//	キー操作→コマンド変換
	bool HitTestSplitter(int xPos, int yPos);
	int HitTestCaptionButton(int xPos, int yPos);
	INT_PTR OnNcCalcSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnNcHitTest(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnNcMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnNcLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnNcPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR OnTimer(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void GetDockSpaceRect(LPRECT pRect);
	void GetCaptionRect(LPRECT pRect);
	bool GetCaptionButtonRect(int nButton, LPRECT pRect);
	void DoMenu(POINT pt, HWND hwndFrom);
	BOOL PostOutlineNotifyToAllEditors(WPARAM wParam, LPARAM lParam);
	DockSideType GetDropRect(POINT ptDrag, POINT ptDrop, LPRECT pRect, bool bForceFloat);
	BOOL Track(POINT ptDrag);
	bool GetTreeFileFullName(HWND, HTREEITEM, std::tstring*, int*);
	bool TagJumpTimer(const TCHAR*, Point, bool);

private:
	//	May 18, 2001 genta
	/*!
		@brief アウトライン解析種別

		0: List, 1: Tree
	*/
	int	m_nViewType;

	// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 1/4
	// (無理矢理なのでどなたか修正お願いします)
	bool m_bWaitTreeProcess;

	// 2002/11/1 frozen
	//! ツリービューをソートする基準
	// 0 デフォルト(ノードに関連づけれられた値順)
	// 1 アルファベット順
	int m_nSortType;

	// 選択中の関数情報
	FuncInfo* m_funcInfo;
	std::tstring m_sJumpFile;

	const TCHAR* m_pszTimerJumpFile;
	Point		m_pointTimerJump;
	bool		m_bTimerJumpAutoClose;

	DockSideType	m_eDockSide;	// 現在の画面の表示位置
	HWND		m_hwndToolTip;	//!< ツールチップ（ボタン用）
	bool		m_bStretching;
	bool		m_bHovering;
	int			m_nHilightedBtn;
	int			m_nCapturingBtn;
	
	TypeConfig type;
	FileTreeSetting	m_fileTreeSetting;

	static LPDLGTEMPLATE m_pDlgTemplate;
	static DWORD m_dwDlgTmpSize;
	static HINSTANCE m_lastRcInstance;		// リソース生存チェック用

	POINT				m_ptDefaultSize;
	POINT				m_ptDefaultSizeClient;
	RECT				m_rcItems[12];
};

