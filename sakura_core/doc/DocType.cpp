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
	nSettingTypeLocked(false)	// 設定値変更可能フラグ
{
}

// 文書種別の設定
void DocType::SetDocumentType(
	TypeConfigNum type,
	bool force,
	bool bTypeOnly
	)
{
	if (!nSettingTypeLocked || force) {
		nSettingType = type;
		if (!DocTypeManager().GetTypeConfig(nSettingType, typeConfig)) {
			// 削除されてる/不正
			nSettingType = DocTypeManager().GetDocumentTypeOfPath(doc.docFile.GetFilePath());
			DocTypeManager().GetTypeConfig(nSettingType, typeConfig);
		}
		if (bTypeOnly) {
			return;	// bTypeOnly == true は特殊ケース（一時利用）に限定
		}
		UnlockDocumentType();
	}else {
		// データは更新しておく
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

	// タイプ別設定更新を反映
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
	アイコンの設定
	
	タイプ別設定に応じてウィンドウアイコンをファイルに関連づけられた物，
	または標準のものに設定する．
	
	@author genta
	@date 2002.09.10
*/
void DocType::SetDocumentIcon()
{
	// Grepモードの時はアイコンを変更しない
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

