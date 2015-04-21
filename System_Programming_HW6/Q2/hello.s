	.file	"hello.c"
	.section	.rodata
.LC0:
	.string	"What is your student ID? :\n"
.LC1:
	.string	"SID : "
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$64, %rsp
	movl	%edi, -52(%rbp)
	movq	%rsi, -64(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movl	$27, %edx
	movl	$.LC0, %esi
	movl	$1, %edi
	movl	$0, %eax
	call	write
	leaq	-32(%rbp), %rax
	movl	$10, %edx
	movq	%rax, %rsi
	movl	$2, %edi
	movl	$0, %eax
	call	read
	movl	%eax, -36(%rbp)
	movl	$6, %edx
	movl	$.LC1, %esi
	movl	$1, %edi
	movl	$0, %eax
	call	write
	movl	-36(%rbp), %edx
	leaq	-32(%rbp), %rax
	movq	%rax, %rsi
	movl	$1, %edi
	movl	$0, %eax
	call	write
	movl	$0, %edi
	call	exit
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
