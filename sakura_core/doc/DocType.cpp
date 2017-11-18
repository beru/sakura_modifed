#include "StdAfx.h"
#include "DocType.h"
#include "EditDoc.h"
#include "EditApp.h"
#include "window/EditWnd.h"
#include "GrepAgent.h"
#include "view/colors/ColorStrategy.h"
#include "view/figures/FigureManager.h"
#include "env/DllSharedData.h"

DocType::DocType(EditDoc& doc)
	:
	doc(doc),
	nSettingType(0),			// Sep. 11, 2002 genta
	typeConfig(GetDllShareData().typeBasis),
	nSettingTypeLocked(false)	// �ݒ�l�ύX�\�t���O
{
}

// ������ʂ̐ݒ�
void DocType::SetDocumentType(
	TypeConfigNum type,
	bool force,
	bool bTypeOnly
	)
{
	if (!nSettingTypeLocked || force) {
		nSettingType = type;
		if (!DocTypeManager().GetTypeConfig(nSettingType, typeConfig)) {
			// �폜����Ă�/�s��
			nSettingType = DocTypeManager().GetDocumentTypeOfPath(doc.docFile.GetFilePath());
			DocTypeManager().GetTypeConfig(nSettingType, typeConfig);
		}
		if (bTypeOnly) {
			return;	// bTypeOnly == true �͓���P�[�X�i�ꎞ���p�j�Ɍ���
		}
		UnlockDocumentType();
	}else {
		// �f�[�^�͍X�V���Ă���
		TypeConfigNum temp = DocTypeManager().GetDocumentTypeOfId(typeConfig.id);
		if (temp.IsValidType()) {
			nSettingType = temp;
			DocTypeManager().GetTypeConfig(nSettingType, typeConfig);
		}else {
			nSettingType = type;
			if (!DocTypeManager().GetTypeConfig(nSettingType, typeConfig)) {
				nSettingType = DocTypeManager().GetDocumentTypeOfPath(doc.docFile.GetFilePath());
				DocTypeManager().GetTypeConfig(nSettingType, typeConfig);
			}
		}
		if (bTypeOnly) {
			return;
		}
	}

	// �^�C�v�ʐݒ�X�V�𔽉f
	ColorStrategyPool::getInstance().OnChangeSetting();
	FigureManager::getInstance().OnChangeSetting();
	this->SetDocumentIcon();	// Sep. 11, 2002 genta
	doc.SetBackgroundImage();
}

void DocType::SetDocumentTypeIdx(int id, bool force)
{
	int setId = typeConfig.id;
	if (!nSettingTypeLocked || force) {
		if (id != -1) {
			setId = id;
		}
	}
	TypeConfigNum temp = DocTypeManager().GetDocumentTypeOfId(setId);
	if (temp.IsValidType()) {
		nSettingType = temp;
		typeConfig.nIdx = temp.GetIndex();
		typeConfig.id = setId;
	}
}

/*!
	�A�C�R���̐ݒ�
	
	�^�C�v�ʐݒ�ɉ����ăE�B���h�E�A�C�R�����t�@�C���Ɋ֘A�Â���ꂽ���C
	�܂��͕W���̂��̂ɐݒ肷��D
	
	@author genta
	@date 2002.09.10
*/
void DocType::SetDocumentIcon()
{
	// Grep���[�h�̎��̓A�C�R����ύX���Ȃ�
	if (EditApp::getInstance().pGrepAgent->bGrepMode) {
		return;
	}
	
	HICON hIconBig, hIconSmall;
	if (this->GetDocumentAttribute().bUseDocumentIcon) {
		doc.pEditWnd->GetRelatedIcon(doc.docFile.GetFilePath(), &hIconBig, &hIconSmall);
	}else {
		doc.pEditWnd->GetDefaultIcon(&hIconBig, &hIconSmall);
	}
	doc.pEditWnd->SetWindowIcon(hIconBig, ICON_BIG);
	doc.pEditWnd->SetWindowIcon(hIconSmall, ICON_SMALL);
}

