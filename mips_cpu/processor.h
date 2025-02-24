#include "memory.h"
#include "regfile.h"
#include "ALU.h"
#include "control.h"

class Processor {
	//forwarding unit
				   
	private:
	unsigned int stall = 0; 
		int opt_level;
		ALU alu;
		control_t control;
		Memory *memory;
		Registers regfile;
		//add other structures as needed

		//pipelined processor

	struct IF_ID{
		uint32_t instruction; //obvious
		uint32_t pc;
	};
   
	struct ID_EX{
		uint32_t forward_a;
			uint32_t forward_b;

		int opcode;
		int rs, rt, rd;

		uint32_t shamt; //?
		uint32_t funct; //function bits
		uint32_t imm; //immediate value
		uint32_t addr; //address value

		uint32_t read_data_1, read_data_2; //product of
	
		control_t control; //preserve control across signals cycles
	};
	

	struct EX_MEM{
		
		uint32_t imm; //persits imm from previous stage

		uint32_t read_data_1, read_data_2; //persist from prev stage
		int rd, rt; //registers
		uint32_t operand_1, operand_2; //operands from prev stage
		uint32_t alu_zero; //same
		uint32_t addr; //address for mem access
		uint32_t alu_result;
		
		control_t control; //preserve control across signals cycles

	};
	
	struct MEM_WB{
		int write_reg; //reg to write to
		uint32_t write_data; //data to write
		
		uint32_t imm;
		uint32_t addr;
		uint32_t alu_zero;
		uint32_t read_data_1, read_data_2;
		
		control_t control; //preserve control across signals cycles
	
	};
	
	//allow access to correct pipeline registers across
	//function contexts

	struct pipelineState{
		IF_ID fetchDecode;
		ID_EX decExe;
		EX_MEM exeMem;
		MEM_WB memWrite;
	};

	pipelineState state;
	pipelineState prevState; //store the previous state so we can simulate shared state across contexts
	
		//add private functions
		void single_cycle_processor_advance();
		void pipelined_processor_advance();

	void detect_data_hazard(){
		//detect load/use
		if (prevState.decExe.control.mem_read && 
			((prevState.decExe.rt == state.decExe.rs) || 
		 	(prevState.decExe.rt == state.decExe.rt))){
				stall = 2; //load use requires stall of one cycle, check stall logic
				return;
		}
		stall = 0;
	}

	int get_forwarding_a(){
		if (!prevState.decExe.rs || !prevState.decExe.rd)
			return 0;

		//Forward from MEM stage
		if (prevState.exeMem.control.reg_write &&  //reg_write signal set
			((prevState.exeMem.rt ==  prevState.decExe.rs) || //I type	
			//prevState.exeMem.rd != 0 && 
			(prevState.exeMem.rd == prevState.decExe.rs))){  //R type
				return 1; //forward from mem
		} 

		if (prevState.memWrite.control.reg_write &&
			(prevState.memWrite.write_reg == prevState.decExe.rs) &&
			((prevState.exeMem.rd != prevState.decExe.rs) || (!prevState.exeMem.control.reg_write)))
				return 2;
	
		return 0; //base case, no forwarding required
	}

	int get_forwarding_b(){
		if (!prevState.decExe.rt || !prevState.decExe.rd)
			return 0;

		//Forward from MEM stage
		if (prevState.exeMem.control.reg_write &&  //reg_write signal set
		   	((prevState.exeMem.rt == prevState.decExe.rt) || //I type
			(prevState.exeMem.rd == prevState.decExe.rt))) //R type
					return 1; //forward from mem

		//Forward from WB stage
		if (prevState.memWrite.control.reg_write &&
			(prevState.memWrite.write_reg == prevState.decExe.rt) &&
			((prevState.exeMem.rd != prevState.decExe.rt) || (!prevState.exeMem.control.reg_write)))
				return 2;

		return 0;
	}


	void detect_control_hazard();
 
	public:
		Processor(Memory *mem) { regfile.pc = 0; memory = mem;}

		//Get PC
		uint32_t getPC() { return regfile.pc; }

		//Prints the Register File
		void printRegFile() { regfile.print(); }
		
		//Initializes the processor appropriately based on the optimization level
		void initialize(int opt_level);

		//Advances the processor to an appropriate state every cycle
		void advance(); 

		//Pipeline stages as functions, should be self explanatory

		void pipelined_fetch();

		void pipelined_decode();

		void pipelined_execute();
	
		void pipelined_mem();

		void pipelined_wb();

		void flush_pipeline();
	
	};
