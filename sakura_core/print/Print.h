/*!	@file
	@brief ����֘A

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, �����

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

#pragma once

#include <WinSpool.h>
#include <CommDlg.h> // PRINTDLG

struct MYDEVMODE {
	BOOL	bPrinterNotFound;	// �v�����^���Ȃ������t���O
	TCHAR	szPrinterDriverName[_MAX_PATH + 1];	// �v�����^�h���C�o��
	TCHAR	szPrinterDeviceName[_MAX_PATH + 1];	// �v�����^�f�o�C�X��
	TCHAR	szPrinterOutputName[_MAX_PATH + 1];	// �v�����^�|�[�g��
	DWORD	dmFields;
	short	dmOrientation;
	short	dmPaperSize;
	short	dmPaperLength;
	short	dmPaperWidth;
	short	dmScale;
	short	dmCopies;
	short	dmDefaultSource;
	short	dmPrintQuality;
	short	dmColor;
	short	dmDuplex;
	short	dmYResolution;
	short	dmTTOption;
	short	dmCollate;
	BCHAR	dmFormName[CCHFORMNAME];
	WORD	dmLogPixels;
	DWORD	dmBitsPerPel;
	DWORD	dmPelsWidth;
	DWORD	dmPelsHeight;
	DWORD	dmDisplayFlags;
	DWORD	dmDisplayFrequency;
};

// 2006.08.14 Moca �p�����̓��� PaperInfo�V��
// �p�����
struct PaperInfo {
	int				nId;			// �p��ID
	short			nAllWidth;		// �� (0.1mm�P��)
	short			nAllHeight;		// ���� (0.1mm�P��)
	const TCHAR*	pszName;		// �p������
};

struct PrintSetting;

// ����ݒ�
#define POS_LEFT	0
#define POS_CENTER	1
#define POS_RIGHT	2
#define HEADER_MAX	100
#define FOOTER_MAX	HEADER_MAX
struct PrintSetting {
	TCHAR		szPrintSettingName[32 + 1];			// ����ݒ�̖��O
	TCHAR		szPrintFontFaceHan[LF_FACESIZE];	// ����t�H���g
	TCHAR		szPrintFontFaceZen[LF_FACESIZE];	// ����t�H���g
	int			nPrintFontWidth;					// ����t�H���g��(1/10mm�P�ʒP��)
	int			nPrintFontHeight;					// ����t�H���g����(1/10mm�P�ʒP��)
	int			nPrintDansuu;						// �i�g�̒i��
	int			nPrintDanSpace;						// �i�ƒi�̌���(1/10mm�P��)
	int			nPrintLineSpacing;					// ����t�H���g�s�� �����̍����ɑ΂��銄��(%)
	int			nPrintMarginTY;						// ����p���}�[�W�� ��(mm�P��)
	int			nPrintMarginBY;						// ����p���}�[�W�� ��(mm�P��)
	int			nPrintMarginLX;						// ����p���}�[�W�� ��(mm�P��)
	int			nPrintMarginRX;						// ����p���}�[�W�� �E(mm�P��)
	short		nPrintPaperOrientation;				// �p������ DMORIENT_PORTRAIT (1) �܂��� DMORIENT_LANDSCAPE (2)
	short		nPrintPaperSize;					// �p���T�C�Y
	bool		bColorPrint;						// �J���[���			// 2013/4/26 Uchi
	bool		bPrintWordWrap;						// �p�����[�h���b�v����
	bool		bPrintKinsokuHead;					// �s���֑�����		//@@@ 2002.04.09 MIK
	bool		bPrintKinsokuTail;					// �s���֑�����		//@@@ 2002.04.09 MIK
	bool		bPrintKinsokuRet;					// ���s�����̂Ԃ牺��	//@@@ 2002.04.13 MIK
	bool		bPrintKinsokuKuto;					// ��Ǔ_�̂Ԃ炳��	//@@@ 2002.04.17 MIK
	bool		bPrintLineNumber;					// �s�ԍ����������

	MYDEVMODE	mdmDevMode;							// �v�����^�ݒ� DEVMODE�p
	BOOL		bHeaderUse[3];						// �w�b�_���g���Ă��邩�H
	EDIT_CHAR	szHeaderForm[3][HEADER_MAX];		// 0:���񂹃w�b�_�B1:�����񂹃w�b�_�B2:�E�񂹃w�b�_�B
	BOOL		bFooterUse[3];						// �t�b�^���g���Ă��邩�H
	EDIT_CHAR	szFooterForm[3][FOOTER_MAX];		// 0:���񂹃t�b�^�B1:�����񂹃t�b�^�B2:�E�񂹃t�b�^�B

	// �w�b�_/�t�b�^�̃t�H���g(lfFaceName���ݒ肳��Ă��Ȃ���Δ��p/�S�p�t�H���g���g�p)
	LOGFONT		lfHeader;							// �w�b�_�t�H���g�pLOGFONT�\����
	int 		nHeaderPointSize;					// �w�b�_�t�H���g�|�C���g�T�C�Y
	LOGFONT		lfFooter;							// �t�b�^�t�H���g�pLOGFONT�\����
	int 		nFooterPointSize;					// �t�b�^�t�H���g�|�C���g�T�C�Y
};


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief ����֘A�@�\

	�I�u�W�F�N�g�w���łȂ��N���X
*/
class Print {
public:
	static const PaperInfo paperInfoArr[];	// �p�����ꗗ
	static const int nPaperInfoArrNum; // �p�����ꗗ�̗v�f��

	/*
	||	static�֐��Q
	*/
	static void SettingInitialize(PrintSetting&, const TCHAR* settingName);

	static TCHAR* GetPaperName(int, TCHAR*);	// �p���̖��O���擾
	// �p���̕��A����
	static BOOL GetPaperSize(
		short*		pnPaperAllWidth,
		short*		pnPaperAllHeight,
		MYDEVMODE*	pDEVMODE
	);
	// �󎚉\���E�s�̌v�Z
	static int CalculatePrintableColumns(PrintSetting&, int width, int nLineNumberColumns);
	static int CalculatePrintableLines(PrintSetting&, int height);

	// �w�b�_�E�t�b�^�̍����v�Z
	static int CalcHeaderHeight(PrintSetting&);
	static int CalcFooterHeight(PrintSetting&);
public:
	/*
	||  Constructors
	*/
	Print();
	~Print();

	/*
	||  Attributes & Operations
	*/
	BOOL GetDefaultPrinter(MYDEVMODE* pMYDEVMODE);		// �f�t�H���g�̃v�����^�����擾
	BOOL PrintDlg(PRINTDLG* pd, MYDEVMODE* pMYDEVMODE);	// �v�����^�����擾
	// ���/Preview�ɕK�v�ȏ����擾
	BOOL GetPrintMetrics(
		MYDEVMODE*	pMYDEVMODE,
		short*		pnPaperAllWidth,	// �p����
		short*		pnPaperAllHeight,	// �p������
		short*		pnPaperWidth,		// �p������\��
		short*		pnPaperHeight,		// �p������\����
		short*		pnPaperOffsetLeft,	// �p���]�����[
		short*		pnPaperOffsetTop,	// �p���]����[
		TCHAR*		pszErrMsg			// �G���[���b�Z�[�W�i�[�ꏊ
	);

	// ��� �W���u�J�n
	BOOL PrintOpen(
		TCHAR*		pszJobName,
		MYDEVMODE*	pMYDEVMODE,
		HDC*		phdc,
		TCHAR*		pszErrMsg		// �G���[���b�Z�[�W�i�[�ꏊ
	);
	void PrintStartPage(HDC);		// ��� �y�[�W�J�n
	void PrintEndPage(HDC);		// ��� �y�[�W�I��
	void PrintClose(HDC);			// ��� �W���u�I�� // 2003.05.02 ����� �s�v��hPrinter�폜

protected:
	/*
	||  �����w���p�֐�
	*/
	// DC�쐬����(�������܂Ƃ߂�) 2003.05.02 �����
	HDC CreateDC(MYDEVMODE* pMYDEVMODE, TCHAR* pszErrMsg);
	
	static const PaperInfo* FindPaperInfo(int id);
private:
	/*
	||  �����o�ϐ�
	*/
	HGLOBAL	hDevMode;							// ���݃v�����^��DEVMODE�ւ̃������n���h��
	HGLOBAL	hDevNames;						// ���݃v�����^��DEVNAMES�ւ̃������n���h��
};

