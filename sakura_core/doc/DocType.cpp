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

#include "StdAfx.h"
#include "DocType.h"
#include "EditDoc.h"
#include "EditApp.h"
#include "window/EditWnd.h"
#include "GrepAgent.h"
#include "view/colors/ColorStrategy.h"
#include "view/figures/FigureManager.h"
#include "env/DllSharedData.h"

DocType::DocType(EditDoc* pDoc)
	:
	m_pDocRef(pDoc),
	m_nSettingType(0),			// Sep. 11, 2002 genta
	m_typeConfig(GetDllShareData().typeBasis),
	m_nSettingTypeLocked(false)	// 設定値変更可能フラグ
{
}

// 文書種別の設定
void DocType::SetDocumentType(
	TypeConfigNum type,
	bool force,
	bool bTypeOnly
	)
{
	if (!m_nSettingTypeLocked || force) {
		m_nSettingType = type;
		if (!DocTypeManager().GetTypeConfig(m_nSettingType, m_typeConfig)) {
			// 削除されてる/不正
			m_nSettingType = DocTypeManager().GetDocumentTypeOfPath(m_pDocRef->m_docFile.GetFilePath());
			DocTypeManager().GetTypeConfig(m_nSettingType, m_typeConfig);
		}
		if (bTypeOnly) {
			return;	// bTypeOnly == true は特殊ケース（一時利用）に限定
		}
		UnlockDocumentType();
	}else {
		// データは更新しておく
		TypeConfigNum temp = DocTypeManager().GetDocumentTypeOfId(m_typeConfig.id);
		if (temp.IsValidType()) {
			m_nSettingType = temp;
			DocTypeManager().GetTypeConfig(m_nSettingType, m_typeConfig);
		}else {
			m_nSettingType = type;
			if (!DocTypeManager().GetTypeConfig(m_nSettingType, m_typeConfig)) {
				m_nSettingType = DocTypeManager().GetDocumentTypeOfPath(m_pDocRef->m_docFile.GetFilePath());
				DocTypeManager().GetTypeConfig(m_nSettingType, m_typeConfig);
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
	m_pDocRef->SetBackgroundImage();
}

void DocType::SetDocumentTypeIdx(int id, bool force)
{
	int setId = m_typeConfig.id;
	if (!m_nSettingTypeLocked || force) {
		if (id != -1) {
			setId = id;
		}
	}
	TypeConfigNum temp = DocTypeManager().GetDocumentTypeOfId(setId);
	if (temp.IsValidType()) {
		m_nSettingType = temp;
		m_typeConfig.nIdx = temp.GetIndex();
		m_typeConfig.id = setId;
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
	if (EditApp::getInstance().m_pGrepAgent->m_bGrepMode) {
		return;
	}
	
	HICON hIconBig, hIconSmall;
	if (this->GetDocumentAttribute().bUseDocumentIcon) {
		m_pDocRef->m_pEditWnd->GetRelatedIcon(m_pDocRef->m_docFile.GetFilePath(), &hIconBig, &hIconSmall);
	}else {
		m_pDocRef->m_pEditWnd->GetDefaultIcon(&hIconBig, &hIconSmall);
	}
	m_pDocRef->m_pEditWnd->SetWindowIcon(hIconBig, ICON_BIG);
	m_pDocRef->m_pEditWnd->SetWindowIcon(hIconSmall, ICON_SMALL);
}

