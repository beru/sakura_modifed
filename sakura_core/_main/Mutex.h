/*!	@file
	@brief Mutex管理
*/

#pragma once

#include <Windows.h>

/** ミューテックスを扱うクラス */
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

/**	スコープから抜けると同時にロックを解除する．

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

