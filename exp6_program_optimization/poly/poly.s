	.file	"poly_202102001015.c"
	.text
	.globl	const_poly_eval
	.type	const_poly_eval, @function
const_poly_eval:
.LFB6:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%esi
	pushl	%ebx
	.cfi_offset 6, -12
	.cfi_offset 3, -16
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	16(%ebp), %eax
	imull	%eax, %eax
	movl	%eax, %ebx
	movl	%ebx, %eax
	sall	$7, %eax
	movl	%ebx, %edx
	sall	$5, %edx
	subl	%edx, %eax
	leal	0(,%ebx,4), %edx
	subl	%edx, %eax
	subl	%ebx, %eax
	movl	%eax, %esi
	imull	16(%ebp), %ebx
	movl	%ebx, %eax
	sall	$6, %eax
	addl	%eax, %ebx
	movl	16(%ebp), %eax
	sall	$5, %eax
	leal	68(%eax), %edx
	movl	16(%ebp), %eax
	addl	%eax, %eax
	addl	%edx, %eax
	addl	%esi, %eax
	addl	%ebx, %eax
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE6:
	.size	const_poly_eval, .-const_poly_eval
	.globl	poly_eval
	.type	poly_eval, @function
poly_eval:
.LFB7:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$16, %esp
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	$0, -12(%ebp)
	movl	$1, -4(%ebp)
	movl	$0, -8(%ebp)
	jmp	.L4
.L5:
	movl	-8(%ebp), %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	-4(%ebp), %eax
	addl	%eax, -12(%ebp)
	movl	-4(%ebp), %eax
	imull	16(%ebp), %eax
	movl	%eax, -4(%ebp)
	addl	$1, -8(%ebp)
.L4:
	movl	-8(%ebp), %eax
	cmpl	12(%ebp), %eax
	jle	.L5
	movl	-12(%ebp), %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE7:
	.size	poly_eval, .-poly_eval
	.globl	poly_eval_ext2
	.type	poly_eval_ext2, @function
poly_eval_ext2:
.LFB8:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	.cfi_offset 7, -12
	.cfi_offset 6, -16
	.cfi_offset 3, -20
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	$0, %edi
	movl	$0, -16(%ebp)
	movl	$1, %esi
	movl	16(%ebp), %eax
	movl	%eax, %ecx
	imull	16(%ebp), %eax
	movl	%eax, -20(%ebp)
	movl	12(%ebp), %eax
	subl	$1, %eax
	movl	%eax, -24(%ebp)
	movl	$0, %ebx
	jmp	.L8
.L9:
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%esi, %eax
	addl	%eax, %edi
	movl	%ebx, %eax
	addl	$1, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%ecx, %eax
	addl	%eax, -16(%ebp)
	movl	-20(%ebp), %eax
	imull	%eax, %esi
	imull	%eax, %ecx
	addl	$2, %ebx
.L8:
	cmpl	-24(%ebp), %ebx
	jle	.L9
	addl	-16(%ebp), %edi
	jmp	.L10
.L11:
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%esi, %eax
	addl	%eax, %edi
	imull	16(%ebp), %esi
	addl	$1, %ebx
.L10:
	cmpl	12(%ebp), %ebx
	jle	.L11
	movl	%edi, %eax
	addl	$12, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE8:
	.size	poly_eval_ext2, .-poly_eval_ext2
	.globl	poly_eval_ext3
	.type	poly_eval_ext3, @function
poly_eval_ext3:
.LFB9:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$20, %esp
	.cfi_offset 7, -12
	.cfi_offset 6, -16
	.cfi_offset 3, -20
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	$0, -16(%ebp)
	movl	$0, -20(%ebp)
	movl	$0, -24(%ebp)
	movl	$1, %esi
	movl	16(%ebp), %eax
	movl	%eax, %ecx
	movl	16(%ebp), %eax
	imull	%eax, %eax
	movl	%eax, %edi
	movl	%edi, %eax
	imull	16(%ebp), %eax
	movl	%eax, -28(%ebp)
	movl	12(%ebp), %eax
	subl	$2, %eax
	movl	%eax, -32(%ebp)
	movl	$0, %ebx
	jmp	.L14
.L15:
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%esi, %eax
	addl	%eax, -16(%ebp)
	movl	%ebx, %eax
	addl	$1, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%ecx, %eax
	addl	%eax, -20(%ebp)
	movl	%ebx, %eax
	addl	$2, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%edi, %eax
	addl	%eax, -24(%ebp)
	movl	-28(%ebp), %eax
	imull	%eax, %esi
	imull	%eax, %ecx
	imull	%eax, %edi
	addl	$3, %ebx
.L14:
	cmpl	-32(%ebp), %ebx
	jle	.L15
	movl	-20(%ebp), %edi
	addl	%edi, -16(%ebp)
	movl	-16(%ebp), %eax
	movl	-24(%ebp), %ecx
	addl	%ecx, %eax
	movl	%eax, -16(%ebp)
	jmp	.L16
.L17:
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%esi, %eax
	addl	%eax, -16(%ebp)
	imull	16(%ebp), %esi
	addl	$1, %ebx
.L16:
	cmpl	12(%ebp), %ebx
	jle	.L17
	movl	-16(%ebp), %eax
	addl	$20, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE9:
	.size	poly_eval_ext3, .-poly_eval_ext3
	.globl	poly_eval_ext4
	.type	poly_eval_ext4, @function
poly_eval_ext4:
.LFB10:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$32, %esp
	.cfi_offset 7, -12
	.cfi_offset 6, -16
	.cfi_offset 3, -20
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	$0, -16(%ebp)
	movl	$0, -20(%ebp)
	movl	$0, -24(%ebp)
	movl	$0, -28(%ebp)
	movl	$1, %edi
	movl	16(%ebp), %eax
	movl	%eax, -32(%ebp)
	movl	16(%ebp), %eax
	imull	%eax, %eax
	movl	%eax, %esi
	movl	%esi, %eax
	imull	16(%ebp), %eax
	movl	%eax, -36(%ebp)
	movl	%esi, %eax
	imull	%esi, %eax
	movl	%eax, -40(%ebp)
	movl	12(%ebp), %eax
	subl	$3, %eax
	movl	%eax, -44(%ebp)
	movl	$0, %ebx
	jmp	.L20
.L21:
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%edi, %eax
	addl	%eax, -16(%ebp)
	movl	%ebx, %eax
	addl	$1, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	movl	-32(%ebp), %ecx
	imull	%ecx, %eax
	addl	%eax, -20(%ebp)
	movl	%ebx, %eax
	addl	$2, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%esi, %eax
	addl	%eax, -24(%ebp)
	movl	%ebx, %eax
	addl	$3, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	movl	-36(%ebp), %edx
	imull	%edx, %eax
	addl	%eax, -28(%ebp)
	movl	-40(%ebp), %eax
	imull	%eax, %edi
	imull	%eax, %ecx
	movl	%ecx, -32(%ebp)
	imull	%eax, %esi
	imull	%eax, %edx
	movl	%edx, -36(%ebp)
	addl	$4, %ebx
.L20:
	cmpl	-44(%ebp), %ebx
	jle	.L21
	movl	-20(%ebp), %esi
	addl	%esi, -16(%ebp)
	movl	-16(%ebp), %eax
	movl	-24(%ebp), %ecx
	addl	%ecx, %eax
	movl	-28(%ebp), %esi
	addl	%esi, %eax
	movl	%eax, -16(%ebp)
	jmp	.L22
.L23:
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%edi, %eax
	addl	%eax, -16(%ebp)
	imull	16(%ebp), %edi
	addl	$1, %ebx
.L22:
	cmpl	12(%ebp), %ebx
	jle	.L23
	movl	-16(%ebp), %eax
	addl	$32, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE10:
	.size	poly_eval_ext4, .-poly_eval_ext4
	.globl	poly_eval_ext5
	.type	poly_eval_ext5, @function
poly_eval_ext5:
.LFB11:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$40, %esp
	.cfi_offset 7, -12
	.cfi_offset 6, -16
	.cfi_offset 3, -20
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	$0, -16(%ebp)
	movl	$0, -36(%ebp)
	movl	$0, -40(%ebp)
	movl	$0, -44(%ebp)
	movl	$0, -48(%ebp)
	movl	$1, -20(%ebp)
	movl	16(%ebp), %eax
	movl	%eax, -24(%ebp)
	movl	16(%ebp), %eax
	imull	%eax, %eax
	movl	%eax, -28(%ebp)
	movl	%eax, %esi
	imull	16(%ebp), %esi
	movl	%esi, %ecx
	imull	16(%ebp), %ecx
	movl	%ecx, -32(%ebp)
	imull	%esi, %eax
	movl	%eax, %edi
	movl	12(%ebp), %eax
	subl	$4, %eax
	movl	%eax, -52(%ebp)
	movl	$0, %ebx
	jmp	.L26
.L27:
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	movl	-20(%ebp), %ecx
	imull	%ecx, %eax
	addl	%eax, -16(%ebp)
	movl	%ebx, %eax
	addl	$1, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	-24(%ebp), %eax
	addl	%eax, -36(%ebp)
	movl	%ebx, %eax
	addl	$2, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	-28(%ebp), %eax
	addl	%eax, -40(%ebp)
	movl	%ebx, %eax
	addl	$3, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	%esi, %eax
	addl	%eax, -44(%ebp)
	movl	%ebx, %eax
	addl	$4, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	imull	-32(%ebp), %eax
	addl	%eax, -48(%ebp)
	imull	%edi, %ecx
	movl	%ecx, -20(%ebp)
	movl	-24(%ebp), %edx
	imull	%edi, %edx
	movl	%edx, -24(%ebp)
	movl	-28(%ebp), %edx
	imull	%edi, %edx
	movl	%edx, -28(%ebp)
	imull	%edi, %esi
	movl	-32(%ebp), %edx
	imull	%edi, %edx
	movl	%edx, -32(%ebp)
	addl	$5, %ebx
.L26:
	cmpl	-52(%ebp), %ebx
	jle	.L27
	movl	-36(%ebp), %ecx
	addl	%ecx, -16(%ebp)
	movl	-16(%ebp), %eax
	movl	-40(%ebp), %esi
	addl	%esi, %eax
	movl	-44(%ebp), %ecx
	addl	%ecx, %eax
	movl	-48(%ebp), %esi
	addl	%esi, %eax
	movl	%eax, -16(%ebp)
	jmp	.L28
.L29:
	movl	%ebx, %eax
	leal	0(,%eax,4), %edx
	movl	8(%ebp), %eax
	addl	%edx, %eax
	movl	(%eax), %eax
	movl	-20(%ebp), %edi
	imull	%edi, %eax
	addl	%eax, -16(%ebp)
	imull	16(%ebp), %edi
	movl	%edi, -20(%ebp)
	addl	$1, %ebx
.L28:
	cmpl	12(%ebp), %ebx
	jle	.L29
	movl	-16(%ebp), %eax
	addl	$40, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE11:
	.size	poly_eval_ext5, .-poly_eval_ext5
	.globl	poly_eval_q
	.type	poly_eval_q, @function
poly_eval_q:
.LFB12:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%esi
	pushl	%ebx
	.cfi_offset 6, -12
	.cfi_offset 3, -16
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	$0, %esi
	movl	12(%ebp), %ebx
	jmp	.L32
.L33:
	movl	%esi, %edx
	imull	16(%ebp), %edx
	movl	%ebx, %eax
	leal	0(,%eax,4), %ecx
	movl	8(%ebp), %eax
	addl	%ecx, %eax
	movl	(%eax), %eax
	leal	(%edx,%eax), %esi
	subl	$1, %ebx
.L32:
	testl	%ebx, %ebx
	jns	.L33
	movl	%esi, %eax
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE12:
	.size	poly_eval_q, .-poly_eval_q
	.globl	peval_fun_tab
	.section	.rodata
.LC0:
	.string	"poly_eval\347\232\204\345\276\252\347\216\257\345\261\225\345\274\2003"
.LC1:
	.string	"poly_eval\347\232\204\345\276\252\347\216\257\345\261\225\345\274\2002"
.LC2:
	.string	"poly_eval\347\232\204\345\276\252\347\216\257\345\261\225\345\274\2004"
.LC3:
	.string	"poly_eval\347\232\204\345\276\252\347\216\257\345\261\225\345\274\2005"
.LC4:
	.string	""
	.section	.data.rel.local,"aw"
	.align 32
	.type	peval_fun_tab, @object
	.size	peval_fun_tab, 40
peval_fun_tab:
	.long	poly_eval_ext3
	.long	.LC0
	.long	poly_eval_ext2
	.long	.LC1
	.long	poly_eval_ext4
	.long	.LC2
	.long	poly_eval_ext5
	.long	.LC3
	.long	0
	.long	.LC4
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
.LFB13:
	.cfi_startproc
	movl	(%esp), %eax
	ret
	.cfi_endproc
.LFE13:
	.ident	"GCC: (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0"
	.section	.note.GNU-stack,"",@progbits
