#pragma once

#include "Eol.h"

class EditDoc;

class DocVisitor {
public:
	DocVisitor(EditDoc& doc) : doc(doc) { }

	void SetAllEol(Eol eol); // 改行コードを統一する
private:
	EditDoc& doc;
};

