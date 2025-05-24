
	.section	".text",#alloc,#execinstr
	.align	8
	.skip	16

	! block 0

	.global	swap_word
	.type	swap_word,2
swap_word:
	save	%sp,-104,%sp
	st	%i1,[%fp+72]
	st	%i0,[%fp+68]

	! block 1
.L14:

! File swap.c:
!    2	void swap_word(int *a,int *b)
!    Note that atomic operation is done on a.....
!	[%fp+68] is address of a
!	[%fp+72] is address of b
!    3	{
!    4		int temp;
!    5		temp=*a;
! get address of a
	ld	[%fp+68],%l0	! get address of a into %l0
	ld	[%fp+72],%l1	! get address of b into %l1
	ld	[%l1+0],%l1		! get contents of b into %l1
	swap	[%l0+0],%l1	! swap %l1 with a


	ld	[%fp+72],%l0	! get address of b
	st	%l1,[%l0+0]		! put old value of a into b


	! block 2
.L13:
	jmp	%i7+8
	restore
	.align	8
	.size	swap_word,(.-swap_word)
