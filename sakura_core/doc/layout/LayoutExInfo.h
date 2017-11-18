#pragma once

#include "util/design_template.h"

class LayoutColorInfo {
public:
	LayoutColorInfo() {}
	virtual ~LayoutColorInfo() {};
	virtual bool IsEqual(const LayoutColorInfo*) const = 0;
};


class LayoutExInfo {
public:
	LayoutExInfo() : pColorInfo(NULL) {}
	~LayoutExInfo() {
		delete pColorInfo;
	}
	void SetColorInfo(LayoutColorInfo* p) {
		if (pColorInfo) {
			delete pColorInfo;
		}
		pColorInfo = p;
	}
	const LayoutColorInfo* GetColorInfo() const {
		return pColorInfo;
	}
	LayoutColorInfo* DetachColorInfo() {
		LayoutColorInfo* p = pColorInfo;
		pColorInfo = nullptr;
		return p;
	}
private:
	LayoutColorInfo* pColorInfo;

private:
	DISALLOW_COPY_AND_ASSIGN(LayoutExInfo);
};

