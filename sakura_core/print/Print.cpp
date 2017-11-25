/*!	@file
	@brief ���
*/
#include "StdAfx.h"
#include <stdlib.h>
#include <WinSpool.h>
#include "Print.h"
#include "_main/global.h"


const PaperInfo Print::paperInfoArr[] = {
	// 	�p��ID, ��
	{DMPAPER_A4,                  2100,  2970, _T("A4 (210 x 297 mm)")},
	{DMPAPER_A3,                  2970,  4200, _T("A3 (297 x 420 mm)")},
	{DMPAPER_A4SMALL,             2100,  2970, _T("A4 small(210 x 297 mm)")},
	{DMPAPER_A5,                  1480,  2100, _T("A5 (148 x 210 mm)")},
	{DMPAPER_B4,                  2500,  3540, _T("B4 (250 x 354 mm)")},
	{DMPAPER_B5,                  1820,  2570, _T("B5 (182 x 257 mm)")},
	{DMPAPER_QUARTO,              2150,  2750, _T("Quarto(215 x 275 mm)")},
	{DMPAPER_ENV_DL,              1100,  2200, _T("DL Envelope(110 x 220 mm)")},
	{DMPAPER_ENV_C5,              1620,  2290, _T("C5 Envelope(162 x 229 mm)")},
	{DMPAPER_ENV_C3,              3240,  4580, _T("C3 Envelope(324 x 458 mm)")},
	{DMPAPER_ENV_C4,              2290,  3240, _T("C4 Envelope(229 x 324 mm)")},
	{DMPAPER_ENV_C6,              1140,  1620, _T("C6 Envelope(114 x 162 mm)")},
	{DMPAPER_ENV_C65,             1140,  2290, _T("C65 Envelope(114 x 229 mm)")},
	{DMPAPER_ENV_B4,              2500,  3530, _T("B4 Envelope(250 x 353 mm)")},
	{DMPAPER_ENV_B5,              1760,  2500, _T("B5 Envelope(176 x 250 mm)")},
	{DMPAPER_ENV_B6,              1760,  1250, _T("B6 Envelope(176 x 125 mm)")},
	{DMPAPER_ENV_ITALY,           1100,  2300, _T("Italy Envelope(110 x 230 mm)")},
	{DMPAPER_LETTER,              2159,  2794, _T("Letter (8 1/2 x 11 inch)")},
	{DMPAPER_LEGAL,               2159,  3556, _T("Legal  (8 1/2 x 14 inch)")},
	{DMPAPER_CSHEET,              4318,  5588, _T("C sheet (17 x 22 inch)")},
	{DMPAPER_DSHEET,              5588,  8634, _T("D sheet (22 x 34 inch)")},
	{DMPAPER_ESHEET,              8634, 11176, _T("E sheet (34 x 44 inch)")},
	{DMPAPER_LETTERSMALL,         2159,  2794, _T("Letter Small (8 1/2 x 11 inch)")},
	{DMPAPER_TABLOID,             2794,  4318, _T("Tabloid (11 x 17 inch)")},
	{DMPAPER_LEDGER,              4318,  2794, _T("Ledger  (17 x 11 inch)")},
	{DMPAPER_STATEMENT,           1397,  2159, _T("Statement (5 1/2 x 8 1/2 inch)")},
	{DMPAPER_EXECUTIVE,           1841,  2667, _T("Executive (7 1/4 x 10 1/2 inch)")},
	{DMPAPER_FOLIO,               2159,  3302, _T("Folio (8 1/2 x 13 inch)")},
	{DMPAPER_10X14,               2540,  3556, _T("10x14 inch sheet")},
	{DMPAPER_11X17,               2794,  4318, _T("11x17 inch sheet")},
	{DMPAPER_NOTE,                2159,  2794, _T("Note (8 1/2 x 11 inch)")},
	{DMPAPER_ENV_9,                984,  2254, _T("#9 Envelope  (3 7/8 x 8 7/8 inch)")},
	{DMPAPER_ENV_10,              1047,  2413, _T("#10 Envelope (4 1/8 x 9 1/2 inch)")},
	{DMPAPER_ENV_11,              1143,  2635, _T("#11 Envelope (4 1/2 x 10 3/8 inch)")},
	{DMPAPER_ENV_12,              1206,  2794, _T("#12 Envelope (4 3/4 x 11 inch)")},
	{DMPAPER_ENV_14,              1270,  2921, _T("#14 Envelope (5 x 11 1/2 inch)")},
	{DMPAPER_ENV_MONARCH,          984,  1905, _T("Monarch Envelope (3 7/8 x 7 1/2 inch)")},
	{DMPAPER_ENV_PERSONAL,         920,  1651, _T("6 3/4 Envelope (3 5/8 x 6 1/2 inch)")},
	{DMPAPER_FANFOLD_US,          3778,  2794, _T("US Std Fanfold (14 7/8 x 11 inch)")},
	{DMPAPER_FANFOLD_STD_GERMAN,  2159,  3048, _T("German Std Fanfold   (8 1/2 x 12 inch)")},
	{DMPAPER_FANFOLD_LGL_GERMAN,  2159,  3302, _T("German Legal Fanfold (8 1/2 x 13 inch)")},
};

const size_t Print::nPaperInfoArrNum = _countof(paperInfoArr);



Print::Print(void)
{
	hDevMode	= NULL;
	hDevNames	= NULL;
	return;
}

Print::~Print(void)
{
	// ���������蓖�čς݂Ȃ�΁A�������
	// 2003.05.18 �����
	if (hDevMode) {
		::GlobalFree(hDevMode);
	}
	if (hDevNames) {
		::GlobalFree(hDevNames);
	}
	hDevMode	= NULL;
	hDevNames	= NULL;
	return;
}



/*! @brief �v�����^�_�C�A���O��\�����āA�v�����^��I������
** 
** @param pPD			[i/o]	�v�����^�_�C�A���O�\����
** @param pMYDEVMODE 	[i/o] 	����ݒ�
*/
BOOL Print::PrintDlg(
	PRINTDLG* pPD,
	MYDEVMODE* pMYDEVMODE
	)
{
	// �f�t�H���g�v�����^���I������Ă��Ȃ���΁A�I������
	if (!hDevMode) {
		if (!GetDefaultPrinter(pMYDEVMODE)) {
			return FALSE;
		}
	}

	//
	//  ���݂̃v�����^�ݒ�̕K�v������ύX
	//
	DEVMODE* pDEVMODE = (DEVMODE*)::GlobalLock(hDevMode);
	pDEVMODE->dmOrientation		= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize		= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength		= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth		= pMYDEVMODE->dmPaperWidth;
	// PrintDlg()��ReAlloc����鎖���l���āA�Ăяo���O��Unlock
	::GlobalUnlock(hDevMode);

	// �v�����^�_�C�A���O��\�����āA�v�����^��I��
	pPD->lStructSize = sizeof(*pPD);
	pPD->hDevMode = hDevMode;
	pPD->hDevNames = hDevNames;
	if (!::PrintDlg(pPD)) {
		// �v�����^��ύX���Ȃ�����
		return FALSE;
	}

	hDevMode = pPD->hDevMode;
	hDevNames = pPD->hDevNames;

	pDEVMODE = (DEVMODE*)::GlobalLock(hDevMode);
	// �v�����^�ݒ� DEVNAMES�p
	DEVNAMES* pDEVNAMES = (DEVNAMES*)::GlobalLock(hDevNames);

	// �v�����^�h���C�o��
	_tcscpy_s(
		pMYDEVMODE->szPrinterDriverName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset
	);
	// �v�����^�f�o�C�X��
	_tcscpy_s(
		pMYDEVMODE->szPrinterDeviceName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset
	);
	// �v�����^�|�[�g��
	_tcscpy_s(
		pMYDEVMODE->szPrinterOutputName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset
	);

	// �v�����^���瓾��ꂽ�AdmFields�͕ύX���Ȃ�
	// �v�����^���T�|�[�g���Ȃ�bit���Z�b�g����ƁA�v�����^�h���C�o�ɂ���ẮA�s����ȓ���������ꍇ������
	// pMYDEVMODE�́A�R�s�[������bit�łP�̂��̂����Z�b�g����
	// ���v�����^���瓾��ꂽ dmFields��1�łȂ�Length,Width���ɁA�Ԉ���������������Ă���v�����^�h���C�o�ł́A
	//   �c�E�����������������Ȃ��s��ƂȂ��Ă���(2003.07.03 �����)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation	= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize		= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength	= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth	= pDEVMODE->dmPaperWidth;

	DEBUG_TRACE(_T(" (����/�o��) �f�o�C�X �h���C�o=[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset);
	DEBUG_TRACE(_T(" (����/�o��) �f�o�C�X��=[%ts]\n"),        (TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset);
	DEBUG_TRACE(_T("�����o�̓��f�B�A (�o�̓|�[�g) =[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset);
	DEBUG_TRACE(_T("�f�t�H���g�̃v�����^��=[%d]\n"),          pDEVNAMES->wDefault);

	::GlobalUnlock(hDevMode);
	::GlobalUnlock(hDevNames);
	return TRUE;
}


/*! @brief �f�t�H���g�̃v�����^���擾���AMYDEVMODE �ɐݒ� 
** 
** @param pMYDEVMODE 	[out] 	����ݒ�
*/
BOOL Print::GetDefaultPrinter(MYDEVMODE* pMYDEVMODE)
{
	PRINTDLG	pd;
	// 2009.08.08 ����ŗp���T�C�Y�A���w�肪�����Ȃ����Ή� syat
	//// ���ł� DEVMODE���擾�ς݂Ȃ�A�������Ȃ�
	//if (hDevMode != NULL) {
	//	return TRUE;
	//}

	// DEVMODE���擾�ς݂łȂ��ꍇ�A�擾����
	if (!hDevMode) {
		//
		// PRINTDLG�\���̂�����������i�_�C�A���O�͕\�����Ȃ��悤�Ɂj
		// PrintDlg()�Ńf�t�H���g�v�����^�̃f�o�C�X���Ȃǂ��擾����
		//
		memset_raw (&pd, 0, sizeof(pd));
		pd.lStructSize	= sizeof(pd);
		pd.Flags		= PD_RETURNDEFAULT;
		if (!::PrintDlg(&pd)) {
			pMYDEVMODE->bPrinterNotFound = TRUE;	// �v�����^���Ȃ������t���O
			return FALSE;
		}
		pMYDEVMODE->bPrinterNotFound = FALSE;	// �v�����^���Ȃ������t���O

		// ������
		memset_raw(pMYDEVMODE, 0, sizeof(*pMYDEVMODE));
		hDevMode = pd.hDevMode;
		hDevNames = pd.hDevNames;
	}

	// MYDEVMODE�ւ̃R�s�[
	DEVMODE* pDEVMODE = (DEVMODE*)::GlobalLock(hDevMode);
	// �v�����^�ݒ� DEVNAMES�p
	DEVNAMES* pDEVNAMES = (DEVNAMES*)::GlobalLock(hDevNames);

	// �v�����^�h���C�o��
	_tcscpy_s(
		pMYDEVMODE->szPrinterDriverName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset
	);
	// �v�����^�f�o�C�X��
	_tcscpy_s(
		pMYDEVMODE->szPrinterDeviceName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset
	);
	// �v�����^�|�[�g��
	_tcscpy_s(
		pMYDEVMODE->szPrinterOutputName,
		(const TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset
	);

	// �v�����^���瓾��ꂽ�AdmFields�͕ύX���Ȃ�
	// �v�����^���T�|�[�g���Ȃ�bit���Z�b�g����ƁA�v�����^�h���C�o�ɂ���ẮA�s����ȓ���������ꍇ������
	// pMYDEVMODE�́A�R�s�[������bit�łP�̂��̂����R�s�[����
	// ���v�����^���瓾��ꂽ dmFields��1�łȂ�Length,Width���ɁA�Ԉ���������������Ă���v�����^�h���C�o�ł́A
	//   �c�E�����������������Ȃ��s��ƂȂ��Ă���(2003.07.03 �����)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation	= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize		= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength	= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth	= pDEVMODE->dmPaperWidth;

	DEBUG_TRACE(_T(" (����/�o��) �f�o�C�X �h���C�o=[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wDriverOffset);
	DEBUG_TRACE(_T(" (����/�o��) �f�o�C�X��=[%ts]\n"),        (TCHAR*)pDEVNAMES + pDEVNAMES->wDeviceOffset);
	DEBUG_TRACE(_T("�����o�̓��f�B�A (�o�̓|�[�g) =[%ts]\n"), (TCHAR*)pDEVNAMES + pDEVNAMES->wOutputOffset);
	DEBUG_TRACE(_T("�f�t�H���g�̃v�����^��=[%d]\n"),          pDEVNAMES->wDefault);

	::GlobalUnlock(hDevMode);
	::GlobalUnlock(hDevNames);
	return TRUE;
}

/*! 
** @brief �v�����^���I�[�v�����AhDC���쐬����
*/
HDC Print::CreateDC(
	MYDEVMODE*	pMYDEVMODE,
	TCHAR*		pszErrMsg		// �G���[���b�Z�[�W�i�[�ꏊ
	)
{
	// �v�����^���I������Ă��Ȃ���΁ANULL��Ԃ�
	if (!hDevMode) {
		return NULL;
	}
	HDC	hdc = NULL;
	HANDLE hPrinter = NULL;
	//
	// OpenPrinter()�ŁA�f�o�C�X���Ńv�����^�n���h�����擾
	//
	if (!::OpenPrinter(
			pMYDEVMODE->szPrinterDeviceName,		// �v�����^�f�o�C�X��
			&hPrinter,					// �v�����^�n���h���̃|�C���^
			NULL
		)
	) {
		auto_sprintf(
			pszErrMsg,
			LS(STR_ERR_CPRINT01),
			pMYDEVMODE->szPrinterDeviceName	// �v�����^�f�o�C�X��
		);
		goto end_of_func;
	}

	DEVMODE* pDEVMODE = (DEVMODE*)::GlobalLock(hDevMode);
	pDEVMODE->dmOrientation	= pMYDEVMODE->dmOrientation;
	pDEVMODE->dmPaperSize	= pMYDEVMODE->dmPaperSize;
	pDEVMODE->dmPaperLength	= pMYDEVMODE->dmPaperLength;
	pDEVMODE->dmPaperWidth	= pMYDEVMODE->dmPaperWidth;

	//
	// DocumentProperties()�ŃA�v���P�[�V�����Ǝ��̃v�����^�ݒ�ɕύX����
	//
	::DocumentProperties(
		NULL,
		hPrinter,
		pMYDEVMODE->szPrinterDeviceName,	// �v�����^�f�o�C�X��
		pDEVMODE,
		pDEVMODE,
		DM_OUT_BUFFER | DM_IN_BUFFER
	);
	// �w��f�o�C�X�ɑ΂���f�o�C�X �R���e�L�X�g���쐬���܂��B
	hdc = ::CreateDC(
		pMYDEVMODE->szPrinterDriverName,	// �v�����^�h���C�o��
		pMYDEVMODE->szPrinterDeviceName,	// �v�����^�f�o�C�X��
		pMYDEVMODE->szPrinterOutputName,	// �v�����^�|�[�g��
		pDEVMODE
	);
	
	// pMYDEVMODE�́A�R�s�[������bit�łP�̂��̂����R�s�[����
	// ���v�����^���瓾��ꂽ dmFields��1�łȂ�Length,Width���ɁA�Ԉ���������������Ă���v�����^�h���C�o�ł́A
	//   �c�E�����������������Ȃ��s��ƂȂ��Ă���(2003.07.03 �����)
	pMYDEVMODE->dmFields = pDEVMODE->dmFields & (DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	pMYDEVMODE->dmOrientation	= pDEVMODE->dmOrientation;
	pMYDEVMODE->dmPaperSize		= pDEVMODE->dmPaperSize;
	pMYDEVMODE->dmPaperLength	= pDEVMODE->dmPaperLength;
	pMYDEVMODE->dmPaperWidth	= pDEVMODE->dmPaperWidth;

	::GlobalUnlock(hDevMode);

end_of_func:;
	if (hPrinter) {
		::ClosePrinter(hPrinter);
	}

	return hdc;
}


// ���/Preview�ɕK�v�ȏ����擾
BOOL Print::GetPrintMetrics(
	MYDEVMODE*	pMYDEVMODE,
	short*		pnPaperAllWidth,	// �p����
	short*		pnPaperAllHeight,	// �p������
	short*		pnPaperWidth,		// �p������\��
	short*		pnPaperHeight,		// �p������\����
	short*		pnPaperOffsetLeft,	// �p���]�����[
	short*		pnPaperOffsetTop,	// �p���]����[
	TCHAR*		pszErrMsg			// �G���[���b�Z�[�W�i�[�ꏊ
	)
{
	BOOL bRet = TRUE;

	// ���݂̐ݒ�ŁA�p���̕��A�������m�肵�ACreateDC�ɓn��
	if (!GetPaperSize(pnPaperAllWidth, pnPaperAllHeight, pMYDEVMODE)) {
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);
	}

	// pMYDEVMODE���g���āAhdc���擾
	HDC hdc = CreateDC(pMYDEVMODE, pszErrMsg);
	if (!hdc) {
		return FALSE;
	}

	// CreateDC���s�ɂ���ē���ꂽ���ۂ̃v�����^�̗p���̕��A�������擾
	if (!GetPaperSize(pnPaperAllWidth, pnPaperAllHeight, pMYDEVMODE)) {
		*pnPaperAllWidth = *pnPaperWidth + 2 * (*pnPaperOffsetLeft);
		*pnPaperAllHeight = *pnPaperHeight + 2 * (*pnPaperOffsetTop);
	}

	// �}�b�s���O ���[�h�̐ݒ�
	::SetMapMode(hdc, MM_LOMETRIC);	// MM_LOMETRIC	���ꂼ��̘_���P�ʂ� 0.1 mm �Ƀ}�b�v����܂��B

	// �ŏ����}�[�W���ƍŏ���}�[�W�����擾(1mm�P��)
	POINT	po;
	if (0 < ::Escape(hdc, GETPRINTINGOFFSET, (int)NULL, NULL, (LPPOINT)&po)) {
		::DPtoLP(hdc, &po, 1);
		*pnPaperOffsetLeft = (short)abs(po.x);	// �p���]�����[
		*pnPaperOffsetTop  = (short)abs(po.y);	// �p���]����[
	}else {
		*pnPaperOffsetLeft = 0;	// �p���]�����[
		*pnPaperOffsetTop  = 0;	// �p���]����[
	}

	// �p���̈���\�ȕ��A����
	po.x = ::GetDeviceCaps(hdc, HORZRES);	// �p������\���������f�B�X�v���C�̕� (mm �P��)
	po.y = ::GetDeviceCaps(hdc, VERTRES);	// �p������\�����������f�B�X�v���C�̍��� (mm �P��) 
	::DPtoLP(hdc, &po, 1);
	*pnPaperWidth  = (short)abs(po.x);
	*pnPaperHeight = (short)abs(po.y);

	::DeleteDC(hdc);

	return bRet;
}


// �p���̕��A����
BOOL Print::GetPaperSize(
	short*		pnPaperAllWidth,
	short*		pnPaperAllHeight,
	MYDEVMODE*	pDEVMODE
	)
{
	short	nWork;
	
	if (pDEVMODE->dmFields &  DM_PAPERSIZE) {
		const PaperInfo* pi = FindPaperInfo(pDEVMODE->dmPaperSize);
		if (pi) {
			*pnPaperAllWidth = pi->nAllWidth;
			*pnPaperAllHeight = pi->nAllHeight;
		}else {
			// 2001.12.21 hor �}�E�X�ŃN���b�N�����܂܃��X�g�O�ɏo��Ƃ����ɂ��邯�ǁA
			//	�ُ�ł͂Ȃ��̂� FALSE ��Ԃ����Ƃɂ���
			return FALSE;
		}
	}
	if (pDEVMODE->dmFields & DM_PAPERLENGTH && pDEVMODE->dmPaperLength != 0) {
		// pDEVMODE->dmPaperLength��1/10mm�P�ʂł���
		*pnPaperAllHeight = pDEVMODE->dmPaperLength/* * 10*/;
	}else {
		pDEVMODE->dmPaperLength = *pnPaperAllHeight;
		pDEVMODE->dmFields |= DM_PAPERLENGTH;
	}
	if (pDEVMODE->dmFields & DM_PAPERWIDTH && pDEVMODE->dmPaperWidth != 0) {
		// pDEVMODE->dmPaperWidth��1/10mm�P�ʂł���
		*pnPaperAllWidth = pDEVMODE->dmPaperWidth/* * 10*/;
	}else {
		pDEVMODE->dmPaperWidth = *pnPaperAllWidth;
		pDEVMODE->dmFields |= DM_PAPERWIDTH;
	}
	// �p���̕���
	if (pDEVMODE->dmOrientation == DMORIENT_LANDSCAPE) {
		nWork = *pnPaperAllWidth;
		*pnPaperAllWidth = *pnPaperAllHeight;
		*pnPaperAllHeight = nWork;
	}
	return TRUE;
}


// ��� �W���u�J�n
BOOL Print::PrintOpen(
	TCHAR*		pszJobName,
	MYDEVMODE*	pMYDEVMODE,
	HDC*		phdc,
	TCHAR*		pszErrMsg		// �G���[���b�Z�[�W�i�[�ꏊ
	)
{
	BOOL bRet = TRUE;
	DOCINFO di = {0};
	// 
	// hdc���擾
	//
	HDC hdc = CreateDC(pMYDEVMODE, pszErrMsg);
	if (!hdc) {
		bRet = FALSE;
		goto end_of_func;
	}

	// �}�b�s���O ���[�h�̐ݒ�
	::SetMapMode(hdc, MM_LOMETRIC);	// MM_LOMETRIC		���ꂼ��̘_���P�ʂ́A0.1 mm �Ƀ}�b�v����܂��B

	//
	//  ����W���u�J�n
	//
	di.cbSize = sizeof(di);
	di.lpszDocName = pszJobName;
	di.lpszOutput  = NULL;
	di.lpszDatatype = NULL;
	di.fwType = 0;
	if (0 >= ::StartDoc(hdc, &di)) {
		auto_sprintf(
			pszErrMsg,
			LS(STR_ERR_CPRINT02),
			pMYDEVMODE->szPrinterDeviceName	// �v�����^�f�o�C�X��
		);
		bRet = FALSE;
		goto end_of_func;
	}

	*phdc = hdc;

end_of_func:;

	return bRet;
}


// ��� �y�[�W�J�n
void Print::PrintStartPage(HDC hdc)
{
	::StartPage(hdc);
}


// ��� �y�[�W�I��
void Print::PrintEndPage(HDC hdc)
{
	::EndPage(hdc);

}


// ��� �W���u�I��
void Print::PrintClose(HDC hdc)
{
	::EndDoc(hdc);
	::DeleteDC(hdc);
}


// �p���̖��O���擾
TCHAR* Print::GetPaperName(int nPaperSize, TCHAR* pszPaperName)
{
	const PaperInfo* paperInfo = FindPaperInfo(nPaperSize);
	if (paperInfo) {
		_tcscpy(pszPaperName, paperInfo->pszName);
	}else {
		_tcscpy(pszPaperName, LS(STR_ERR_CPRINT03));
	}
	return pszPaperName;
}

/*!
	�p�����̎擾
*/
const PaperInfo* Print::FindPaperInfo(int id)
{
	for (int i=0; i<nPaperInfoArrNum; ++i) {
		if (paperInfoArr[i].nId == id) {
			return &(paperInfoArr[i]);
		}
	}
	return NULL;
}


/*!	@brief PrintSetting�̏�����

	�����ł�mdmDevMode�� �v�����^�ݒ�͎擾�E���������Ȃ�
*/
void Print::SettingInitialize(PrintSetting& pPrintSetting, const TCHAR* settingName)
{
	_tcscpy_s(pPrintSetting.szPrintSettingName, settingName);		// ����ݒ�̖��O
	_tcscpy(pPrintSetting.szPrintFontFaceHan, _T("�l�r ����"));		// ����t�H���g
	_tcscpy(pPrintSetting.szPrintFontFaceZen, _T("�l�r ����"));		// ����t�H���g
	pPrintSetting.bColorPrint = false;			// �J���[���
	pPrintSetting.nPrintFontWidth = 12;			// ����t�H���g��(1/10mm�P��)
	pPrintSetting.nPrintFontHeight = pPrintSetting.nPrintFontWidth * 2;	// ����t�H���g����(1/10mm�P�ʒP��)
	pPrintSetting.nPrintDansuu = 1;				// �i�g�̒i��
	pPrintSetting.nPrintDanSpace = 70; 			// �i�ƒi�̌���(1/10mm)
	pPrintSetting.bPrintWordWrap = true;		// �p�����[�h���b�v����
	pPrintSetting.bPrintKinsokuHead = false;	// �s���֑�����
	pPrintSetting.bPrintKinsokuTail = false;	// �s���֑�����
	pPrintSetting.bPrintKinsokuRet  = false;	// ���s�������Ԃ牺����
	pPrintSetting.bPrintKinsokuKuto = false;	// 
	pPrintSetting.bPrintLineNumber = false;		// �s�ԍ����������
	pPrintSetting.nPrintLineSpacing = 30;		// ����t�H���g�s�� �����̍����ɑ΂��銄��(%)
	pPrintSetting.nPrintMarginTY = 100;			// ����p���}�[�W�� ��(1/10mm�P��)
	pPrintSetting.nPrintMarginBY = 200;			// ����p���}�[�W�� ��(1/10mm�P��)
	pPrintSetting.nPrintMarginLX = 200;			// ����p���}�[�W�� ��(1/10mm�P��)
	pPrintSetting.nPrintMarginRX = 100;			// ����p���}�[�W�� �E(1/10mm�P��)
	pPrintSetting.nPrintPaperOrientation = DMORIENT_PORTRAIT;	// �p������ DMORIENT_PORTRAIT (1) �܂��� DMORIENT_LANDSCAPE (2)
	pPrintSetting.nPrintPaperSize = DMPAPER_A4;	// �p���T�C�Y
	// �v�����^�ݒ� DEVMODE�p
	// �v�����^�ݒ���擾����̂̓R�X�g��������̂ŁA��ق�
	//	print.GetDefaultPrinterInfo(&(pPrintSetting.mdmDevMode));
	pPrintSetting.bHeaderUse[0] = TRUE;
	pPrintSetting.bHeaderUse[1] = FALSE;
	pPrintSetting.bHeaderUse[2] = FALSE;
	wcscpy(pPrintSetting.szHeaderForm[0], L"$f");
	wcscpy(pPrintSetting.szHeaderForm[1], L"");
	wcscpy(pPrintSetting.szHeaderForm[2], L"");
	pPrintSetting.bFooterUse[0] = TRUE;
	pPrintSetting.bFooterUse[1] = FALSE;
	pPrintSetting.bFooterUse[2] = FALSE;
	wcscpy(pPrintSetting.szFooterForm[0], L"");
	wcscpy(pPrintSetting.szFooterForm[1], L"- $p -");
	wcscpy(pPrintSetting.szFooterForm[2], L"");
}


/*!
	�󎚉\�����̌v�Z
*/
int Print::CalculatePrintableColumns(PrintSetting& ps, int nPaperAllWidth, int nLineNumberColumns)
{
	int nPrintablePaperWidth = nPaperAllWidth - ps.nPrintMarginLX - ps.nPrintMarginRX;
	if (nPrintablePaperWidth < 0) { return 0; }

	int nPrintSpaceWidth = (ps.nPrintDansuu - 1) * ps.nPrintDanSpace
						 + (ps.nPrintDansuu) * (nLineNumberColumns * ps.nPrintFontWidth);
	if (nPrintablePaperWidth < nPrintSpaceWidth) { return 0; }

	int nEnableColumns =
		(nPrintablePaperWidth - nPrintSpaceWidth
		) / ps.nPrintFontWidth / ps.nPrintDansuu;	// �󎚉\����/�y�[�W
	return nEnableColumns;
}


/*!
	�󎚉\�s���̌v�Z
*/
int Print::CalculatePrintableLines(
	PrintSetting& ps,
	int nPaperAllHeight
	)
{
	int nPrintablePaperHeight = nPaperAllHeight - ps.nPrintMarginTY - ps.nPrintMarginBY;
	if (nPrintablePaperHeight < 0) { return 0; }

	int nPrintSpaceHeight = (ps.nPrintFontHeight * ps.nPrintLineSpacing / 100);

	int nEnableLines =
		(nPrintablePaperHeight - CalcHeaderHeight(ps)*2 - CalcFooterHeight(ps)*2 + nPrintSpaceHeight) /
		(ps.nPrintFontHeight + nPrintSpaceHeight);	// �󎚉\�s��/�y�[�W
	if (nEnableLines < 0) { return 0; }
	return nEnableLines;
}


/*!
	�w�b�_�����̌v�Z(�s���蕪����)
*/
int Print::CalcHeaderHeight(PrintSetting& ps)
{
	if (ps.szHeaderForm[0][0] == _T('\0')
		&& ps.szHeaderForm[1][0] == _T('\0')
		&& ps.szHeaderForm[2][0] == _T('\0')
	) {
		// �g���ĂȂ���� 0
		return 0;
	}

	int nHeight;
	if (ps.lfHeader.lfFaceName[0] == _T('\0')) {
		// �t�H���g�w�薳��
		nHeight = ps.nPrintFontHeight;
	}else {
		// �t�H���g�̃T�C�Y�v�Z(pt->1/10mm)
		nHeight = ps.nHeaderPointSize * 254 / 720;
	}
	return nHeight * (ps.nPrintLineSpacing + 100) / 100;	// �s����v�Z
}

/*!
	�t�b�^�����̌v�Z(�s���蕪����)
*/
int Print::CalcFooterHeight(PrintSetting& ps)
{
	if (ps.szFooterForm[0][0] == _T('\0')
	 && ps.szFooterForm[1][0] == _T('\0')
	 && ps.szFooterForm[2][0] == _T('\0')
	) {
		// �g���ĂȂ���� 0
		return 0;
	}

	int nHeight;
	if (ps.lfFooter.lfFaceName[0] == _T('\0')) {
		// �t�H���g�w�薳��
		nHeight = ps.nPrintFontHeight;
	}else {
		// �t�H���g�̃T�C�Y�v�Z(pt->1/10mm)
		nHeight = ps.nFooterPointSize * 254 / 720;
	}
	return nHeight * (ps.nPrintLineSpacing + 100) / 100;	// �s����v�Z
}

