/*!	@file
	@brief タグジャンプリストダイアログボックス
*/

#pragma once

#include "dlg/Dialog.h"
#include "recent/RecentTagjumpKeyword.h"

// タグファイル名
#define TAG_FILENAME_T _T("tags")

class SortedTagJumpList;

/*!	@brief ダイレクトタグジャンプ候補一覧ダイアログ

	ダイレクトタグジャンプで複数の候補がある場合及び
	キーワード指定タグジャンプのためのダイアログボックス制御
*/
class DlgTagJumpList : public Dialog {
public:
	/*
	||  Constructors
	*/
	DlgTagJumpList(bool bDirectTagJump);
	~DlgTagJumpList();

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoModal(HINSTANCE, HWND, LPARAM);	// モーダルダイアログの表示 

//	bool AddParamA(const char*, const char*, int, const char*, const char*, int depth, int baseDirId);	// 登録
	bool GetSelectedParam(TCHAR* s0, TCHAR* s1, int* n2, TCHAR* s3, TCHAR* s4, int* depth, TCHAR* fileBase);	// 取得
	void SetFileName(const TCHAR* pszFileName);
	void SetKeyword(const wchar_t* pszKeyword);
	int  FindDirectTagJump();

	bool GetSelectedFullPathAndLine(TCHAR* fullPath, int count, int* lineNum, int* depth);

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND, WPARAM wParam, LPARAM lParam);
	BOOL	OnBnClicked(int);
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
	BOOL	OnSize(WPARAM wParam, LPARAM lParam);
	BOOL	OnMove(WPARAM wParam, LPARAM lParam);
	BOOL	OnMinMaxInfo(LPARAM lParam);
	BOOL	OnNotify(WPARAM wParam, LPARAM lParam);
	// @@ 2005.03.31 MIK キーワード入力エリアのイベント処理
	BOOL	OnCbnSelChange(HWND hwndCtl, int wID);
	BOOL	OnCbnEditChange(HWND hwndCtl, int wID);
	//BOOL	OnEnChange(HWND hwndCtl, int wID);
	BOOL	OnTimer(WPARAM wParam);
	LPVOID	GetHelpIdTable(void);

private:
	void	StopTimer(void);
	void	StartTimer(int);

	void	SetData(void);	// ダイアログデータの設定
	int		GetData(void);	// ダイアログデータの取得
	void	UpdateData(bool);

	TCHAR*	GetNameByType(const TCHAR type, const TCHAR* name);	// タイプを名前に変換する。
	int		SearchBestTag(void);	// もっとも確率の高そうなインデックスを返す。
	const TCHAR* GetFileName(void);
	const TCHAR* GetFilePath(void) { return pszFileName ? pszFileName : _T(""); }
	void Empty(void);
	void SetTextDir();
	void FindNext(bool);
	void find_key(const wchar_t* keyword);
	int find_key_core(int, const wchar_t*, bool, bool, bool, bool, int);
	
	bool IsDirectTagJump();
	
	void ClearPrevFindInfo();
	bool GetFullPathAndLine(int index, TCHAR *fullPath, int count, int *lineNum, int *depth);


	// depthから完全パス名(相対パス/絶対パス)を作成する
	static TCHAR* GetFullPathFromDepth(TCHAR*, int, TCHAR*, const TCHAR*, int);
	static TCHAR* CopyDirDir(TCHAR* dest, const TCHAR* target, const TCHAR* base);
public:
	static int CalcMaxUpDirectory(const TCHAR*);
	static TCHAR* DirUp(TCHAR* dir);

private:

	struct TagFindState {
		int   nDepth;
		int   nMatchAll;
		int   nNextMode;
		int   nLoop;
		bool  bJumpPath;
		TCHAR szCurPath[1024];
	};
	
	bool	bDirectTagJump;

	int			nIndex;				// 選択された要素番号
	TCHAR*		pszFileName;		// 編集中のファイル名
	wchar_t*	pszKeyword;			// キーワード(DoModalのlParam != 0を指定した場合に指定できる)
	int			nLoop;				// さかのぼれる階層数
	SortedTagJumpList*	pList;		// タグジャンプ情報
	UINT	nTimerId;				// タイマ番号
	bool	bTagJumpICase;			// 大文字小文字を同一視
	bool	bTagJumpAnyWhere;		// 文字列の途中にマッチ
	bool	bTagJumpExactMatch;		// 完全一致(画面無し)

	int 	nTop;					// ページめくりの表示の先頭(0開始)
	bool	bNextItem;				// まだ次にヒットするものがある

	// 絞り込み検索用
	TagFindState* psFindPrev;		// 前回の最後に検索した状態
	TagFindState* psFind0Match;		// 前回の1つもHitしなかった最後のtags

	NativeW	strOldKeyword;			// 前回のキーワード
	bool	bOldTagJumpICase;		// 前回の大文字小文字を同一視
	bool	bOldTagJumpAnyWhere;	// 前回の文字列の途中にマッチ

	ComboBoxItemDeleter		comboDel;
	RecentTagJumpKeyword	recentKeyword;
	
	POINT	ptDefaultSize;
	RECT	rcItems[11];

private:
	DISALLOW_COPY_AND_ASSIGN(DlgTagJumpList);
};


