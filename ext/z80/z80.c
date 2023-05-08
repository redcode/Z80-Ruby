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

#if Z_USIZE_BITS == Z_UINT_BITS
#	define VALUE_TO_USIZE NUM2UINT
#	define USIZE_TO_VALUE UINT2NUM
#else
#	define VALUE_TO_USIZE (zusize)NUM2ULL
#	define USIZE_TO_VALUE ULL2NUM
#endif

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

#define UINT_TO_BOOL(value) value ? Qtrue : Qfalse

#define MACRO( member, macro) macro(*obj)
#define DIRECT(member, macro) (*obj).member

#define ACCESSOR(type, member, access, with, c_to_ruby, ruby_to_c) \
	static VALUE Z80__##member(VALUE self)			   \
		{OBJ; return c_to_ruby(access(member, with));}	   \
								   \
	static VALUE Z80__set_##member(VALUE self, VALUE value)	   \
		{OBJ; access(member, with) = (type)ruby_to_c(value);}


ACCESSOR(zuint16, memptr,    MACRO,  Z80_MEMPTR,  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, pc,	     MACRO,  Z80_PC,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, sp,	     MACRO,  Z80_SP,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, xy,	     MACRO,  Z80_XY,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, ix,	     MACRO,  Z80_IX,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, iy,	     MACRO,  Z80_IY,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, af,	     MACRO,  Z80_AF,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, bc,	     MACRO,  Z80_BC,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, de,	     MACRO,  Z80_DE,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, hl,	     MACRO,  Z80_HL,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, af_,	     MACRO,  Z80_AF_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, bc_,	     MACRO,  Z80_BC_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, de_,	     MACRO,  Z80_DE_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint16, hl_,	     MACRO,  Z80_HL_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  memptrh,   MACRO,  Z80_MEMPTRH, UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  memptrl,   MACRO,  Z80_MEMPTRL, UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  pch,	     MACRO,  Z80_PCH,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  pcl,	     MACRO,  Z80_PCL,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  sph,	     MACRO,  Z80_SPH,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  spl,	     MACRO,  Z80_SPL,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  xyh,	     MACRO,  Z80_XYH,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  yl,	     MACRO,  Z80_XYL,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  ixh,	     MACRO,  Z80_IXH,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  ixl,	     MACRO,  Z80_IXL,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  iyh,	     MACRO,  Z80_IYH,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  iyl,	     MACRO,  Z80_IYL,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  a,	     MACRO,  Z80_A,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  f,	     MACRO,  Z80_F,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  b,	     MACRO,  Z80_B,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  c,	     MACRO,  Z80_C,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  d,	     MACRO,  Z80_D,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  e,	     MACRO,  Z80_E,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  h,	     MACRO,  Z80_H,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  l,	     MACRO,  Z80_L,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  a_,	     MACRO,  Z80_A_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  f_,	     MACRO,  Z80_F_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  b_,	     MACRO,  Z80_B_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  c_,	     MACRO,  Z80_C_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  d_,	     MACRO,  Z80_D_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  e_,	     MACRO,  Z80_E_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  h_,	     MACRO,  Z80_H_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  l_,	     MACRO,  Z80_L_,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  r,	     DIRECT, Z_EMPTY,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  i,	     DIRECT, Z_EMPTY,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  r7,	     DIRECT, Z_EMPTY,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  im,	     DIRECT, Z_EMPTY,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  request,   DIRECT, Z_EMPTY,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  iff1,	     DIRECT, Z_EMPTY,	  UINT_TO_BOOL, RB_TEST )
ACCESSOR(zuint8,  iff2,	     DIRECT, Z_EMPTY,	  UINT_TO_BOOL, RB_TEST )
ACCESSOR(zuint8,  q,	     DIRECT, Z_EMPTY,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  options,   DIRECT, Z_EMPTY,	  UINT2NUM,	NUM2UINT)
ACCESSOR(zuint8,  int_line,  DIRECT, Z_EMPTY,	  UINT_TO_BOOL,	RB_TEST )
ACCESSOR(zuint8,  halt_line, DIRECT, Z_EMPTY,	  UINT_TO_BOOL,	RB_TEST )


static VALUE Z80__release	 (VALUE self)		   {OBJ; return self;}
static VALUE Z80__power		 (VALUE self, VALUE state) {OBJ; z80_power(obj, RB_TEST(state)); return self;}
static VALUE Z80__instant_reset	 (VALUE self)		   {OBJ; z80_instant_reset(obj); return self;}
static VALUE Z80__int		 (VALUE self, VALUE state) {OBJ; z80_int(obj, RB_TEST(state)); return self;}
static VALUE Z80__nmi		 (VALUE self)		   {OBJ; z80_nmi(obj); return self;}
static VALUE Z80__execute	 (VALUE self, VALUE cycles){OBJ; return USIZE_TO_VALUE(z80_execute(obj, VALUE_TO_USIZE(cycles)));}
static VALUE Z80__run		 (VALUE self, VALUE cycles){OBJ; return USIZE_TO_VALUE(z80_run    (obj, VALUE_TO_USIZE(cycles)));}
static VALUE Z80__full_r	 (VALUE self)		   {OBJ; return UINT2NUM(z80_r		    (obj));}
static VALUE Z80__refresh_address(VALUE self)		   {OBJ; return UINT2NUM(z80_refresh_address(obj));}
static VALUE Z80__in_cycle	 (VALUE self)		   {OBJ; return UINT2NUM(z80_in_cycle	    (obj));}
static VALUE Z80__out_cycle	 (VALUE self)		   {OBJ; return UINT2NUM(z80_out_cycle	    (obj));}


#define DEFINE_ACCESSOR(name)			\
	rb_define_method(klass, #name ,	   Z80__##name,	    0); \
	rb_define_method(klass, #name "=", Z80__set_##name, 1);


void Init_z80(void)
	{
	VALUE klass = rb_const_get(rb_cObject, rb_intern("Z80"));

	rb_define_alloc_func(klass, (VALUE (*)(VALUE))Z80__alloc);

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
	DEFINE_ACCESSOR(memptr	 )
	DEFINE_ACCESSOR(pc	 )
	DEFINE_ACCESSOR(sp	 )
	DEFINE_ACCESSOR(xy	 )
	DEFINE_ACCESSOR(ix	 )
	DEFINE_ACCESSOR(iy	 )
	DEFINE_ACCESSOR(af	 )
	DEFINE_ACCESSOR(bc	 )
	DEFINE_ACCESSOR(de	 )
	DEFINE_ACCESSOR(hl	 )
	DEFINE_ACCESSOR(af_	 )
	DEFINE_ACCESSOR(bc_	 )
	DEFINE_ACCESSOR(de_	 )
	DEFINE_ACCESSOR(hl_	 )
	DEFINE_ACCESSOR(memptrh	 )
	DEFINE_ACCESSOR(memptrl	 )
	DEFINE_ACCESSOR(pch	 )
	DEFINE_ACCESSOR(pcl	 )
	DEFINE_ACCESSOR(sph	 )
	DEFINE_ACCESSOR(spl	 )
	DEFINE_ACCESSOR(xyh	 )
	DEFINE_ACCESSOR(yl	 )
	DEFINE_ACCESSOR(ixh	 )
	DEFINE_ACCESSOR(ixl	 )
	DEFINE_ACCESSOR(iyh	 )
	DEFINE_ACCESSOR(iyl	 )
	DEFINE_ACCESSOR(a	 )
	DEFINE_ACCESSOR(f	 )
	DEFINE_ACCESSOR(b	 )
	DEFINE_ACCESSOR(c	 )
	DEFINE_ACCESSOR(d	 )
	DEFINE_ACCESSOR(e	 )
	DEFINE_ACCESSOR(h	 )
	DEFINE_ACCESSOR(l	 )
	DEFINE_ACCESSOR(a_	 )
	DEFINE_ACCESSOR(f_	 )
	DEFINE_ACCESSOR(b_	 )
	DEFINE_ACCESSOR(c_	 )
	DEFINE_ACCESSOR(d_	 )
	DEFINE_ACCESSOR(e_	 )
	DEFINE_ACCESSOR(h_	 )
	DEFINE_ACCESSOR(l_	 )
	DEFINE_ACCESSOR(r	 )
	DEFINE_ACCESSOR(i	 )
	DEFINE_ACCESSOR(r7	 )
	DEFINE_ACCESSOR(im	 )
	DEFINE_ACCESSOR(request	 )
	DEFINE_ACCESSOR(iff1	 )
	DEFINE_ACCESSOR(iff2	 )
	DEFINE_ACCESSOR(q	 )
	DEFINE_ACCESSOR(options	 )
	DEFINE_ACCESSOR(int_line )
	DEFINE_ACCESSOR(halt_line)
	}


/* z80.c EOF */
