#pragma once

class EditDoc;
class FuncInfoArr;
struct OneRule;

class DocOutline {
public:
	DocOutline(EditDoc& doc) : doc(doc) { }
	void	MakeFuncList_C(FuncInfoArr*, bool bVisibleMemberFunc = true);			// C/C++�֐����X�g�쐬
	void	MakeFuncList_PLSQL(FuncInfoArr*);										// PL/SQL�֐����X�g�쐬
	void	MakeTopicList_txt(FuncInfoArr*);										// �e�L�X�g�E�g�s�b�N���X�g�쐬
	void	MakeFuncList_Java(FuncInfoArr*);										// Java�֐����X�g�쐬
	void	MakeTopicList_cobol(FuncInfoArr*);										// COBOL �A�E�g���C�����
	void	MakeTopicList_asm(FuncInfoArr*);										// �A�Z���u�� �A�E�g���C�����
	void	MakeFuncList_Perl(FuncInfoArr*);										// Perl�֐����X�g�쐬	// Sep. 8, 2000 genta
	void	MakeFuncList_VisualBasic(FuncInfoArr*);									// Visual Basic�֐����X�g�쐬 // June 23, 2001 N.Nakatani
	void	MakeFuncList_python(FuncInfoArr* pFuncInfoArr);							// Python �A�E�g���C����� // 2007.02.08 genta
	void	MakeFuncList_Erlang(FuncInfoArr* pFuncInfoArr);							// Erlang �A�E�g���C����� // 2009.08.10 genta
	void	MakeTopicList_wztxt(FuncInfoArr*);										// �K�w�t���e�L�X�g �A�E�g���C����� // 2003.05.20 zenryaku
	void	MakeTopicList_html(FuncInfoArr*);										// HTML �A�E�g���C����� // 2003.05.20 zenryaku
	void	MakeTopicList_tex(FuncInfoArr*);										// TeX �A�E�g���C����� // 2003.07.20 naoh
	void	MakeFuncList_RuleFile(FuncInfoArr*, std::tstring&);						// ���[���t�@�C�����g���ă��X�g�쐬 2002.04.01 YAZAKI
	int		ReadRuleFile(const TCHAR*, OneRule*, int, bool&, std::wstring&);		// ���[���t�@�C���Ǎ� 2002.04.01 YAZAKI
	void	MakeFuncList_BookMark(FuncInfoArr*);									// �u�b�N�}�[�N���X�g�쐬 // 2001.12.03 hor
private:
	EditDoc& doc;
};

