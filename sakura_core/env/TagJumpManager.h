#pragma once

// 要先行定義
// #define DllSharedData.h

// タグジャンプ情報
struct TagJump {
	HWND hwndReferer;	// 参照元ウィンドウ
	Point point;		// ライン, カラム
};


// 共有メモリ内構造体
struct Share_TagJump {
	// 型
	typedef StaticVector<
		StaticString<wchar_t, _MAX_PATH>,
		MAX_TAGJUMP_KEYWORD
	> ATagJumpKeywords;

	// データ
	int					tagJumpNum;					// タグジャンプ情報の有効データ数
	int					tagJumpTop;					// スタックの一番上の位置
	TagJump				tagJumps[MAX_TAGJUMPNUM];	// タグジャンプ情報
	ATagJumpKeywords	aTagJumpKeywords;
	bool				bTagJumpICase;				// 大文字小文字を同一視
	bool				bTagJumpAnyWhere;			// 文字列の途中にマッチ
};


class TagJumpManager {
public:
	TagJumpManager() {
		pShareData = &GetDllShareData();
	}
	// タグジャンプ関連
	void PushTagJump(const TagJump*);		// タグジャンプ情報の保存
	bool PopTagJump(TagJump*);				// タグジャンプ情報の参照
private:
	DllSharedData* pShareData;
};

