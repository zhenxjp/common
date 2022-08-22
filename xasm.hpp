#pragma once
#pragma pack(push, 1)
// 写入各种汇编指令

//JMP rel32
class jmp_rel32
{
public:
	struct asm_cmd {
		uint8_t opcode_;
		int32_t rel32_;
	};

	static void write(void* cmd_addr, int32_t rel32) {
		auto* cmd = (asm_cmd*)cmd_addr;

		cmd->opcode_ = 0xe9;
		cmd->rel32_ = rel32;
	}

	static uint8_t size() { return sizeof(asm_cmd); }
};


// PUSH imm32
class push_imm32
{
public:
	struct asm_cmd {
		uint8_t  opcode_;
		uint32_t imm32_;
	};

	static void write(void* cmd_addr, uint32_t imm32) {
		auto* cmd = (asm_cmd*)cmd_addr;

		cmd->opcode_ = 0x68;//jmp
		cmd->imm32_ = imm32;
	}

	static uint8_t size() { return sizeof(asm_cmd); }
};


// mov dword ptr[rsp + offset],imm32
class mov_rsp_ptr_imm32
{
public:
	struct asm_cmd {
		uint8_t  opcode_;
		uint8_t  para1_;
		uint8_t  reg_type_;
		int8_t  offset_;
		uint32_t imm32_;
	};

	static void write(void* cmd_addr, int8_t off, uint32_t imm32) {
		auto* cmd = (asm_cmd*)cmd_addr;

		cmd->opcode_ = 0xc7;//mov
		cmd->para1_ = 0x44;// to reg ptr
		cmd->reg_type_ = 0x24;//rsp
		cmd->offset_ = off;
		cmd->imm32_ = imm32;
	}

	static uint8_t size() { return sizeof(asm_cmd); }
};

// ret
class ret
{
public:
	struct asm_cmd {
		uint8_t  opcode_;
	};

	static void write(void* cmd_addr) {
		auto* cmd = (asm_cmd*)cmd_addr;

		cmd->opcode_ = 0xc3;
	}

	static uint8_t size() { return sizeof(asm_cmd); }
};



#pragma pack(pop)



