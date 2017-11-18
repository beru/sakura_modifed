#pragma once

#include "GrepEnumFileBase.h"

class GrepEnumFolders : public GrepEnumFileBase {
private:

public:
	GrepEnumFolders() {
	}

	virtual
	~GrepEnumFolders() {
	}

	virtual
	bool IsValid(WIN32_FIND_DATA& w32fd, LPCTSTR pFile = NULL) override {
		if (1
			&& (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			&& (_tcscmp(w32fd.cFileName, _T(".")) != 0)
			&& (_tcscmp(w32fd.cFileName, _T("..")) != 0)
		) {
			if (GrepEnumFileBase::IsValid(w32fd, pFile)) {
				return true;
			}
		}
		return false;
	}
};

