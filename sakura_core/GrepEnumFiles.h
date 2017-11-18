#pragma once

#include "GrepEnumFileBase.h"

class GrepEnumFiles : public GrepEnumFileBase {
private:

public:
	GrepEnumFiles() {
	}

	virtual
	~GrepEnumFiles() {
	}

	virtual
	bool IsValid(WIN32_FIND_DATA& w32fd, LPCTSTR pFile = NULL) {
		if (!(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if (GrepEnumFileBase::IsValid(w32fd, pFile)) {
				return true;
			}
		}
		return false;
	}
};

