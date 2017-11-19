#include "StdAfx.h"
#include "EditApp.h"
#include "doc/EditDoc.h"
#include "window/EditWnd.h"
#include "LoadAgent.h"
#include "SaveAgent.h"
#include "uiparts/VisualProgress.h"
#include "recent/MruListener.h"
#include "macro/SMacroMgr.h"
#include "PropertyManager.h"
#include "GrepAgent.h"
#include "_main/AppMode.h"
#include "_main/CommandLine.h"
#include "util/module.h"
#include "util/shell.h"

void EditApp::Create(HINSTANCE hInst, int nGroupId)
{
	this->hInst = hInst;

	// �w���p�쐬
	icons.Create(hInst);	//	CreateImage List

	// �h�L�������g�̍쐬
	pEditDoc = new EditDoc(*this);

	// IO�Ǘ�
	pLoadAgent = new LoadAgent();
	pSaveAgent = new SaveAgent();
	pVisualProgress = new VisualProgress();

	// GREP���[�h�Ǘ�
	pGrepAgent = new GrepAgent();

	// �ҏW���[�h
	AppMode::getInstance();	// �E�B���h�E�����O�ɃC�x���g���󂯎�邽�߂ɂ����ŃC���X�^���X�쐬

	// �}�N��
	pSMacroMgr = new SMacroMgr();

	// �E�B���h�E�̍쐬
	pEditWnd = &EditWnd::getInstance();

	pEditDoc->Create(pEditWnd);
	pEditWnd->Create(pEditDoc, &icons, nGroupId);

	// MRU�Ǘ�
	pMruListener = new MruListener();

	// �v���p�e�B�Ǘ�
	pPropertyManager = new PropertyManager();
	pPropertyManager->Create(
		pEditWnd->GetHwnd(),
		&GetIcons(),
		&pEditWnd->GetMenuDrawer()
	);
}

EditApp::~EditApp()
{
	delete pSMacroMgr;
	delete pPropertyManager;
	delete pMruListener;
	delete pGrepAgent;
	delete pVisualProgress;
	delete pSaveAgent;
	delete pLoadAgent;
	delete pEditDoc;
}

// ���ʐݒ� �v���p�e�B�V�[�g
bool EditApp::OpenPropertySheet(int nPageNum)
{
	// �v���p�e�B�V�[�g�̍쐬
	bool bRet = pPropertyManager->OpenPropertySheet(pEditWnd->GetHwnd(), nPageNum, false);
	if (bRet) {
		// �}�N���o�^�ύX�𔽉f���邽�߁C�ǂݍ��ݍς݂̃}�N����j������
		pSMacroMgr->UnloadAll();
	}
	return bRet;
}

// �^�C�v�ʐݒ� �v���p�e�B�V�[�g
bool EditApp::OpenPropertySheetTypes(int nPageNum, TypeConfigNum nSettingType)
{
	bool bRet = pPropertyManager->OpenPropertySheetTypes(pEditWnd->GetHwnd(), nPageNum, nSettingType);

	return bRet;
}

