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
#include "uiparts/SoundSet.h"
#include "uiparts/ImageListMgr.h"
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
	HINSTANCE GetAppInstance() const { return hInst; }	// �C���X�^���X�n���h���擾

	// �E�B���h�E���
	EditWnd* GetEditWindow() { return pEditWnd; }		// �E�B���h�E�擾

	EditDoc*		GetDocument() { return pEditDoc; }
	ImageListMgr&	GetIcons() { return icons; }

	bool OpenPropertySheet(int nPageNum);
	bool OpenPropertySheetTypes(int nPageNum, TypeConfigNum nSettingType);

public:
	HINSTANCE			hInst;

	// �h�L�������g
	EditDoc*			pEditDoc;

	// �E�B���h�E
	EditWnd*			pEditWnd;

	// IO�Ǘ�
	LoadAgent*			pLoadAgent;
	SaveAgent*			pSaveAgent;
	VisualProgress*		pVisualProgress;

	// ���̑��w���p
	MruListener*		pMruListener;		// MRU�Ǘ�
	SMacroMgr*			pSMacroMgr;			// �}�N���Ǘ�
private:
	PropertyManager*	pPropertyManager;	// �v���p�e�B�Ǘ�
public:
	GrepAgent*			pGrepAgent;			// GREP���[�h
	SoundSet			soundSet;			// �T�E���h�Ǘ�

	// GUI�I�u�W�F�N�g
	ImageListMgr		icons;				// Image List
};


// WM_QUIT���o��O
class AppExitException : public std::exception {
public:
	const char* what() const throw() { return "AppExitException"; }
};

