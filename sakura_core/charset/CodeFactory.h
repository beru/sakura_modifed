#pragma once

class CodeBase;

class CodeFactory {
public:
	// eCodeTypeに適合する CodeBaseインスタンス を生成
	static CodeBase* CreateCodeBase(
		EncodingType	eCodeType,		// 文字コード
		int			nFlag			// bit 0: MIME Encodeされたヘッダをdecodeするかどうか
	);
};

