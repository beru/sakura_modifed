#pragma once

#include "mem/Memory.h"

// ��Memory��protect�p�����邱�Ƃɂ��A���܂莩�R��Memory���g���Ȃ��悤�ɂ��Ă���
class Native : protected Memory {
public:
	// Memory*�|�C���^�𓾂�
	Memory* _GetMemory() { return static_cast<Memory*>(this); }
	const Memory* _GetMemory() const { return static_cast<const Memory*>(this); }

public:
	// �ėp
	void Clear(); // ����ۂɂ���
};

#include "mem/NativeA.h"
#include "mem/NativeW.h"

