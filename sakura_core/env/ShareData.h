/*!	@file
	@brief �v���Z�X�ԋ��L�f�[�^�ւ̃A�N�Z�X

	@author Norio Nakatani
	@date 1998/05/26  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, aroka, Moca, MIK, YAZAKI, hor
	Copyright (C) 2003, Moca, aroka, MIK, genta
	Copyright (C) 2004, Moca, novice, genta
	Copyright (C) 2005, MIK, genta, ryoji, aroka, Moca
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, Uchi
	Copyright (C) 2011, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
// 2007.09.23 kobake m_nSEARCHKEYArrNum,      m_szSEARCHKEYArr      �� searchKeys      �ɂ܂Ƃ߂܂���
// 2007.09.23 kobake m_nREPLACEKEYArrNum,     m_szREPLACEKEYArr     �� replaceKeys     �ɂ܂Ƃ߂܂���
// 2007.09.23 kobake m_nGREPFILEArrNum,       m_szGREPFILEArr       �� grepFiles       �ɂ܂Ƃ߂܂���
// 2007.09.23 kobake m_nGREPFOLDERArrNum,     m_szGREPFOLDERArr     �� grepFolders     �ɂ܂Ƃ߂܂���
// 2007.09.23 kobake m_szCmdArr,              m_nCmdArrNum          �� m_aCommands        �ɂ܂Ƃ߂܂���
// 2007.09.23 kobake m_nTagJumpKeywordArrNum, m_szTagJumpKeywordArr �� aTagJumpKeywords �ɂ܂Ƃ߂܂���
// 2007.12.13 kobake DllSharedData�ւ̊ȈՃA�N�Z�T��p��

#pragma once

#include "SelectLang.h"		// 2011.04.10 nasukoji

class ShareData;

// 2010.04.19 Moca DllSharedData�֘A��DllSharedData.h���Œ���K�v�ȏꏊ�ֈړ�
// ShareData.h�́A������Interface�����񋟂��܂���B�ʂ�DllSharedData.h��include���邱�ƁB
struct DllSharedData;
struct TypeConfig;
class Mutex;

/*!	@brief ���L�f�[�^�̊Ǘ�

	ShareData��Process�̃����o�ł��邽�߁C���҂̎����͓���ł��D
	�{����Process�I�u�W�F�N�g��ʂ��ăA�N�Z�X����ׂ��ł����C
	Process���̃f�[�^�̈�ւ̃|�C���^��static�ϐ��ɕۑ����邱�Ƃ�
	Singleton�̂悤�ɂǂ�����ł��A�N�Z�X�ł���\���ɂȂ��Ă��܂��D

	���L�������ւ̃|�C���^��m_pShareData�ɕێ����܂��D���̃����o��
	���J����Ă��܂����CShareData�ɂ����Map/Unmap����邽�߂�
	ChareData�̏��łɂ���ă|�C���^m_pShareData�������ɂȂ邱�Ƃ�
	���ӂ��Ă��������D

	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�ShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
*/
class ShareData : public TSingleton<ShareData> {
	friend class TSingleton<ShareData>;
	ShareData();
	~ShareData();

public:
	/*
	||  Attributes & Operations
	*/
	bool InitShareData();	// ShareData�N���X�̏���������
	void RefreshString();	// ����I����ɋ��L���������̕�������X�V����
	
	// MRU�n
	bool IsPathOpened(const TCHAR* pszPath, HWND* phwndOwner); // �w��t�@�C�����J����Ă��邩���ׂ�
	bool ActiveAlreadyOpenedWindow(const TCHAR* pszPath, HWND* phwndOwner, EncodingType nCharCode); // �w��t�@�C�����J����Ă��邩���ׂA���d�I�[�v�����̕����R�[�h�Փ˂��m�F // 2007.03.16

	// �f�o�b�O  ���͎�Ƀ}�N���E�O���R�}���h���s�p
	void TraceOut(LPCTSTR lpFmt, ...);	// �A�E�g�v�b�g�E�B���h�E�ɏo��(printf�t�H�[�}�b�g)
	void TraceOutString(const wchar_t* pszStr, int len = -1);	// �A�E�g�v�b�g�E�B���h�E�ɏo��(�����H������)
	void SetTraceOutSource(HWND hwnd) { m_hwndTraceOutSource = hwnd; }	// TraceOut�N�����E�B���h�E�̐ݒ�
	bool OpenDebugWindow(HWND hwnd, bool bAllwaysActive);	// �f�o�b�O�E�B���h�E���J��

	bool IsPrivateSettings(void);

	// �}�N���֘A
	int	 GetMacroFilename(int idx, TCHAR* pszPath, int nBufLen); // idx�Ŏw�肵���}�N���t�@�C�����i�t���p�X�j���擾����	// Jun. 14, 2003 genta �����ǉ��D�����ύX
	bool BeReloadWhenExecuteMacro(int idx);	// idx�Ŏw�肵���}�N���́A���s���邽�тɃt�@�C����ǂݍ��ސݒ肩�H

	// �^�C�v�ʐݒ�(�R���g���[���v���Z�X��p)
	void CreateTypeSettings();
	std::vector<TypeConfig*>& GetTypeSettings();

	// ���ۉ��Ή��̂��߂̕������ύX����(�R���g���[���v���Z�X��p)
	void ConvertLangValues(std::vector<std::wstring>& values, bool bSetValues);

	static Mutex& GetMutexShareWork();

protected:
	/*
	||  �����w���p�֐�
	*/

	// Jan. 30, 2005 genta �������֐��̕���
	void InitKeyword(DllSharedData&);
	bool InitKeyAssign(DllSharedData&); // 2007.11.04 genta �N�����~�̂��ߒl��Ԃ�
	void RefreshKeyAssignString(DllSharedData&);
	void InitToolButtons(DllSharedData&);
	void InitTypeConfigs(DllSharedData&, std::vector<TypeConfig*>&);
	void InitPopupMenu(DllSharedData&);

public:
	static void InitFileTree(FileTree*);

private:
	SelectLang		m_selectLang;			// ���b�Z�[�W���\�[�XDLL�ǂݍ��ݗp�i�v���Z�X��1�j		// 2011.04.10 nasukoji
	HANDLE			m_hFileMap;
	DllSharedData*	m_pShareData;
	std::vector<TypeConfig*>* 	m_pvTypeSettings;	// (�R���g���[���v���Z�X�̂�)
	HWND			m_hwndTraceOutSource;	// TraceOutA()�N�����E�B���h�E�i���������N�������w�肵�Ȃ��Ă��ނ悤�Ɂj

};

