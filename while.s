	.text
	.file	"main"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$16, %rsp
	.cfi_def_cfa_offset 32
	.cfi_offset %rbx, -16
	movl	$0, 12(%rsp)
	leaq	.L__unnamed_1(%rip), %rbx
	cmpl	$4, 12(%rsp)
	jg	.LBB0_3
	.p2align	4, 0x90
.LBB0_2:
	movl	12(%rsp), %esi
	movq	%rbx, %rdi
	xorl	%eax, %eax
	callq	printf@PLT
	incl	12(%rsp)
	cmpl	$4, 12(%rsp)
	jle	.LBB0_2
.LBB0_3:
	xorl	%eax, %eax
	addq	$16, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"%d\n"
	.size	.L__unnamed_1, 4

	.section	".note.GNU-stack","",@progbits
