#pragma once

#include "mem/Memory.h"

// ※Memoryをprotect継承することにより、あまり自由にMemoryを使えないようにしておく
class Native : protected Memory {
public:
	// Memory*ポインタを得る
	Memory* _GetMemory() { return static_cast<Memory*>(this); }
	const Memory* _GetMemory() const { return static_cast<const Memory*>(this); }

public:
	// 汎用
	void Clear(); // 空っぽにする
};

#include "mem/NativeA.h"
#include "mem/NativeW.h"

