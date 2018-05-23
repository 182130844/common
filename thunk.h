#pragma once
#include <Windows.h>

// shenzhen
// 5/23 2018
// shadow_yuan@qq.com

#pragma pack(push, 1)

struct ThunkData {
	unsigned char mov_;   
	unsigned long this_; // mov ecx, this
	unsigned char jmp_;
	unsigned long addr_;// jump addr

	bool Init(unsigned long proc, void* pthis) {
		mov_ = 0xB9; // mov ecx
		this_ = (unsigned long)pthis;
		jmp_ = 0xE9; // jmp
		addr_ = (unsigned long)proc - (unsigned long)this - sizeof(ThunkData);
		unsigned long flOldProtect = 0;
		VirtualProtect((LPVOID)this, sizeof(ThunkData), PAGE_EXECUTE_READWRITE, &flOldProtect);
		FlushInstructionCache(GetCurrentProcess(), this, sizeof(ThunkData));
		return true;
	}

	void* GetCodeAddress() {
		return this;
	}
};

#pragma pack(pop)