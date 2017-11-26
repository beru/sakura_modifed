#pragma once

#include "util/design_template.h"

// タグジャンプリスト

#define MAX_TAG_STRING_LENGTH _MAX_PATH	// 管理する文字列の最大長

/*!	@brief ダイレクトタグジャンプ用検索結果をソートして保持する．*/
class SortedTagJumpList {
public:
	SortedTagJumpList(int max);
	~SortedTagJumpList();

	int AddBaseDir(const TCHAR* baseDir);
	bool AddParamA(const char* keyword, const char* filename, int no, char type, const char* note, int depth, const int baseDirId);
	bool GetParam(int index, TCHAR* keyword, TCHAR* filename, int* no, TCHAR* type, TCHAR* note, int* depth, TCHAR* baseDir);
	int GetCount(void) { return nCount; }
	void Empty(void);
	bool IsOverflow(void) { return bOverflow; }

	typedef struct tagjump_info_t {
		struct tagjump_info_t*	next;	// 次のリスト
		TCHAR*	keyword;	// キーワード
		TCHAR*	filename;	// ファイル名
		int		no;			// 行番号
		TCHAR	type;		// 種類
		TCHAR*	note;		// 備考
		int		depth;		// (さかのぼる)階層
		int		baseDirId;	// ファイル名のベースディレクトリ
	} TagJumpInfo;

	TagJumpInfo* GetPtr(int index);

	/*!	@brief 管理数の最大値を取得する */
	int GetCapacity(void) const { return capacity; }

private:
	TagJumpInfo* pTagjump;	// タグジャンプ情報
	std::vector<std::tstring> baseDirArr;	// ベースディレクトリ情報
	int		nCount;		// 個数
	bool	bOverflow;	// オーバーフロー
	
	const int	capacity;	// 管理する情報の最大数

	void Free(TagJumpInfo* item);

private:
	DISALLOW_COPY_AND_ASSIGN(SortedTagJumpList);
};

