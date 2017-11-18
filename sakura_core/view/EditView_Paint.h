#pragma once

class EditView;


// クリッピング領域を計算する際のフラグ
enum class PaintAreaType {
	LineNumber = (1 << 0), // 行番号
	Ruler      = (1 << 1), // ルーラー
	Body       = (1 << 2), // 本文

	// 特殊
	All        = PaintAreaType::LineNumber | PaintAreaType::Ruler | PaintAreaType::Body, // ぜんぶ
};

class EditView_Paint {
public:
	virtual EditView& GetEditView() = 0;
	virtual const EditView& GetEditView() const = 0;

public:
	virtual ~EditView_Paint() {}
	void Call_OnPaint(
		int nPaintFlag,   // 描画する領域を選択する
		bool bUseMemoryDC // メモリDCを使用する
	);
};

