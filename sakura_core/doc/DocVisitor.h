#pragma once

#include "Eol.h"

class EditDoc;

class DocVisitor {
public:
	DocVisitor(EditDoc& doc) : doc(doc) { }

	void SetAllEol(Eol eol); // ���s�R�[�h�𓝈ꂷ��
private:
	EditDoc& doc;
};

