#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Numeric.h"
#include "parse/WordParse.h"
#include "util/string_ex2.h"
#include "doc/layout/Layout.h"
#include "types/TypeSupport.h"

static int IsNumber(const StringRef& str, int offset);	// 数値ならその長さを返す

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         半角数値                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_Numeric::BeginColor(const StringRef& str, size_t nPos)
{
	if (!str.IsValid()) return false;

	int	nnn;

	if (1
		&& _IsPosKeywordHead(str, nPos)
		&& (nnn = IsNumber(str, nPos)) > 0
	) {		// 半角数字を表示する
		// キーワード文字列の終端をセットする
		this->nCommentEnd = nPos + nnn;
		return true;	// 半角数値である
	}
	return false;
}


bool Color_Numeric::EndColor(const StringRef& str, size_t nPos)
{
	return (nPos == this->nCommentEnd);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
 * 数値なら長さを返す。
 * 10進数の整数または小数。16進数(正数)。
 * 文字列   数値(色分け)
 * ---------------------
 * 123      123
 * 0123     0123
 * 0xfedc   0xfedc
 * -123     -123
 * &H9a     &H9a     (ただしソース中の#ifを有効にしたとき)
 * -0x89a   0x89a
 * 0.5      0.5
 * 0.56.1   0.56 , 1 (ただしソース中の#ifを有効にしたら"0.56.1"になる)
 * .5       5        (ただしソース中の#ifを有効にしたら".5"になる)
 * -.5      5        (ただしソース中の#ifを有効にしたら"-.5"になる)
 * 123.     123
 * 0x567.8  0x567 , 8
 */
/*
 * 半角数値
 *   1, 1.2, 1.2.3, .1, 0xabc, 1L, 1F, 1.2f, 0x1L, 0x2F, -.1, -1, 1e2, 1.2e+3, 1.2e-3, -1e0
 *   10進数, 16進数, LF接尾語, 浮動小数点数, 負符号
 *   IPアドレスのドット連結(本当は数値じゃないんだよね)
 */
static int IsNumber(const StringRef& str, /*const wchar_t* buf,*/ int offset/*, int length*/)
{
	register const wchar_t* p;
	register const wchar_t* q;
	register int i = 0;
	register int d = 0;
	register int f = 0;

	p = str.GetPtr() + offset;
	q = str.GetPtr() + str.GetLength();

	if (*p == L'0') {  // 10進数,Cの16進数
		++p; ++i;
		if ((p < q) && (*p == L'x')) {  // Cの16進数
			++p; ++i;
			while (p < q) {
				if (0
					|| (*p >= L'0' && *p <= L'9')
					|| (*p >= L'A' && *p <= L'F')
					|| (*p >= L'a' && *p <= L'f')
				) {
					++p; ++i;
				}else {
					break;
				}
			}
			// "0x" なら "0" だけが数値
			if (i == 2) return 1;
			
			// 接尾語
			if (p < q) {
				if (*p == L'L' || *p == L'l' || *p == L'F' || *p == L'f') {
					++p; ++i;
				}
			}
			return i;
		}else if (*p >= L'0' && *p <= L'9') {
			++p; ++i;
			while (p < q) {
				if (*p < L'0' || *p > L'9') {
					if (*p == L'.') {
						if (f == 1) break;  // 指数部に入っている
						++d;
						if (d > 1) {
							if (*(p - 1) == L'.') break;  // "." が連続なら中断
						}
					}else if (*p == L'E' || *p == L'e') {
						if (f == 1) break;  // 指数部に入っている
						if (p + 2 < q) {
							if (1
								&& (*(p + 1) == L'+' || *(p + 1) == L'-')
								&& (*(p + 2) >= L'0' && *(p + 2) <= L'9')
							) {
								++p; ++i;
								++p; ++i;
								f = 1;
							}else if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
								++p; ++i;
								f = 1;
							}else {
								break;
							}
						}else if (p + 1 < q) {
							if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
								++p; ++i;
								f = 1;
							}else {
								break;
							}
						}else {
							break;
						}
					}else {
						break;
					}
				}
				++p; ++i;
			}
			if (*(p - 1)  == L'.') return i - 1;  // 最後が "." なら含めない
			// 接尾語
			if (p < q) {
				if (0
					|| ((d == 0) && (*p == L'L' || *p == L'l'))
					|| *p == L'F'
					|| *p == L'f'
				) {
					++p; ++i;
				}
			}
			return i;
		}else if (*p == L'.') {
			while (p < q) {
				if (*p < L'0' || *p > L'9') {
					if (*p == L'.') {
						if (f == 1) break;  // 指数部に入っている
						++d;
						if (d > 1) {
							if (*(p - 1) == L'.') break;  // "." が連続なら中断
						}
					}else if (*p == L'E' || *p == L'e') {
						if (f == 1) break;  // 指数部に入っている
						if (p + 2 < q) {
							if (1
								&& (*(p + 1) == L'+' || *(p + 1) == L'-')
								&& (*(p + 2) >= L'0' && *(p + 2) <= L'9')
							) {
								++p; ++i;
								++p; ++i;
								f = 1;
							}else if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
								++p; ++i;
								f = 1;
							}else {
								break;
							}
						}else if (p + 1 < q) {
							if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
								++p; ++i;
								f = 1;
							}else {
								break;
							}
						}else {
							break;
						}
					}else {
						break;
					}
				}
				++p; ++i;
			}
			if (*(p - 1)  == L'.') return i - 1;  // 最後が "." なら含めない
			// 接尾語
			if (p < q) {
				if (*p == L'F' || *p == L'f') {
					++p; ++i;
				}
			}
			return i;
		}else if (*p == L'E' || *p == L'e') {
			++p; ++i;
			while (p < q) {
				if (*p < L'0' || *p > L'9') {
					if ((*p == L'+' || *p == L'-') && (*(p - 1) == L'E' || *(p - 1) == L'e')) {
						if (p + 1 < q) {
							if (*(p + 1) < L'0' || *(p + 1) > L'9') {
								// "0E+", "0E-"
								break;
							}
						}else {
							// "0E-", "0E+"
							break;
						}
					}else {
						break;
					}
				}
				++p; ++i;
			}
			if (i == 2) return 1;  // "0E", 0e" なら "0" が数値
			// 接尾語
			if (p < q) {
				if (0
					|| ((d == 0) && (*p == L'L' || *p == L'l'))
					|| *p == L'F' || *p == L'f'
				) {
					++p; ++i;
				}
			}
			return i;
		}else {
			// "0" だけが数値
			//if (*p == L'.') return i - 1;  // 最後が "." なら含めない
			if (p < q) {
				if (0
					|| ((d == 0) && (*p == L'L' || *p == L'l'))
					|| *p == L'F' || *p == L'f'
				) {
					++p; ++i;
				}
			}
			return i;
		}
	}else if (*p >= L'1' && *p <= L'9') { // 10進数
		++p; ++i;
		while (p < q) {
			if (*p < L'0' || *p > L'9') {
				if (*p == L'.') {
					if (f == 1) break;  // 指数部に入っている
					++d;
					if (d > 1) {
						if (*(p - 1) == L'.') break;  // "." が連続なら中断
					}
				}else if (*p == L'E' || *p == L'e') {
					if (f == 1) break;  // 指数部に入っている
					if (p + 2 < q) {
						if (1
							&& (*(p + 1) == L'+' || *(p + 1) == L'-')
							&& (*(p + 2) >= L'0' && *(p + 2) <= L'9')
						) {
							++p; ++i;
							++p; ++i;
							f = 1;
						}else if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
							++p; ++i;
							f = 1;
						}else {
							break;
						}
					}else if (p + 1 < q) {
						if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
							++p; ++i;
							f = 1;
						}else {
							break;
						}
					}else {
						break;
					}
				}else {
					break;
				}
			}
			++p; ++i;
		}
		if (*(p - 1) == L'.') return i - 1;  // 最後が "." なら含めない
		// 接尾語
		if (p < q) {
			if (0
				|| ((d == 0) && (*p == L'L' || *p == L'l'))
				|| *p == L'F'
				|| *p == L'f'
			) {
				++p; ++i;
			}
		}
		return i;
	}else if (*p == L'-') {  // マイナス
		++p; ++i;
		while (p < q) {
			if (*p < L'0' || *p > L'9') {
				if (*p == L'.') {
					if (f == 1) break;  // 指数部に入っている
					++d;
					if (d > 1) {
						if (*(p - 1) == L'.') break;  // "." が連続なら中断
					}
				}else if (*p == L'E' || *p == L'e') {
					if (f == 1) break;  // 指数部に入っている
					if (p + 2 < q) {
						if (1
							&& (*(p + 1) == L'+' || *(p + 1) == L'-')
							&& (*(p + 2) >= L'0' && *(p + 2) <= L'9')
						) {
							++p; ++i;
							++p; ++i;
							f = 1;
						}else if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
							++p; ++i;
							f = 1;
						}else {
							break;
						}
					}else if (p + 1 < q) {
						if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
							++p; ++i;
							f = 1;
						}else {
							break;
						}
					}else {
						break;
					}
				}else {
					break;
				}
			}
			++p; ++i;
		}
		// "-", "-." だけなら数値でない
		if (i == 1) return 0;
		if (*(p - 1) == L'.') {
			--i;
			if (i == 1) return 0;
			return i;
		}
		// 接尾語
		if (p < q) {
			if (0
				|| ((d == 0) && (*p == L'L' || *p == L'l'))
				|| *p == L'F'
				|| *p == L'f'
			) {
				++p; ++i;
			}
		}
		return i;
	}else if (*p == L'.') {  // 小数点
		++d;
		++p; ++i;
		while (p < q) {
			if (*p < L'0' || *p > L'9') {
				if (*p == L'.') {
					if (f == 1) break;  // 指数部に入っている
					++d;
					if (d > 1) {
						if (*(p - 1) == L'.') break;  // "." が連続なら中断
					}
				}else if (*p == L'E' || *p == L'e') {
					if (f == 1) break;  // 指数部に入っている
					if (p + 2 < q) {
						if (1
							&& (*(p + 1) == L'+' || *(p + 1) == L'-')
							&& (*(p + 2) >= L'0' && *(p + 2) <= L'9')
						) {
							++p; ++i;
							++p; ++i;
							f = 1;
						}else if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
							++p; ++i;
							f = 1;
						}else {
							break;
						}
					}else if (p + 1 < q) {
						if (*(p + 1) >= L'0' && *(p + 1) <= L'9') {
							++p; ++i;
							f = 1;
						}else {
							break;
						}
					}else {
						break;
					}
				}else {
					break;
				}
			}
			++p; ++i;
		}
		// "." だけなら数値でない
		if (i == 1) return 0;
		if (*(p - 1)  == L'.') return i - 1;  // 最後が "." なら含めない
		// 接尾語
		if (p < q) {
			if (*p == L'F' || *p == L'f') {
				++p; ++i;
			}
		}
		return i;
	}

	// 数値ではない
	return 0;
}

