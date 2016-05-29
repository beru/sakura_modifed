/*!	@file
	@brief �A�E�g���C����� �f�[�^�z��

	@author Norio Nakatani
	@date	1998/06/23 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

class FuncInfo;
#include <string>
#include <map>
#include "util/design_template.h"

// �W���I�ȕt�����萔
#define FL_OBJ_DEFINITION	0	// �e�N���X�̒�`�ʒu
#define FL_OBJ_DECLARE		1	// �֐��v���g�^�C�v�錾
#define FL_OBJ_FUNCTION		2	// �֐�
#define FL_OBJ_CLASS		3	// �N���X
#define FL_OBJ_STRUCT		4	// �\����
#define FL_OBJ_ENUM			5	// �񋓑�
#define FL_OBJ_UNION		6	// ���p��
#define FL_OBJ_NAMESPACE	7	// ���O���
#define FL_OBJ_INTERFACE	8	// �C���^�t�F�[�X
#define FL_OBJ_GLOBAL		9	// �O���[�o���i�g�ݍ��݉�͂ł͎g�p���Ȃ��j
#define FL_OBJ_ELEMENT_MAX	30	// �v���O�C���Œǉ��\�Ȓ萔�̏��

// �A�E�g���C����� �f�[�^�z��
class FuncInfoArr {
public:
	FuncInfoArr();		// FuncInfoArr�N���X�\�z
	~FuncInfoArr();	// FuncInfoArr�N���X����
	FuncInfo* GetAt(size_t);		// 0<=�̎w��ԍ��̃f�[�^��Ԃ�
	void AppendData(FuncInfo*);	// �z��̍Ō�Ƀf�[�^��ǉ�����
	void AppendData(size_t, size_t, const TCHAR*, int, size_t nDepth = 0);		// �z��̍Ō�Ƀf�[�^��ǉ����� 2002.04.01 YAZAKI �[������
	void AppendData(size_t, size_t, const NOT_TCHAR*, int, size_t nDepth = 0);	// �z��̍Ō�Ƀf�[�^��ǉ����� 2002.04.01 YAZAKI �[������
	void AppendData(size_t nLogicLine, size_t nLogicCol, size_t nLayoutLine, size_t nLayoutCol, const TCHAR*, const TCHAR*, int, size_t nDepth = 0);	/* �z��̍Ō�Ƀf�[�^��ǉ����� 2010.03.01 syat ������*/
	void AppendData(size_t nLogicLine, size_t nLogicCol, size_t nLayoutLine, size_t nLayoutCol, const NOT_TCHAR*, const NOT_TCHAR*, int, size_t nDepth = 0);	/* �z��̍Ō�Ƀf�[�^��ǉ����� 2010.03.01 syat ������*/
	size_t	GetNum(void) {	return nFuncInfoArrNum; }	// �z��v�f����Ԃ�
	void Empty(void);
	void DUMP(void);
	void SetAppendText(int info, const std::wstring& s, bool overwrite);
	std::wstring GetAppendText(int info);
	size_t AppendTextLenMax() { return nAppendTextLenMax; }

public:
	SFilePath	szFilePath;	// ��͑Ώۃt�@�C����
private:
	size_t		nFuncInfoArrNum;	// �z��v�f��
	FuncInfo**	ppcFuncInfoArr;	// �z��
	std::map<int, std::wstring>	appendTextArr;	// �ǉ�������̃��X�g
	size_t		nAppendTextLenMax;

private:
	DISALLOW_COPY_AND_ASSIGN(FuncInfoArr);
};

