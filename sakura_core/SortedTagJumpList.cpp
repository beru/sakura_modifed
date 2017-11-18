#include "StdAfx.h"
#include "SortedTagJumpList.h"

/*!
	@date 2005.04.23 genta 管理数の最大値を指定する引数追加
*/
SortedTagJumpList::SortedTagJumpList(int max)
	:
	pTagjump(nullptr),
	nCount(0),
	bOverflow(false),
	capacity(max)
{
	// id == 0 を 空文字列にする
	baseDirArr.push_back(_T(""));
}

SortedTagJumpList::~SortedTagJumpList()
{
	Empty();
}

/*
	指定されたアイテムのメモリを解放する。

	@param[in] item 削除するアイテム
*/
void SortedTagJumpList::Free(TagJumpInfo* item)
{
	free(item->keyword);
	free(item->filename);
	free(item->note);
	free(item);
	return;
}

/*
	リストをすべて解放する。
*/
void SortedTagJumpList::Empty(void)
{
	auto p = pTagjump;
	while (p) {
		auto next = p->next;
		Free(p);
		p = next;
	}
	pTagjump = nullptr;
	nCount = 0;
	bOverflow = false;
	baseDirArr.clear();
	baseDirArr.push_back(_T(""));
}

/*
	基準フォルダを登録し、基準フォルダIDを取得
	@date 2010.07.23 Moca 新規追加
*/
int SortedTagJumpList::AddBaseDir(const TCHAR* baseDir)
{
	baseDirArr.push_back(baseDir);
	return baseDirArr.size() - 1;
}

/*
	アイテムをソートされた状態でリストに追加する。
	アイテムが最大数を超える場合は、超えるアイテムを削除する。
	文字列はコピーを作成するので、呼び出し側は文字列のアドレス先を保持する必要はない。
	
	@param[in] keyword	キーワード
	@param[in] filename	ファイル名
	@param[in] no		行番号
	@param[in] type		種類
	@param[in] note		備考
	@param[in] depth	(さかのぼる)階層
	@param[in] baseDirId	基準フォルダID。0で空文字列指定 (AddBaseDirの戻り値)
	@retval true  追加した
	@retval false 追加失敗
	@date 2010.07.23 Moca baseDirId 追加
*/
bool SortedTagJumpList::AddParamA(
	const char* keyword,
	const char* filename,
	int no,
	char type,
	const char* note,
	int depth,
	int baseDirId
	)
{
	TagJumpInfo*	p;
	TagJumpInfo*	prev;
	TagJumpInfo*	item;
	// 3つめはSJIS用保険
	char typeStr[] = {type, '\0', '\0'};

	// アイテムを作成する。
	item = (TagJumpInfo*)malloc(sizeof(TagJumpInfo));
	if (!item) {
		return false;
	}
	item->keyword  = _tcsdup(to_tchar(keyword));
	item->filename = _tcsdup(to_tchar(filename));
	item->no       = no;
	item->type     = to_tchar(typeStr)[0];
	item->note     = _tcsdup(to_tchar(note));
	item->depth    = depth;
	item->next     = nullptr;
	item->baseDirId = baseDirId;

	// 文字列長ガード
	if (_tcslen(item->keyword	) >= MAX_TAG_STRING_LENGTH) item->keyword[ MAX_TAG_STRING_LENGTH-1] = 0;
	if (_tcslen(item->filename	) >= MAX_TAG_STRING_LENGTH) item->filename[MAX_TAG_STRING_LENGTH-1] = 0;
	if (_tcslen(item->note		) >= MAX_TAG_STRING_LENGTH) item->note[    MAX_TAG_STRING_LENGTH-1] = 0;

	// アイテムをリストの適当な位置に追加する。
	prev = nullptr;
	for (p=pTagjump; p; p=p->next) {
		if (_tcscmp(p->keyword, item->keyword) > 0) {
			break;
		}
		prev = p;
	}
	item->next = p;
	if (prev) prev->next = item;
	else      pTagjump = item;
	++nCount;

	// 最大数を超えたら最後のアイテムを削除する。
	if (nCount > capacity) {
		prev = nullptr;
		for (p=pTagjump; p->next; p=p->next) {
			prev = p;
		}
		if (prev) prev->next = nullptr;
		else      pTagjump = nullptr;
		Free(p);
		nCount--;
		bOverflow = true;
	}
	return true;
}

/*
	指定の情報を取得する。

	@param[out] keyword		キーワード
	@param[out] filename	ファイル名
	@param[out] no			行番号
	@param[out] type		種類
	@param[out] note		備考
	@param[out] depth		(さかのぼる)階層
	@param[out] baseDir		ファイル名の基準フォルダ
	@return 処理結果

	@note 不要な情報の場合は引数に NULL を指定する。
*/
bool SortedTagJumpList::GetParam(
	int index,
	TCHAR* keyword,
	TCHAR* filename,
	int* no,
	TCHAR* type,
	TCHAR* note,
	int* depth,
	TCHAR* baseDir
	)
{
	if (keyword ) keyword[0] = 0;
	if (filename) filename[0] = 0;
	if (no      ) *no    = 0;
	if (type    ) *type  = 0;
	if (note    ) note[0] = 0;
	if (depth   ) *depth = 0;
	if (baseDir ) baseDir[0] = 0;

	SortedTagJumpList::TagJumpInfo* p;
	p = GetPtr(index);
	if (p) {
		if (keyword ) _tcscpy(keyword, p->keyword);
		if (filename) _tcscpy(filename, p->filename);
		if (no      ) *no    = p->no;
		if (type    ) *type  = p->type;
		if (note    ) _tcscpy(note, p->note);
		if (depth   ) *depth = p->depth;
		if (baseDir) {
			if (0 <= p->baseDirId && (size_t)p->baseDirId < baseDirArr.size()) {
				auto_strcpy(baseDir, baseDirArr[p->baseDirId].c_str());
			}
		}
		return true;
	}
	return false;
}

/*
	指定の情報を構造体ポインタで取得する。
	取得した情報は参照なので解放してはならない。

	@param[in] index 要素番号
	@return タグジャンプ情報
*/
SortedTagJumpList::TagJumpInfo* SortedTagJumpList::GetPtr(int index)
{
	int	i = 0;
	for (auto p=pTagjump; p; p=p->next) {
		if (index == i) {
			return p;
		}
		++i;
	}
	return nullptr;
}

