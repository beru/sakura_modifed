#include "StdAfx.h"
#include <vector>
#include "StdApi.h"
#include "charset/charcode.h"
#include "_os/OsVersionInfo.h"

using namespace std;

namespace ApiWrap {

	/*!
		MakeSureDirectoryPathExists �� UNICODE �ŁB
		szDirPath �Ŏw�肳�ꂽ���ׂẴf�B���N�g�����쐬���܂��B
		�f�B���N�g���̋L�q�́A���[�g����J�n���܂��B

		@param DirPath
			�L���ȃp�X�����w�肷��Anull �ŏI��镶����ւ̃|�C���^���w�肵�܂��B
			�p�X�̍Ō�̃R���|�[�l���g���t�@�C�����ł͂Ȃ��f�B���N�g���ł���ꍇ�A
			������̍Ō�ɉ~�L���i\�j���L�q���Ȃ���΂Ȃ�܂���B 

		@returns
			�֐�����������ƁATRUE ���Ԃ�܂��B
			�֐������s����ƁAFALSE ���Ԃ�܂��B

		@note
			�w�肳�ꂽ�e�f�B���N�g�����܂����݂��Ȃ��ꍇ�A�����̃f�B���N�g�������ɍ쐬���܂��B
			�ꕔ�̃f�B���N�g���݂̂��쐬�����ꍇ�A���̊֐��� FALSE ��Ԃ��܂��B

		@author
			kobake

		@date
			2007.10.15
	*/
	BOOL MakeSureDirectoryPathExistsW(LPCWSTR szDirPath)
	{
		const wchar_t* p = szDirPath - 1;
		for (;;) {
			p = wcschr(p + 1, L'\\');
			if (!p) break; // '\\'�𑖍����I������̂ŏI��

			// �擪����p�܂ł̕��������� -> szBuf
			wchar_t szBuf[_MAX_PATH];
			wcsncpy_s(szBuf, _countof(szBuf), szDirPath, p - szDirPath);

			// ���݂��邩
			int nAcc = _waccess(szBuf, 0);
			if (nAcc == 0) continue; // ���݂���Ȃ�A����

			// �f�B���N�g���쐬
			int nDir = _wmkdir(szBuf);
			if (nDir == -1) return FALSE; // �G���[�����������̂ŁAFALSE��Ԃ�
		}
		return TRUE;
	}

	LPWSTR CharNextW_AnyBuild(
		LPCWSTR lpsz
	)
	{
		//$$ �T���Q�[�g�y�A����
		if (*lpsz) return const_cast<LPWSTR>(lpsz + 1);
		else return const_cast<LPWSTR>(lpsz);
	}

	LPWSTR CharPrevW_AnyBuild(
		LPCWSTR lpszStart,
		LPCWSTR lpszCurrent
	)
	{
		//$$ �T���Q�[�g�y�A����
		if (lpszCurrent > lpszStart) return const_cast<LPWSTR>(lpszCurrent - 1);
		else return const_cast<LPWSTR>(lpszStart);
	}

#if 1
	BOOL GetTextExtentPoint32W_AnyBuild(
		HDC		hdc, 
		LPCWSTR	lpString, 
		int		cbString, 
		LPSIZE	lpSize
	)
	{
		vector<char> buf;
		wcstombs_vector(lpString, cbString, &buf);
		return GetTextExtentPoint32A(
			hdc,
			&buf[0],
			(int)buf.size() - 1,
			lpSize
		);
	}
#endif

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    �`��API �s����b�v                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	/*
		Vista��SetPixel�������Ȃ����߁A��֊֐���p�ӁB

		�Q�l�Fhttp://forums.microsoft.com/MSDN-JA/ShowPost.aspx?PostID=3228018&SiteID=7
		> Vista �� Aero �� OFF �ɂ���� SetPixel �����܂������Ȃ������ł��B
		> �������ASP1 �ł��C������Ă��Ȃ��Ƃ��B
	*/
	void SetPixelSurely(HDC hdc, int x, int y, COLORREF c)
	{
		if (!IsWinVista_or_later()) {
		// Vista���O�FSetPixel���Ăяo��
			::SetPixel(hdc, x, y, c);
		}else {
		// Vista�ȍ~�FSetPixel�G�~�����[�g
			static HPEN hPen = NULL;
			static COLORREF clrPen = 0;
			if (hPen && c != clrPen) {
				DeleteObject(hPen);
				hPen = NULL;
			}
			// �y������
			if (!hPen) {
				hPen = CreatePen(PS_SOLID, 1, clrPen = c);
			}
			// �`��
			HPEN hpnOld = (HPEN)SelectObject(hdc, hPen);
			::MoveToEx(hdc, x, y, NULL);
			::LineTo(hdc, x + 1, y + 1);
			SelectObject(hdc, hpnOld);
		}
	}
}

