#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���O                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �A�v�����B2007.09.21 kobake ����
	#define _APP_NAME_(TYPE) TYPE("sakura")

#ifdef _DEBUG
	#define _APP_NAME_2_(TYPE) TYPE("(�f�o�b�O��)")
#else
	#define _APP_NAME_2_(TYPE) TYPE("")
#endif

#define _GSTR_APPNAME_(TYPE)  _APP_NAME_(TYPE) _APP_NAME_2_(TYPE) // ��:UNICODE�f�o�b�O��_T("sakura(�f�o�b�O��)")

#define GSTR_APPNAME    (_GSTR_APPNAME_(_T)   )
#define GSTR_APPNAME_A  (_GSTR_APPNAME_(ATEXT))
#define GSTR_APPNAME_W  (_GSTR_APPNAME_(LTEXT))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �e�L�X�g�G���A                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Feb. 18, 2003 genta �ő�l�̒萔���ƒl�ύX
const size_t LINESPACE_MAX = 128;
const size_t COLUMNSPACE_MAX = 64;

// Aug. 14, 2005 genta �萔��`�ǉ�
// 2007.09.07 kobake �萔���ύX: MAXLINESIZE��MAXLINEKETAS
// 2007.09.07 kobake �萔���ύX: MINLINESIZE��MINLINEKETAS
const size_t MAXLINEKETAS		= 10240;	// 1�s�̌����̍ő�l
const size_t MINLINEKETAS		= 10;		// 1�s�̌����̍ŏ��l

// 2014.08.02 �萔��`�ǉ� katze
const size_t LINENUMWIDTH_MIN = 2;
const size_t LINENUMWIDTH_MAX = 11;

