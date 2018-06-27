
//=====================================================================
// 
// shadow_yuan@qq.com
// shenzhen 5/23 2018
//
//=====================================================================

#pragma once

template<typename dst_type, typename src_type>
dst_type union_cast(src_type src) {
	union {
		src_type src;
		dst_type dst;
	}u;
	u.src = src;
	return u.dst;
};

#ifdef _WIN32
#include <Windows.h>

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
#endif