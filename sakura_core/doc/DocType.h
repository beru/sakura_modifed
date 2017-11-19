/*
	ドキュメント種別の管理
*/
#pragma once

#include "types/Type.h" // TypeConfig
#include "env/DocTypeManager.h"

class DocType {
public:
	// 生成と破棄
	DocType(EditDoc& doc);
	
	// ロック機能
	void LockDocumentType() { nSettingTypeLocked = true; }
	void UnlockDocumentType() { nSettingTypeLocked = false; }
	bool GetDocumentLockState() { return nSettingTypeLocked; }
	
	// 文書種別の設定と取得
	void SetDocumentType(TypeConfigNum type, bool force, bool bTypeOnly = false);	// 文書種別の設定
	void SetDocumentTypeIdx(int id = -1, bool force = false);
	TypeConfigNum GetDocumentType() const {					// 文書種別の取得
		return nSettingType;
	}
	const TypeConfig& GetDocumentAttribute() const {		// 文書種別の詳細情報
		return typeConfig;
	}
	TypeConfig& GetDocumentAttributeWrite() {				// 文書種別の詳細情報
		return typeConfig;
	}

	// 拡張機能
	void SetDocumentIcon();						// アイコンの設定

private:
	EditDoc&		doc;
	TypeConfigNum	nSettingType;
	TypeConfig		typeConfig;
	bool			nSettingTypeLocked;		// 文書種別の一時設定状態
};

