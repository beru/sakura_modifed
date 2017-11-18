#pragma once

// �v��s��`
// #include "DllSharedData.h"

// �����Ǘ�
class FormatManager {
public:
	FormatManager() {
		pShareData = &GetDllShareData();
	}
	// ����
	// ���LDllSharedData�ˑ�
	const TCHAR* MyGetDateFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen);
	const TCHAR* MyGetTimeFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen);

	// ���LDllSharedData��ˑ�
	const TCHAR* MyGetDateFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nDateFormatType, const TCHAR* szDateFormat);
	const TCHAR* MyGetTimeFormat(const SYSTEMTIME& systime, TCHAR* pszDest, int nDestLen, int nTimeFormatType, const TCHAR* szTimeFormat);
private:
	DllSharedData* pShareData;
};

