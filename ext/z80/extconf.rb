require 'mkmf'
dir_config 'Z80'

abort "missing header Z80.h" unless have_header  'Z80.h'
abort "missing library Z80"  unless have_library 'Z80'

%w(power instant_reset int nmi execute run).each do |function|
	abort "missing z80_#{function}()" unless have_func "z80_#{function}"
end

have_func 'z80_special_reset'

%w(break r refresh_address in_cycle out_cycle).each do |function|
	abort "missing z80_#{function}()" unless have_func("z80_#{function}", 'Z80.h')
end

%w(
	MAXIMUM_CYCLES
	MAXIMUM_CYCLES_PER_STEP
	MINIMUM_CYCLES_PER_STEP
	HOOK
	SF ZF YF HF XF PF NF CF
	OPTION_OUT_VC_255
	OPTION_LD_A_IR_BUG
	OPTION_HALT_SKIP
	OPTION_XQ
	OPTION_IM0_RETX_NOTIFICATIONS
	OPTION_YQ
	MODEL_ZILOG_NMOS
	MODEL_ZILOG_CMOS
	MODEL_NEC_NMOS
	MODEL_ST_CMOS
	REQUEST_REJECT_NMI
	REQUEST_NMI
	REQUEST_INT
	RESUME_HALT
	RESUME_XY
	RESUME_IM0_XY
	MEMPTR PC SP XY IX IY AF BC DE HL AF_ BC_ DE_ HL_
	MEMPTRH MEMPTRL PCH PCL SPH SPL XYH XYL IXH IXL IYH IYL
	A F B C D E H L A_ F_ B_ C_ D_ E_ H_ L_
).each do |macro|
	abort "missing Z80_#{macro}" unless have_macro("Z80_#{macro}", 'Z80.h')
end

create_makefile "z80/z80"
