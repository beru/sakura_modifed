#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "convert/Decode_Base64Decode.h"
#include "convert/Decode_UuDecode.h"
#include "io/BinaryStream.h"

// ViewCommanderクラスのコマンド(変換系)関数群

// 小文字
void ViewCommander::Command_ToLower(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_TOLOWER);
	return;
}

// 大文字
void ViewCommander::Command_ToUpper(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_TOUPPER);
	return;
}

// 全角→半角
void ViewCommander::Command_ToHankaku(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_TOHANKAKU);
	return;
}


// 半角＋全ひら→全角・カタカナ
void ViewCommander::Command_ToZenkakuKata(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_TOZENKAKUKATA);
	return;
}


// 半角＋全カタ→全角・ひらがな
void ViewCommander::Command_ToZenkakuHira(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_TOZENKAKUHIRA);
	return;
}


// 半角英数→全角英数
void ViewCommander::Command_ToZenEi(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_TOZENEI);
	return;
}


// 全角英数→半角英数
void ViewCommander::Command_ToHanEi(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_TOHANEI);
	return;
}


// 全角カタカナ→半角カタカナ
void ViewCommander::Command_ToHankata(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_TOHANKATA);
	return;
}


// 半角カタカナ→全角カタカナ
void ViewCommander::Command_HanKataToZenkakuKata(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_HANKATATOZENKATA);
	return;
}


// 半角カタカナ→全角ひらがな
void ViewCommander::Command_HanKataToZenKakuHira(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_HANKATATOZENHIRA);
	return;
}


// TAB→空白
void ViewCommander::Command_TabToSpace(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_TABTOSPACE);
	return;
}

// 空白→TAB
void ViewCommander::Command_SpaceToTab(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_SPACETOTAB);
	return;
}


// 自動判別→SJISコード変換
void ViewCommander::Command_CodeCnv_Auto2SJIS(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_AUTO2SJIS);
	return;
}


// E-Mail(JIS→SJIS)コード変換
void ViewCommander::Command_CodeCnv_EMail(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_EMAIL);
	return;
}


// EUC→SJISコード変換
void ViewCommander::Command_CodeCnv_EUC2SJIS(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_EUC2SJIS);
	return;
}


// Unicode→SJISコード変換
void ViewCommander::Command_CodeCnv_Unicode2SJIS(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_UNICODE2SJIS);
	return;
}


// UnicodeBE→SJISコード変換
void ViewCommander::Command_CodeCnv_UnicodeBE2SJIS(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_UNICODEBE2SJIS);
	return;
}


// UTF-8→SJISコード変換
void ViewCommander::Command_CodeCnv_UTF82SJIS(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_UTF82SJIS);
	return;
}


// UTF-7→SJISコード変換
void ViewCommander::Command_CodeCnv_UTF72SJIS(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_UTF72SJIS);
	return;
}


// SJIS→JISコード変換
void ViewCommander::Command_CodeCnv_SJIS2JIS(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_SJIS2JIS);
	return;
}


// SJIS→EUCコード変換
void ViewCommander::Command_CodeCnv_SJIS2EUC(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_SJIS2EUC);
	return;
}


// SJIS→UTF-8コード変換
void ViewCommander::Command_CodeCnv_SJIS2UTF8(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_SJIS2UTF8);
	return;
}


// SJIS→UTF-7コード変換
void ViewCommander::Command_CodeCnv_SJIS2UTF7(void)
{
	// 選択エリアのテキストを指定方法で変換
	view.ConvSelectedArea(F_CODECNV_SJIS2UTF7);
	return;
}


// Base64デコードして保存
void ViewCommander::Command_Base64Decode(void)
{
	// テキストが選択されているか
	if (!view.GetSelectionInfo().IsTextSelected()) {
		ErrorBeep();
		return;
	}
	// 選択範囲のデータを取得
	// 正常時はtrue,範囲未選択の場合はfalseを返す
	NativeW ctextBuf;
	if (!view.GetSelectedDataSimple(ctextBuf)) {
		ErrorBeep();
		return;
	}

	// Base64デコード
	Memory memBuf;
	bool bret = Decode_Base64Decode().CallDecode(ctextBuf, &memBuf);
	if (!bret) {
		return;
	}
	ctextBuf.Clear();

	// 保存ダイアログ モーダルダイアログの表示
	TCHAR szPath[_MAX_PATH] = _T("");
	if (!GetDocument().docFileOperation.SaveFileDialog(szPath)) {
		return;
	}

	// データ
	size_t nDataLen;
	const void* pData = memBuf.GetRawPtr(&nDataLen);

	// カキコ
	BinaryOutputStream out(szPath);
	if (!out) goto err;
	if (nDataLen != out.Write(pData, nDataLen)) goto err;

	return;

err:
	ErrorBeep();
	ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD14), szPath);
}


// uudecodeして保存
void ViewCommander::Command_UUDecode(void)
{
	// テキストが選択されているか
	if (!view.GetSelectionInfo().IsTextSelected()) {
		ErrorBeep();
		return;
	}

	// 選択範囲のデータを取得 -> memBuf
	// 正常時はtrue,範囲未選択の場合はfalseを返す
	NativeW ctextBuf;
	if (!view.GetSelectedDataSimple(ctextBuf)) {
		ErrorBeep();
		return;
	}

	// uudecode(デコード)  ctextBuf -> memBin, szPath
	Memory memBin;
	TCHAR szPath[_MAX_PATH] = _T("");
	Decode_UuDecode decoder;
	if (!decoder.CallDecode(ctextBuf, &memBin)) {
		return;
	}
	decoder.CopyFilename(szPath);
	ctextBuf.Clear();

	// 保存ダイアログ モーダルダイアログの表示
	if (!GetDocument().docFileOperation.SaveFileDialog(szPath)) {
		return;
	}

	// データ
	size_t nDataLen;
	const void* pData = memBin.GetRawPtr(&nDataLen);

	// カキコ
	BinaryOutputStream out(szPath);
	if (!out) goto err;
	if (nDataLen != out.Write(pData, nDataLen)) goto err;

	// 完了
	return;

err:
	ErrorBeep();
	ErrorMessage(view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD16), szPath);
}

