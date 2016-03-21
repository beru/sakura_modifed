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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �A�N�Z�T                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �ǂ�����ł��A�N�Z�X�ł���A���L�f�[�^�A�N�Z�T�B2007.10.30 kobake
struct DllSharedData;

// DllSharedData�ւ̊ȈՃA�N�Z�T
inline DllSharedData& GetDllShareData()
{
	extern DllSharedData* g_theDLLSHAREDATA;

	assert(g_theDLLSHAREDATA);
	return *g_theDLLSHAREDATA;
}

inline DllSharedData& GetDllShareData(bool bNullCheck)
{
	extern DllSharedData* g_theDLLSHAREDATA;

	if (bNullCheck) {
		assert(g_theDLLSHAREDATA);
	}
	return *g_theDLLSHAREDATA;
}

// DllSharedData���m�ۂ�����A�܂�������ĂԁB�j������O�ɂ��ĂԁB
inline
void SetDllShareData(DllSharedData* pShareData)
{
	extern DllSharedData* g_theDLLSHAREDATA;

	g_theDLLSHAREDATA = pShareData;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���L�������\���v�f                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 2010.04.19 Moca CShareData����DllSharedData�����o��include��DllSharedData.h�Ɉړ�

#include "config/maxdata.h"

#include "env/AppNodeManager.h"	// Share_Nodes
// 2007.09.28 kobake Common�\���̂�ShareData.h���番��
#include "env/CommonSetting.h"
#include "env/SearchKeywordManager.h"	// Share_SearchKeywords
#include "env/TagJumpManager.h"		// Share_TagJump
#include "env/FileNameManager.h"		// Share_FileNameManagement

#include "EditInfo.h"
#include "types/Type.h" // TypeConfig
#include "print/Print.h" // PrintSetting
#include "recent/SShare_History.h"	// SShare_History


// ���L�t���O
struct Share_Flags {
	bool	bEditWndChanging;		// �ҏW�E�B���h�E�ؑ֒�	// 2007.04.03 ryoji
	/*	@@@ 2002.1.24 YAZAKI
		�L�[�{�[�h�}�N���́A�L�^�I���������_�Ńt�@�C���um_szKeyMacroFileName�v�ɏ����o�����Ƃɂ���B
		bRecordingKeyMacro��true�̂Ƃ��́A�L�[�{�[�h�}�N���̋L�^���Ȃ̂ŁAm_szKeyMacroFileName�ɃA�N�Z�X���Ă͂Ȃ�Ȃ��B
	*/
	bool	bRecordingKeyMacro;		// �L�[�{�[�h�}�N���̋L�^��
	HWND	hwndRecordingKeyMacro;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
};

// ���L���[�N�o�b�t�@
struct Share_WorkBuffer {
	// 2007.09.16 kobake char�^���ƁA��ɕ�����ł���Ƃ�������������̂ŁABYTE�^�ɕύX�B�ϐ������ύX�B
	//           UNICODE�łł́A�]���ɗ̈���g�����Ƃ��\�z����邽�߁AANSI�ł�2�{�m�ہB
private:
	BYTE m_pWork[32000 * sizeof(TCHAR)];
public:
	template <class T>
	T* GetWorkBuffer() { return reinterpret_cast<T*>(m_pWork); }

	template <class T>
	size_t GetWorkBufferCount() { return sizeof(m_pWork)/sizeof(T); }

public:
	EditInfo	editInfo_MYWM_GETFILEINFO;	// MYWM_GETFILEINFO�f�[�^�󂯓n���p	####�������Ȃ�
	LogicPoint	logicPoint;
	TypeConfig	typeConfig;
};

// ���L�n���h��
struct Share_Handles {
	HWND	hwndTray;
	HWND	hwndDebug;
	HACCEL	hAccel;
};

// EXE���
struct Share_Version {
	DWORD	dwProductVersionMS;
	DWORD	dwProductVersionLS;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   ���L�������\���̖{��                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DllSharedData {
	// -- -- �o�[�W���� -- -- //
	/*!
		�f�[�^�\�� Version	// Oct. 27, 2000 genta
		�f�[�^�\���̈قȂ�o�[�W�����̓����N����h������
		�K���擪�ɂȂ��Ă͂Ȃ�Ȃ��D
	*/
	unsigned int				vStructureVersion;
	unsigned int				nSize;

	// -- -- ��ۑ��Ώ� -- -- //
	Share_Version				version;		// ���Ǎ��͍s��Ȃ����A�����͍s��
	Share_WorkBuffer			workBuffer;
	Share_Flags					flags;
	Share_Nodes					nodes;
	Share_Handles				handles;

	CharWidthCache				charWidth;								// �������p�S�p�L���b�V��
	DWORD						dwCustColors[16];						// �t�H���gDialog�J�X�^���p���b�g

	// �v���O�C��
	short						plugCmdIcons[MAX_PLUGIN*MAX_PLUG_CMD];	// �v���O�C�� �R�}���h ICON �ԍ�	// 2010/7/3 Uchi
	int							maxToolBarButtonNum;					// �c�[���o�[�{�^�� �ő�l			// 2010/7/5 Uchi

	// -- -- �ۑ��Ώ� -- -- //
	// �ݒ�
	CommonSetting				common;									// ���ʐݒ�
	int							nTypesCount;							// �^�C�v�ʐݒ萔
	TypeConfig					typeBasis;								// �^�C�v�ʐݒ�: ����
	TypeConfigMini				typesMini[MAX_TYPES];					// �^�C�v�ʐݒ�(mini)
	PrintSetting				printSettingArr[MAX_PrintSettingARR];	// ����y�[�W�ݒ�
	int							nLockCount;								// ���b�N�J�E���g
	
	// ���̑�
	Share_SearchKeywords		searchKeywords;
	Share_TagJump				tagJump;
	Share_FileNameManagement	fileNameManagement;
	SShare_History				history;

	// �O���R�}���h���s�_�C�A���O�̃I�v�V����
	int							nExecFlgOpt;				// �O���R�}���h���s�I�v�V����	// 2006.12.03 maru �I�v�V�����̊g���̂���
	// DIFF�����\���_�C�A���O�̃I�v�V����
	int							nDiffFlgOpt;				// DIFF�����\��	//@@@ 2002.05.27 MIK
	// �^�O�t�@�C���̍쐬�_�C�A���O�̃I�v�V����
	TCHAR						szTagsCmdLine[_MAX_PATH];	// TAGS�R�}���h���C���I�v�V����	//@@@ 2003.05.12 MIK
	int							nTagsOpt;					// TAGS�I�v�V����(�`�F�b�N)	//@@@ 2003.05.12 MIK

	// -- -- �e���|���� -- -- //
	// �w��s�փW�����v�_�C�A���O�̃I�v�V����
	bool						bLineNumIsCRLF_ForJump;		// �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v��
};

class ShareDataLockCounter {
public:
	ShareDataLockCounter();
	~ShareDataLockCounter();

	static int GetLockCounter();
	static void WaitLock(HWND, ShareDataLockCounter** = NULL);
private:
};

