/*     ______  ______ ______
      /\___  \/\  __ \\  __ \
 ____ \/__/  /\_\  __ \\ \/\ \ ______________________________________
|        /\_____\\_____\\_____\                                      |
|  Zilog \/_____//_____//_____/ CPU Emulator - Ruby Binding          |
|  Copyright (C) 2023-2024 Manuel Sainz de Baranda y Goñi.           |
|                                                                    |
|  Permission to use, copy, modify, and/or distribute this software  |
|  for any purpose with or without fee is hereby granted.            |
|                                                                    |
|  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL     |
|  WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED     |
|  WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL      |
|  THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR        |
|  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM    |
|  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,   |
|  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN         |
|  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.          |
|                                                                    |
'===================================================================*/

#include <ruby.h>
#include <ruby/version.h>
#include <Z80.h>
#include <Z/macros/array.h>
#include <inttypes.h>
#include <stdio.h>

static rb_data_type_t const z80_data_type;

#define GET_Z80	 \
	Z80 *z80; \
	TypedData_Get_Struct(self, Z80, &z80_data_type, z80);

enum {	FetchOpcode, Fetch, Read, Write, In, Out,
	Halt, Nop,
	NMIA, INTA, INTFetch,
	ld_i_a, ld_r_a, reti, retn,
	Hook, Illegal,
	Context
};


/* Callbacks: Dummy Bridges */

static zuint8 dummy_read(void *context, zuint16 address)
	{
	Z_UNUSED(context) Z_UNUSED(address)
	return 0; /* nop */
	}


static void dummy_write(void *context, zuint16 address, zuint8 value)
	{Z_UNUSED(context) Z_UNUSED(address)}


static zuint8 dummy_in(void *context, zuint16 port)
	{
	Z_UNUSED(context) Z_UNUSED(port)
	return 255;
	}


/* Callbacks: Bridges */

#define PROC_CALL(index, arity, ...)			       \
	rb_funcall(					       \
		external[index], rb_intern("call"), arity + 1, \
		external[Context] Z_IF(arity)(Z_COMMA) __VA_ARGS__)

#define METHOD_CALL(index, arity, ...)			  \
	rb_funcall(					  \
		external[index], rb_intern("call"), arity \
		Z_IF(arity)(Z_COMMA) __VA_ARGS__)


#define CALLBACK_BRIDGES(receiver, call)				     \
									     \
static zuint8 receiver##_fetch_opcode(VALUE *external, zuint16 address)	     \
	{return (zuint8)NUM2UINT(call(FetchOpcode, 1, UINT2NUM(address)));}  \
									     \
									     \
static zuint8 receiver##_fetch(VALUE *external, zuint16 address)	     \
	{return (zuint8)NUM2UINT(call(Fetch, 1, UINT2NUM(address)));}	     \
									     \
									     \
static zuint8 receiver##_read(VALUE *external, zuint16 address)		     \
	{return (zuint8)NUM2UINT(call(Read, 1, UINT2NUM(address)));}	     \
									     \
									     \
static void receiver##_write(VALUE *external, zuint16 address, zuint8 value) \
	{call(Write, 2, UINT2NUM(address), UINT2NUM(value));}		     \
									     \
									     \
static zuint8 receiver##_in(VALUE *external, zuint16 port)		     \
	{return (zuint8)NUM2UINT(call(In, 1, UINT2NUM(port)));}		     \
									     \
									     \
static void receiver##_out(VALUE *external, zuint16 port, zuint8 value)	     \
	{call(Out, 2, UINT2NUM(port), UINT2NUM(value));}		     \
									     \
									     \
static void receiver##_halt(VALUE *external, zuint8 state)		     \
	{call(Halt, 1, UINT2NUM(state));}				     \
									     \
									     \
static zuint8 receiver##_nop(VALUE *external, zuint16 address)		     \
	{return (zuint8)NUM2UINT(call(Nop, 1, UINT2NUM(address)));}	     \
									     \
									     \
static zuint8 receiver##_nmia(VALUE *external, zuint16 address)		     \
	{return (zuint8)NUM2UINT(call(NMIA, 1, UINT2NUM(address)));}	     \
									     \
									     \
static zuint8 receiver##_inta(VALUE *external, zuint16 address)		     \
	{return (zuint8)NUM2UINT(call(INTA, 1, UINT2NUM(address)));}	     \
									     \
									     \
static zuint8 receiver##_int_fetch(VALUE *external, zuint16 address)	     \
	{return (zuint8)NUM2UINT(call(INTFetch, 1, UINT2NUM(address)));}     \
									     \
									     \
static void receiver##_ld_i_a(VALUE *external) {call(ld_i_a, 0);}	     \
static void receiver##_ld_r_a(VALUE *external) {call(ld_r_a, 0);}	     \
static void receiver##_reti  (VALUE *external) {call(reti,   0);}	     \
static void receiver##_retn  (VALUE *external) {call(reti,   0);}	     \
									     \
									     \
static zuint8 receiver##_hook(VALUE *external, zuint16 address)		     \
	{return (zuint8)NUM2UINT(call(Hook, 1, UINT2NUM(address)));}	     \
									     \
									     \
static zuint8 receiver##_illegal(VALUE *external, zuint8 opcode)	     \
	{return (zuint8)NUM2UINT(call(Illegal, 1, UINT2NUM(opcode)));}


CALLBACK_BRIDGES(proc,	 PROC_CALL  )
CALLBACK_BRIDGES(method, METHOD_CALL)

#undef PROC_CALL
#undef METHOD_CALL
#undef CALLBACK_BRIDGES


/* MARK: - Callbacks: Accessors */

typedef struct {
	zusize offset;
	void*  dummy;
	void*  proc_bridge;
	void*  method_bridge;
} CallbackInfo;

static CallbackInfo const callback_info_table[] = {
	{Z_MEMBER_OFFSET(Z80, fetch_opcode), dummy_read,  proc_fetch_opcode, method_fetch_opcode},
	{Z_MEMBER_OFFSET(Z80, fetch	  ), dummy_read,  proc_fetch,	     method_fetch,	},
	{Z_MEMBER_OFFSET(Z80, read	  ), dummy_read,  proc_read,	     method_read,	},
	{Z_MEMBER_OFFSET(Z80, write	  ), dummy_write, proc_write,	     method_write,	},
	{Z_MEMBER_OFFSET(Z80, in	  ), dummy_read,  proc_in,	     method_in,		},
	{Z_MEMBER_OFFSET(Z80, out	  ), dummy_write, proc_out,	     method_out,	},
	{Z_MEMBER_OFFSET(Z80, halt	  ), NULL,	  proc_halt,	     method_halt,	},
	{Z_MEMBER_OFFSET(Z80, nop	  ), NULL,	  proc_nop,	     method_nop,	},
	{Z_MEMBER_OFFSET(Z80, nmia	  ), NULL,	  proc_nmia,	     method_nmia,	},
	{Z_MEMBER_OFFSET(Z80, inta	  ), NULL,	  proc_inta,	     method_inta,	},
	{Z_MEMBER_OFFSET(Z80, int_fetch	  ), NULL,	  proc_int_fetch,    method_int_fetch,	},
	{Z_MEMBER_OFFSET(Z80, ld_i_a	  ), NULL,	  proc_ld_i_a,	     method_ld_i_a,	},
	{Z_MEMBER_OFFSET(Z80, ld_r_a	  ), NULL,	  proc_ld_r_a,	     method_ld_r_a,	},
	{Z_MEMBER_OFFSET(Z80, reti	  ), NULL,	  proc_reti,	     method_reti,	},
	{Z_MEMBER_OFFSET(Z80, retn	  ), NULL,	  proc_retn,	     method_retn,	},
	{Z_MEMBER_OFFSET(Z80, hook	  ), NULL,	  proc_hook,	     method_hook,	},
	{Z_MEMBER_OFFSET(Z80, illegal	  ), NULL,	  proc_illegal,	     method_illegal,	}};


static void set_callback(VALUE self, VALUE object, zuint index)
	{
	CallbackInfo const *callback_info;
	VALUE *external;
	GET_Z80;

	external = (VALUE *)z80->context + index;
	*external = object;
	callback_info = callback_info_table + index;

	*(void **)((char *)z80 + callback_info->offset) = object != Qnil
		? *(void **)((char *)z80 + callback_info->offset) = callback_info->proc_bridge
		: callback_info->dummy;
	}


#define CALLBACK_ACCESSOR(member, index)				    \
									    \
static VALUE Z80__set_##member(VALUE self, VALUE callback)		    \
	{								    \
	set_callback(self, callback, index);				    \
	return Qnil;							    \
	}								    \
									    \
									    \
static VALUE Z80__##member(VALUE self)					    \
	{								    \
	GET_Z80;							    \
	if (rb_block_given_p()) set_callback(self, rb_block_proc(), index); \
	return *((VALUE *)z80->context + index);			    \
	}


CALLBACK_ACCESSOR(fetch_opcode, FetchOpcode)
CALLBACK_ACCESSOR(fetch,	Fetch	   )
CALLBACK_ACCESSOR(read,		Read	   )
CALLBACK_ACCESSOR(write,	Write	   )
CALLBACK_ACCESSOR(in,		In	   )
CALLBACK_ACCESSOR(out,		Out	   )
CALLBACK_ACCESSOR(halt,		Halt	   )
CALLBACK_ACCESSOR(nop,		Nop	   )
CALLBACK_ACCESSOR(nmia,		NMIA	   )
CALLBACK_ACCESSOR(inta,		INTA	   )
CALLBACK_ACCESSOR(int_fetch,	INTFetch   )
CALLBACK_ACCESSOR(ld_i_a,	ld_i_a	   )
CALLBACK_ACCESSOR(ld_r_a,	ld_r_a	   )
CALLBACK_ACCESSOR(reti,		reti	   )
CALLBACK_ACCESSOR(retn,		retn	   )
CALLBACK_ACCESSOR(hook,		Hook	   )
CALLBACK_ACCESSOR(illegal,	Illegal	   )

#undef CALLBACK_ACCESSOR


/* MARK: - Other Accessors */

static VALUE Z80__set_context(VALUE self, VALUE context)
	{
	GET_Z80;
	*((VALUE *)z80->context + Context) = context;
	return Qnil;
	}


static VALUE Z80__context(VALUE self)
	{
	GET_Z80;
	return *((VALUE *)z80->context + Context);
	}


#define INTEGER_ACCESSOR(type, member, access, with, converter_affix)	   \
									   \
	static VALUE Z80__##member(VALUE self)				   \
		{							   \
		GET_Z80;						   \
		return converter_affix##2NUM(access(member, with));	   \
		}							   \
									   \
									   \
	static VALUE Z80__set_##member(VALUE self, VALUE value) 	   \
		{							   \
		GET_Z80;						   \
		access(member, with) = (type)NUM2##converter_affix(value); \
		return value;						   \
		}


#define MACRO( member, macro) macro(*z80)
#define DIRECT(member, macro) z80->member

INTEGER_ACCESSOR(zusize,  cycles,      DIRECT, Z_EMPTY,	    SIZET)
INTEGER_ACCESSOR(zusize,  cycle_limit, DIRECT, Z_EMPTY,	    SIZET)
INTEGER_ACCESSOR(zuint16, memptr,      MACRO,  Z80_MEMPTR,  UINT )
INTEGER_ACCESSOR(zuint16, pc,	       MACRO,  Z80_PC,	    UINT )
INTEGER_ACCESSOR(zuint16, sp,	       MACRO,  Z80_SP,	    UINT )
INTEGER_ACCESSOR(zuint16, xy,	       MACRO,  Z80_XY,	    UINT )
INTEGER_ACCESSOR(zuint16, ix,	       MACRO,  Z80_IX,	    UINT )
INTEGER_ACCESSOR(zuint16, iy,	       MACRO,  Z80_IY,	    UINT )
INTEGER_ACCESSOR(zuint16, af,	       MACRO,  Z80_AF,	    UINT )
INTEGER_ACCESSOR(zuint16, bc,	       MACRO,  Z80_BC,	    UINT )
INTEGER_ACCESSOR(zuint16, de,	       MACRO,  Z80_DE,	    UINT )
INTEGER_ACCESSOR(zuint16, hl,	       MACRO,  Z80_HL,	    UINT )
INTEGER_ACCESSOR(zuint16, af_,	       MACRO,  Z80_AF_,	    UINT )
INTEGER_ACCESSOR(zuint16, bc_,	       MACRO,  Z80_BC_,	    UINT )
INTEGER_ACCESSOR(zuint16, de_,	       MACRO,  Z80_DE_,	    UINT )
INTEGER_ACCESSOR(zuint16, hl_,	       MACRO,  Z80_HL_,	    UINT )
INTEGER_ACCESSOR(zuint8,  memptrh,     MACRO,  Z80_MEMPTRH, UINT )
INTEGER_ACCESSOR(zuint8,  memptrl,     MACRO,  Z80_MEMPTRL, UINT )
INTEGER_ACCESSOR(zuint8,  pch,	       MACRO,  Z80_PCH,	    UINT )
INTEGER_ACCESSOR(zuint8,  pcl,	       MACRO,  Z80_PCL,	    UINT )
INTEGER_ACCESSOR(zuint8,  sph,	       MACRO,  Z80_SPH,	    UINT )
INTEGER_ACCESSOR(zuint8,  spl,	       MACRO,  Z80_SPL,	    UINT )
INTEGER_ACCESSOR(zuint8,  xyh,	       MACRO,  Z80_XYH,	    UINT )
INTEGER_ACCESSOR(zuint8,  xyl,	       MACRO,  Z80_XYL,	    UINT )
INTEGER_ACCESSOR(zuint8,  ixh,	       MACRO,  Z80_IXH,	    UINT )
INTEGER_ACCESSOR(zuint8,  ixl,	       MACRO,  Z80_IXL,	    UINT )
INTEGER_ACCESSOR(zuint8,  iyh,	       MACRO,  Z80_IYH,	    UINT )
INTEGER_ACCESSOR(zuint8,  iyl,	       MACRO,  Z80_IYL,	    UINT )
INTEGER_ACCESSOR(zuint8,  a,	       MACRO,  Z80_A,	    UINT )
INTEGER_ACCESSOR(zuint8,  f,	       MACRO,  Z80_F,	    UINT )
INTEGER_ACCESSOR(zuint8,  b,	       MACRO,  Z80_B,	    UINT )
INTEGER_ACCESSOR(zuint8,  c,	       MACRO,  Z80_C,	    UINT )
INTEGER_ACCESSOR(zuint8,  d,	       MACRO,  Z80_D,	    UINT )
INTEGER_ACCESSOR(zuint8,  e,	       MACRO,  Z80_E,	    UINT )
INTEGER_ACCESSOR(zuint8,  h,	       MACRO,  Z80_H,	    UINT )
INTEGER_ACCESSOR(zuint8,  l,	       MACRO,  Z80_L,	    UINT )
INTEGER_ACCESSOR(zuint8,  a_,	       MACRO,  Z80_A_,	    UINT )
INTEGER_ACCESSOR(zuint8,  f_,	       MACRO,  Z80_F_,	    UINT )
INTEGER_ACCESSOR(zuint8,  b_,	       MACRO,  Z80_B_,	    UINT )
INTEGER_ACCESSOR(zuint8,  c_,	       MACRO,  Z80_C_,	    UINT )
INTEGER_ACCESSOR(zuint8,  d_,	       MACRO,  Z80_D_,	    UINT )
INTEGER_ACCESSOR(zuint8,  e_,	       MACRO,  Z80_E_,	    UINT )
INTEGER_ACCESSOR(zuint8,  h_,	       MACRO,  Z80_H_,	    UINT )
INTEGER_ACCESSOR(zuint8,  l_,	       MACRO,  Z80_L_,	    UINT )
INTEGER_ACCESSOR(zuint8,  r,	       DIRECT, Z_EMPTY,	    UINT )
INTEGER_ACCESSOR(zuint8,  i,	       DIRECT, Z_EMPTY,	    UINT )
INTEGER_ACCESSOR(zuint8,  r7,	       DIRECT, Z_EMPTY,	    UINT )
INTEGER_ACCESSOR(zuint8,  im,	       DIRECT, Z_EMPTY,	    UINT )
INTEGER_ACCESSOR(zuint8,  request,     DIRECT, Z_EMPTY,	    UINT )
INTEGER_ACCESSOR(zuint8,  resume,      DIRECT, Z_EMPTY,	    UINT )
INTEGER_ACCESSOR(zuint8,  q,	       DIRECT, Z_EMPTY,	    UINT )
INTEGER_ACCESSOR(zuint8,  options,     DIRECT, Z_EMPTY,	    UINT )

#undef MACRO
#undef DIRECT
#undef INTEGER_ACCESSOR


#define BOOL_NUM_TO_UINT(value) \
	(value == Qfalse ? 0U : (value == Qtrue ? 1U : !!NUM2UINT(value)))


#define BOOLEAN_ACCESSOR(member)			\
							\
static VALUE Z80__##member(VALUE self)			\
	{						\
	GET_Z80;					\
	return UINT2NUM(z80->member);			\
	}						\
							\
static VALUE Z80__##member##_p(VALUE self)		\
	{						\
	GET_Z80;					\
	return z80->member ? Qtrue : Qfalse;		\
	}						\
							\
							\
static VALUE Z80__set_##member(VALUE self, VALUE value) \
	{						\
	GET_Z80;					\
	z80->member = BOOL_NUM_TO_UINT(value);		\
	return value;					\
	}


BOOLEAN_ACCESSOR(iff1	  )
BOOLEAN_ACCESSOR(iff2	  )
BOOLEAN_ACCESSOR(int_line )
BOOLEAN_ACCESSOR(halt_line)

#undef BOOLEAN_ACCESSOR


#define FLAG_ACCESSOR(flag, mask, shift)		  \
							  \
static VALUE Z80__##flag(VALUE self)			  \
	{						  \
	GET_Z80;					  \
	return UINT2NUM((Z80_F(*z80) >> shift) & 1);	  \
	}						  \
							  \
							  \
static VALUE Z80__##flag##_p(VALUE self)		  \
	{						  \
	GET_Z80;					  \
	return Z80_F(*z80) & Z80_##mask ? Qtrue : Qfalse; \
	}						  \
							  \
							  \
static VALUE Z80__set_##flag(VALUE self, VALUE value)	  \
	{						  \
	GET_Z80;					  \
							  \
	Z80_F(*z80) =					  \
		(Z80_F(*z80) & ~(1U << shift)) |	  \
		(BOOL_NUM_TO_UINT(value) << shift);	  \
							  \
	return value;					  \
	}


FLAG_ACCESSOR(sf, SF, 7)
FLAG_ACCESSOR(zf, ZF, 6)
FLAG_ACCESSOR(yf, YF, 5)
FLAG_ACCESSOR(hf, HF, 4)
FLAG_ACCESSOR(xf, XF, 3)
FLAG_ACCESSOR(pf, PF, 2)
FLAG_ACCESSOR(nf, NF, 1)
FLAG_ACCESSOR(cf, CF, 0)

#undef FLAG_ACCESSOR
#undef BOOL_NUM_TO_UINT


/* MARK: - Methods */

static VALUE Z80__power(int argc, VALUE *argv, VALUE self)
	{
	Z80 *z80;

	if (argc > 1) rb_raise(
		rb_eArgError,
		"wrong number of arguments (given %d, expected 0 or 1)",
		argc);

	TypedData_Get_Struct(self, Z80, &z80_data_type, z80);
	z80_power(z80, RB_TEST(argv[0]));
	return self;
	}


static VALUE Z80__instant_reset(VALUE self)
	{
	GET_Z80;
	z80_instant_reset(z80);
	return self;
	}


static VALUE Z80__int(VALUE self, VALUE state)
	{
	GET_Z80;
	z80_int(z80, RB_TEST(state));
	return self;
	}


static VALUE Z80__nmi(VALUE self)
	{
	GET_Z80;
	z80_nmi(z80);
	return self;
	}


static VALUE Z80__execute(VALUE self, VALUE cycles)
	{
	GET_Z80;
	return SIZET2NUM(z80_execute(z80, NUM2SIZET(cycles)));
	}


static VALUE Z80__run(VALUE self, VALUE cycles)
	{
	GET_Z80;
	return SIZET2NUM(z80_run(z80, NUM2SIZET(cycles)));
	}


static VALUE Z80__terminate(VALUE self)
	{
	GET_Z80;
	z80_break(z80);
	return self;
	}


static VALUE Z80__full_r(VALUE self)
	{
	GET_Z80;
	return UINT2NUM(z80_r(z80));
	}


static VALUE Z80__refresh_address(VALUE self)
	{
	GET_Z80;
	return UINT2NUM(z80_refresh_address(z80));
	}


static VALUE Z80__in_cycle(VALUE self)
	{
	GET_Z80;
	return UINT2NUM(z80_in_cycle(z80));
	}


static VALUE Z80__out_cycle(VALUE self)
	{
	GET_Z80;
	return UINT2NUM(z80_out_cycle(z80));
	}


static struct {char const* name; zuint offset;} const

uint16_members[] = {
	{"memptr", Z_MEMBER_OFFSET(Z80, memptr	)},
	{"pc",	   Z_MEMBER_OFFSET(Z80, pc	)},
	{"sp",	   Z_MEMBER_OFFSET(Z80, sp	)},
	{"xy",	   Z_MEMBER_OFFSET(Z80, xy	)},
	{"ix",	   Z_MEMBER_OFFSET(Z80, ix_iy[0])},
	{"iy",	   Z_MEMBER_OFFSET(Z80, ix_iy[1])},
	{"af",	   Z_MEMBER_OFFSET(Z80, af	)},
	{"bc",	   Z_MEMBER_OFFSET(Z80, bc	)},
	{"de",	   Z_MEMBER_OFFSET(Z80, de	)},
	{"hl",	   Z_MEMBER_OFFSET(Z80, hl	)},
	{"af_",	   Z_MEMBER_OFFSET(Z80, af_	)},
	{"bc_",	   Z_MEMBER_OFFSET(Z80, bc_	)},
	{"de_",	   Z_MEMBER_OFFSET(Z80, de_	)},
	{"hl_",	   Z_MEMBER_OFFSET(Z80, hl_	)}},

uint8_members[] = {
	{"i",	      Z_MEMBER_OFFSET(Z80, i	    )},
	{"r",	      Z_MEMBER_OFFSET(Z80, r	    )},
	{"r7",	      Z_MEMBER_OFFSET(Z80, r7	    )},
	{"q",	      Z_MEMBER_OFFSET(Z80, q	    )},
	{"im",	      Z_MEMBER_OFFSET(Z80, im	    )},
	{"request",   Z_MEMBER_OFFSET(Z80, request  )},
	{"resume",    Z_MEMBER_OFFSET(Z80, resume   )},
	{"options",   Z_MEMBER_OFFSET(Z80, options  )},
	{"iff1",      Z_MEMBER_OFFSET(Z80, iff1	    )},
	{"iff2",      Z_MEMBER_OFFSET(Z80, iff2	    )},
	{"int_line",  Z_MEMBER_OFFSET(Z80, int_line )},
	{"halt_line", Z_MEMBER_OFFSET(Z80, halt_line)}};


static VALUE Z80__to_h(int argc, VALUE *argv, VALUE self)
	{
	Z80 *z80;
	VALUE hash;
	VALUE kv[(Z_ARRAY_SIZE(uint16_members) + Z_ARRAY_SIZE(uint8_members)) * 2];
	int i, j, uint8_member_count;

	if (argc > 1) rb_raise(
		rb_eArgError,
		"wrong number of arguments (given %d, expected 0 or 1)",
		argc);

	TypedData_Get_Struct(self, Z80, &z80_data_type, z80);
	hash = rb_hash_new();

	uint8_member_count =
		Z_ARRAY_SIZE(uint8_members) -
		((argc && RB_TEST(argv[0])) << 2); /* 4 or 0 */

	for (i = j = 0; j < Z_ARRAY_SIZE(uint16_members);)
		{
		kv[i++] = rb_id2sym(rb_intern(uint16_members[j].name));
		kv[i++] = UINT2NUM(*(zuint16 *)(void *)((char *)z80 + uint16_members[j++].offset));
		}

	for (j = 0; j < uint8_member_count;)
		{
		kv[i++] = rb_id2sym(rb_intern(uint8_members[j].name));
		kv[i++] = UINT2NUM(*((zuint8 *)z80 + uint8_members[j++].offset));
		}

	while (j < Z_ARRAY_SIZE(uint8_members))
		{
		kv[i++] = rb_id2sym(rb_intern(uint8_members[j].name));
		kv[i++] = *((zuint8 *)z80 + uint8_members[j++].offset) ? Qtrue : Qfalse;
		}

	rb_hash_bulk_insert_into_st_table(Z_ARRAY_SIZE(kv), kv, hash);
	return hash;
	}


static char const one_hyphen[2] = "1-";


static VALUE Z80__print(VALUE self)
	{
	zuint8 f;
	GET_Z80;
	f = Z80_F(*z80);

	/*
	PC 0000  AF 0000  AF' 0000  IX 0000
	SP 0000  BC 0000  BC' 0000  IY 0000
	IR 0000  DE 0000  DE' 0000  XY 0000
	WZ 0000  HL 0000  HL' 0000  Q  00
	S Z Y H X P N C  IFF1 0  IM 0  EI 1
	- - 1 - - 1 - -  IFF2 0  R7 0  RI 1
	/INT  high  RS 00  data 00 00 00 00
	/HALT high  RQ 00  data 00 00 00 00*/

	printf(	"PC %04" PRIX16		   "  AF %04" PRIX16 "  AF' %04" PRIX16 "  IX %04" PRIX16 "\n"
		"SP %04" PRIX16		   "  BC %04" PRIX16 "  BC' %04" PRIX16 "  IY %04" PRIX16 "\n"
		"IR %02" PRIX8 "%02" PRIX8 "  DE %04" PRIX16 "  DE' %04" PRIX16 "  XY %04" PRIX16 "\n"
		"WZ %04" PRIX16		   "  HL %04" PRIX16 "  HL' %04" PRIX16 "  Q  %02" PRIX8  "\n"
		"S Z Y H X P N C"      "  IFF1 %" PRIu8 "  IM %" PRIu8 "  EI %" PRIu8 "\n"
		"%c %c %c %c %c %c %c %c  IFF2 %" PRIu8 "  R7 %" PRIu8 "  RI %" PRIu8 "\n",
		Z80_PC(*z80),	  Z80_AF(*z80), Z80_AF_(*z80), Z80_IX(*z80),
		Z80_SP(*z80),	  Z80_BC(*z80), Z80_BC_(*z80), Z80_IY(*z80),
		z80->i, z80->r,	  Z80_DE(*z80), Z80_DE_(*z80), Z80_IX(*z80),
		Z80_MEMPTR(*z80), Z80_HL(*z80), Z80_HL_(*z80), z80->q,
		z80->iff1, z80->im, '\0',
		one_hyphen[!(f & Z80_SF)],
		one_hyphen[!(f & Z80_ZF)],
		one_hyphen[!(f & Z80_YF)],
		one_hyphen[!(f & Z80_HF)],
		one_hyphen[!(f & Z80_XF)],
		one_hyphen[!(f & Z80_PF)],
		one_hyphen[!(f & Z80_NF)],
		one_hyphen[!(f & Z80_CF)],
		z80->iff2, z80->r7 >> 7, '\0');

	return Qnil;
	}


/* MARK: - Object Lifecycle */

static void Z80__mark(Z80 *z80)
	{
	VALUE *externals = z80->context;

	for (int i = 18; i;) if (externals[--i] != Qnil)
		rb_gc_mark_movable(externals[i]);
	}


static void Z80__free(Z80 *z80)
	{
	free(z80->context);
	xfree(z80);
	}


static size_t Z80__memsize(const void *z80)
	{return sizeof(Z80) + sizeof(VALUE[18]);}


static void Z80__compact(Z80 *z80)
	{
	VALUE *externals = z80->context;

	for (int i = 18; i;) if (externals[--i] != Qnil)
		externals[i] = rb_gc_location(externals[i]);
	}


static rb_data_type_t const z80_data_type = {
	.wrap_struct_name = "z80",
	.function = {
#		if defined(RUBY_API_VERSION_MAJOR) && RUBY_API_VERSION_MAJOR >= 3
			.dcompact = (void (*)(void *))Z80__compact,
#		endif
		.dmark = (void (*)(void *))Z80__mark,
		.dfree = (void (*)(void *))Z80__free,
		.dsize = NULL},
	.flags = RUBY_TYPED_FREE_IMMEDIATELY};


static VALUE Z80__alloc(VALUE klass)
	{
	Z80 *z80;
	VALUE object = TypedData_Make_Struct(klass, Z80, &z80_data_type, z80);
	VALUE *externals = z80->context = malloc(sizeof(VALUE[18]));

	for (int i = 18; i;) externals[--i] = Qnil;

	z80->options	  = Z80_MODEL_ZILOG_NMOS;
	z80->fetch_opcode =
	z80->fetch	  =
	z80->read	  = dummy_read;
	z80->write	  =
	z80->out	  = dummy_write;
	z80->in		  = dummy_in;
	z80->halt	  = NULL;
	z80->nop	  =
	z80->nmia	  =
	z80->inta	  =
	z80->int_fetch	  =
	z80->hook	  = NULL;
	z80->ld_i_a	  =
	z80->ld_r_a	  =
	z80->reti	  =
	z80->retn	  = NULL;
	z80->illegal	  = NULL;

	return object;
	}


/* Library Initialization */

void Init_z80(void)
	{
	VALUE module, klass = rb_define_class("Z80", rb_cObject);

	rb_define_alloc_func(klass, Z80__alloc);

	rb_define_const(klass, "MAXIMUM_CYCLES",	  ULL2NUM(Z80_MAXIMUM_CYCLES	      ));
	rb_define_const(klass, "MAXIMUM_CYCLES_PER_STEP", UINT2NUM(Z80_MAXIMUM_CYCLES_PER_STEP));
	rb_define_const(klass, "HOOK",			  UINT2NUM(Z80_HOOK		      ));

	rb_define_const(klass, "SF", UINT2NUM(Z80_SF));
	rb_define_const(klass, "ZF", UINT2NUM(Z80_ZF));
	rb_define_const(klass, "YF", UINT2NUM(Z80_YF));
	rb_define_const(klass, "HF", UINT2NUM(Z80_HF));
	rb_define_const(klass, "XF", UINT2NUM(Z80_XF));
	rb_define_const(klass, "PF", UINT2NUM(Z80_PF));
	rb_define_const(klass, "NF", UINT2NUM(Z80_NF));
	rb_define_const(klass, "CF", UINT2NUM(Z80_CF));

	module = rb_define_module_under(klass, "Option");

	rb_define_const(module, "OUT_VC_255",		  UINT2NUM(Z80_OPTION_OUT_VC_255	    ));
	rb_define_const(module, "LD_A_IR_BUG",		  UINT2NUM(Z80_OPTION_LD_A_IR_BUG	    ));
	rb_define_const(module, "HALT_SKIP",		  UINT2NUM(Z80_OPTION_HALT_SKIP		    ));
	rb_define_const(module, "XQ",			  UINT2NUM(Z80_OPTION_XQ		    ));
	rb_define_const(module, "IM0_RETX_NOTIFICATIONS", UINT2NUM(Z80_OPTION_IM0_RETX_NOTIFICATIONS));
	rb_define_const(module, "YQ",			  UINT2NUM(Z80_OPTION_YQ		    ));

	module = rb_define_module_under(module, "Model");

	rb_define_const(module, "ZILOG_NMOS", UINT2NUM(Z80_MODEL_ZILOG_NMOS));
	rb_define_const(module, "ZILOG_CMOS", UINT2NUM(Z80_MODEL_ZILOG_CMOS));
	rb_define_const(module, "NEC_NMOS",   UINT2NUM(Z80_MODEL_NEC_NMOS  ));
	rb_define_const(module, "ST_CMOS",    UINT2NUM(Z80_MODEL_ST_CMOS   ));

	module = rb_define_module_under(klass, "Request");

	rb_define_const(module, "REJECT_NMI",	 UINT2NUM(Z80_REQUEST_REJECT_NMI   ));
	rb_define_const(module, "NMI",		 UINT2NUM(Z80_REQUEST_NMI	   ));
	rb_define_const(module, "INT",		 UINT2NUM(Z80_REQUEST_INT	   ));
/*	rb_define_const(module, "SPECIAL_RESET", UINT2NUM(Z80_REQUEST_SPECIAL_RESET));*/

	module = rb_define_module_under(klass, "Resume");

	rb_define_const(module, "HALT",	  UINT2NUM(Z80_RESUME_HALT  ));
	rb_define_const(module, "XY",	  UINT2NUM(Z80_RESUME_XY    ));
	rb_define_const(module, "IM0_XY", UINT2NUM(Z80_RESUME_IM0_XY));

#	define DEFINE_ACCESSOR(name)					\
		rb_define_method(klass, #name,	   Z80__##name,	    0); \
		rb_define_method(klass, #name "=", Z80__set_##name, 1);

	DEFINE_ACCESSOR(fetch_opcode)
	DEFINE_ACCESSOR(fetch	    )
	DEFINE_ACCESSOR(read	    )
	DEFINE_ACCESSOR(write	    )
	DEFINE_ACCESSOR(in	    )
	DEFINE_ACCESSOR(out	    )
	DEFINE_ACCESSOR(halt	    )
	DEFINE_ACCESSOR(nop	    )
	DEFINE_ACCESSOR(nmia	    )
	DEFINE_ACCESSOR(inta	    )
	DEFINE_ACCESSOR(int_fetch   )
	DEFINE_ACCESSOR(ld_i_a	    )
	DEFINE_ACCESSOR(ld_r_a	    )
	DEFINE_ACCESSOR(reti	    )
	DEFINE_ACCESSOR(retn	    )
	DEFINE_ACCESSOR(hook	    )
	DEFINE_ACCESSOR(illegal	    )
	DEFINE_ACCESSOR(context	    )
	DEFINE_ACCESSOR(cycles	    )
	DEFINE_ACCESSOR(cycle_limit )
	DEFINE_ACCESSOR(memptr	    )
	DEFINE_ACCESSOR(pc	    )
	DEFINE_ACCESSOR(sp	    )
	DEFINE_ACCESSOR(xy	    )
	DEFINE_ACCESSOR(ix	    )
	DEFINE_ACCESSOR(iy	    )
	DEFINE_ACCESSOR(af	    )
	DEFINE_ACCESSOR(bc	    )
	DEFINE_ACCESSOR(de	    )
	DEFINE_ACCESSOR(hl	    )
	DEFINE_ACCESSOR(af_	    )
	DEFINE_ACCESSOR(bc_	    )
	DEFINE_ACCESSOR(de_	    )
	DEFINE_ACCESSOR(hl_	    )
	DEFINE_ACCESSOR(memptrh	    )
	DEFINE_ACCESSOR(memptrl	    )
	DEFINE_ACCESSOR(pch	    )
	DEFINE_ACCESSOR(pcl	    )
	DEFINE_ACCESSOR(sph	    )
	DEFINE_ACCESSOR(spl	    )
	DEFINE_ACCESSOR(xyh	    )
	DEFINE_ACCESSOR(xyl	    )
	DEFINE_ACCESSOR(ixh	    )
	DEFINE_ACCESSOR(ixl	    )
	DEFINE_ACCESSOR(iyh	    )
	DEFINE_ACCESSOR(iyl	    )
	DEFINE_ACCESSOR(a	    )
	DEFINE_ACCESSOR(f	    )
	DEFINE_ACCESSOR(b	    )
	DEFINE_ACCESSOR(c	    )
	DEFINE_ACCESSOR(d	    )
	DEFINE_ACCESSOR(e	    )
	DEFINE_ACCESSOR(h	    )
	DEFINE_ACCESSOR(l	    )
	DEFINE_ACCESSOR(a_	    )
	DEFINE_ACCESSOR(f_	    )
	DEFINE_ACCESSOR(b_	    )
	DEFINE_ACCESSOR(c_	    )
	DEFINE_ACCESSOR(d_	    )
	DEFINE_ACCESSOR(e_	    )
	DEFINE_ACCESSOR(h_	    )
	DEFINE_ACCESSOR(l_	    )
	DEFINE_ACCESSOR(r	    )
	DEFINE_ACCESSOR(i	    )
	DEFINE_ACCESSOR(r7	    )
	DEFINE_ACCESSOR(im	    )
	DEFINE_ACCESSOR(request	    )
	DEFINE_ACCESSOR(resume	    )
	DEFINE_ACCESSOR(q	    )
	DEFINE_ACCESSOR(options	    )

#	define DEFINE_BOOLEAN_ACCESSOR(name) \
		DEFINE_ACCESSOR(name)	     \
		rb_define_method(klass, #name "?", Z80__##name##_p, 0);

	DEFINE_BOOLEAN_ACCESSOR(iff1	 )
	DEFINE_BOOLEAN_ACCESSOR(iff2	 )
	DEFINE_BOOLEAN_ACCESSOR(int_line )
	DEFINE_BOOLEAN_ACCESSOR(halt_line)
	DEFINE_BOOLEAN_ACCESSOR(sf	 )
	DEFINE_BOOLEAN_ACCESSOR(zf	 )
	DEFINE_BOOLEAN_ACCESSOR(yf	 )
	DEFINE_BOOLEAN_ACCESSOR(hf	 )
	DEFINE_BOOLEAN_ACCESSOR(xf	 )
	DEFINE_BOOLEAN_ACCESSOR(pf	 )
	DEFINE_BOOLEAN_ACCESSOR(nf	 )
	DEFINE_BOOLEAN_ACCESSOR(cf	 )

#	undef DEFINE_ACCESSOR
#	undef DEFINE_FLAG_ACCESSOR

	rb_define_method(klass, "power",	   Z80__power,		-1);
	rb_define_method(klass, "instant_reset",   Z80__instant_reset,	 0);
	rb_define_method(klass, "int",		   Z80__int,		 1);
	rb_define_method(klass, "nmi",		   Z80__nmi,		 0);
	rb_define_method(klass, "execute",	   Z80__execute,	 1);
	rb_define_method(klass, "run",		   Z80__run,		 1);
	rb_define_method(klass, "terminate",	   Z80__terminate,	 0);
	rb_define_method(klass, "full_r",	   Z80__full_r,		 0);
	rb_define_method(klass, "refresh_address", Z80__refresh_address, 0);
	rb_define_method(klass, "in_cycle",	   Z80__in_cycle,	 0);
	rb_define_method(klass, "out_cycle",	   Z80__out_cycle,	 0);
	rb_define_method(klass, "to_h",		   Z80__to_h,		-1);
	rb_define_method(klass, "print",	   Z80__print,		 0);
/*	rb_define_method(klass, "to_s",		   Z80__to_s,		 0);*/

	rb_define_alias(klass, "t",	"cycles"  );
	rb_define_alias(klass, "t=",	"cycles=" );
	rb_define_alias(klass, "wz",	"memptr"  );
	rb_define_alias(klass, "wz=",	"memptr=" );
	rb_define_alias(klass, "wzh",	"memptrh" );
	rb_define_alias(klass, "wzh=",	"memptrh=");
	rb_define_alias(klass, "wzl",	"memptrl" );
	rb_define_alias(klass, "wzl=",	"memptrl=");
	rb_define_alias(klass, "vf",	"pf"	  );
	rb_define_alias(klass, "vf=",	"pf="	  );
	rb_define_alias(klass, "state", "to_h"	  );
	}


/* z80.c EOF */
