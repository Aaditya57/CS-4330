  .set noat
    .text
    .align  2
    .globl  __start
    .ent    __start
    .type   __start, @function
__start:
   addi $4, $0, 7
   add $5, $4
   add $6, $4
   addi $0, $0, 0
   addi $0, $0, 0
   addi $0, $0, 0
   addi $0, $0, 0
   addi $0, $0, 0

#label:
#   or $4, $5, $6
    .end    __start
    .size   __start, .-__start
