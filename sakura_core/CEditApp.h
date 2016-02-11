/*
	Copyright (C) 2007, kobake

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

// 2007.10.23 kobake �쐬

#include "util/design_template.h"
#include "uiparts/CSoundSet.h"
#include "uiparts/CImageListMgr.h"
class EditDoc;
class EditWnd;
class LoadAgent;
class SaveAgent;
class VisualProgress;
class MruListener;
class CSMacroMgr;
class PropertyManager;
class GrepAgent;
enum EFunctionCode;

// �G�f�B�^�����A�v���P�[�V�����N���X�BCNormalProcess1�ɂ��A1���݁B
class EditApp : public TSingleton<EditApp> {
	friend class TSingleton<EditApp>;
	EditApp() {}
	virtual ~EditApp();

public:
	void Create(HINSTANCE hInst, int);

	// ���W���[�����
	HINSTANCE GetAppInstance() const { return m_hInst; }	// �C���X�^���X�n���h���擾

	// �E�B���h�E���
	EditWnd* GetEditWindow() { return m_pcEditWnd; }		// �E�B���h�E�擾

	EditDoc*		GetDocument() { return m_pcEditDoc; }
	ImageListMgr&	GetIcons() { return m_cIcons; }

	bool OpenPropertySheet(int nPageNum);
	bool OpenPropertySheetTypes(int nPageNum, CTypeConfig nSettingType);

public:
	HINSTANCE			m_hInst;

	// �h�L�������g
	EditDoc*			m_pcEditDoc;

	// �E�B���h�E
	EditWnd*			m_pcEditWnd;

	// IO�Ǘ�
	LoadAgent*			m_pcLoadAgent;
	SaveAgent*			m_pcSaveAgent;
	VisualProgress*	m_pcVisualProgress;

	// ���̑��w���p
	MruListener*		m_pcMruListener;		// MRU�Ǘ�
	CSMacroMgr*			m_pcSMacroMgr;			// �}�N���Ǘ�
private:
	PropertyManager*	m_pcPropertyManager;	// �v���p�e�B�Ǘ�
public:
	GrepAgent*			m_pcGrepAgent;			// GREP���[�h
	SoundSet			m_cSoundSet;			// �T�E���h�Ǘ�

	// GUI�I�u�W�F�N�g
	ImageListMgr		m_cIcons;				// Image List
};


// WM_QUIT���o��O
class AppExitException : public std::exception {
public:
	const char* what() const throw() { return "AppExitException"; }
};

