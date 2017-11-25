#pragma once

class TextArea;
class EditView;
class EditDoc;
class TextMetrics;
class Graphics;

class Ruler {
public:
	Ruler(const EditView& editView, const EditDoc& editDoc);
	virtual ~Ruler();
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     インターフェース                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	
	// ルーラー描画 (背景とキャレット)
	void DispRuler(HDC);
	
	// ルーラーの背景のみ描画
	void DrawRulerBg(Graphics& gr);
	
	void SetRedrawFlag() { bRedrawRuler = true; }
	bool GetRedrawFlag() { return bRedrawRuler; }
	
private:
	// ルーラーのキャレットのみ描画 2002.02.25 Add By KK
	void DrawRulerCaret(Graphics& gr);
	
	void _DrawRulerCaret(Graphics& gr, int nCaretDrawX, int nCaretWidth);
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	// 参照
	const EditView&	editView;
	const EditDoc&	editDoc;
	
	// 状態
	bool	bRedrawRuler;		// ルーラー全体を描き直す時 = true
	int		nOldRulerDrawX;	// 前回描画したルーラーのキャレット位置
	int		nOldRulerWidth;	// 前回描画したルーラーのキャレット幅
};

