  .set noat
    .text
    .align  2
    .globl  __start
    .ent    __start
    .type   __start, @function
__start:
   addi $0, $2, 9
   addi $0, $3, 5
   addi $0, $4, 9
   addi $0, $5, 5
   addi $0, $6, 9

label:
   or $4, $5, $6
    .end    __start
    .size   __start, .-__start
