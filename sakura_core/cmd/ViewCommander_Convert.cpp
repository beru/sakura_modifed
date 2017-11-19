#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "convert/Decode_Base64Decode.h"
#include "convert/Decode_UuDecode.h"
#include "io/BinaryStream.h"

// ViewCommander�N���X�̃R�}���h(�ϊ��n)�֐��Q

// ������
void ViewCommander::Command_ToLower(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_TOLOWER);
	return;
}

// �啶��
void ViewCommander::Command_ToUpper(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_TOUPPER);
	return;
}

// �S�p�����p
void ViewCommander::Command_ToHankaku(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_TOHANKAKU);
	return;
}


// ���p�{�S�Ђ灨�S�p�E�J�^�J�i
void ViewCommander::Command_ToZenkakuKata(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_TOZENKAKUKATA);
	return;
}


// ���p�{�S�J�^���S�p�E�Ђ炪��
void ViewCommander::Command_ToZenkakuHira(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_TOZENKAKUHIRA);
	return;
}


// ���p�p�����S�p�p��
void ViewCommander::Command_ToZenEi(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_TOZENEI);
	return;
}


// �S�p�p�������p�p��
void ViewCommander::Command_ToHanEi(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_TOHANEI);
	return;
}


// �S�p�J�^�J�i�����p�J�^�J�i
void ViewCommander::Command_ToHankata(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_TOHANKATA);
	return;
}


// ���p�J�^�J�i���S�p�J�^�J�i
void ViewCommander::Command_HanKataToZenkakuKata(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_HANKATATOZENKATA);
	return;
}


// ���p�J�^�J�i���S�p�Ђ炪��
void ViewCommander::Command_HanKataToZenKakuHira(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_HANKATATOZENHIRA);
	return;
}


// TAB����
void ViewCommander::Command_TabToSpace(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_TABTOSPACE);
	return;
}

// �󔒁�TAB
void ViewCommander::Command_SpaceToTab(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_SPACETOTAB);
	return;
}


// �������ʁ�SJIS�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_Auto2SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_AUTO2SJIS);
	return;
}


// E-Mail(JIS��SJIS)�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_EMail(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_EMAIL);
	return;
}


// EUC��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_EUC2SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_EUC2SJIS);
	return;
}


// Unicode��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_Unicode2SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_UNICODE2SJIS);
	return;
}


// UnicodeBE��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_UnicodeBE2SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_UNICODEBE2SJIS);
	return;
}


// UTF-8��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_UTF82SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_UTF82SJIS);
	return;
}


// UTF-7��SJIS�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_UTF72SJIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_UTF72SJIS);
	return;
}


// SJIS��JIS�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_SJIS2JIS(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_SJIS2JIS);
	return;
}


// SJIS��EUC�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_SJIS2EUC(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_SJIS2EUC);
	return;
}


// SJIS��UTF-8�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_SJIS2UTF8(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_SJIS2UTF8);
	return;
}


// SJIS��UTF-7�R�[�h�ϊ�
void ViewCommander::Command_CodeCnv_SJIS2UTF7(void)
{
	// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
	view.ConvSelectedArea(F_CODECNV_SJIS2UTF7);
	return;
}


// Base64�f�R�[�h���ĕۑ�
void ViewCommander::Command_Base64Decode(void)
{
	// �e�L�X�g���I������Ă��邩
	if (!view.GetSelectionInfo().IsTextSelected()) {
		ErrorBeep();
		return;
	}
	// �I��͈͂̃f�[�^���擾
	// ���펞��true,�͈͖��I���̏ꍇ��false��Ԃ�
	NativeW ctextBuf;
	if (!view.GetSelectedDataSimple(ctextBuf)) {
		ErrorBeep();
		return;
	}

	// Base64�f�R�[�h
	Memory memBuf;
	bool bret = Decode_Base64Decode().CallDecode(ctextBuf, &memBuf);
	if (!bret) {
		return;
	}
	ctextBuf.Clear();

	// �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��
	TCHAR szPath[_MAX_PATH] = _T("");
	if (!GetDocument().docFileOperation.SaveFileDialog(szPath)) {
		return;
	}

	// �f�[�^
	size_t nDataLen;
	const void* pData = memBuf.GetRawPtr(&nDataLen);

	// �J�L�R
	BinaryOutputStream out(szPath);
	if (!out) goto err;
	if (nDataLen != out.Write(pData, nDataLen)) goto err;

	return;

err:
	ErrorBeep();
	ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD14), szPath);
}


// uudecode���ĕۑ�
void ViewCommander::Command_UUDecode(void)
{
	// �e�L�X�g���I������Ă��邩
	if (!view.GetSelectionInfo().IsTextSelected()) {
		ErrorBeep();
		return;
	}

	// �I��͈͂̃f�[�^���擾 -> memBuf
	// ���펞��true,�͈͖��I���̏ꍇ��false��Ԃ�
	NativeW ctextBuf;
	if (!view.GetSelectedDataSimple(ctextBuf)) {
		ErrorBeep();
		return;
	}

	// uudecode(�f�R�[�h)  ctextBuf -> memBin, szPath
	Memory memBin;
	TCHAR szPath[_MAX_PATH] = _T("");
	Decode_UuDecode decoder;
	if (!decoder.CallDecode(ctextBuf, &memBin)) {
		return;
	}
	decoder.CopyFilename(szPath);
	ctextBuf.Clear();

	// �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\��
	if (!GetDocument().docFileOperation.SaveFileDialog(szPath)) {
		return;
	}

	// �f�[�^
	size_t nDataLen;
	const void* pData = memBin.GetRawPtr(&nDataLen);

	// �J�L�R
	BinaryOutputStream out(szPath);
	if (!out) goto err;
	if (nDataLen != out.Write(pData, nDataLen)) goto err;

	// ����
	return;

err:
	ErrorBeep();
	ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD16), szPath);
}

