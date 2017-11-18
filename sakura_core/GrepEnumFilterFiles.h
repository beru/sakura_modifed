#pragma once

#include "GrepEnumFiles.h"

class GrepEnumFilterFiles : public GrepEnumFiles {
private:

public:
	GrepEnumFiles grepEnumExceptFiles;

public:
	GrepEnumFilterFiles() {
	}

	virtual ~GrepEnumFilterFiles() {
	}

	virtual
	bool IsValid(WIN32_FIND_DATA& w32fd, LPCTSTR pFile = NULL) override {
		if (GrepEnumFiles::IsValid(w32fd, pFile)) {
			if (grepEnumExceptFiles.IsValid(w32fd, pFile)) {
				return true;
			}
		}
		return false;
	}

	int Enumerates(
		LPCTSTR lpBaseFolder,
		const GrepEnumKeys& grepEnumKeys,
		const GrepEnumOptions& option,
		GrepEnumFiles& pExcept
		)
	{
		grepEnumExceptFiles.Enumerates(lpBaseFolder, grepEnumKeys.vecExceptFileKeys, option, nullptr);
		return GrepEnumFiles::Enumerates(lpBaseFolder, grepEnumKeys.vecSearchFileKeys, option, &pExcept);
	}
};

