#pragma once

class Eol;

// �T�N���G�f�B�^�p�N���b�v�{�[�h�N���X�B��X�͂��̒��őS�ẴN���b�v�{�[�hAPI���Ă΂������B
class Clipboard {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Clipboard(HWND hwnd); // �R���X�g���N�^���ŃN���b�v�{�[�h���J�����
	virtual ~Clipboard(); // �f�X�g���N�^����Close���Ă΂��

	// �C���^�[�t�F�[�X
	void Empty(); // �N���b�v�{�[�h����ɂ���
	void Close(); // �N���b�v�{�[�h�����
	bool SetText(const wchar_t* pData, size_t nDataLen, bool bColumnSelect, bool bLineSelect, UINT uFormat = (UINT)-1);   // �e�L�X�g��ݒ肷��
	bool SetHtmlText(const NativeW& memBUf);
	bool GetText(NativeW* pMemBuf, bool* pbColumnSelect, bool* pbLineSelect, const Eol& eol, UINT uGetFormat = (UINT)-1); // �e�L�X�g���擾����
	bool IsIncludeClipboradFormat(const wchar_t* pFormatName);
	bool SetClipboradByFormat(const StringRef& str, const wchar_t* pFormatName, int nMode, int nEndMode);
	bool GetClipboradByFormat(NativeW& mem, const wchar_t* pFormatName, int nMode, int nEndMode, const Eol& eol);
	
	// ���Z�q
	operator bool() const { return bOpenResult != FALSE; } // �N���b�v�{�[�h���J�����Ȃ�true
	
private:
	HWND hwnd;
	BOOL bOpenResult;
	
	// -- -- static�C���^�[�t�F�[�X -- -- //
public:
	static bool HasValidData();    // �N���b�v�{�[�h���ɁA�T�N���G�f�B�^�ň�����f�[�^�������true
	static CLIPFORMAT GetSakuraFormat(); // �T�N���G�f�B�^�Ǝ��̃N���b�v�{�[�h�f�[�^�`��
	static int GetDataType();      // �N���b�v�{�[�h�f�[�^�`��(CF_UNICODETEXT��)�̎擾
};

