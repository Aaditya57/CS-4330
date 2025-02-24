  .set noat
    .text
    .align  2
    .globl  __start
    .ent    __start
    .type   __start, @function
__start:
   addi $4, $0, 7
   add $5, $4, $0
   add $6, $4, $0
   addi $7, $0, 8   # writes to rt
   add  $8, $0, $7  # reads $4 as rt - tests rt forwarding
   add  $9, $7, $5   # reads $4 as rs - tests rs forwarding
   addi $0, $0, 0
   addi $0, $0, 0
   addi $0, $0, 0
   addi $0, $0, 0
   addi $0, $0, 0
   addi $0, $0, 0
   addi $0, $0, 0
   addi $0, $0, 0

#label:
#   or $4, $5, $6
    .end    __start
    .size   __start, .-__start
