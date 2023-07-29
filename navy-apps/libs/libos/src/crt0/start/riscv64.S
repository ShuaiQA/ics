.globl  _start
_start:
  move s0, zero
	mv sp, a0
	wfi
  jal call_main
