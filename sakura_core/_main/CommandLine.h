/*!	@file
	@brief �R�}���h���C���p�[�T �w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CControlTray��蕪��
	Copyright (C) 2002, genta
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include "global.h"
#include "EditInfo.h"
#include "util/design_template.h"

class Memory;

/*!	�����I�v�V����
	20020118 aroka
*/
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


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/

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
		@date 2002.12.05 genta
	*/
	static int AtoiOptionInt(const TCHAR* arg) {
		return (arg[0] == _T('"') || arg[0] == _T('\'')) ?
			_ttoi(arg + 1) : _ttoi(arg);
	}

// member accessor method
public:
	bool IsNoWindow() const {return bNoWindow;}
	bool IsWriteQuit() const {return bWriteQuit;}	// 2007.05.19 ryoji sakuext�p�ɒǉ�
	bool IsGrepMode() const {return bGrepMode;}
	bool IsGrepDlg() const {return bGrepDlg;}
	bool IsDebugMode() const {return bDebugMode;}
	bool IsViewMode() const {return bViewMode;}
	const EditInfo& GetEditInfo() const { return fi; }
	const GrepInfo& GetGrepInfo() const { return gi; }
	int GetGroupId() const {return nGroup;}	// 2007.06.26 ryoji
	LPCWSTR GetMacro() const { return mMacro.GetStringPtr(); }
	LPCWSTR GetMacroType() const { return mMacroType.GetStringPtr(); }
	LPCWSTR GetProfileName() const{ return mProfile.GetStringPtr(); }
	bool IsSetProfile() const{ return bSetProfile; }
	void SetProfileName(LPCWSTR s){
		bSetProfile = true;
		mProfile.SetString(s);
	}
	bool IsProfileMgr() { return bProfileMgr; }
	int GetFileNum(void) { return fileNames.size(); }
	const TCHAR* GetFileName(int i) { return i < GetFileNum() ? fileNames[i].c_str() : NULL; }
	void ClearFile(void) { fileNames.clear(); }
	void ParseCommandLine(LPCTSTR pszCmdLineSrc, bool bResponse = true);

// member valiables
private:
	bool		bGrepMode;		// [out] true: Grep Mode
	bool		bGrepDlg;		// Grep�_�C�A���O
	bool		bDebugMode;		
	bool		bNoWindow;		// [out] true: �ҏWWindow���J���Ȃ�
	bool		bWriteQuit;		// [out] true: �ݒ��ۑ����ďI��	// 2007.05.19 ryoji sakuext�p�ɒǉ�
	bool		bProfileMgr;
	bool		bSetProfile;
	EditInfo	fi;				//
	GrepInfo	gi;				//
	bool		bViewMode;		// [out] true: Read Only
	int			nGroup;			// �O���[�vID	// 2007.06.26 ryoji
	NativeW	mMacro;				// [out] �}�N���t�@�C�����^�}�N����
	NativeW	mMacroType;			// [out] �}�N�����
	NativeW	mProfile;			// �v���t�@�C����
	std::vector<std::tstring> fileNames;	// �t�@�C����(����)
};

