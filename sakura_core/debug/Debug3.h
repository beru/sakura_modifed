#pragma once

// DebugMonitorLib用関数

#ifdef USE_DEBUGMON
int DebugMonitor_Output(const wchar_t* szInstanceId, const wchar_t* szText);
LPCWSTR GetWindowsMessageName(UINT msg);
#endif

