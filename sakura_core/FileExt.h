#pragma once

// �I�[�v���_�C�A���O�p�t�@�C���g���q�Ǘ�

#include "_main/global.h"
#include "config/maxdata.h"
#include "util/design_template.h"

class FileExt {
public:
	FileExt();
	~FileExt();

	bool AppendExt(const TCHAR* pszName, const TCHAR* pszExt);
	bool AppendExtRaw(const TCHAR* pszName, const TCHAR* pszExt);
	const TCHAR* GetName(int nIndex);
	const TCHAR* GetExt(int nIndex);

	// �_�C�A���O�ɓn���g���q�t�B���^���擾����B(lpstrFilter�ɒ��ڎw��\)
	//2��Ăяo���ƌÂ��o�b�t�@�������ɂȂ邱�Ƃ�����̂ɒ���
	const TCHAR* GetExtFilter(void);

	int GetCount(void) { return nCount; }

protected:
	// 2014.10.30 syat ConvertTypesExtToDlgExt��CDocTypeManager�Ɉړ�
	//bool ConvertTypesExtToDlgExt( const TCHAR *pszSrcExt, TCHAR *pszDstExt );

private:
	struct FileExtInfoTag {
		TCHAR	szName[64];						// ���O(64�����ȉ��̂͂���szTypeName)
		TCHAR	szExt[MAX_TYPES_EXTS * 3 + 1];	// �g���q(64�����ȉ��̂͂���szTypeExts) �Ȃ� "*." ��ǉ�����̂ł���Ȃ�ɕK�v
	};

	int nCount;
	FileExtInfoTag*	puFileExtInfo;
	std::vector<TCHAR>	vstrFilter;

private:
	DISALLOW_COPY_AND_ASSIGN(FileExt);
};

