#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Numeric.h"
#include "parse/WordParse.h"
#include "util/string_ex2.h"
#include "doc/layout/Layout.h"
#include "types/TypeSupport.h"

static int IsNumber(const StringRef& str, int offset);	// ���l�Ȃ炻�̒�����Ԃ�

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ���p���l                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_Numeric::BeginColor(const StringRef& str, size_t nPos)
{
	if (!str.IsValid()) return false;

	int	nnn;

	if (1
		&& _IsPosKeywordHead(str, nPos)
		&& (nnn = IsNumber(str, nPos)) > 0
	) {		// ���p������\������
		// �L�[���[�h������̏I�[���Z�b�g����
		this->nCommentEnd = nPos + nnn;
		return true;	// ���p���l�ł��� // 2002/03/13 novice
	}
	return false;
}


bool Color_Numeric::EndColor(const StringRef& str, size_t nPos)
{
	return (nPos == this->nCommentEnd);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
 * ���l�Ȃ璷����Ԃ��B
 * 10�i���̐����܂��͏����B16�i��(����)�B
 * ������   ���l(�F����)
 * ---------------------
 * 123      123
 * 0123     0123
 * 0xfedc   0xfedc
 * -123     -123
 * &H9a     &H9a     (�������\�[�X����#if��L���ɂ����Ƃ�)
 * -0x89a   0x89a
 * 0.5      0.5
 * 0.56.1   0.56 , 1 (�������\�[�X����#if��L���ɂ�����"0.56.1"�ɂȂ�)
 * .5       5        (�������\�[�X����#if��L���ɂ�����".5"�ɂȂ�)
 * -.5      5        (�������\�[�X����#if��L���ɂ�����"-.5"�ɂȂ�)
 * 123.     123
 * 0x567.8  0x567 , 8
 */
/*
 * ���p���l
 *   1, 1.2, 1.2.3, .1, 0xabc, 1L, 1F, 1.2f, 0x1L, 0x2F, -.1, -1, 1e2, 1.2e+3, 1.2e-3, -1e0
 *   10�i��, 16�i��, LF�ڔ���, ���������_��, ������
 *   IP�A�h���X�̃h�b�g�A��(�{���͐��l����Ȃ��񂾂��)
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

	if (*p == L'0') {  // 10�i��,C��16�i��
		++p; ++i;
		if ((p < q) && (*p == L'x')) {  // C��16�i��
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
			// "0x" �Ȃ� "0" ���������l
			if (i == 2) return 1;
			
			// �ڔ���
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
						if (f == 1) break;  // �w�����ɓ����Ă���
						++d;
						if (d > 1) {
							if (*(p - 1) == L'.') break;  // "." ���A���Ȃ璆�f
						}
					}else if (*p == L'E' || *p == L'e') {
						if (f == 1) break;  // �w�����ɓ����Ă���
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
			if (*(p - 1)  == L'.') return i - 1;  // �Ōオ "." �Ȃ�܂߂Ȃ�
			// �ڔ���
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
						if (f == 1) break;  // �w�����ɓ����Ă���
						++d;
						if (d > 1) {
							if (*(p - 1) == L'.') break;  // "." ���A���Ȃ璆�f
						}
					}else if (*p == L'E' || *p == L'e') {
						if (f == 1) break;  // �w�����ɓ����Ă���
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
			if (*(p - 1)  == L'.') return i - 1;  // �Ōオ "." �Ȃ�܂߂Ȃ�
			// �ڔ���
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
			if (i == 2) return 1;  // "0E", 0e" �Ȃ� "0" �����l
			// �ڔ���
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
			// "0" ���������l
			//if (*p == L'.') return i - 1;  // �Ōオ "." �Ȃ�܂߂Ȃ�
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
	}else if (*p >= L'1' && *p <= L'9') { // 10�i��
		++p; ++i;
		while (p < q) {
			if (*p < L'0' || *p > L'9') {
				if (*p == L'.') {
					if (f == 1) break;  // �w�����ɓ����Ă���
					++d;
					if (d > 1) {
						if (*(p - 1) == L'.') break;  // "." ���A���Ȃ璆�f
					}
				}else if (*p == L'E' || *p == L'e') {
					if (f == 1) break;  // �w�����ɓ����Ă���
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
		if (*(p - 1) == L'.') return i - 1;  // �Ōオ "." �Ȃ�܂߂Ȃ�
		// �ڔ���
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
	}else if (*p == L'-') {  // �}�C�i�X
		++p; ++i;
		while (p < q) {
			if (*p < L'0' || *p > L'9') {
				if (*p == L'.') {
					if (f == 1) break;  // �w�����ɓ����Ă���
					++d;
					if (d > 1) {
						if (*(p - 1) == L'.') break;  // "." ���A���Ȃ璆�f
					}
				}else if (*p == L'E' || *p == L'e') {
					if (f == 1) break;  // �w�����ɓ����Ă���
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
		// "-", "-." �����Ȃ琔�l�łȂ�
		if (i == 1) return 0;
		if (*(p - 1) == L'.') {
			--i;
			if (i == 1) return 0;
			return i;
		}
		// �ڔ���
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
	}else if (*p == L'.') {  // �����_
		++d;
		++p; ++i;
		while (p < q) {
			if (*p < L'0' || *p > L'9') {
				if (*p == L'.') {
					if (f == 1) break;  // �w�����ɓ����Ă���
					++d;
					if (d > 1) {
						if (*(p - 1) == L'.') break;  // "." ���A���Ȃ璆�f
					}
				}else if (*p == L'E' || *p == L'e') {
					if (f == 1) break;  // �w�����ɓ����Ă���
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
		// "." �����Ȃ琔�l�łȂ�
		if (i == 1) return 0;
		if (*(p - 1)  == L'.') return i - 1;  // �Ōオ "." �Ȃ�܂߂Ȃ�
		// �ڔ���
		if (p < q) {
			if (*p == L'F' || *p == L'f') {
				++p; ++i;
			}
		}
		return i;
	}

	// ���l�ł͂Ȃ�
	return 0;
}

