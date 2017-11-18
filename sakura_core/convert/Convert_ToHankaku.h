#pragma once

#include "Convert.h"


// ���p�ɂł�����̂͑S�����p�ɕϊ�
class Converter_ToHankaku : public Converter {
public:
	bool DoConvert(NativeW* pData);
};

enum class ToHankakuMode {
	Katakana	= 0x01, // �J�^�J�i�ɉe���A��
	Hiragana	= 0x02, // �Ђ炪�Ȃɉe���A��
	Alnum		= 0x04, // �p�����ɉe���A��
};

