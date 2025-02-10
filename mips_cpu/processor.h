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
		uint32_t instruction; //raw instruction
	};
   
	struct ID_EX{
		int opcode; //store the raw decoded opcode

		//instruction specific fields//
		int rs, rt, rd; //store operand registers
		int shamt, funct; //store R-Type vals
		uint32_t imm; //store I-Type immediate
		int addr; //address for J-Type instructions

		uint32_t read_data_1;
		uint32_t read_data_2; //store results of read data (?)			
	};
    

	struct EX_MEM{
		uint32_t operand_1; //operands for write back (?)
		uint32_t operand_2;

		uint32_t alu_zero; //alu zero signal

		uint32_t alu_result; //result of calculation

		uint32_t read_data_mem;
		uint32_t write_data_mem; //signals
	};
	
	struct MEM_WB{
		int write_reg; //register to write back
		uint32_t write_data; //data to write back		
	};
	
	//allow access to correct pipeline registers across
	//function contexts

	struct pipelineState{
		IF_ID fetchDecode;
		ID_EX decodeExe;
		EX_MEM exeMem;
		MEM_WB memWrite;
	};

	pipelineState state;

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

	void pipelined_writeb();
	
	void pipeline_cycle();	
};
