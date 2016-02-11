/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�ϊ��n)�֐��Q

	2012/12/20	CViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, Stonee, Misaka
	Copyright (C) 2002, ai

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "convert/CDecode_Base64Decode.h"
#include "convert/CDecode_UuDecode.h"
#include "io/CBinaryStream.h"


// ������
void ViewCommander::Command_TOLOWER(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_TOLOWER);
	return;
}


// �啶��
void ViewCommander::Command_TOUPPER(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_TOUPPER);
	return;
}


// �S�p�����p
void ViewCommander::Command_TOHANKAKU(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_TOHANKAKU);
	return;
}


// ���p�{�S�Ђ灨�S�p�E�J�^�J�i		// Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
void ViewCommander::Command_TOZENKAKUKATA(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_TOZENKAKUKATA);
	return;
}


// ���p�{�S�J�^���S�p�E�Ђ炪��		// Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
void ViewCommander::Command_TOZENKAKUHIRA(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_TOZENKAKUHIRA);
	return;
}


// ���p�p�����S�p�p��				// July. 30, 2001 Misaka
void ViewCommander::Command_TOZENEI(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_TOZENEI);
	return;
}


// �S�p�p�������p�p��
void ViewCommander::Command_TOHANEI(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_TOHANEI);
	return;
}


// �S�p�J�^�J�i�����p�J�^�J�i		// Aug. 29, 2002 ai
void ViewCommander::Command_TOHANKATA(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_TOHANKATA);
	return;
}


// ���p�J�^�J�i���S�p�J�^�J�i
void ViewCommander::Command_HANKATATOZENKAKUKATA(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_HANKATATOZENKATA);
	return;
}


// ���p�J�^�J�i���S�p�Ђ炪��
void ViewCommander::Command_HANKATATOZENKAKUHIRA(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_HANKATATOZENHIRA);
	return;
}


// TAB����
void ViewCommander::Command_TABTOSPACE(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_TABTOSPACE);
	return;
}

// �󔒁�TAB	 //---- Stonee, 2001/05/27
void ViewCommander::Command_SPACETOTAB(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_SPACETOTAB);
	return;
}


// �������ʁ�SJIS�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_AUTO2SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_AUTO2SJIS);
	return;
}


// E-Mail(JIS��SJIS)�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_EMAIL(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_EMAIL);
	return;
}


// EUC��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_EUC2SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_EUC2SJIS);
	return;
}


// Unicode��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_UNICODE2SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_UNICODE2SJIS);
	return;
}


// UnicodeBE��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_UNICODEBE2SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_UNICODEBE2SJIS);
	return;
}


// UTF-8��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_UTF82SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_UTF82SJIS);
	return;
}


// UTF-7��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_UTF72SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_UTF72SJIS);
	return;
}


// SJIS��JIS�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_SJIS2JIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_SJIS2JIS);
	return;
}


// SJIS��EUC�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_SJIS2EUC(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_SJIS2EUC);
	return;
}


// SJIS��UTF-8�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_SJIS2UTF8(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_SJIS2UTF8);
	return;
}


// SJIS��UTF-7�R�[�h�ϊ�
void ViewCommander::Command_CODECNV_SJIS2UTF7(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	m_pCommanderView->ConvSelectedArea(F_CODECNV_SJIS2UTF7);
	return;
}


// Base64�f�R�[�h���ĕۑ�
void ViewCommander::Command_BASE64DECODE(void)
{
	// �e�L�X�g���I������Ă��邩
	if (!m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		ErrorBeep();
		return;
	}
	// �I��͈͂̃f�[�^���擾
	// ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ�
	NativeW ctextBuf;
	if (!m_pCommanderView->GetSelectedDataSimple(ctextBuf)) {
		ErrorBeep();
		return;
	}

	// Base64�f�R�[�h
	Memory cmemBuf;
	bool bret = Decode_Base64Decode().CallDecode(ctextBuf, &cmemBuf);
	if (!bret) {
		return;
	}
	ctextBuf.Clear();

	// �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��
	TCHAR szPath[_MAX_PATH] = _T("");
	if (!GetDocument()->m_docFileOperation.SaveFileDialog(szPath)) {
		return;
	}

	// �f�[�^
	int nDataLen;
	const void* pData = cmemBuf.GetRawPtr(&nDataLen);

	// �J�L�R
	BinaryOutputStream out(szPath);
	if (!out) goto err;
	if (nDataLen != out.Write(pData, nDataLen)) goto err;

	return;

err:
	ErrorBeep();
	ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD14), szPath);
}


// uudecode���ĕۑ�
void ViewCommander::Command_UUDECODE(void)
{
	// �e�L�X�g���I������Ă��邩
	if (!m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̃f�[�^���擾 -> cmemBuf
	// ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ�
	NativeW ctextBuf;
	if (!m_pCommanderView->GetSelectedDataSimple(ctextBuf)) {
		ErrorBeep();
		return;
	}

	// uudecode(�f�R�[�h)  ctextBuf -> cmemBin, szPath
	Memory cmemBin;
	TCHAR szPath[_MAX_PATH] = _T("");
	Decode_UuDecode decoder;
	if (!decoder.CallDecode(ctextBuf, &cmemBin)) {
		return;
	}
	decoder.CopyFilename(szPath);
	ctextBuf.Clear();

	// �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��
	if (!GetDocument()->m_docFileOperation.SaveFileDialog(szPath)) {
		return;
	}

	// �f�[�^
	int nDataLen;
	const void* pData = cmemBin.GetRawPtr(&nDataLen);

	// �J�L�R
	BinaryOutputStream out(szPath);
	if (!out) goto err;
	if (nDataLen != out.Write(pData, nDataLen)) goto err;

	// ����
	return;

err:
	ErrorBeep();
	ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD16), szPath);
}

