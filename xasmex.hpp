#pragma once
#include "xasm.hpp"
#include "xcom.hpp"

// ret：两个地址的偏移
static int64_t xx_get_offset(void* src, void* dst) {
	return (char*)dst - (char*)src;
}

// ret：是否超过int32范围
static bool xx_int32_overflow(int64_t val) {
	return val < INT32_MIN || val > INT32_MAX;
}

static int64_t xx_get_jmp32_offset(void* src, void* dst) {
	return xx_get_offset((char*)src + jmp_rel32::size(), dst);
}


/// ////////////////////////////////////////////////////////////////

static uint32_t xx_setjmp32(void* src, void* dst) {
	int32_t offset = (int32_t)xx_get_jmp32_offset(src, dst);
	jmp_rel32::write(src, offset);
	return jmp_rel32::size();
}

static uint32_t xx_setjmp64(void* src, void* dst) {
	char* cmd_addr = (char*)src;

	push_imm32::write(cmd_addr, (uint32_t)(uintptr_t)dst);
	cmd_addr += push_imm32::size();

	mov_rsp_ptr_imm32::write(cmd_addr, 4, (uint32_t)(((uintptr_t)dst) >> 32));
	cmd_addr += mov_rsp_ptr_imm32::size();

	ret::write(cmd_addr);

	return push_imm32::size() + mov_rsp_ptr_imm32::size() + ret::size();
}

static uint32_t xx_setjmp(void* src, void* dst) {

	int64_t dis = xx_get_jmp32_offset(src, dst);
	if (xx_int32_overflow(dis))
		return xx_setjmp64(src, dst);
	else
		return xx_setjmp32(src, dst);
}