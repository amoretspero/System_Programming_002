	.file	"Q1.c"
	.comm	pid,4,4
	.globl	counter
	.data
	.align 4
	.type	counter, @object
	.size	counter, 4
counter:
	.long	1
	.section	.rodata
.LC0:
	.string	"%d"
	.text
	.globl	handler1
	.type	handler1, @function
handler1:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	$5, counter(%rip)
	movl	counter(%rip), %eax
	movl	%eax, %esi
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movq	stdout(%rip), %rax
	movq	%rax, %rdi
	call	fflush
	movl	$0, %edi
	call	exit
	.cfi_endproc
.LFE2:
	.size	handler1, .-handler1
	.section	.rodata
.LC1:
	.string	"%d%d"
	.text
	.globl	main
	.type	main, @function
main:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$handler1, %esi
	movl	$10, %edi
	call	signal
	movl	counter(%rip), %eax
	movl	%eax, %esi
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movq	stdout(%rip), %rax
	movq	%rax, %rdi
	call	fflush
	call	fork
	movl	%eax, pid(%rip)
	movl	pid(%rip), %eax
	testl	%eax, %eax
	jne	.L3
.L4:
	jmp	.L4
.L3:
	movl	pid(%rip), %eax
	movl	$10, %esi
	movl	%eax, %edi
	call	kill
	movl	$0, %edx
	movl	$0, %esi
	movl	$-1, %edi
	movl	$0, %eax
	call	waitpid
	movl	counter(%rip), %eax
	addl	$1, %eax
	movl	%eax, counter(%rip)
	movl	counter(%rip), %edx
	movl	counter(%rip), %eax
	movl	%eax, %esi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %edi
	call	exit
	.cfi_endproc
.LFE3:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
