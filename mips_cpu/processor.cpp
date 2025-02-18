#include <cstdint>
#include <iostream>
#include "processor.h"
using namespace std;

#ifdef ENABLE_DEBUG
#define DEBUG(x) x
#else
#define DEBUG(x) 
#endif

void Processor::initialize(int level) {
	// Initialize Control
	control = {.reg_dest = 0, 
			   .jump = 0,
			   .jump_reg = 0,
			   .link = 0,
			   .shift = 0,
			   .branch = 0,
			   .bne = 0,
			   .mem_read = 0,
			   .mem_to_reg = 0,
			   .ALU_op = 0,
			   .mem_write = 0,
			   .halfword = 0,
			   .byte = 0,
			   .ALU_src = 0,
			   .reg_write = 0,
			   .zero_extend = 0};
   
	opt_level = level;
	// Optimization level-specific initialization
}

void Processor::advance() {
	switch (opt_level) {
		case 0: single_cycle_processor_advance();
				break;
		case 1: pipelined_processor_advance();
				break;
		// other optimization levels go here
		default: break;
	}
}

void Processor::single_cycle_processor_advance() {
	// fetch
	uint32_t instruction;
	memory->access(regfile.pc, instruction, 0, 1, 0);
	DEBUG(cout << "\nPC: 0x" << std::hex << regfile.pc << std::dec << "\n");
	// increment pc
	regfile.pc += 4;
	
	// decode into contol signals
	control.decode(instruction);
	DEBUG(control.print());

	// extract rs, rt, rd, imm, funct 
	int opcode = (instruction >> 26) & 0x3f;
	int rs = (instruction >> 21) & 0x1f;
	int rt = (instruction >> 16) & 0x1f;
	int rd = (instruction >> 11) & 0x1f;
	int shamt = (instruction >> 6) & 0x1f;
	int funct = instruction & 0x3f;
	uint32_t imm = (instruction & 0xffff);
	int addr = instruction & 0x3ffffff;
	// Variables to read data into
	uint32_t read_data_1 = 0;
	uint32_t read_data_2 = 0;
	
	// Read from reg file
	regfile.access(rs, rt, read_data_1, read_data_2, 0, 0, 0);
	
	// Execution 
	alu.generate_control_inputs(control.ALU_op, funct, opcode);
   
	// Sign Extend Or Zero Extend the immediate
	// Using Arithmetic right shift in order to replicate 1 
	imm = control.zero_extend ? imm : (imm >> 15) ? 0xffff0000 | imm : imm;
	
	// Find operands for the ALU Execution
	// Operand 1 is always R[rs] -> read_data_1, except sll and srl
	// Operand 2 is immediate if ALU_src = 1, for I-type
	uint32_t operand_1 = control.shift ? shamt : read_data_1;
	uint32_t operand_2 = control.ALU_src ? imm : read_data_2;
	uint32_t alu_zero = 0;

	uint32_t alu_result = alu.execute(operand_1, operand_2, alu_zero);
	
	
	uint32_t read_data_mem = 0;
	uint32_t write_data_mem = 0;

	// Memory
	// First read no matter whether it is a load or a store
	memory->access(alu_result, read_data_mem, 0, control.mem_read | control.mem_write, 0);
	// Stores: sb or sh mask and preserve original leftmost bits
	write_data_mem = control.halfword ? (read_data_mem & 0xffff0000) | (read_data_2 & 0xffff) : 
					control.byte ? (read_data_mem & 0xffffff00) | (read_data_2 & 0xff): read_data_2;
	// Write to memory only if mem_write is 1, i.e store
	memory->access(alu_result, read_data_mem, write_data_mem, control.mem_read, control.mem_write);
	// Loads: lbu or lhu modify read data by masking
	read_data_mem &= control.halfword ? 0xffff : control.byte ? 0xff : 0xffffffff;

	int write_reg = control.link ? 31 : control.reg_dest ? rd : rt;

	uint32_t write_data = control.link ? regfile.pc+8 : control.mem_to_reg ? read_data_mem : alu_result;  

	// Write Back
	regfile.access(0, 0, read_data_2, read_data_2, write_reg, control.reg_write, write_data);
	
	// Update PC
	regfile.pc += (control.branch && !control.bne && alu_zero) || (control.bne && !alu_zero) ? imm << 2 : 0; 
	regfile.pc = control.jump_reg ? read_data_1 : control.jump ? (regfile.pc & 0xf0000000) & (addr << 2): regfile.pc;
}

void Processor::pipelined_fetch(){
	memory->access(regfile.pc, state.fetchDecode.instruction, 0, 1, 0); //fetch instruction into IF/ID reg

	DEBUG(cout << "\nPC: 0x" << std::hex << regfile.pc << std::dec << "\n");
	// increment pc
	regfile.pc += 4;
}

void Processor::pipelined_decode(){
	// decode into contol signals
	control.decode(prevState.fetchDecode.instruction); //reach back to IF/ID for instruction
	DEBUG(control.print());

	uint32_t instruction = prevState.fetchDecode.instruction; //extract instruction from IF/ID

	// extract rs, rt, rd, imm, funct 
	state.decodeExe.opcode = (instruction >> 26) & 0x3f; //send opcode to ID/EX

	//push reg values to next pipeline reg (ID/EX)
	int rs = state.decodeExe.rs = (instruction >> 21) & 0x1f;
	int rt = state.decodeExe.rt = (instruction >> 16) & 0x1f;
	state.decodeExe.rd = (instruction >> 11) & 0x1f;

	//send instruction specific values to ID/EX
	//(note, not all of these will be used)
	state.decodeExe.shamt = (instruction >> 6) & 0x1f;
	state.decodeExe.funct = instruction & 0x3f;
	state.decodeExe.imm = (instruction & 0xffff);
	state.decodeExe.addr = instruction & 0x3ffffff;
	
	// Variables to read data into
	state.decodeExe.read_data_1 = 0;
	state.decodeExe.read_data_2 = 0;
	
	// Read from reg file
	regfile.access(rs, rt, state.decodeExe.read_data_1, state.decodeExe.read_data_2, 0, 0, 0);
}

void Processor::pipelined_execute(){
	// Execution 
	alu.generate_control_inputs(control.ALU_op, prevState.decodeExe.funct, prevState.decodeExe.opcode); //generate control inputs
											  //from decodeExe vals
	// Sign Extend Or Zero Extend the immediate
	// Using Arithmetic right shift in order to replicate 1 
	int imm = prevState.decodeExe.imm;
	prevState.decodeExe.imm = control.zero_extend ? imm : (imm >> 15) ? 0xffff0000 | imm : imm; //line loks weird, check later (why redefine imm?)
	
	// Find operands for the ALU Execution
	// Operand 1 is always R[rs] -> read_data_1, except sll and srl
	// Operand 2 is immediate if ALU_src = 1, for I-type
	uint32_t operand_1 = state.exeMem.operand_1 = control.shift ? state.decodeExe.shamt : state.decodeExe.read_data_1;
	uint32_t operand_2 = state.exeMem.operand_2 = control.ALU_src ? state.decodeExe.imm : state.decodeExe.read_data_2;
	state.exeMem.alu_zero = 0;

	state.exeMem.alu_result = alu.execute(operand_1, operand_2, state.exeMem.alu_zero);
	
	
	state.exeMem.read_data_mem = 0;
	state.exeMem.write_data_mem = 0;
}

void Processor::pipelined_mem(){
	// Memory
	// First read no matter whether it is a load or a store

	memory->access(prevState.exeMem.alu_result, prevState.exeMem.read_data_mem, 0, control.mem_read | control.mem_write, 0);
	
	// Stores: sb or sh mask and preserve original leftmost bits
	prevState.exeMem.write_data_mem = control.halfword ? (prevState.exeMem.read_data_mem & 0xffff0000) | (prevState.decodeExe.read_data_2 & 0xffff) : 
					control.byte ? (prevState.exeMem.read_data_mem & 0xffffff00) | (prevState.decodeExe.read_data_2 & 0xff): prevState.decodeExe.read_data_2;
	// Write to memory only if mem_write is 1, i.e store
	memory->access(prevState.exeMem.alu_result, prevState.exeMem.read_data_mem, prevState.exeMem.write_data_mem, 
								control.mem_read, control.mem_write);

	// Loads: lbu or lhu modify read data by masking
	state.exeMem.read_data_mem &= control.halfword ? 0xffff : control.byte ? 0xff : 0xffffffff;

	state.memWrite.write_reg = control.link ? 31 : control.reg_dest ? prevState.decodeExe.rd : prevState.decodeExe.rt;

	state.memWrite.write_data = control.link ? regfile.pc+8 : control.mem_to_reg ? prevState.exeMem.read_data_mem : prevState.exeMem.alu_result;  
}

void Processor::pipelined_writeb(){
	// Write Back
	regfile.access(0, 0, prevState.decodeExe.read_data_2, prevState.decodeExe.read_data_2, prevState.memWrite.write_reg, control.reg_write, prevState.memWrite.write_data);
	
	// Update PC
	regfile.pc += (control.branch && !control.bne && state.exeMem.alu_zero) || (control.bne && !prevState.exeMem.alu_zero) ? prevState.decodeExe.imm << 2 : 0; 
	regfile.pc = control.jump_reg ? prevState.decodeExe.read_data_1 : control.jump ? (regfile.pc & 0xf0000000) & (prevState.decodeExe.addr << 2): regfile.pc;

}


void Processor::pipeline_cycle(){
	//state = prevState; //update the state (regs) to contrain the state from the previous cycle
	prevState = state;

	pipelined_fetch();
	pipelined_decode();
	pipelined_execute();
	pipelined_mem();
	pipelined_writeb();
}

void Processor::pipelined_processor_advance() {
	pipeline_cycle();
}

