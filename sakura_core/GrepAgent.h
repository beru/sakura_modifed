/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include "doc/DocListener.h"
#include "io/FileLoad.h"

class DlgCancel;
class EditView;
class SearchStringPattern;
class GrepEnumKeys;
class GrepEnumFiles;
class GrepEnumFolders;

struct GrepOption {
	bool		bGrepReplace;			// Grep�u��
	bool		bGrepSubFolder;			// �T�u�t�H���_�������������
	bool		bGrepStdout;			// �W���o�̓��[�h
	bool		bGrepHeader;			// �w�b�_�E�t�b�_�\��
	EncodingType	nGrepCharSet;		// �����R�[�h�Z�b�g�I��
	int			nGrepOutputLineType;	// 0:�q�b�g�������o��, 1: �q�b�g�s���o��, 2: �ۃq�b�g�s���o��
	int			nGrepOutputStyle;		// �o�͌`�� 1: Normal, 2: WZ��(�t�@�C���P��) 3: ���ʂ̂�
	bool		bGrepOutputFileOnly;	// �t�@�C�����ŏ��̂݌���
	bool		bGrepOutputBaseFolder;	// �x�[�X�t�H���_�\��
	bool		bGrepSeparateFolder;	// �t�H���_���ɕ\��
	bool		bGrepPaste;				// Grep�u���F�N���b�v�{�[�h����\��t����
	bool		bGrepBackup;			// Grep�u���F�o�b�N�A�b�v

	GrepOption() : 
		 bGrepReplace(false)
		,bGrepSubFolder(true)
		,bGrepStdout(false)
		,bGrepHeader(true)
		,nGrepCharSet(CODE_AUTODETECT)
		,nGrepOutputLineType(1)
		,nGrepOutputStyle(1)
		,bGrepOutputFileOnly(false)
		,bGrepOutputBaseFolder(false)
		,bGrepSeparateFolder(false)
		,bGrepPaste(false)
		,bGrepBackup(false)
	{}
};

//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
//	Mar. 28, 2004 genta DoGrepFile����s�v�Ȉ������폜
class GrepAgent : public DocListenerEx {
public:
	GrepAgent();

	// �C�x���g
	CallbackResultType OnBeforeClose();
	void OnAfterSave(const SaveInfo& saveInfo);

	static void CreateFolders( const TCHAR* pszPath, std::vector<std::tstring>& vPaths );
	static std::tstring ChopYen( const std::tstring& str );

	// Grep���s
	DWORD DoGrep(
		EditView&				viewDst,
		bool					bGrepReplace,
		const NativeW*			pmGrepKey,
		const NativeW*			pmGrepReplace,
		const NativeT*			pmGrepFile,
		const NativeT*			pmGrepFolder,
		bool					bGrepCurFolder,
		bool					bGrepSubFolder,
		bool					bGrepStdout,
		bool					bGrepHeader,
		const SearchOption&		searchOption,
		EncodingType			nGrepCharSet,			// 2002/09/21 Moca �����R�[�h�Z�b�g�I��
		int						nGrepOutputLineType,
		int						nGrepOutputStyle,
		bool					bGrepOutputFileOnly,	// [in] �t�@�C�����ŏ��̂ݏo��
		bool					bGrepOutputBaseFolder,	// [in] �x�[�X�t�H���_�\��
		bool					bGrepSeparateFolder,	// [in] �t�H���_���ɕ\��
		bool					bGrepPaste,
		bool					bGrepBackup
	);

private:
	// Grep���s
	int DoGrepTree(
		EditView&				viewDst,
		DlgCancel&				dlgCancel,			// [in] Cancel�_�C�A���O�ւ̃|�C���^
		const wchar_t*			pszKey,				// [in] �����p�^�[��
		size_t					nKeyLen,
		const NativeW&			mGrepReplace,
		const GrepEnumKeys&		grepEnumKeys,		// [in] �����Ώۃt�@�C���p�^�[��(!�ŏ��O�w��)
		GrepEnumFiles&			grepExceptAbsFiles,
		GrepEnumFolders&		grepExceptAbsFolders,
		const TCHAR*			pszPath,			// [in] �����Ώۃp�X
		const TCHAR*			pszBasePath,		// [in] �����Ώۃp�X(�x�[�X)
		const SearchOption&		searchOption,		// [in] �����I�v�V����
		const GrepOption&		grepOption,			// [in] Grep�I�v�V����
		const SearchStringPattern& pattern,			// [in] �����p�^�[��
		Bregexp&				regexp,				// [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
		int						nNest,				// [in] �l�X�g���x��
		bool&					bOutputBaseFolder,
		int*					pnHitCount,			// [i/o] �q�b�g���̍��v
		NativeW&				memMessage
	);

	// Grep���s
	int DoGrepFile(
		EditView&				viewDst,
		DlgCancel&				dlgCancel,
		const wchar_t*			pszKey,
		size_t					nKeyLen,
		const TCHAR*			pszFile,
		const SearchOption&		searchOption,
		const GrepOption&		grepOption,
		const SearchStringPattern& pattern,
		Bregexp&				regexp,		//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
		int*					pnHitCount,
		const TCHAR*			pszFullPath,
		const TCHAR*			pszBaseFolder,
		const TCHAR*			pszFolder,
		const TCHAR*			pszRelPath,
		bool&					bOutputBaseFolder,
		bool&					bOutputFolderName,
		NativeW&				memMessage
	);

	int DoGrepReplaceFile(
		EditView&				viewDst,
		DlgCancel&				dlgCancel,
		const wchar_t*			pszKey,
		size_t					nKeyLen,
		const NativeW&			mGrepReplace,
		const TCHAR*			pszFile,
		const SearchOption&		searchOption,
		const GrepOption&		grepOption,
		const SearchStringPattern& pattern,
		Bregexp&				regexp,
		int*					pnHitCount,
		const TCHAR*			pszFullPath,
		const TCHAR*			pszBaseFolder,
		const TCHAR*			pszFolder,
		const TCHAR*			pszRelPath,
		bool&					bOutputBaseFolder,
		bool&					bOutputFolderName,
		NativeW&				memMessage
	);

	// Grep���ʂ�pszWork�Ɋi�[
	void SetGrepResult(
		// �f�[�^�i�[��
		NativeW&		memMessage,
		// �}�b�`�����t�@�C���̏��
		const TCHAR*	pszFilePath,	//	�t���p�X or ���΃p�X
		const TCHAR*	pszCodeName,	//	�����R�[�h���"[SJIS]"�Ƃ�
		// �}�b�`�����s�̏��
		LONGLONG		nLine,			//	�}�b�`�����s�ԍ�
		int				nColumn,		//	�}�b�`�������ԍ�
		const wchar_t*	pCompareData,	//	�s�̕�����
		int				nLineLen,		//	�s�̕�����̒���
		int				nEolCodeLen,	//	EOL�̒���
		// �}�b�`����������̏��
		const wchar_t*	pMatchData,		//	�}�b�`����������
		int				nMatchLen,		//	�}�b�`����������̒���
		// �I�v�V����
		const GrepOption&	grepOption
	);
	void AddTail(
		EditWnd& editWnd,
		EditView& editView,
		const NativeW& mem,
		bool bAddStdout
	);

public: //$$ ��
	bool	m_bGrepMode;		// Grep���[�h��
	bool	m_bGrepRunning;		// Grep������
private:
	ULONGLONG m_lastStaticCurFileSetTime;
	ULONGLONG m_lastViewDstAddedTime;
	std::vector<std::pair<const wchar_t*, LogicInt>> m_searchWords;
	NativeW m_memBuf;
	NativeW m_unicodeBuffer;
	FileLoad m_fl;	// 2012/12/18 Uchi ��������t�@�C���̃f�t�H���g�̕����R�[�h���擾����l��
};

