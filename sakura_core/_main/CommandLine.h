#pragma once

#include "global.h"
#include "EditInfo.h"
#include "util/design_template.h"

class Memory;

struct GrepInfo {
	NativeW			mGrepKey;				// �����L�[
	NativeW			mGrepRep;				// �u���L�[
	NativeT			mGrepFile;				// �����Ώۃt�@�C��
	NativeT			mGrepFolder;			// �����Ώۃt�H���_
	SearchOption	grepSearchOption;		// �����I�v�V����
	bool			bGrepCurFolder;			// �J�����g�f�B���N�g�����ێ�
	bool			bGrepStdout;			// �W���o�̓��[�h
	bool			bGrepHeader;			// �w�b�_���\��
	bool			bGrepSubFolder;			// �T�u�t�H���_����������
	EncodingType	charEncoding;			// �����R�[�h�Z�b�g
	int				nGrepOutputStyle;		// ���ʏo�͌`��
	int				nGrepOutputLineType;	// ���ʏo�́F�s���o��/�Y������/�ۃ}�b�`�s
	bool			bGrepOutputFileOnly;	// �t�@�C�����ŏ��̂݌���
	bool			bGrepOutputBaseFolder;	// �x�[�X�t�H���_�\��
	bool			bGrepSeparateFolder;	// �t�H���_���ɕ\��
	bool			bGrepReplace;			// Grep�u��
	bool			bGrepPaste;				// �N���b�v�{�[�h����\��t��
	bool			bGrepBackup;			// �u���Ńo�b�N�A�b�v��ۑ�
};


/*!
	@brief �R�}���h���C���p�[�T �N���X
*/
class CommandLine : public TSingleton<CommandLine> {
	friend class TSingleton<CommandLine>;
	CommandLine();

	static int CheckCommandLine(
		LPTSTR	str,		// [in] ���؂��镶����i�擪��-�͊܂܂Ȃ��j
		TCHAR**	arg,		// [out] ����������ꍇ�͂��̐擪�ւ̃|�C���^
		int*	arglen		// [out] �����̒���
	);

	/*!
		���p���ň͂܂�Ă��鐔�l��F������悤�ɂ���
	*/
	static int AtoiOptionInt(const TCHAR* arg) {
		return (arg[0] == _T('"') || arg[0] == _T('\'')) ?
			_ttoi(arg + 1) : _ttoi(arg);
	}

// member accessor method
public:
	bool IsNoWindow() const {return bNoWindow;}
	bool IsWriteQuit() const {return bWriteQuit;}
	bool IsGrepMode() const {return bGrepMode;}
	bool IsGrepDlg() const {return bGrepDlg;}
	bool IsDebugMode() const {return bDebugMode;}
	bool IsViewMode() const {return bViewMode;}
	const EditInfo& GetEditInfo() const { return fi; }
	const GrepInfo& GetGrepInfo() const { return gi; }
	int GetGroupId() const {return nGroup;}
	LPCWSTR GetMacro() const { return mMacro.GetStringPtr(); }
	LPCWSTR GetMacroType() const { return mMacroType.GetStringPtr(); }
	LPCWSTR GetProfileName() const{ return mProfile.GetStringPtr(); }
	bool IsSetProfile() const{ return bSetProfile; }
	void SetProfileName(LPCWSTR s){
		bSetProfile = true;
		mProfile.SetString(s);
	}
	bool IsProfileMgr() { return bProfileMgr; }
	size_t GetFileNum(void) { return fileNames.size(); }
	const TCHAR* GetFileName(size_t i) { return i < GetFileNum() ? fileNames[i].c_str() : NULL; }
	void ClearFile(void) { fileNames.clear(); }
	void ParseCommandLine(LPCTSTR pszCmdLineSrc, bool bResponse = true);

// member valiables
private:
	bool		bGrepMode;		// [out] true: Grep Mode
	bool		bGrepDlg;		// Grep�_�C�A���O
	bool		bDebugMode;		
	bool		bNoWindow;		// [out] true: �ҏWWindow���J���Ȃ�
	bool		bWriteQuit;		// [out] true: �ݒ��ۑ����ďI��
	bool		bProfileMgr;
	bool		bSetProfile;
	EditInfo	fi;				//
	GrepInfo	gi;				//
	bool		bViewMode;		// [out] true: Read Only
	int			nGroup;			// �O���[�vID
	NativeW	mMacro;				// [out] �}�N���t�@�C�����^�}�N����
	NativeW	mMacroType;			// [out] �}�N�����
	NativeW	mProfile;			// �v���t�@�C����
	std::vector<std::tstring> fileNames;	// �t�@�C����(����)
};

