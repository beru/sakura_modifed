#pragma once

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

