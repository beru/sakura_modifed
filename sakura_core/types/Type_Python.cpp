/*! @file
	@brief Python�A�E�g���C�����

	@author genta
	@date 2007.02.24 �V�K�쐬
*/
/*
	Copyright (C) 2007, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.

*/
#include "StdAfx.h"
#include "doc/EditDoc.h"
#include "doc/DocOutline.h"
#include "doc/logic/DocLine.h"
#include "outline/FuncInfoArr.h"


/*!
	�֐��ɗp���邱�Ƃ��ł��镶�����ǂ����̔���
	
	@date 2007.02.08 genta

	@note �����ɂ�1�����ڂɐ������g�����Ƃ͏o���Ȃ����C
		����͎��s���Ă݂�Ζ��炩�ɂ킩�邱�ƂȂ̂�
		�����܂Ō����Ƀ`�F�b�N���Ȃ�
*/
inline bool Python_IsWordChar(wchar_t c) {
	return (L'_' == c ||
			(L'a' <= c && c <= L'z')||
			(L'A' <= c && c <= L'Z')||
			(L'0' <= c && c <= L'9')
		);
}


/*! python�̃p�[�X��Ԃ��Ǘ�����\����

	��͒��ɉ�͊֐��̊Ԃ������n�����D
	���̃N���X�͌��݂̏�ԂƁC������̐�����ێ�����D
	��͈ʒu�͉�͊֐��ԂŃp�����[�^�Ƃ��ēn�����̂�
	���̒��ł͕ێ����Ȃ��D

	[��ԑJ��]
	�J�n : STATE_NORMAL

	STATE_NORMAL/STATE_CONTINUE��STATE_CONTINUE�̑J��
	- �p���s�}�[�N�L��

	STATE_NORMAL/STATE_CONTINUE��STATE_NORMAL�̑J��
	- �p���s�}�[�N���Ȃ��s���ɒB����
	- �R�����g�ɒB����

	STATE_NORMAL��STATE_STRING�̑J��
	- ���p������

	STATE_STRING��STATE_NORMAL�̑J��
	- �K��̕�����I���L��
	- short string�ŕ�����̏I�����������p�����p���s�}�[�N���Ȃ��s���ɒB����

	@date 2007.02.12 genta
*/
struct OutlinePython {
	enum {
		STATE_NORMAL,	// �ʏ�s : �s�����܂�
		STATE_STRING,	// ������
		STATE_CONTINUE,	// �p���s : �O�̍s����̑����Ȃ̂ōs���Ƃ݂͂Ȃ���Ȃ�
	} state;
	
	int quote_char;	// ���p���L��
	bool raw_string;	// �G�X�P�[�v�L�������Ȃ�true
	bool long_string;	// ���������񒆂Ȃ�true

	OutlinePython();

	/*	�e��Ԃɂ����镶����X�L�������s��
		Scan*���Ăт������Ƃ��͊��ɂ��̏�ԂɂȂ��Ă��邱�Ƃ��O��D
		�����Ԃ���ʂ̏�ԂɈڂ�Ƃ���܂ł������D
		�ʂ̏�ԂɈڂ锻�肪��₱�����΂����́CEnter*�Ƃ��Ċ֐��ɂ���D
	*/	
	int ScanNormal(const wchar_t* data, int linelen, int start_offset);
	int ScanString(const wchar_t* data, int linelen, int start_offset);
	int EnterString(const wchar_t* data, int linelen, int start_offset);
	void DoScanLine(const wchar_t* data, int linelen, int start_offset);
	
	bool IsLogicalLineTop(void) const { return state == STATE_NORMAL; }
};

/*!�R���X�g���N�^: ������

	������Ԃ�STATE_NORMAL�ɐݒ肷��D
*/
OutlinePython::OutlinePython()
	:
	state(STATE_NORMAL),
	raw_string(false),
	long_string(false)
{
}

/*! @brief Python������̓�����ŕ������ʂ����肷��

	������̎�ނ�K�؂ɔ��ʂ��C������Ԃ�ݒ肷��D
	start_offset�͊J�n���p�����w���Ă��邱�ƁD

	- ���p��1��: short string
	- ���p��3��: long string
	- ���p���̑O��r��R���� : raw string

	@param[in] data �Ώە�����
	@param[in] linelen �f�[�^�̒���
	@param[in] start_offset �����J�n�ʒu
	
	@return ������̈ʒu

	@invariant
		state != STATE_STRING

	@note ���p���̈ʒu�ŌĂт����΁C��������͕K��STATE_STRING�ɂȂ��Ă���͂��D
		���p���ȊO�̈ʒu�ŌĂт������ꍇ�͉������Ȃ��Ŕ�����D
*/
int OutlinePython::EnterString(const wchar_t* data, int linelen, int start_offset)
{
	assert(state != STATE_STRING);

	int col = start_offset;
	//	������J�n�`�F�b�N
	if (data[col] == '\"' || data[col] == '\'') {
		int quote_char = data[col];
		state = STATE_STRING;
		this->quote_char = quote_char;
		//	������̊J�n
		if (col >= 1 &&
			(data[col - 1] == 'r' || data[col - 1] == 'R')
		) {
			//	�����ɂ͒��O��SHIFT_JIS��2�o�C�g�ڂ��ƌ딻�肷��\�������邪
			//	�������������Ȃ��R�[�h�͑���ɂ��Ȃ�
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

/*! @brief Python�v���O�����̏���

	�v���O�����{�̕����̏����D������̊J�n�C�p���s�C�R�����g�C�ʏ�s�����`�F�b�N����D
	�s�����肪�I�������ň����n�����̂ŁC�֐��E�N���X��`�͍l�����Ȃ��ėǂ��D
	
	�ȉ��̏ꍇ�ɏ������I������
	- �s��: STATE_NORMAL�Ƃ��ď����I��
	- �R�����g: STATE_NORMAL�Ƃ��ď����I��
	- ������̊J�n: EnterString() �ɂĕ������ʂ̔�����s������STATE_STRING�Ƃ��ď����I��
	- �p���s: STATE_CONTINUE�Ƃ��ď����I��

	@param[in] data �Ώە�����
	@param[in] linelen �f�[�^�̒���
	@param[in] start_offset �����J�n�ʒu
	
	@invaliant
		state == STATE_NORMAL || state == STATE_CONTINUE
	
	@return ������̈ʒu
*/
int OutlinePython::ScanNormal(const wchar_t* data, int linelen, int start_offset)
{
	assert(state == STATE_NORMAL || state == STATE_CONTINUE);
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	for (int col=start_offset; col<linelen; ++col) {
		int nCharChars = NativeW::GetSizeOfChar(data, linelen, col);
		if (1 < nCharChars) {
			col += (nCharChars - 1);
			continue;
		}
		//	�R�����g
		if (data[col] == '#') {
			//	�R�����g�͍s���Ɠ��������Ȃ̂�
			//	�킴�킴�Ɨ����Ĉ����K�v��������
			//	�����ŕЂ�t���Ă��܂���
			state = STATE_NORMAL;
			break;
		//	������
		}else if (data[col] == '\"' || data[col] == '\'') {
			return EnterString(data, linelen, col);
		//	�p���s��������Ȃ�
		}else if (data[col] == '\\') {
			//	CR��CRLF��LF�ōs��
			//	�ŏI�s�ɂ͉��s�R�[�h���Ȃ����Ƃ����邪�C����ȍ~�ɂ͉����Ȃ��̂ŉe�����Ȃ�
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


/*! @brief python������(1�s)�𒲍�����

	�^����ꂽ��Ԃ���Python������̏�ԕω���ǂ��C
	�ŏI�I�ȏ�Ԃ����肷��D
	
	������̊J�n�����EnterString()�֐��ŏ����ς݂ł���C���̌��ʂ�
	state, raw_string, long_string, m_quote_char�ɗ^�����Ă���D
	
	raw_string��true�Ȃ�backslash�ɂ��G�X�P�[�v�������s��Ȃ�
	long_string�Ȃ�quote_char��3�����܂ŕ�����ƂȂ�D

	@param[in] data �Ώە�����
	@param[in] linelen �f�[�^�̒���
	@param[in] start_offset �����J�n�ʒu
	
	@return ������̈ʒu
	
	@invariant
		state==STATE_STRING

	@author genta
	@date 2007.02.12 �V�K�쐬
	@date 2007.03.23 genta ������̌p���s�̏�����ǉ�

*/
int OutlinePython::ScanString(const wchar_t* data, int linelen, int start_offset)
{
	assert(state == STATE_STRING);
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	for (int col=start_offset; col<linelen; ++col) {
		int nCharChars = NativeW::GetSizeOfChar(data, linelen, col);
		if (1 < nCharChars) {
			col += (nCharChars - 1);
			continue;
		}
		//	raw���[�h�ȊO�ł̓G�X�P�[�v���`�F�b�N
		//	raw���[�h�ł��p���s�̓`�F�b�N
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
				// \r\n���܂Ƃ߂�\n�Ƃ��Ĉ����K�v������
				if (col + 1 >= linelen ||
					data[col + 2] == key
				) {
					// �{���ɍs��
					++col;
					continue;
				}else if (data[col + 2] == WCODE::LF) {
					col += 2;	//	 CRLF
				}
			}
		//	short string + ���s�̏ꍇ�̓G���[���狭�����A
		}else if (WCODE::IsLineDelimiter(data[col], bExtEol)) {
			// ���Ƃ�
			if (!long_string) {
				//	������̖����𔭌�����
				state = STATE_NORMAL;
				return col + 1;
			}
		//	���p��������������I���`�F�b�N
		}else if (data[col] == quote_char) {
			if (!long_string) {
				//	������̖����𔭌�����
				state = STATE_NORMAL;
				return col + 1;
			}
			//	long string�̏ꍇ
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

/*!	Python��������s���܂ŃX�L�������Ď��̍s�̏�Ԃ����肷��

	state�ɐݒ肳�ꂽ���݂̏�Ԃ���J�n����data��start_offset����linelen�ɒB����܂�
	�������C�s���ɂ������Ԃ�state�Ɋi�[����D

	���݂̏�Ԃɉ����ăT�u���[�`���ɉ�͏������˗�����D
	�T�u���[�`��Scan**�ł͕�����data��start_offset�����ԑJ�ڂ���������܂ŏ�����
	�����C�ʂ̏�ԂɑJ�ڂ�������ɏ����ς݂̌��ʒu��Ԃ��ďI������D

	���̊֐��ɖ߂�����͍ēx���݂̏�Ԃɉ����ď����˗����s���D������s���ɒB����܂ŌJ��Ԃ��D

	@param[in] data �Ώە�����
	@param[in] linelen �f�[�^�̒���
	@param[in] start_offset �����J�n�ʒu

*/
void OutlinePython::DoScanLine(const wchar_t* data, int linelen, int start_offset)
{
	int col = start_offset;
	while (col < linelen) {
		if (state == STATE_NORMAL || state == STATE_CONTINUE) {
			col = ScanNormal(data, linelen, col);
		}else if (state == STATE_STRING) {
			col = ScanString(data, linelen, col);
		}else {
			//	���肦�Ȃ��G���[
			return;
		}
	}
}


/*!	@brief python�֐����X�g�쐬

	class, def �Ŏn�܂�s���疼�O�𔲂��o���D
	
	class CLASS_NAME(superclass):
	def FUNCTION_NAME(parameters):

	������ƃR�����g�����O����K�v������D

	�ʏ�̍s���̏ꍇ�Ɋ֐��E�N���X����Ɠo�^�������s���D
	Python���L�̋󔒂̐��𐔂��ăl�X�g���x���𔻒肷��D
	indent_level��z��Ƃ��ėp���Ă���C�C���f���g���x�����Ƃ̃X�y�[�X�����i�[����D
	�Ȃ��CTAB��8����؂�ւ̈ړ��Ɖ��߂��邱�ƂɂȂ��Ă���D
	
	�ʏ�̍s���łȂ�(�����񒆂���ьp���s)�C���邢�͍s���̏����������
	��ԋ@�B python_analyze_state �ɔ��菈�����˗�����D

	@par ������
	'' "" ����OK
	���p��3�A���Ń����O������
	���O��r��R����������G�X�P�[�v�L���𖳎�(���������s�̃G�X�P�[�v�͗L��)
	
	@par �R�����g
	#�Ŏn�܂�C�s�̌p���͂Ȃ��D

	@date 2007.02.08 genta �V�K�쐬
*/
void DocOutline::MakeFuncList_python(FuncInfoArr* pFuncInfoArr)
{
	LogicInt	nLineLen;
	LogicInt	nLineCount;

	OutlinePython python_analyze_state;

	const int MAX_DEPTH = 10;
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;

	int indent_level[MAX_DEPTH]; // �e���x���̃C���f���g���ʒu()
	indent_level[0] = 0;	// do as python does.
	int depth_index = 0;

	for (nLineCount=LogicInt(0); nLineCount<doc.docLineMgr.GetLineCount(); ++nLineCount) {
		const wchar_t*	pLine;
		int depth;	//	indent depth
		LogicInt col = LogicInt(0);	//	current working column position

		pLine = doc.docLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		
		if (python_analyze_state.IsLogicalLineTop()) {
			//	indent check
			//	May 15, 2007 genta ���ʒucol�̓f�[�^�I�t�Z�b�gd�ƓƗ��ɂ��Ȃ���
			//	�������r�����������Ȃ�
			for (depth=0, col=LogicInt(0); col<nLineLen; ++col) {
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
				nLineLen - col > LogicInt(3 + 1)
				&& wcsncmp_literal(pLine + col, L"def") == 0
			) {
				//	"def"
				nItemFuncId = 1;
				col += LogicInt(3); // strlen(def)
			}else if (
				nLineLen - col > LogicInt(5 + 1)
				&& wcsncmp_literal(pLine + col, L"class") == 0
			) {
				// class
				nItemFuncId = 4;
				col += LogicInt(5); // strlen(class)
			}else {
				python_analyze_state.DoScanLine(pLine, nLineLen, col);
				continue;
			}

			//	2006.02.28 genta ��؂�`�F�b�N
			//	define, classic�����ΏۂɂȂ�Ȃ��悤�ɁC���ɃX�y�[�X���^�u��
			//	�������Ƃ��m�F�D
			//	�{���͌p���s�Ƃ��Ď��̍s�Ɋ֐������������Ƃ����@��͉\����
			//	���G�ɂȂ�̂őΉ����Ȃ��D
			int c = pLine[col];
			if (c != L' ' && c != L'\t') {
				python_analyze_state.DoScanLine(pLine, nLineLen, col);
				continue;
			}

			//	adjust current depth level
			//	�֐������̎��s���̂���ɚ��Ɋ֐�������P�[�X���l��
			//	def/class�ȊO�̃C���f���g�͋L�^���Ȃ����������̂�
			//	���o���s�Ɗm�肵�Ă���C���f���g���x���̔�����s��
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

			//	2007.02.08 genta �蔲���R�����g
			//	�����ɂ́C�����Ōp���s�����邱�Ƃ��\�����C
			//	����ȃ��A�ȃP�[�X�͍l�����Ȃ�
			
			//	skip whitespace
			while (col < nLineLen && C_IsSpace(pLine[col], bExtEol))
				++col;

			int w_end;
			for (w_end=col;
				w_end<nLineLen && Python_IsWordChar(pLine[w_end]);
				++w_end)
				;
			
			//	2007.02.08 genta �蔲���R�����g
			//	�����ɂ́C���̌�Ɋ��ʂɈ͂܂ꂽ�������邢�͌p�����N���X��
			//	���ʂɈ͂܂�ē���C�����:�Ƒ�����
			//	�p���s�̉\��������̂ŁC�����܂Ń`�F�b�N���Ȃ�
			
			//	�����܂łœo�^�v��OK�Ƃ݂Ȃ�
			
			//	���̂�����͎b��

			wchar_t szWord[512];	// �K���ɑ傫�Ȑ�(python�ł͖��O�̒����̏��������̂��H)
			int len = w_end - col;
			
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
					//	��������ē����
					len = _countof(szWord) - 8;
				}
				// class
				wcscpy(szWord + len, LSW(STR_OUTLINE_PYTHON_CLASS));
			}
			
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			LayoutPoint ptPosXY;
			doc.layoutMgr.LogicToLayout(
				LogicPoint(LogicInt(0), nLineCount),
				&ptPosXY
			);
			pFuncInfoArr->AppendData(
				nLineCount + LogicInt(1),
				ptPosXY.GetY2() + LayoutInt(1),
				szWord,
				nItemFuncId,
				depth_index
			);
			col = LogicInt(w_end); // �N���X�E�֐���`�̑����͂�������
		}
		python_analyze_state.DoScanLine(pLine, nLineLen, col);
	}
}

