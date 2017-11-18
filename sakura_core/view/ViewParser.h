#pragma once

class EditView;

// 品詞解析クラス
class ViewParser {
public:
	ViewParser(const EditView& editView) : editView(editView) { }
	virtual ~ViewParser() {}

	// カーソル直前の単語を取得
	size_t GetLeftWord(NativeW* pMemWord, int nMaxWordLen) const;

	// キャレット位置の単語を取得
	// 2006.03.24 fon
	bool GetCurrentWord(NativeW* pMemWord) const;

private:
	const EditView& editView;
};

