! This routine attempts to store a u_int value
! to the alternate space 0x2f. I do not know if
! it is working or not. This is needed for SS1000
! (i.e. sun4d architecture.) This has to be debugged
! in the process of making sun4d timer14 work.
	.section	".text",#alloc,#execinstr
	.align	8
	.skip	16

	! block 0

	.global	st_a_2f
	.type	st_a_2f,2
st_a_2f:
	save	%sp,-96,%sp
	st	%i1,[%fp+72]
	st	%i0,[%fp+68]

	! block 1
.L48:

! File ld_st_alt.c:
!    1	#include <sys/types.h>
!    2	
!    3	void	st_a_2f(u_int value, u_int *addr)
!    4	{
!    5		*addr=value;

	ld	[%fp+68],%l1
	ld	[%fp+72],%l0
	sta	%l1,[%l0]0x2f

	! block 2
.L47:
	jmp	%i7+8
	restore
	.size	st_a_2f,(.-st_a_2f)
	.align	8
	.align	8
	.skip	16

	! block 0

	.global	ld_a_2f
	.type	ld_a_2f,2
ld_a_2f:
	save	%sp,-104,%sp
	st	%i0,[%fp+68]

	! block 1
.L52:

! File ld_st_alt.c:
!    6	}
!    7	
!   11	u_int	ld_a_2f(u_int *addr)
!   12	{
!   13		return(*addr);

	ld	[%fp+68],%l0
	lda	[%l0]0x2f,%l0
	st	%l0,[%fp-4]
	ba	.L51
	nop

	! block 2
.L51:
	ld	[%fp-4],%l0
	mov	%l0,%i0
	jmp	%i7+8
	restore
	.size	ld_a_2f,(.-ld_a_2f)
	.align	8

	.file	"ld_st_alt.c"
	.xstabs	".stab.index","Xt ; V=3.0 ; R=3.0",60,0,0,0
	.xstabs	".stab.index","/home/sinan/TIMER14/ddrv; /opt/SUNWspro/bin/../SC3.0.1/bin/cc -S  ld_st_alt.c -W0,-xp",52,0,0,0
	.ident	"@(#)types.h	1.30	93/11/09 SMI"
	.ident	"@(#)feature_tests.h	1.6	93/07/09 SMI"
	.ident	"@(#)machtypes.h	1.8	93/07/09 SMI"
	.ident	"@(#)isa_defs.h	1.1	93/07/01 SMI"
	.ident	"@(#)select.h	1.10	92/07/14 SMI"
	.ident	"@(#)time.h	2.40	94/05/24 SMI"
	.ident	"@(#)time.h	1.21	94/06/08 SMI"
	.ident	"@(#)siginfo.h	1.34	93/08/03 SMI"
	.ident	"@(#)machsig.h	1.9	92/07/14 SMI"
	.ident	"acomp: SC3.0.1 13 Jul 1994 Sun C 3.0.1"
