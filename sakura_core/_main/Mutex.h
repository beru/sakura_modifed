/*!	@file
	@brief Mutex�Ǘ�

	@author ryoji
	@date 2007.07.05
*/
/*
	Copyright (C) 2007, ryoji, genta

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

#include <Windows.h>

/** �~���[�e�b�N�X�������N���X
	@date 2007.07.05 ryoji �V�K�쐬
*/
class Mutex {
public:
	Mutex(
		BOOL bInitialOwner,
		LPCTSTR pszName,
		LPSECURITY_ATTRIBUTES psa = NULL
	) {
		hObj = ::CreateMutex(psa, bInitialOwner, pszName);
	}
	~Mutex() {
		if (hObj) {
			::CloseHandle(hObj);
			hObj = NULL;
		}
	}
	bool Lock(DWORD dwTimeout = INFINITE) {
		DWORD dwRet = ::WaitForSingleObject(hObj, dwTimeout);
		return (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED);
	}
	bool Unlock() {
		return ::ReleaseMutex(hObj) != 0;
	}
	operator HANDLE() const { return hObj; }
protected:
	HANDLE hObj;
};

/**	�X�R�[�v���甲����Ɠ����Ƀ��b�N����������D

	@date 2007.07.07 genta �V�K�쐬

	@code
	Mutex aMutex;
	
    void function()
    {
        //  other processing
        {
            LockGuard<Mutex> aGuard(aMutex);
            //  aMutex is locked
            //  do something protected by "aMutex"

        } // aMutex is automatically released
        //  other processing
    }
	@endcode
*/
template <class EXCLUSIVE_OBJECT>
class LockGuard {
	EXCLUSIVE_OBJECT& o_;
public:
	LockGuard(EXCLUSIVE_OBJECT& ex) : o_(ex) {
		o_.Lock();
	}
	template <class PARAM>
	LockGuard(EXCLUSIVE_OBJECT& ex, PARAM p) : o_(ex) {
		o_.Lock(p);
	}
	
	~LockGuard() {
		o_.Unlock();
	}
};

