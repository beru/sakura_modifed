#include "StdAfx.h"
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "outline/FuncInfoArr.h"

/** Erlang アウトライン解析 管理＆解析
*/
struct OutlineErlang {
	enum {
		STATE_NORMAL,				// 解析中でない
		STATE_FUNC_CANDIDATE_FIN,	// 関数らしきもの(行頭のatom)を解析済み
		STATE_FUNC_ARGS1,			// 最初の引数確認中
		STATE_FUNC_ARGS,			// 2つめ以降の引数確認中
		STATE_FUNC_ARGS_FIN,		// 関数の解析を完了
		STATE_FUNC_FOUND,			// 関数を発見．データの取得が可能
	} state;

	wchar_t func[64];			// 関数名(Arity含む) = 表示名
	size_t lnum;					// 関数の行番号
	int argcount;				// 発見した引数の数
	wchar_t parenthesis[32];	// 括弧のネストを管理するもの
	int parenthesis_ptr;		// 括弧のネストレベル
	
	OutlineErlang();
	bool Parse(const wchar_t* buf, size_t linelen, size_t linenum);
	
	const wchar_t* ScanFuncName(const wchar_t* buf, const wchar_t* end, const wchar_t* p);
	const wchar_t* EnterArgs(const wchar_t* end, const wchar_t* p);
	const wchar_t* ScanArgs1(const wchar_t* end, const wchar_t* p);
	const wchar_t* ScanArgs(const wchar_t* end, const wchar_t* p);
	const wchar_t* EnterCond(const wchar_t* end, const wchar_t* p);
	const wchar_t* GetFuncName() const { return func; }
	size_t GetFuncLine() const { return lnum; }

private:
	// helper functions
	bool IsAtomHead(wchar_t wc)
	{
		return (L'a' <= wc && wc <= L'z')
			|| (wc == L'_') || (wc == L'@');
	}

	bool IsAlNum(wchar_t wc)
	{
		return IsAtomHead(wc) || (L'A' <= wc && wc <= L'Z') || (L'0' <= wc && wc <= L'9');
	}

	bool IsComment(wchar_t wc)
	{
		return (wc == L'%');
	}

	bool IsSpace(wchar_t wc)
	{
		return (wcschr(L" \t\r\n", wc) != 0);
	}
	
	void build_arity(int);
};

OutlineErlang::OutlineErlang() :
	state(STATE_NORMAL),
	lnum(0),
	argcount(0)
{
}

/** 関数名の取得

	@param[in] buf 行(先頭から)
	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	関数名はatom．atomは 小文字アルファベット，_, @ のいずれかから始まる
	英数文字列か，あるいはシングルクォーテーションで囲まれた文字列．
*/
const wchar_t* OutlineErlang::ScanFuncName(const wchar_t* buf, const wchar_t* end, const wchar_t* p)
{
	assert(state == STATE_NORMAL);

	if (p > buf || ! (IsAtomHead(*p) || *p == L'\'')) {
		return end;
	}
	
	if (*p == L'\'') {
		do {
			++p;
		}while (*p != L'\'' && p < end);
		if (p >= end) {
			// invalid atom
			return p;
		}
		++p;
	}else {
		do {
			++p;
		}while (IsAlNum(*p) && p < end);
	}
	
	size_t buf_len = _countof(func);
	size_t len = p - buf;
	if (buf[0] == L'\'') {
		++buf;
		len -= 2;
		--buf_len;
	}
	len = len < buf_len - 1 ? len : buf_len - 1;
	wcsncpy(func, buf, len);
	func[len] = L'\0';
	state = STATE_FUNC_CANDIDATE_FIN;
	return p;
}

/** パラメータの発見

	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	関数名の取得が完了し，パラメータ先頭の括弧を探す．
*/
const wchar_t* OutlineErlang::EnterArgs(const wchar_t* end, const wchar_t* p)
{
	assert(state == STATE_FUNC_CANDIDATE_FIN);

	while (IsSpace(*p) && p < end)
		++p;
	
	if (p >= end)
		return end;

	if (IsComment(*p)) {
		return end;
	}else if (*p == L'(') { //)
		state = STATE_FUNC_ARGS1;
		argcount = 0;
		parenthesis_ptr = 1;
		parenthesis[0] = *p;
		++p;
		return p;
	}
	// not a function
	state = STATE_NORMAL;
	return end;
}

/** 先頭パラメータの発見

	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	パラメータが0個と1個以上の判別のために状態を設けている．
*/
const wchar_t* OutlineErlang::ScanArgs1(const wchar_t* end, const wchar_t* p)
{
	assert(state == STATE_FUNC_ARGS1);
	
	while (IsSpace(*p) && p < end)
		++p;

	if (p >= end)
		return end;

	if (*p == /* (*/ L')') {
		// no argument
		state = STATE_FUNC_ARGS_FIN;
		++p;
	}else if (IsComment(*p)) {
		return end;
	}else {
		// argument found
		state = STATE_FUNC_ARGS;
		++argcount;
	}
	return p;
}

/** パラメータの解析とカウント

	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	パラメータを解析する．パラメータの数と末尾の閉じ括弧を正しく判別するために，
	引用符，括弧，パラメータの区切りのカンマに着目する．
	引用符は改行を含むことができない．
*/
const wchar_t* OutlineErlang::ScanArgs(const wchar_t* end, const wchar_t* p)
{
	assert(state == STATE_FUNC_ARGS);

	const int parptr_max = _countof(parenthesis);
	wchar_t quote = L'\0'; // 先頭位置を保存
	for (const wchar_t* head=p; p<end; ++p) {
		if (quote) {
			if (*p == quote)
				quote = L'\0';
		}else {
			if (wcschr(L"([{", *p)) {	//)
				// level up
				if (parenthesis_ptr < parptr_max) {
					parenthesis[parenthesis_ptr] = *p;
				}
				++parenthesis_ptr;
			}else if (wcschr(L")]}", *p)) {	//)
				wchar_t op;
				switch (*p) {
				case L')': op = L'('; break;
				case L']': op = L'['; break;
				case L'}': op = L'{'; break;
				default:
					PleaseReportToAuthor(NULL, LS(STR_OUTLINE_ERLANG_SCANARGS));
					op = 0;
					break;
				}
				// level down
				--parenthesis_ptr;
				while (1 <= parenthesis_ptr && parenthesis_ptr < parptr_max) {
					if (parenthesis[parenthesis_ptr] != op) {
						// if unmatch then skip
						--parenthesis_ptr;
					}else {
						break;
					}
				}
				
				// check level
				if (parenthesis_ptr == 0) {
					state = STATE_FUNC_ARGS_FIN;
					++p;
					return p;
				}
			}else if (*p == L',' && parenthesis_ptr == 1) {
				++argcount;
			}else if (*p == L';') {
				//	セミコロンは複数の文の区切り．
				//	パラメータ中には現れないので，解析が失敗している
				//	括弧の閉じ忘れが考えられるので，仕切り直し
				state = STATE_NORMAL;
				return end;
			}else if (*p == L'.') {
				//	ピリオドは式の末尾か，小数点として使われる．
				if (p > head && (L'0' <= p[-1] && p[-1] <= L'9')) {
					//	小数点かもしれないので，そのままにする
				}else {
					//	引数の途中で文末が現れたのは解析が失敗している
					//	括弧の閉じ忘れが考えられるので，仕切り直し
					state = STATE_NORMAL;
					return end;
				}
			}else if (*p == L'"') {
				quote = L'"';
			}else if (*p == L'\'') {
				quote = L'\'';
			}else if (IsComment(*p)) {
				return end;
			}
		}
	}
	return p;
}

/** 関数本体の区切り，または条件文の検出

	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	パラメータ本体を表す記号(->)か条件文の開始キーワード(when)を
	見つけたら，関数発見とする．
	それ以外の場合は関数ではなかったと考える．
*/
const wchar_t* OutlineErlang::EnterCond(const wchar_t* end, const wchar_t* p)
{
	while (IsSpace(*p) && p < end)
		++p;

	if (p >= end)
		return end;

	if (p + 1 < end && wcsncmp(p, L"->", 2) == 0) {
		p += 2;
		state = STATE_FUNC_FOUND;
	}else if (p + 3 < end && wcsncmp(p, L"when", 4) == 0) {
		state = STATE_FUNC_FOUND;
		p += 4;
	}else if (IsComment(*p)) {
		return end;
	}else {
		state = STATE_NORMAL;
	}
	return end;
}

/** 行の解析

	@param[in] buf 行(先頭から)
	@param[in] linelen 行の長さ
	@param[in] linenum 行番号
*/
bool OutlineErlang::Parse(const wchar_t* buf, size_t linelen, size_t linenum)
{
	const wchar_t* pos = buf;
	const wchar_t* const end = buf + linelen;
	if (state == STATE_FUNC_FOUND) {
		state = STATE_NORMAL;
	}
	if (state == STATE_NORMAL) {
		pos = ScanFuncName(buf, end, pos);
		if (state != STATE_NORMAL) {
			lnum = linenum;
		}
	}
	while (pos < end) {
		switch (state) {
		case STATE_FUNC_CANDIDATE_FIN:
			pos = EnterArgs(end, pos); break;
		case STATE_FUNC_ARGS1:
			pos = ScanArgs1(end, pos); break;
		case STATE_FUNC_ARGS:
			pos = ScanArgs(end, pos); break;
		case STATE_FUNC_ARGS_FIN:
			pos = EnterCond(end, pos); break;
		default:
			PleaseReportToAuthor(NULL, _T("OutlineErlang::Parse Unknown State: %d"), state);
			break;
		}
		if (state == STATE_FUNC_FOUND) {
			build_arity(argcount);
			break;
		}
	}
	return state == STATE_FUNC_FOUND;
}

/** 関数名の後ろに Arity (引数の数)を付加する

	@param[in] arity 引数の数
	
	関数名の後ろに /パラメータ数 の形で文字列を追加する．
	バッファが不足する場合はできるところまで書き込む．
	そのため，10個以上の引数がある場合に，引数の数の下位桁が欠けることがある．
*/ 
void OutlineErlang::build_arity(int arity)
{
	size_t len = wcslen(func);
	const size_t buf_size = _countof(func);
	wchar_t* p = &func[len];
	wchar_t numstr[12];
	
	if (len + 1 >= buf_size)
		return; // no room

	numstr[0] = L'/';
	_itow(arity, numstr + 1, 10);
	wcsncpy(p, numstr, buf_size - len - 1);
	func[buf_size - 1] = L'\0';
}

/** Erlang アウトライン解析

	@par 主な仮定と方針
	関数宣言は1カラム目から記載されている．
	
	
	@par 解析アルゴリズム
	1カラム目がアルファベットの場合: 関数らしいとして解析開始 / 関数名を保存
	スペースは読み飛ばす
	(を発見したら) まで引数を数える．その場合入れ子の括弧と文字列を考慮
	-> または when があれば関数定義と見なす(次の行にまたがっても良い)
	途中 % (コメント) が現れたら行末まで読み飛ばす
*/
void DocOutline::MakeFuncList_Erlang(FuncInfoArr* pFuncInfoArr)
{
	OutlineErlang erl_state_machine;
	for (size_t nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		size_t nLineLen;
		const wchar_t* pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if (erl_state_machine.Parse(pLine, nLineLen, nLineCount)) {
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			Point ptPosXY = doc.layoutMgr.LogicToLayout(Point(0, (int)erl_state_machine.GetFuncLine()));
			pFuncInfoArr->AppendData(
				(int)erl_state_machine.GetFuncLine() + 1,
				ptPosXY.y + 1,
				erl_state_machine.GetFuncName(),
				0,
				0
			);
		}
	}
}

