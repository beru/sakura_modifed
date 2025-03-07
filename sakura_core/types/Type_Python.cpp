#include "StdAfx.h"
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "outline/FuncInfoArr.h"


/*!
	関数に用いることができる文字かどうかの判定
	
	@note 厳密には1文字目に数字を使うことは出来ないが，
		それは実行してみれば明らかにわかることなので
		そこまで厳密にチェックしない
*/
inline bool Python_IsWordChar(wchar_t c) {
	return (L'_' == c ||
			(L'a' <= c && c <= L'z')||
			(L'A' <= c && c <= L'Z')||
			(L'0' <= c && c <= L'9')
		);
}


/*! pythonのパース状態を管理する構造体

	解析中に解析関数の間を引き渡される．
	このクラスは現在の状態と，文字列の性質を保持する．
	解析位置は解析関数間でパラメータとして渡されるので
	この中では保持しない．

	[状態遷移]
	開始 : STATE_NORMAL

	STATE_NORMAL/STATE_CONTINUE→STATE_CONTINUEの遷移
	- 継続行マーク有り

	STATE_NORMAL/STATE_CONTINUE→STATE_NORMALの遷移
	- 継続行マークがなく行末に達した
	- コメントに達した

	STATE_NORMAL→STATE_STRINGの遷移
	- 引用符あり

	STATE_STRING→STATE_NORMALの遷移
	- 規定の文字列終了記号
	- short stringで文字列の終了を示す引用符も継続行マークもなく行末に達した
*/
struct OutlinePython {
	enum {
		STATE_NORMAL,	// 通常行 : 行頭を含む
		STATE_STRING,	// 文字列中
		STATE_CONTINUE,	// 継続行 : 前の行からの続きなので行頭とはみなされない
	} state;
	
	int quote_char;	// 引用符記号
	bool raw_string;	// エスケープ記号無視ならtrue
	bool long_string;	// 長い文字列中ならtrue

	OutlinePython();

	/*	各状態における文字列スキャンを行う
		Scan*が呼びだされるときは既にその状態になっていることが前提．
		ある状態から別の状態に移るところまでを扱う．
		別の状態に移る判定がややこしいばあいは，Enter*として関数にする．
	*/	
	size_t ScanNormal(const wchar_t* data, size_t linelen, size_t start_offset);
	size_t ScanString(const wchar_t* data, size_t linelen, size_t start_offset);
	size_t EnterString(const wchar_t* data, size_t linelen, size_t start_offset);
	void DoScanLine(const wchar_t* data, size_t linelen, size_t start_offset);
	
	bool IsLogicalLineTop(void) const { return state == STATE_NORMAL; }
};

/*!コンストラクタ: 初期化

	初期状態をSTATE_NORMALに設定する．
*/
OutlinePython::OutlinePython()
	:
	state(STATE_NORMAL),
	raw_string(false),
	long_string(false)
{
}

/*! @brief Python文字列の入り口で文字列種別を決定する

	文字列の種類を適切に判別し，内部状態を設定する．
	start_offsetは開始引用符を指していること．

	- 引用符1つ: short string
	- 引用符3つ: long string
	- 引用符の前にrかRあり : raw string

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置
	
	@return 調査後の位置

	@invariant
		state != STATE_STRING

	@note 引用符の位置で呼びだせば，抜けた後は必ずSTATE_STRINGになっているはず．
		引用符以外の位置で呼びだした場合は何もしないで抜ける．
*/
size_t OutlinePython::EnterString(const wchar_t* data, size_t linelen, size_t start_offset)
{
	assert(state != STATE_STRING);

	size_t col = start_offset;
	//	文字列開始チェック
	if (data[col] == '\"' || data[col] == '\'') {
		int quote_char = data[col];
		state = STATE_STRING;
		this->quote_char = quote_char;
		//	文字列の開始
		if (col >= 1 &&
			(data[col - 1] == 'r' || data[col - 1] == 'R')
		) {
			//	厳密には直前がSHIFT_JISの2バイト目だと誤判定する可能性があるが
			//	そういう動かないコードは相手にしない
			raw_string = true;
		}else {
			raw_string = false;
		}
		if (col + 2 < linelen &&
			data[col + 1] == quote_char &&
			data[col + 2] == quote_char
		) {
			long_string = true;
			col += 2;
		}else {
			long_string = false;
		}
		++col;
	}
	return col;
}

/*! @brief Pythonプログラムの処理

	プログラム本体部分の処理．文字列の開始，継続行，コメント，通常行末をチェックする．
	行頭判定が終わった後で引き渡されるので，関数・クラス定義は考慮しなくて良い．
	
	以下の場合に処理を終了する
	- 行末: STATE_NORMALとして処理終了
	- コメント: STATE_NORMALとして処理終了
	- 文字列の開始: EnterString() にて文字列種別の判定を行った後STATE_STRINGとして処理終了
	- 継続行: STATE_CONTINUEとして処理終了

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置
	
	@invaliant
		state == STATE_NORMAL || state == STATE_CONTINUE
	
	@return 調査後の位置
*/
size_t OutlinePython::ScanNormal(const wchar_t* data, size_t linelen, size_t start_offset)
{
	assert(state == STATE_NORMAL || state == STATE_CONTINUE);
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	for (size_t col=start_offset; col<linelen; ++col) {
		size_t nCharChars = NativeW::GetSizeOfChar(data, linelen, col);
		if (1 < nCharChars) {
			col += (nCharChars - 1);
			continue;
		}
		//	コメント
		if (data[col] == '#') {
			//	コメントは行末と同じ扱いなので
			//	わざわざ独立して扱う必要性が薄い
			//	ここで片を付けてしまおう
			state = STATE_NORMAL;
			break;
		//	文字列
		}else if (data[col] == '\"' || data[col] == '\'') {
			return EnterString(data, linelen, col);
		//	継続行かもしれない
		}else if (data[col] == '\\') {
			//	CRかCRLFかLFで行末
			//	最終行には改行コードがないことがあるが，それ以降には何もないので影響しない
			if (
				(
					linelen - 2 == col
					&& (data[col + 1] == WCODE::CR && data[col + 2] == WCODE::LF)
				) || (
					linelen - 1 == col
					&& (WCODE::IsLineDelimiter(data[col + 1], bExtEol))
				)
			) {
				state = STATE_CONTINUE;
				break;
			}
		}
	}
	return linelen;
}


/*! @brief python文字列(1行)を調査する

	与えられた状態からPython文字列の状態変化を追い，
	最終的な状態を決定する．
	
	文字列の開始判定はEnterString()関数で処理済みであり，その結果が
	state, raw_string, long_string, quote_charに与えられている．
	
	raw_stringがtrueならbackslashによるエスケープ処理を行わない
	long_stringならquote_charが3つ続くまで文字列となる．

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置
	
	@return 調査後の位置
	
	@invariant
		state==STATE_STRING

*/
size_t OutlinePython::ScanString(const wchar_t* data, size_t linelen, size_t start_offset)
{
	assert(state == STATE_STRING);
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	for (size_t col=start_offset; col<linelen; ++col) {
		size_t nCharChars = NativeW::GetSizeOfChar(data, linelen, col);
		if (1 < nCharChars) {
			col += (nCharChars - 1);
			continue;
		}
		//	rawモード以外ではエスケープをチェック
		//	rawモードでも継続行はチェック
		if (data[col] == '\\' && col + 1 < linelen) {
			wchar_t key = data[col + 1];
			if (! raw_string) {
				if (key == L'\\' ||
					key == L'\"' ||
					key == L'\''
				) {
					++col;
					//	ignore
					continue;
				}
			}
			if (WCODE::IsLineDelimiter(key, bExtEol)) {
				// \r\nをまとめて\nとして扱う必要がある
				if (col + 1 >= linelen ||
					data[col + 2] == key
				) {
					// 本当に行末
					++col;
					continue;
				}else if (data[col + 2] == WCODE::LF) {
					col += 2;	//	 CRLF
				}
			}
		//	short string + 改行の場合はエラーから強制復帰
		}else if (WCODE::IsLineDelimiter(data[col], bExtEol)) {
			// あとで
			if (!long_string) {
				//	文字列の末尾を発見した
				state = STATE_NORMAL;
				return col + 1;
			}
		//	引用符が見つかったら終了チェック
		}else if (data[col] == quote_char) {
			if (!long_string) {
				//	文字列の末尾を発見した
				state = STATE_NORMAL;
				return col + 1;
			}
			//	long stringの場合
			if (col + 2 < linelen &&
				data[col + 1] == quote_char &&
				data[col + 2] == quote_char
			) {
				state = STATE_NORMAL;
				return col + 3;
			}
		}
	}
	return linelen;
}

/*!	Python文字列を行末までスキャンして次の行の状態を決定する

	stateに設定された現在の状態から開始してdataをstart_offsetからlinelenに達するまで
	走査し，行末における状態をstateに格納する．

	現在の状態に応じてサブルーチンに解析処理を依頼する．
	サブルーチンScan**では文字列dataのstart_offsetから状態遷移が発生するまで処理を
	続け，別の状態に遷移した直後に処理済みの桁位置を返して終了する．

	この関数に戻った後は再度現在の状態に応じて処理依頼を行う．これを行末に達するまで繰り返す．

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置

*/
void OutlinePython::DoScanLine(const wchar_t* data, size_t linelen, size_t start_offset)
{
	size_t col = start_offset;
	while (col < linelen) {
		if (state == STATE_NORMAL || state == STATE_CONTINUE) {
			col = ScanNormal(data, linelen, col);
		}else if (state == STATE_STRING) {
			col = ScanString(data, linelen, col);
		}else {
			//	ありえないエラー
			return;
		}
	}
}


/*!	@brief python関数リスト作成

	class, def で始まる行から名前を抜き出す．
	
	class CLASS_NAME(superclass):
	def FUNCTION_NAME(parameters):

	文字列とコメントを除外する必要がある．

	通常の行頭の場合に関数・クラス判定と登録処理を行う．
	Python特有の空白の数を数えてネストレベルを判定する．
	indent_levelを配列として用いており，インデントレベルごとのスペース数を格納する．
	なお，TABは8桁区切りへの移動と解釈することになっている．
	
	通常の行頭でない(文字列中および継続行)，あるいは行頭の処理完了後は
	状態機械 python_analyze_state に判定処理を依頼する．

	@par 文字列
	'' "" 両方OK
	引用符3連続でロング文字列
	直前にrかRがあったらエスケープ記号を無視(ただし改行のエスケープは有効)
	
	@par コメント
	#で始まり，行の継続はない．
*/
void DocOutline::MakeFuncList_python(FuncInfoArr* pFuncInfoArr)
{
	OutlinePython python_analyze_state;

	const int MAX_DEPTH = 10;
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	int indent_level[MAX_DEPTH]; // 各レベルのインデント桁位置()
	indent_level[0] = 0;	// do as python does.
	int depth_index = 0;

	for (size_t nLineCount=0; nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		const wchar_t*	pLine;
		int depth;	//	indent depth
		size_t col = 0;	//	current working column position

		size_t nLineLen;
		pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		
		if (python_analyze_state.IsLogicalLineTop()) {
			//	indent check
			//	桁位置colはデータオフセットdと独立にしないと
			//	文字列比較がおかしくなる
			for (depth=0, col=0; col<nLineLen; ++col) {
				//	calculate indent level
				if (pLine[col] == L' ') {
					++depth;
				}else if (pLine[col] == L'\t') {
					depth = (depth + 8) & ~7;
				}else {
					break;
				}
			}
			if (WCODE::IsLineDelimiter(pLine[col], bExtEol)
				|| pLine[col] == L'\0'
				|| pLine[col] == L'#'
			) {
				//	blank line or comment line are ignored
				continue;
			}
			
			int nItemFuncId = 0;	// topic type
			if (
				nLineLen - col > 3 + 1
				&& wcsncmp_literal(pLine + col, L"def") == 0
			) {
				//	"def"
				nItemFuncId = 1;
				col += 3; // strlen(def)
			}else if (
				nLineLen - col > 5 + 1
				&& wcsncmp_literal(pLine + col, L"class") == 0
			) {
				// class
				nItemFuncId = 4;
				col += 5; // strlen(class)
			}else {
				python_analyze_state.DoScanLine(pLine, nLineLen, col);
				continue;
			}

			//	区切りチェック
			//	define, classic等が対象にならないように，後ろにスペースかタブが
			//	続くことを確認．
			//	本当は継続行として次の行に関数名を書くことも文法上は可能だが
			//	複雑になるので対応しない．
			int c = pLine[col];
			if (c != L' ' && c != L'\t') {
				python_analyze_state.DoScanLine(pLine, nLineLen, col);
				continue;
			}

			//	adjust current depth level
			//	関数内部の実行文のさらに奧に関数があるケースを考慮
			//	def/class以外のインデントは記録しない方がいいので
			//	見出し行と確定してからインデントレベルの判定を行う
			for (int i=depth_index; i>=0; --i) {
				if (depth == indent_level[i]) {
					depth_index = i;
					break;
				}else if (depth > indent_level[i] && i < MAX_DEPTH - 1) {
					depth_index = i + 1;
					indent_level[depth_index] = depth;
					break;
				}
			}

			//	手抜きコメント
			//	厳密には，ここで継続行を入れることが可能だが，
			//	そんなレアなケースは考慮しない
			
			//	skip whitespace
			while (col < nLineLen && C_IsSpace(pLine[col], bExtEol)) {
				++col;
			}

			size_t w_end;
			for (w_end=col;
				w_end<nLineLen && Python_IsWordChar(pLine[w_end]);
				++w_end)
				;
			
			//	厳密には，この後に括弧に囲まれた引数あるいは継承元クラスが
			//	括弧に囲まれて入り，さらに:と続くが
			//	継続行の可能性があるので，そこまでチェックしない
			
			//	ここまでで登録要件OKとみなす
			
			//	このあたりは暫定

			wchar_t szWord[512];	// 適当に大きな数(pythonでは名前の長さの上限があるのか？)
			size_t len = w_end - col;
			
			if (len > 0) {
				if (len > _countof(szWord) - 1) {
					len = _countof(szWord) - 1;
				}
				wcsncpy(szWord, pLine + col, len);
				szWord[len] = L'\0';
			}else {
				wcscpy_s(szWord, LSW(STR_OUTLINE_PYTHON_UNDEFINED));
				len = 8;
			}
			if (nItemFuncId == 4) {
				if (_countof(szWord) - 8  < len) {
					//	後ろを削って入れる
					len = _countof(szWord) - 8;
				}
				// class
				wcscpy(szWord + len, LSW(STR_OUTLINE_PYTHON_CLASS));
			}
			
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			Point ptPosXY = doc.layoutMgr.LogicToLayout(Point(0, (int)nLineCount));
			pFuncInfoArr->AppendData(
				nLineCount + 1,
				ptPosXY.y + 1,
				szWord,
				nItemFuncId,
				depth_index
			);
			col = w_end; // クラス・関数定義の続きはここから
		}
		python_analyze_state.DoScanLine(pLine, nLineLen, col);
	}
}

