#include "StdAfx.h"
#include "ImageListMgr.h"
#include "env/CommonSetting.h"
#include "util/module.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h"

const uint32_t MAX_X = MAX_TOOLBAR_ICON_X;
const uint32_t MAX_Y = MAX_TOOLBAR_ICON_Y;

ImageListMgr::ImageListMgr()
	:
	cx(16),
	cy(16),
	cTrans(RGB(0, 0, 0)),
	hIconBitmap(NULL),
	nIconCount(MAX_TOOLBAR_ICON_COUNT)
{
}

/*!	�̈���w��F�œh��Ԃ� */
static
void FillSolidRect(
	HDC hdc,
	int x,
	int y,
	int cx,
	int cy,
	COLORREF clr
	)
{
//	ASSERT_VALID(this);
//	ASSERT(hDC);

	RECT rect;
	::SetBkColor(hdc, clr);
	::SetRect(&rect, x, y, x + cx, y + cy);
	::ExtTextOutW_AnyBuild(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
}

//	Destructor
ImageListMgr::~ImageListMgr()
{
	// Image List�̑���ɕ`��pbitmap�����
	if (hIconBitmap) {
		DeleteObject(hIconBitmap);
	}
}

/*
	@brief Image List�̍쐬
	
	���\�[�X�܂��̓t�@�C������bitmap��ǂݍ����
	�`��p�ɕێ�����D
	
	@param hInstance [in] bitmap���\�[�X�����C���X�^���X
*/
bool ImageListMgr::Create(HINSTANCE hInstance)
{
	MY_RUNNINGTIMER(runningTimer, "CImageListMgr::Create");
	if (hIconBitmap) {	//	���ɍ\�z�ς݂Ȃ疳������
		return true;
	}

	HBITMAP	hRscbmp;			//	���\�[�X����ǂݍ��񂾂ЂƂ����܂��Bitmap
	HBITMAP	hFOldbmp = NULL;	//	SetObject�œ���ꂽ1�O�̃n���h����ێ�����
	HDC		dcFrom = 0;			//	�`��p
	int		nRetPos;			//	�㏈���p
	cx = cy  = 16;

	nRetPos = 0;
	do {
		TCHAR szPath[_MAX_PATH];
		GetInidirOrExedir(szPath, FN_TOOL_BMP);
		hRscbmp = (HBITMAP)::LoadImage(NULL, szPath, IMAGE_BITMAP, 0, 0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);

		if (!hRscbmp) {	// ���[�J���t�@�C���̓ǂݍ��ݎ��s���̓��\�[�X����擾
			//	���̃u���b�N���͏]���̏���
			//	���\�[�X����Bitmap��ǂݍ���
			hRscbmp = (HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(IDB_MYTOOL), IMAGE_BITMAP, 0, 0,
				LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );
			if (!hRscbmp) {
				//	����I���Ɠ����R�[�h����dcFrom��s���ɉ�����Ă��܂�
				nRetPos = 2;
				break;
			}
		}
		hIconBitmap = hRscbmp;

		//	���ߐF�𓾂邽�߂�DC�Ƀ}�b�v����
		dcFrom = CreateCompatibleDC(0);	//	�]�����p
		if (!dcFrom) {
			nRetPos = 1;
			break;
		}

		//	�܂�bitmap��dc��map����
		//	�������邱�Ƃ�CreateCompatibleBitmap��
		//	hRscbmp�Ɠ����`����bitmap������D
		//	�P��CreateCompatibleDC(0)�Ŏ擾����dc��
		//	�X�N���[����DC�ɑ΂���CreateCompatibleBitmap��
		//	�g���ƃ��m�N��Bitmap�ɂȂ�D
		hFOldbmp = (HBITMAP)SelectObject(dcFrom, hRscbmp);
		if (!hFOldbmp) {
			nRetPos = 4;
			break;
		}

		cTrans = GetPixel(dcFrom, 0, 0);//	�擾�����摜��(0,0)�̐F��w�i�F�Ƃ��Ďg��
		
		//	���͂⏈���Ƃ͖��֌W�����C��w�̂��߂ɃR�����g�̂ݎc���Ă�����
		//---------------------------------------------------------
		//	Bitmap��MemoryDC��Assign����Ă���Ԃ�bitmap�n���h����
		//	�g���Ă�������bitmap���擾�ł��Ȃ��D
		//	�܂�CDC�ւ̕`�施�߂𔭍s���Ă����̏��Bitmap��
		//	���f�����킯�ł͂Ȃ��D
		//	Bitmap��DC������O���ď��߂ē��e�̕ۏ؂��ł���

		//	DC��map/unmap�����x�ɑ傫���e�����邽�߁C
		//	������Bitmap������Ĉꊇ�o�^����悤�ɕύX
		//	����ɂ����250msec���炢���x�����P�����D
		//---------------------------------------------------------

	}while (0);	//	1�񂵂��ʂ�Ȃ�. break�ł����܂Ŕ��

	//	�㏈��
	switch (nRetPos) {
	case 0:
		//	hRscBmp��dcFrom����؂藣���Ă����K�v������
		//	�A�C�R���`��ύX���ɉ߂��č폜����Ă���
		SelectObject(dcFrom, hFOldbmp);
	case 4:
		DeleteDC(dcFrom);
	case 2:
	case 1:
		//	hRscbmp�� hIconBitmap �Ƃ��ăI�u�W�F�N�g��
		//	���������ێ������̂ŉ�����Ă͂Ȃ�Ȃ�
		break;
	}

	return nRetPos == 0;
}


/*! �r�b�g�}�b�v�̕\�� �D�F�𓧖��`�� */
void ImageListMgr::MyBitBlt(
	HDC drawdc, 
	int nXDest, 
	int nYDest, 
	int nWidth, 
	int nHeight, 
	HBITMAP bmp, 
	int nXSrc, 
	int nYSrc,
	COLORREF colToTransParent	// BMP�̒��̓����ɂ���F
	) const
{
//	HBRUSH	brShadow, brHilight;
	// create a monochrome memory DC
	HDC hdcMask = CreateCompatibleDC(drawdc);
	HBITMAP bmpMask = CreateCompatibleBitmap(hdcMask, nWidth, nHeight);
	HBITMAP bmpMaskOld = (HBITMAP)SelectObject(hdcMask, bmpMask);
	// ���r�b�g�}�b�v�pDC
	HDC hdcMem = ::CreateCompatibleDC(drawdc);
	HBITMAP bmpMemOld = (HBITMAP)::SelectObject(hdcMem, bmp);
	// ��ƗpDC
	HDC hdcMem2 = ::CreateCompatibleDC(drawdc);
	HBITMAP bmpMem2 = CreateCompatibleBitmap(drawdc, nWidth, nHeight);
	HBITMAP bmpMem2Old = (HBITMAP)SelectObject(hdcMem2, bmpMem2);
	
	// build a mask
	SetBkColor(hdcMem, colToTransParent);
	BitBlt(hdcMask, 0, 0, nWidth, nHeight, hdcMem, nXSrc, nYSrc, SRCCOPY);

	// �}�X�N�`��(�����ɂ��Ȃ��������������`��)
	::SetBkColor(drawdc, RGB(255, 255, 255) /* colBkColor */);
	::SetTextColor(drawdc, RGB(0, 0, 0));
	::BitBlt(drawdc, nXDest, nYDest, nWidth, nHeight, hdcMask, 0, 0, SRCAND /* SRCCOPY */); 

	// �r�b�g�}�b�v�`��(�����ɂ���F���������ă}�X�N��OR�`��)
	::SetBkColor(hdcMem2, colToTransParent/*RGB(0, 0, 0)*/);
	::SetTextColor(hdcMem2, RGB(0, 0, 0));
	::BitBlt(hdcMem2, 0, 0, nWidth, nHeight, hdcMask, 0, 0, SRCCOPY);
	::BitBlt(hdcMem2, 0, 0, nWidth, nHeight, hdcMem, nXSrc, nYSrc, SRCINVERT/*SRCPAINT*/);
	::BitBlt(drawdc, nXDest, nYDest, nWidth, nHeight, hdcMem2,  0, 0, /*SRCCOPY*/SRCPAINT);

	::SelectObject(hdcMask, bmpMaskOld);
	::DeleteObject(bmpMask);
	::DeleteDC(hdcMask);
	::SelectObject(hdcMem, bmpMemOld);
	::DeleteDC(hdcMem);
	::SelectObject(hdcMem2, bmpMem2Old);
	::DeleteObject(bmpMem2);
	::DeleteDC(hdcMem2);
	return;
}

/*! ���j���[�A�C�R���̒W�F�\�� */
void ImageListMgr::DitherBlt2(
	HDC drawdc,
	int nXDest,
	int nYDest,
	int nWidth, 
    int nHeight,
    HBITMAP bmp,
    int nXSrc,
    int nYSrc
    ) const
{

	//COLORREF colToTransParent = RGB(192, 192, 192);	// BMP�̒��̓����ɂ���F
	COLORREF colToTransParent = cTrans;

	// create a monochrome memory DC
	HDC hdcMask = CreateCompatibleDC(drawdc);
	HBITMAP bmpMask = CreateCompatibleBitmap(hdcMask, nWidth, nHeight);
	HBITMAP bmpMaskOld = (HBITMAP)SelectObject(hdcMask, bmpMask);

	HDC hdcMem = CreateCompatibleDC(drawdc);
	HBITMAP bmpMemOld = (HBITMAP)SelectObject(hdcMem, bmp);

	//	hdcMem�ɏ������ނƌ���bitmap��j�󂵂Ă��܂�
	HDC hdcMem2 = ::CreateCompatibleDC(drawdc);
	HBITMAP bmpMem2 = CreateCompatibleBitmap(drawdc, nWidth, nHeight);
	HBITMAP bmpMem2Old = (HBITMAP)SelectObject(hdcMem2, bmpMem2);

	// build a mask
	SetBkColor(hdcMem, colToTransParent);
	BitBlt(hdcMask, 0, 0, nWidth, nHeight, hdcMem, nXSrc, nYSrc, SRCCOPY);
	SetBkColor(hdcMem, RGB(255, 255, 255));
	BitBlt(hdcMask, 0, 0, nWidth, nHeight, hdcMem, nXSrc, nYSrc, SRCPAINT);

	// Copy the image from the toolbar into the memory DC
	// and draw it (grayed) back into the toolbar.
    // SK: Looks better on the old shell
	COLORREF coltxOld = ::SetTextColor(drawdc, RGB(0, 0, 0));
	COLORREF colbkOld = ::SetBkColor(drawdc, RGB(255, 255, 255));
	::SetBkColor(hdcMem2, RGB(0, 0, 0));
#if 0
	::SetTextColor(hdcMem2, ::GetSysColor(COLOR_BTNHILIGHT));
	::BitBlt(hdcMem2, 0, 0, nWidth, nHeight, hdcMask, 0, 0, SRCCOPY);
	::BitBlt(drawdc, nXDest + 1, nYDest + 1, nWidth, nHeight, hdcMask, 0, 0, SRCAND);
	::BitBlt(drawdc, nXDest + 1, nYDest + 1, nWidth, nHeight, hdcMem2, 0, 0, SRCPAINT);
	::SetTextColor(hdcMem2, ::GetSysColor(COLOR_BTNSHADOW));
#else
	::SetTextColor(hdcMem2, (::GetSysColor(COLOR_BTNSHADOW) != ::GetSysColor(COLOR_BTNFACE) ? ::GetSysColor(COLOR_3DSHADOW) : ::GetSysColor(COLOR_BTNHILIGHT)));
#endif
	::BitBlt(hdcMem2, 0, 0, nWidth, nHeight, hdcMask, 0, 0, SRCCOPY);
	::BitBlt(drawdc, nXDest, nYDest, nWidth, nHeight, hdcMask, 0, 0, SRCAND);
	::BitBlt(drawdc, nXDest, nYDest, nWidth, nHeight, hdcMem2, 0, 0, SRCPAINT);
	::SetTextColor(drawdc, coltxOld);
	::SetBkColor(drawdc, colbkOld);

	// reset DCs
	SelectObject(hdcMask, bmpMaskOld);
	DeleteDC(hdcMask);

	SelectObject(hdcMem, bmpMemOld);
	DeleteDC(hdcMem);

	::SelectObject(hdcMem2, bmpMem2Old);
	::DeleteObject(bmpMem2);
	::DeleteDC(hdcMem2);

	DeleteObject(bmpMask);
	return;

}

/*! @brief �A�C�R���̕`��

	�w�肳�ꂽDC�̎w�肳�ꂽ���W�ɃA�C�R����`�悷��D

	@param index [in] �`�悷��A�C�R���ԍ�
	@param dc [in] �`�悷��Device Context
	@param x [in] �`�悷��X���W
	@param y [in] �`�悷��Y���W
	@param fstyle [in] �`��X�^�C��
	@param bgColor [in] �w�i�F(���������̕`��p)

	@note �`��X�^�C���Ƃ��ėL���Ȃ̂́CILD_NORMAL, ILD_MASK
*/
bool ImageListMgr::Draw(
	int index,
	HDC dc,
	int x,
	int y,
	int fstyle
	) const
{
	if (!hIconBitmap) {
		return false;
	}
	if (index < 0) {
		return false;
	}

	if (fstyle == ILD_MASK) {
		DitherBlt2(dc, x, y, cx, cy, hIconBitmap,
			(index % MAX_X) * cx, (index / MAX_X) * cy);
	}else {
		MyBitBlt(dc, x, y, cx, cy, hIconBitmap,
			(index % MAX_X) * cx, (index / MAX_X) * cy, cTrans);
	}
	return true;
}

/*!	�A�C�R������Ԃ� */
size_t ImageListMgr::Count() const
{
	return nIconCount;
//	return MAX_X * MAX_Y;
}

/*!	�A�C�R����ǉ����Ă���ID��Ԃ� */
int ImageListMgr::Add(const TCHAR* szPath)
{
	if ((nIconCount % MAX_X) == 0) {
		Extend();
	}
	size_t index = nIconCount;
	++nIconCount;

	// �A�C�R����ǂݍ���
	HBITMAP hExtBmp = (HBITMAP)::LoadImage(NULL, szPath, IMAGE_BITMAP, 0, 0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION);

	if (!hExtBmp) {
		return -1;
	}

	// hIconBitmap�ɃR�s�[����
	HDC hDestDC = ::CreateCompatibleDC(0);
	HBITMAP hOldDestBmp = (HBITMAP)::SelectObject(hDestDC, hIconBitmap);

	HDC hExtDC = ::CreateCompatibleDC(0);
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hExtDC, hExtBmp);
	COLORREF cTrans = GetPixel(hExtDC, 0, 0);//	�擾�����摜��(0,0)�̐F��w�i�F�Ƃ��Ďg��
	::SelectObject(hExtDC, hOldBmp);
	::DeleteDC(hExtDC);

	MyBitBlt(
		hDestDC,
		(int)((index % MAX_X) * cx),
		(int)((index / MAX_X) * cy),
		cx, cy, hExtBmp, 0, 0, cTrans
	);

	::SelectObject(hDestDC, hOldDestBmp);
	::DeleteDC(hDestDC);
	::DeleteObject(hExtBmp);

	return (int)index;
}

// �r�b�g�}�b�v����s�iMAX_X�j�g������
void ImageListMgr::Extend(bool bExtend)
{
	size_t curY = nIconCount / MAX_X;
	if (curY < MAX_Y) {
		curY = MAX_Y;
	}

	HDC hSrcDC = ::CreateCompatibleDC(0);
	HBITMAP hSrcBmpOld = (HBITMAP)::SelectObject(hSrcDC, hIconBitmap);

	// 1�s�g�������r�b�g�}�b�v���쐬
	HDC hDestDC = ::CreateCompatibleDC(hSrcDC);
	HBITMAP hDestBmp = ::CreateCompatibleBitmap(hSrcDC, (int)(MAX_X * cx), (int)(curY + (bExtend ? 1 : 0)) * cy);
	HBITMAP hDestBmpOld = (HBITMAP)::SelectObject(hDestDC, hDestBmp);

	::BitBlt(hDestDC, 0, 0, (int)(MAX_X * cx), (int)(curY * cy), hSrcDC, 0, 0, SRCCOPY);

	// �g�����������͓��ߐF�œh��
	if (bExtend) {
		FillSolidRect(hDestDC, 0, (int)(curY * cy), MAX_X * cx, cy, cTrans);
	}

	::SelectObject(hSrcDC, hSrcBmpOld);
	::DeleteObject(hIconBitmap);
	::DeleteDC(hSrcDC);

	::SelectObject(hDestDC, hDestBmpOld);
	::DeleteDC(hDestDC);

	// �r�b�g�}�b�v�̍����ւ�
	hIconBitmap = hDestBmp;
}

void ImageListMgr::ResetExtend()
{
	nIconCount = MAX_TOOLBAR_ICON_COUNT;
	Extend(false);
}

