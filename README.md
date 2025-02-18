MIPS Processor Emulator for UVA CS-4330 (Advanced Computer Architecture)

  This repository holds a fork of an existing code base representing an emulation of a 32-bit MIPS processor. This fork will incremenally be updated with custom optimizations as part of the semester long project.

The main branch contains the stable version of the latest incremental optimization.

Status:

    -Current optimization: none (single-cycle)
    -WIP optimization: pipelining

Known issues to fix:
  
    -pipeline increments as expected, data seems to be arbitrarily overwritten from registers
    
    -all r-type instructions store values in r[0], which likely contributes to the above
    
    -no fault or dependency handling is implemented.
