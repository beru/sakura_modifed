#include "StdAfx.h"
#include "PropertyManager.h"
#include "env/DllSharedData.h"
#include "env/DocTypeManager.h"
#include <memory>

void PropertyManager::Create(
	HWND			hwndOwner,
	ImageListMgr*	pImageList,
	MenuDrawer*		pMenuDrawer
	)
{
	this->hwndOwner = hwndOwner;
	this->pImageList = pImageList;
	this->pMenuDrawer = pMenuDrawer;

	nPropComPageNum = -1;
	nPropTypePageNum = -1;
}

// ���ʐݒ� �v���p�e�B�V�[�g
bool PropertyManager::OpenPropertySheet(
	HWND	hWnd,
	int		nPageNum,
	bool	bTrayProc
	)
{
	bool bRet;
	auto pcPropCommon = std::make_unique<PropCommon>();
	pcPropCommon->Create(hwndOwner, pImageList, pMenuDrawer);

	// ���ʐݒ�̈ꎞ�ݒ�̈��SharaData���R�s�[����
	pcPropCommon->InitData();

	if (nPageNum != -1) {
		nPropComPageNum = nPageNum;
	}

	// �v���p�e�B�V�[�g�̍쐬
	if (pcPropCommon->DoPropertySheet(nPropComPageNum, bTrayProc)) {

		// ShareData �� �ݒ��K�p�E�R�s�[����
		// �O���[�v���ɕύX���������Ƃ��̓O���[�vID�����Z�b�g����
		auto& csTabBar = GetDllShareData().common.tabBar;
		bool bGroup = (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin);

		// ������ɃL�[���[�h���㏑�����Ȃ��悤��
		ShareDataLockCounter* pLock = nullptr;
		ShareDataLockCounter::WaitLock(pcPropCommon->hwndParent, &pLock);

		pcPropCommon->ApplyData();
		// note: ��{�I�ɂ����œK�p���Ȃ��ŁAMYWM_CHANGESETTING���炽�ǂ��ēK�p���Ă��������B
		// ���E�B���h�E�ɂ͍Ō�ɒʒm����܂��B���́AOnChangeSetting �ɂ���܂��B
		// �����ł����K�p���Ȃ��ƁA�ق��̃E�B���h�E���ύX����܂���B

		if (bGroup != (csTabBar.bDispTabWnd && !csTabBar.bDispTabWndMultiWin )) {
			AppNodeManager::getInstance().ResetGroupId();
		}

		// �A�N�Z�����[�^�e�[�u���̍č쐬
		::SendMessage(GetDllShareData().handles.hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL);

		// �ݒ�ύX�𔽉f������
		// �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����
		AppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_CHANGESETTING_ALL,
			hWnd
		);

		delete pLock;
		bRet = true;
	}else {
		bRet = false;
	}

	// �Ō�ɃA�N�Z�X�����V�[�g���o���Ă���
	nPropComPageNum = pcPropCommon->GetPageNum();

	return bRet;
}


// �^�C�v�ʐݒ� �v���p�e�B�V�[�g
bool PropertyManager::OpenPropertySheetTypes(
	HWND		hWnd,
	int			nPageNum,
	TypeConfigNum	nSettingType
	)
{
	bool bRet;
	auto pcPropTypes = std::make_unique<PropTypes>();
	pcPropTypes->Create(G_AppInstance(), hwndOwner);

	auto pType = std::make_unique<TypeConfig>();
	DocTypeManager().GetTypeConfig(nSettingType, *pType);
	pcPropTypes->SetTypeData(*pType);
	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX��ProperySheet���Ŏ擾����悤��

	if (nPageNum != -1) {
		nPropTypePageNum = nPageNum;
	}

	// �v���p�e�B�V�[�g�̍쐬
	if (pcPropTypes->DoPropertySheet(nPropTypePageNum)) {
		ShareDataLockCounter* pLock = nullptr;
		ShareDataLockCounter::WaitLock(pcPropTypes->GetHwndParent(), &pLock);

		pcPropTypes->GetTypeData(*pType);

		DocTypeManager().SetTypeConfig(nSettingType, *pType);

		// �A�N�Z�����[�^�e�[�u���̍č쐬
		// ::SendMessage(GetDllShareData().handles.hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)PM_CHANGESETTING_ALL);

		// �ݒ�ύX�𔽉f������
		// �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g����
		AppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)nSettingType.GetIndex(),
			(LPARAM)PM_CHANGESETTING_TYPE,
			hWnd
		);
		if (pcPropTypes->GetChangeKeywordSet()) {
			AppNodeGroupHandle(0).SendMessageToAllEditors(
				WM_COMMAND,
				(WPARAM)MAKELONG(F_REDRAW, 0),
				(LPARAM)0,
				hWnd
			);
		}

		delete pLock;
		bRet = true;
	}else {
		bRet = false;
	}

	// �Ō�ɃA�N�Z�X�����V�[�g���o���Ă���
	nPropTypePageNum = pcPropTypes->GetPageNum();

	return bRet;
}

