/*     ______  ______ ______
      /\___  \/\  __ \\  __ \
 ____ \/__/  /\_\  __ \\ \/\ \ ______________________________________
|        /\_____\\_____\\_____\                                      |
|  Zilog \/_____//_____//_____/ CPU Emulator - Ruby Binding          |
|  Copyright (C) 2023 Manuel Sainz de Baranda y Goñi.                |
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
#include <Z80.h>
#include <Z/macros/array.h>
#include <inttypes.h>
#include <stdio.h>

#define OBJ Z80 *obj; Data_Get_Struct(self, Z80, obj);


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


static void Z80__free(Z80 *object)
	{
	}


static VALUE Z80__alloc(VALUE klass)
	{
	VALUE obj;
	Z80 *z80;

	obj = Data_Make_Struct(klass, Z80, NULL, Z80__free, z80);
	return obj;
	}


static VALUE Z80__init(VALUE self)
	{
	Z80 *z80;

	Data_Get_Struct(self, Z80, z80);
	z80->options	  = Z80_MODEL_ZILOG_NMOS;
	z80->context	  = NULL;
	z80->fetch_opcode =
	z80->fetch	  =
	z80->read	  = dummy_read;
	z80->write	  =
	z80->out	  = dummy_write;
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
	}


static VALUE Z80__release(VALUE self)
	{
	OBJ;
	return self;
	}


static VALUE Z80__power(VALUE self, VALUE state)
	{
	OBJ;
	z80_power(obj, RB_TEST(state));
	return self;
	}


static VALUE Z80__instant_reset(VALUE self)
	{
	OBJ;
	z80_instant_reset(obj);
	return self;
	}


static VALUE Z80__int(VALUE self, VALUE state)
	{
	OBJ;
	z80_int(obj, RB_TEST(state));
	return self;
	}


static VALUE Z80__nmi(VALUE self)
	{
	OBJ;
	z80_nmi(obj);
	return self;
	}


static VALUE Z80__execute(VALUE self, VALUE cycles)
	{
	OBJ;
	return SIZET2NUM(z80_execute(obj, NUM2SIZET(cycles)));
	}


static VALUE Z80__run(VALUE self, VALUE cycles)
	{
	OBJ;
	return SIZET2NUM(z80_run(obj, NUM2SIZET(cycles)));
	}


static VALUE Z80__full_r(VALUE self)
	{
	OBJ;
	return UINT2NUM(z80_r(obj));
	}


static VALUE Z80__refresh_address(VALUE self)
	{
	OBJ;
	return UINT2NUM(z80_refresh_address(obj));
	}


static VALUE Z80__in_cycle(VALUE self)
	{
	OBJ;
	return UINT2NUM(z80_in_cycle(obj));
	}


static VALUE Z80__out_cycle(VALUE self)
	{
	OBJ;
	return UINT2NUM(z80_out_cycle(obj));
	}


#define UINT_TO_BOOL(value) value ? Qtrue : Qfalse

#define MACRO( member, macro) macro(*obj)
#define DIRECT(member, macro) (*obj).member

#define ACCESSOR(type, member, access, with, c_to_ruby, ruby_to_c)		 \
										 \
	static VALUE Z80__##member(VALUE self)					 \
		{								 \
		OBJ;								 \
		return c_to_ruby(access(member, with));				 \
		}								 \
										 \
										 \
	static VALUE Z80__set_##member(VALUE self, VALUE value)			 \
		{								 \
		OBJ;								 \
		return c_to_ruby(access(member, with) = (type)ruby_to_c(value)); \
		}


ACCESSOR(zusize,  cycles,      DIRECT, Z_EMPTY,	    SIZET2NUM,	  NUM2SIZET)
ACCESSOR(zusize,  cycle_limit, DIRECT, Z_EMPTY,	    SIZET2NUM,	  NUM2SIZET)
ACCESSOR(zuint16, memptr,      MACRO,  Z80_MEMPTR,  UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, pc,	       MACRO,  Z80_PC,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, sp,	       MACRO,  Z80_SP,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, xy,	       MACRO,  Z80_XY,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, ix,	       MACRO,  Z80_IX,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, iy,	       MACRO,  Z80_IY,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, af,	       MACRO,  Z80_AF,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, bc,	       MACRO,  Z80_BC,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, de,	       MACRO,  Z80_DE,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, hl,	       MACRO,  Z80_HL,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, af_,	       MACRO,  Z80_AF_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, bc_,	       MACRO,  Z80_BC_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, de_,	       MACRO,  Z80_DE_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint16, hl_,	       MACRO,  Z80_HL_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  memptrh,     MACRO,  Z80_MEMPTRH, UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  memptrl,     MACRO,  Z80_MEMPTRL, UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  pch,	       MACRO,  Z80_PCH,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  pcl,	       MACRO,  Z80_PCL,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  sph,	       MACRO,  Z80_SPH,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  spl,	       MACRO,  Z80_SPL,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  xyh,	       MACRO,  Z80_XYH,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  yl,	       MACRO,  Z80_XYL,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  ixh,	       MACRO,  Z80_IXH,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  ixl,	       MACRO,  Z80_IXL,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  iyh,	       MACRO,  Z80_IYH,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  iyl,	       MACRO,  Z80_IYL,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  a,	       MACRO,  Z80_A,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  f,	       MACRO,  Z80_F,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  b,	       MACRO,  Z80_B,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  c,	       MACRO,  Z80_C,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  d,	       MACRO,  Z80_D,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  e,	       MACRO,  Z80_E,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  h,	       MACRO,  Z80_H,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  l,	       MACRO,  Z80_L,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  a_,	       MACRO,  Z80_A_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  f_,	       MACRO,  Z80_F_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  b_,	       MACRO,  Z80_B_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  c_,	       MACRO,  Z80_C_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  d_,	       MACRO,  Z80_D_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  e_,	       MACRO,  Z80_E_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  h_,	       MACRO,  Z80_H_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  l_,	       MACRO,  Z80_L_,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  r,	       DIRECT, Z_EMPTY,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  i,	       DIRECT, Z_EMPTY,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  r7,	       DIRECT, Z_EMPTY,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  im,	       DIRECT, Z_EMPTY,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  request,     DIRECT, Z_EMPTY,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  iff1,	       DIRECT, Z_EMPTY,	    UINT_TO_BOOL, RB_TEST )
ACCESSOR(zuint8,  iff2,	       DIRECT, Z_EMPTY,	    UINT_TO_BOOL, RB_TEST )
ACCESSOR(zuint8,  q,	       DIRECT, Z_EMPTY,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  options,     DIRECT, Z_EMPTY,	    UINT2NUM,	  NUM2UINT)
ACCESSOR(zuint8,  int_line,    DIRECT, Z_EMPTY,	    UINT_TO_BOOL, RB_TEST )
ACCESSOR(zuint8,  halt_line,   DIRECT, Z_EMPTY,	    UINT_TO_BOOL, RB_TEST )


#define FLAG_ACCESSOR(flag, shift)				       \
								       \
static VALUE Z80__##flag(VALUE self)				       \
	{							       \
	OBJ;							       \
	return UINT2NUM((Z80_F(*obj) >> shift) & 1);		       \
	}							       \
								       \
static VALUE Z80__set_##flag(VALUE self, VALUE value)		       \
	{							       \
	zuint8 bit = (zuint8)(NUM2UINT(value) & 1);		       \
								       \
	OBJ;							       \
	Z80_F(*obj) = (Z80_F(*obj) & ~(1U << shift)) | (bit << shift); \
	return UINT2NUM(bit);					       \
	}


FLAG_ACCESSOR(sf, 7)
FLAG_ACCESSOR(zf, 6)
FLAG_ACCESSOR(yf, 5)
FLAG_ACCESSOR(hf, 4)
FLAG_ACCESSOR(xf, 3)
FLAG_ACCESSOR(pf, 2)
FLAG_ACCESSOR(nf, 1)
FLAG_ACCESSOR(cf, 0)


typedef struct {char const* name; zuint offset;} NameAndOffset;

static NameAndOffset const data_members_16[] = {
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
	{"hl_",	   Z_MEMBER_OFFSET(Z80, hl_	)}
};

static NameAndOffset const data_members_8[] = {
	{"r",	      Z_MEMBER_OFFSET(Z80, r	    )},
	{"i",	      Z_MEMBER_OFFSET(Z80, i	    )},
	{"r7",	      Z_MEMBER_OFFSET(Z80, r7	    )},
	{"im",	      Z_MEMBER_OFFSET(Z80, im	    )},
	{"request",   Z_MEMBER_OFFSET(Z80, request  )},
	{"resume",    Z_MEMBER_OFFSET(Z80, resume   )},
	{"iff1",      Z_MEMBER_OFFSET(Z80, iff1	    )},
	{"iff2",      Z_MEMBER_OFFSET(Z80, iff2	    )},
	{"q",	      Z_MEMBER_OFFSET(Z80, q	    )},
	{"options",   Z_MEMBER_OFFSET(Z80, options  )},
	{"int_line",  Z_MEMBER_OFFSET(Z80, int_line )},
	{"halt_line", Z_MEMBER_OFFSET(Z80, halt_line)}
};


static VALUE Z80__to_h(VALUE self)
	{
	VALUE hash = rb_hash_new();
	VALUE kv[(Z_ARRAY_SIZE(data_members_16) + Z_ARRAY_SIZE(data_members_8)) * 2];
	int i = 0, j;
	OBJ;

	for (j = 0; j < Z_ARRAY_SIZE(data_members_16);)
		{
		kv[i++] = rb_id2sym(rb_intern(data_members_16[j].name));
		kv[i++] = UINT2NUM(*(zuint16 *)(void *)((char *)obj + data_members_16[j++].offset));
		}

	for (j = 0; j < Z_ARRAY_SIZE(data_members_8);)
		{
		kv[i++] = rb_id2sym(rb_intern(data_members_8[j].name));
		kv[i++] = UINT2NUM(*((zuint8 *)obj + data_members_8[j++].offset));
		}

	rb_hash_bulk_insert_into_st_table(Z_ARRAY_SIZE(kv), kv, hash);
	return hash;
	}


static char const one_hyphen[2] = "1-";


static VALUE Z80__print(VALUE self)
	{
	OBJ;
	zuint8 f = Z80_F(*obj);

	printf(	"PC %04" PRIX16		   "  AF %04" PRIX16 "  AF' %04" PRIX16 "  IX %04" PRIX16 "\n"
		"SP %04" PRIX16		   "  BC %04" PRIX16 "  BC' %04" PRIX16 "  IY %04" PRIX16 "\n"
		"IR %02" PRIX8 "%02" PRIX8 "  DE %04" PRIX16 "  DE' %04" PRIX16 "  XY %04" PRIX16 "\n"
		"WZ %04" PRIX16		   "  HL %04" PRIX16 "  HL' %04" PRIX16 "  Q  %02" PRIX8  "\n"
		"S Z Y H X P N C"      "  IFF1 %" PRIu8 "  IM %" PRIu8 "  EI %" PRIu8 "\n"
		"%c %c %c %c %c %c %c %c  IFF2 %" PRIu8 "  R7 %" PRIu8 "  RI %" PRIu8 "\n",
		Z80_PC(*obj),	  Z80_AF(*obj), Z80_AF_(*obj), Z80_IX(*obj),
		Z80_SP(*obj),	  Z80_BC(*obj), Z80_BC_(*obj), Z80_IY(*obj),
		obj->i, obj->r,	  Z80_DE(*obj), Z80_DE_(*obj), Z80_IX(*obj),
		Z80_MEMPTR(*obj), Z80_HL(*obj), Z80_HL_(*obj), obj->q,
		obj->iff1, obj->im, '\0',
		one_hyphen[!(f & Z80_SF)],
		one_hyphen[!(f & Z80_ZF)],
		one_hyphen[!(f & Z80_YF)],
		one_hyphen[!(f & Z80_HF)],
		one_hyphen[!(f & Z80_XF)],
		one_hyphen[!(f & Z80_PF)],
		one_hyphen[!(f & Z80_NF)],
		one_hyphen[!(f & Z80_CF)],
		obj->iff2, obj->r7 >> 7, '\0');

	return Qnil;
	}


/*static VALUE Z80__to_s(VALUE self)
	{
	return Qnil;
	}*/


/*
PC 0000  AF 0000  AF' 0000  IX 0000
SP 0000  BC 0000  BC' 0000  IY 0000
IR 0000  DE 0000  DE' 0000  XY 0000
WZ 0000  HL 0000  HL' 0000  Q  00
S Z Y H X P N C  IFF1 0  IM 0  EI 1
- - 1 - - 1 - -  IFF2 0  R7 0  RI 1
/INT  high  RS 00  data 00 00 00 00
/HALT high  RQ 00  data 00 00 00 00*/

#define DEFINE_ACCESSOR(name)					\
	rb_define_method(klass, #name ,	   Z80__##name,	    0); \
	rb_define_method(klass, #name "=", Z80__set_##name, 1);


void Init_z80(void)
	{
	VALUE klass = rb_const_get(rb_cObject, rb_intern("Z80"));
	VALUE module;

	rb_define_alloc_func(klass, (VALUE (*)(VALUE))Z80__alloc);

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

	rb_define_method(klass, "initialize",      Z80__init,		 0);
	rb_define_method(klass, "free",	           Z80__release,	 0);
	rb_define_method(klass, "power",           Z80__power,		 1);
	rb_define_method(klass, "instant_reset",   Z80__instant_reset,	 0);
	rb_define_method(klass, "int",		   Z80__int,		 1);
	rb_define_method(klass, "nmi",		   Z80__nmi,		 0);
	rb_define_method(klass, "execute",	   Z80__execute,	 1);
	rb_define_method(klass, "run",		   Z80__run,		 1);
	rb_define_method(klass, "full_r",          Z80__refresh_address, 0);
	rb_define_method(klass, "refresh_address", Z80__refresh_address, 0);
	rb_define_method(klass, "in_cycle",	   Z80__in_cycle,	 0);
	rb_define_method(klass, "out_cycle",	   Z80__out_cycle,	 0);

	DEFINE_ACCESSOR(cycles	   )
	DEFINE_ACCESSOR(cycle_limit)
	DEFINE_ACCESSOR(memptr	   )
	DEFINE_ACCESSOR(pc	   )
	DEFINE_ACCESSOR(sp	   )
	DEFINE_ACCESSOR(xy	   )
	DEFINE_ACCESSOR(ix	   )
	DEFINE_ACCESSOR(iy	   )
	DEFINE_ACCESSOR(af	   )
	DEFINE_ACCESSOR(bc	   )
	DEFINE_ACCESSOR(de	   )
	DEFINE_ACCESSOR(hl	   )
	DEFINE_ACCESSOR(af_	   )
	DEFINE_ACCESSOR(bc_	   )
	DEFINE_ACCESSOR(de_	   )
	DEFINE_ACCESSOR(hl_	   )
	DEFINE_ACCESSOR(memptrh	   )
	DEFINE_ACCESSOR(memptrl	   )
	DEFINE_ACCESSOR(pch	   )
	DEFINE_ACCESSOR(pcl	   )
	DEFINE_ACCESSOR(sph	   )
	DEFINE_ACCESSOR(spl	   )
	DEFINE_ACCESSOR(xyh	   )
	DEFINE_ACCESSOR(yl	   )
	DEFINE_ACCESSOR(ixh	   )
	DEFINE_ACCESSOR(ixl	   )
	DEFINE_ACCESSOR(iyh	   )
	DEFINE_ACCESSOR(iyl	   )
	DEFINE_ACCESSOR(a	   )
	DEFINE_ACCESSOR(f	   )
	DEFINE_ACCESSOR(b	   )
	DEFINE_ACCESSOR(c	   )
	DEFINE_ACCESSOR(d	   )
	DEFINE_ACCESSOR(e	   )
	DEFINE_ACCESSOR(h	   )
	DEFINE_ACCESSOR(l	   )
	DEFINE_ACCESSOR(a_	   )
	DEFINE_ACCESSOR(f_	   )
	DEFINE_ACCESSOR(b_	   )
	DEFINE_ACCESSOR(c_	   )
	DEFINE_ACCESSOR(d_	   )
	DEFINE_ACCESSOR(e_	   )
	DEFINE_ACCESSOR(h_	   )
	DEFINE_ACCESSOR(l_	   )
	DEFINE_ACCESSOR(r	   )
	DEFINE_ACCESSOR(i	   )
	DEFINE_ACCESSOR(r7	   )
	DEFINE_ACCESSOR(im	   )
	DEFINE_ACCESSOR(request	   )
	DEFINE_ACCESSOR(iff1	   )
	DEFINE_ACCESSOR(iff2	   )
	DEFINE_ACCESSOR(q	   )
	DEFINE_ACCESSOR(options	   )
	DEFINE_ACCESSOR(int_line   )
	DEFINE_ACCESSOR(halt_line  )
	DEFINE_ACCESSOR(sf	   )
	DEFINE_ACCESSOR(zf	   )
	DEFINE_ACCESSOR(yf	   )
	DEFINE_ACCESSOR(hf	   )
	DEFINE_ACCESSOR(xf	   )
	DEFINE_ACCESSOR(pf	   )
	DEFINE_ACCESSOR(nf	   )
	DEFINE_ACCESSOR(cf	   )

	rb_define_method(klass, "to_h",	 Z80__to_h,  0);
	rb_define_method(klass, "print", Z80__print, 0);
/*	rb_define_method(klass, "to_s",	 Z80__to_s,  0);*/

	rb_define_alias(klass, "t",	"cycles"  );
	rb_define_alias(klass, "t=",	"cycles=" );
	rb_define_alias(klass, "wz",	"memptr"  );
	rb_define_alias(klass, "wz=",	"memptr=" );
	rb_define_alias(klass, "w",	"memptrh" );
	rb_define_alias(klass, "w=",	"memptrh=");
	rb_define_alias(klass, "z",	"memptrl" );
	rb_define_alias(klass, "z=",	"memptrl=");
	rb_define_alias(klass, "vf",	"pf"	  );
	rb_define_alias(klass, "vf=",	"pf="	  );
	rb_define_alias(klass, "state", "to_h"	  );
	}


/* z80.c EOF */
