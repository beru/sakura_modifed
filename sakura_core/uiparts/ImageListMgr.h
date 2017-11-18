#pragma once

#include "_main/global.h"

/*! @brief ImageList�̊Ǘ�

	�A�C�R���C���[�W���Ǘ�����

	@note �C���[�W���X�g�ւ̃r�b�g�}�b�v�̓o�^��Bitblt���s��������
		VAIO�Ƒ����������u���[�X�N���[�����������Ă����D
		�܂��C���[�W���X�g��IE3�ȑO��common component�Ɋ܂܂�Ă��Ȃ����߂�
		����Win95�ŃC���[�W�̕\�����ł��Ȃ������D������������邽�߂�ImageList�̎g�p����߂�
		�����̓Ǝ��`��ɖ߂����D
*/
class ImageListMgr {
public:

	//	constructor
	ImageListMgr();
	~ImageListMgr();

	bool Create(HINSTANCE hInstance);	//	����
	
	/*! @brief �A�C�R���̕`��
	
		�w�肳�ꂽDC�̎w�肳�ꂽ���W�ɃA�C�R����`�悷��D
	
		@param index [in] �`�悷��A�C�R���ԍ�
		@param dc [in] �`�悷��Device Context
		@param x [in] �`�悷��X���W
		@param y [in] �`�悷��Y���W
		@param fstyle [in] �`��X�^�C��
	*/
	bool Draw(int index, HDC dc, int x, int y, int fstyle) const	//	�`��
	;
	
	// �A�C�R������Ԃ�
	size_t Count(void) const;	//	�A�C�R����
	
	// �A�C�R���̕�
	int  GetCx(void) const { return cx; }
	// �A�C�R���̍���
	int  GetCy(void) const { return cy; }
	
	// �A�C�R����ǉ�����
	int Add(const TCHAR* szPath);

	// �A�C�R���̒ǉ������ɖ߂�
	void ResetExtend();

	/*!
		�C���[�W��ToolBar�ւ̓o�^
	
		@param hToolBar [in] �o�^����ToolBar
		@param id [in] �o�^����擪�A�C�R���ԍ�
	*/
	void  SetToolBarImages(HWND hToolBar, int id = 0) const {}

protected:
	int cx;			// width of icon
	int cy;			// height of icon
	/*!	@brief ���ߐF
	
		�`������O�ōs�����߁C���ߐF���o���Ă����K�v������D
	*/
	COLORREF cTrans;
	
	/*! �A�C�R���p�r�b�g�}�b�v��ێ�����	*/
	HBITMAP hIconBitmap;

	size_t nIconCount;	// �A�C�R���̌�

	//	�I���W�i���e�L�X�g�G�f�B�^����̕`��֐�
	void MyBitBlt(HDC drawdc, int nXDest, int nYDest, 
					int nWidth, int nHeight, HBITMAP bmp,
					int nXSrc, int nYSrc, COLORREF colToTransParent) const;
	void DitherBlt2(HDC drawdc, int nXDest, int nYDest, int nWidth, 
                        int nHeight, HBITMAP bmp, int nXSrc, int nYSrc) const;

	// �r�b�g�}�b�v����s�g������
	void Extend(bool = true);

};

