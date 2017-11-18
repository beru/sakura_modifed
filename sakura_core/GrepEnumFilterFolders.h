#pragma once

#include "GrepEnumFolders.h"

class GrepEnumFilterFolders : public GrepEnumFolders {
private:

public:
	GrepEnumFolders grepEnumExceptFolders;

public:
	GrepEnumFilterFolders() {
	}

	virtual
	~GrepEnumFilterFolders() {
	}

	virtual
	bool IsValid(WIN32_FIND_DATA& w32fd, LPCTSTR pFile = NULL) override {
		if (GrepEnumFolders::IsValid(w32fd, pFile)) {
			if (grepEnumExceptFolders.IsValid(w32fd, pFile)) {
				return true;
			}
		}
		return false;
	}

	int Enumerates(
		LPCTSTR lpBaseFolder,
		const GrepEnumKeys& grepEnumKeys,
		const GrepEnumOptions& option,
		GrepEnumFolders& except
		)
	{
		grepEnumExceptFolders.Enumerates(lpBaseFolder, grepEnumKeys.vecExceptFolderKeys, option, NULL);
		return GrepEnumFolders::Enumerates(lpBaseFolder, grepEnumKeys.vecSearchFolderKeys, option, &except);
	}
};

