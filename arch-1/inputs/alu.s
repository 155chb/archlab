		.text
		.globl main
main:	addiu $5, $zero, 10
		addi  $6, $5, -5
		addiu $7, $zero, -5
        addi  $5, $5, -1
		addi  $8, $zero, -5
        sub   $9, $5, $6
        sub   $9, $5, $8
        subu  $9, $5, $8
        mult  $5, $6
        div   $5, $6
        mult  $5, $8
        lui   $6, 0x0fff
        addi  $7, $0, 0x0fff
        mult  $6, $7
        div   $5, $8
        multu $5, $8
        divu  $5, $8
		syscall