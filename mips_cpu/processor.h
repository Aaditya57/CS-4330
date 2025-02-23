#include "memory.h"
#include "regfile.h"
#include "ALU.h"
#include "control.h"
class Processor {
    private:
        int opt_level;
        ALU alu;
        control_t control;
        Memory *memory;
        Registers regfile;
        // add other structures as needed

        // pipelined processor

	struct IF_ID{
		uint32_t instruction; //obvious
	};
   
	struct ID_EX{
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

        // add private functions
        void single_cycle_processor_advance();
        void pipelined_processor_advance();

	void detect_data_hazard();
	void detect_control_hazard();
 
    public:
        Processor(Memory *mem) { regfile.pc = 0; memory = mem;}

        // Get PC
        uint32_t getPC() { return regfile.pc; }

        // Prints the Register File
        void printRegFile() { regfile.print(); }
        
        // Initializes the processor appropriately based on the optimization level
        void initialize(int opt_level);

        // Advances the processor to an appropriate state every cycle
        void advance(); 

	// Pipeline stages as functions, should be self explanatory

	void pipelined_fetch();

	void pipelined_decode();

	void pipelined_execute();
	
	void pipelined_mem();

	void pipelined_wb();
	
};
