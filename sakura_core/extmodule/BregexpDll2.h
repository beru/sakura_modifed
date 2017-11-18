#pragma once

#include "DllHandler.h"

typedef struct bregexp {
	const wchar_t* outp;	// result string start ptr
	const wchar_t* outendp;	// result string end ptr
	const int   splitctr;	// split result counter
	const wchar_t** splitp;	// split result pointer ptr
	int rsv1;				// reserved for external use
	wchar_t* parap;			// parameter start ptr ie. "s/xxxxx/yy/gi"
	wchar_t* paraendp;		// parameter end ptr
	wchar_t* transtblp;		// translate table ptr
	wchar_t** startp;		// match string start ptr
	wchar_t** endp;			// match string end ptr
	int nparens;			// number of parentheses
} BREGEXP_W;

// BREGONIG.DLL�����b�v�������́B
// 2007.09.13 kobake �쐬
class BregexpDll2 : public DllImp {
public:
	BregexpDll2();
	virtual ~BregexpDll2();

protected:
	// DllImp�C���^�t�F�[�X
	virtual LPCTSTR GetDllNameImp(int nIndex); // Jul. 5, 2001 genta �C���^�[�t�F�[�X�ύX�ɔ��������ǉ�
	virtual bool InitDllImp();

protected:
	// DLL�֐��̌^
	typedef int            (__cdecl *BREGEXP_BMatchW2)        (const wchar_t* str, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef int            (__cdecl *BREGEXP_BSubstW2)        (const wchar_t* str, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef int            (__cdecl *BREGEXP_BTransW2)        (const wchar_t* str, wchar_t* target, wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef int            (__cdecl *BREGEXP_BSplitW2)        (const wchar_t* str, wchar_t* target, wchar_t* targetendp, int limit, BREGEXP_W** rxp, wchar_t* msg);
	typedef void           (__cdecl *BREGEXP_BRegfreeW2)      (BREGEXP_W* rx);
	typedef const wchar_t* (__cdecl *BREGEXP_BRegexpVersionW2)(void);
	typedef int            (__cdecl *BREGEXP_BMatchExW2)      (const wchar_t* str, const wchar_t* targetbeg, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);
	typedef int            (__cdecl *BREGEXP_BSubstExW2)      (const wchar_t* str, const wchar_t* targetbeg, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg);

public:
	// UNICODE�C���^�[�t�F�[�X��񋟂���
	int BMatch(const wchar_t* str, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg)
	{
		return pBMatch(str, target, targetendp, rxp, msg);
	}
	int BSubst(const wchar_t* str, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg)
	{
		return pBSubst(str, target, targetendp, rxp, msg);
	}
	int BTrans(const wchar_t* str, wchar_t* target, wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg)
	{
		return pBTrans(str, target, targetendp, rxp, msg);
	}
	int BSplit(const wchar_t* str, wchar_t* target, wchar_t* targetendp, int limit, BREGEXP_W** rxp, wchar_t* msg)
	{
		return pBSplit(str, target, targetendp, limit, rxp, msg);
	}
	void BRegfree(BREGEXP_W* rx)
	{
		return pBRegfree(rx);
	}
	const wchar_t* BRegexpVersion(void)
	{
		return pBRegexpVersion();
	}
	int BMatchEx(const wchar_t* str, const wchar_t* targetbeg, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg)
	{
		return pBMatchEx(str, targetbeg, target, targetendp, rxp, msg);
	}
	int BSubstEx(const wchar_t* str, const wchar_t* targetbeg, const wchar_t* target, const wchar_t* targetendp, BREGEXP_W** rxp, wchar_t* msg)
	{
		return pBSubstEx(str, targetbeg, target, targetendp, rxp, msg);
	}

	// �֐������邩�ǂ���
	bool ExistBMatchEx() const { return pBMatchEx != nullptr; }
	bool ExistBSubstEx() const { return pBSubstEx != nullptr; }

private:
	// DLL���֐��|�C���^
	BREGEXP_BMatchW2         pBMatch;
	BREGEXP_BSubstW2         pBSubst;
	BREGEXP_BTransW2         pBTrans;
	BREGEXP_BSplitW2         pBSplit;
	BREGEXP_BRegfreeW2       pBRegfree;
	BREGEXP_BRegexpVersionW2 pBRegexpVersion;
	BREGEXP_BMatchExW2       pBMatchEx;
	BREGEXP_BSubstExW2       pBSubstEx;
};

