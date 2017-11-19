#pragma once

class EditDoc;
class FuncInfoArr;
struct OneRule;

class DocOutline {
public:
	DocOutline(EditDoc& doc) : doc(doc) { }
	void	MakeFuncList_C(FuncInfoArr*, bool bVisibleMemberFunc = true);			// C/C++関数リスト作成
	void	MakeFuncList_PLSQL(FuncInfoArr*);										// PL/SQL関数リスト作成
	void	MakeTopicList_txt(FuncInfoArr*);										// テキスト・トピックリスト作成
	void	MakeFuncList_Java(FuncInfoArr*);										// Java関数リスト作成
	void	MakeTopicList_cobol(FuncInfoArr*);										// COBOL アウトライン解析
	void	MakeTopicList_asm(FuncInfoArr*);										// アセンブラ アウトライン解析
	void	MakeFuncList_Perl(FuncInfoArr*);										// Perl関数リスト作成
	void	MakeFuncList_VisualBasic(FuncInfoArr*);									// Visual Basic関数リスト作成
	void	MakeFuncList_python(FuncInfoArr* pFuncInfoArr);							// Python アウトライン解析
	void	MakeFuncList_Erlang(FuncInfoArr* pFuncInfoArr);							// Erlang アウトライン解析
	void	MakeTopicList_wztxt(FuncInfoArr*);										// 階層付きテキスト アウトライン解析
	void	MakeTopicList_html(FuncInfoArr*);										// HTML アウトライン解析
	void	MakeTopicList_tex(FuncInfoArr*);										// TeX アウトライン解析
	void	MakeFuncList_RuleFile(FuncInfoArr*, std::tstring&);						// ルールファイルを使ってリスト作成
	int		ReadRuleFile(const TCHAR*, OneRule*, int, bool&, std::wstring&);		// ルールファイル読込
	void	MakeFuncList_BookMark(FuncInfoArr*);									// ブックマークリスト作成
private:
	EditDoc& doc;
};

