#pragma once

bool GetDateTimeFormat(TCHAR* szResult, int size, const TCHAR* format, const SYSTEMTIME& systime);
UINT32 ParseVersion(const TCHAR* ver);	// �o�[�W�����ԍ��̉��
int CompareVersion(const TCHAR* verA, const TCHAR* verB);	// �o�[�W�����ԍ��̔�r

