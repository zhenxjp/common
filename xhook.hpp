#pragma once
#include "xasmex.hpp"


// 偏移重定位,从src+val_offset获取绝对addr，写入trampoline+val_offset
// val_offset:偏移的位移
static bool xx_reloc_offset(void* src, void* trampoline, uint32_t val_offset)
{
	// 获取绝对地址
	int32_t* src_offset = (int32_t*)((char*)src + val_offset);
	char* src_next_cmd = (char*)src + val_offset + sizeof(int32_t);
	char* real_addr = src_next_cmd + *src_offset;

	// 判断trampoline相对于绝对地址的偏移是否超过int32
	char* tra_next_cmd = (char*)trampoline + val_offset + sizeof(int32_t);
	int64_t tra_offset_val = xx_get_offset(tra_next_cmd, real_addr);
	if (xx_int32_overflow(tra_offset_val)) {
		return false;
	}

	// trampoline重定位
	int32_t* tra_offset = (int32_t*)((char*)trampoline + val_offset);
	*tra_offset = (int32_t)tra_offset_val;
	return true;
}

// 制作跳板，hook以后再跳回去
// ret:跳板长度
static uint32_t xx_make_trampoline(void* src, void* trampoline, uint32_t copy_len) {
	memcpy(trampoline, src, copy_len);
	return copy_len + xx_setjmp((char*)trampoline + copy_len, (char*)src + copy_len);
}

// 方便把跳板转为函数指针
template <class T>
static T* xx_trampoline_to_func(T* func_type, char* trampoline)
{
	return (T*)(trampoline);
}

// 借助跳板恢复函数
static void xx_recover_src(void* src, void* trampoline, uint32_t copy_len) {
	memcpy(src, trampoline, copy_len);
}