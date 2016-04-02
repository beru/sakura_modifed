/*
	Copyright (C) 2011, Moca

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

#include "util/design_template.h"

class LayoutColorInfo {
public:
	LayoutColorInfo() {}
	virtual ~LayoutColorInfo() {};
	virtual bool IsEqual(const LayoutColorInfo*) const = 0;
};


class LayoutExInfo {
public:
	LayoutExInfo() : m_pColorInfo(NULL) {}
	~LayoutExInfo() {
		delete m_pColorInfo;
	}
	void SetColorInfo(LayoutColorInfo* p) {
		if (m_pColorInfo) {
			delete m_pColorInfo;
		}
		m_pColorInfo = p;
	}
	const LayoutColorInfo* GetColorInfo() const {
		return m_pColorInfo;
	}
	LayoutColorInfo* DetachColorInfo() {
		LayoutColorInfo* p = m_pColorInfo;
		m_pColorInfo = nullptr;
		return p;
	}
private:
	LayoutColorInfo* m_pColorInfo;

private:
	DISALLOW_COPY_AND_ASSIGN(LayoutExInfo);
};

