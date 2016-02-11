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
class SMacroMgr;
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
	EditWnd* GetEditWindow() { return m_pEditWnd; }		// �E�B���h�E�擾

	EditDoc*		GetDocument() { return m_pEditDoc; }
	ImageListMgr&	GetIcons() { return m_icons; }

	bool OpenPropertySheet(int nPageNum);
	bool OpenPropertySheetTypes(int nPageNum, TypeConfigNum nSettingType);

public:
	HINSTANCE			m_hInst;

	// �h�L�������g
	EditDoc*			m_pEditDoc;

	// �E�B���h�E
	EditWnd*			m_pEditWnd;

	// IO�Ǘ�
	LoadAgent*			m_pLoadAgent;
	SaveAgent*			m_pSaveAgent;
	VisualProgress*		m_pVisualProgress;

	// ���̑��w���p
	MruListener*		m_pMruListener;		// MRU�Ǘ�
	SMacroMgr*			m_pSMacroMgr;		// �}�N���Ǘ�
private:
	PropertyManager*	m_pPropertyManager;	// �v���p�e�B�Ǘ�
public:
	GrepAgent*			m_pGrepAgent;		// GREP���[�h
	SoundSet			m_soundSet;			// �T�E���h�Ǘ�

	// GUI�I�u�W�F�N�g
	ImageListMgr		m_icons;			// Image List
};


// WM_QUIT���o��O
class AppExitException : public std::exception {
public:
	const char* what() const throw() { return "AppExitException"; }
};

