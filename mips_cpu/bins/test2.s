  .set noat
    .text
    .align  2
    .globl  __start
    .ent    __start
    .type   __start, @function
__start:
   addi $2, $2, 9
   addi $3, $3, 5
   addi $4, $4, 9
   addi $5, $5, 5
   addi $6, $6, 9

#label:
#   or $4, $5, $6
    .end    __start
    .size   __start, .-__start
