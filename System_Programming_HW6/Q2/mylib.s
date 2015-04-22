# mylib.s
.section .text
.globl my_write
.globl my_read
.globl my_exit

#void my_write(unsigned int fd, const char* string, size_t length);

my_write :
	push	%ebp
	mov	%esp, %ebp
	push	%ebx

	mov	$4, %eax
	mov	8(%ebp), %ebx
	mov	12(%ebp), %ecx
	mov	16(%ebp), %edx

	int	$0x80

	pop 	%ebx
	pop	%ebp

	ret

#void my_read(unsigned int fd, const char* string, size_t length);

my_read :
	push	%ebp
	mov	%esp, %ebp
	push	%ebx

	mov	$3, %eax
	mov	8(%ebp), %ebx
	mov	12(%ebp), %ecx
	mov	16(%ebp), %edx

	int	$0x80

	pop 	%ebx
	pop	%ebp

	ret

#void my_exit(int nr);

my_exit :
	push	%ebp
	mov	%esp, %ebp
	push	%ebx

	mov	$1, %eax
	mov	8(%ebp), %ebx
	int	$0x80

	pop	%ebx
	pop	%ebp

	ret
