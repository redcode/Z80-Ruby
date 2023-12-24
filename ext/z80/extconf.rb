require 'mkmf'
dir_config 'Z80'

abort "missing header Z80.h" unless have_header  'Z80.h'
abort "missing library Z80"  unless have_library 'Z80'

%w(
	z80_power
	z80_instant_reset
	z80_int
	z80_nmi
	z80_execute
	z80_run
).each do |function|
	abort "missing #{function}()" unless have_func function
end

have_func 'z80_special_reset'

%w(
	z80_break
	z80_r
	z80_refresh_address
	z80_in_cycle
	z80_out_cycle
).each do |function|
	abort "missing #{function}()" unless have_func(function, 'Z80.h')
end

%w(
	Z80_MAXIMUM_CYCLES
	Z80_MAXIMUM_CYCLES_PER_STEP
	Z80_MINIMUM_CYCLES_PER_STEP
	Z80_HOOK
	Z80_OPTION_OUT_VC_255
	Z80_OPTION_LD_A_IR_BUG
	Z80_OPTION_HALT_SKIP
	Z80_OPTION_XQ
	Z80_OPTION_IM0_RETX_NOTIFICATIONS
	Z80_OPTION_YQ
	Z80_MODEL_ZILOG_NMOS
	Z80_MODEL_ZILOG_CMOS
	Z80_MODEL_NEC_NMOS
	Z80_MODEL_ST_CMOS
	Z80_REQUEST_REJECT_NMI
	Z80_REQUEST_NMI
	Z80_REQUEST_INT
	Z80_RESUME_HALT
	Z80_RESUME_XY
	Z80_RESUME_IM0_XY
	Z80_MEMPTR
	Z80_PC
	Z80_SP
	Z80_XY
	Z80_IX
	Z80_IY
	Z80_AF
	Z80_BC
	Z80_DE
	Z80_HL
	Z80_AF_
	Z80_BC_
	Z80_DE_
	Z80_HL_
	Z80_MEMPTRH
	Z80_MEMPTRL
	Z80_PCH
	Z80_PCL
	Z80_SPH
	Z80_SPL
	Z80_XYH
	Z80_XYL
	Z80_IXH
	Z80_IXL
	Z80_IYH
	Z80_IYL
	Z80_A
	Z80_F
	Z80_B
	Z80_C
	Z80_D
	Z80_E
	Z80_H
	Z80_L
	Z80_A_
	Z80_F_
	Z80_B_
	Z80_C_
	Z80_D_
	Z80_E_
	Z80_H_
	Z80_L_
).each do |macro|
	abort "missing #{macro}" unless have_macro(macro, 'Z80.h')
end

create_makefile "z80/z80"
