#include "StdAfx.h"
#include "DocFile.h"

/*
	�ۑ����̃t�@�C���̃p�X�i�}�N���p�j�̎擾
*/
const TCHAR* DocFile::GetSaveFilePath(void) const
{
	if (szSaveFilePath.IsValidPath()) {
		return szSaveFilePath;
	}else {
		return GetFilePath();
	}
}

