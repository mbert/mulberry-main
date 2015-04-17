





	.file	"x86cpuid.s"
.text
.globl	OPENSSL_ia32_cpuid
.type	OPENSSL_ia32_cpuid,@function
.align	16
OPENSSL_ia32_cpuid:
	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	xorl	%edx,		%edx
	pushfl
	popl	%eax
	movl	%eax,		%ecx
	xorl	$2097152,	%eax
	pushl	%eax
	popfl
	pushfl
	popl	%eax
	xorl	%eax,		%ecx
	btl	$21,		%ecx
	jnc	.L000done
	xorl	%eax,		%eax
	.byte	0x0f,0xa2
	xorl	%eax,		%eax
	cmpl	$1970169159,	%ebx
	.byte	15,149,192
	movl	%eax,		%ebp
	cmpl	$1231384169,	%edx
	.byte	15,149,192
	orl	%eax,		%ebp
	cmpl	$1818588270,	%ecx
	.byte	15,149,192
	orl	%eax,		%ebp
	movl	$1,		%eax
	.byte	0x0f,0xa2
	cmpl	$0,		%ebp
	jne	.L001notP4
	andl	$3840,		%eax
	cmpl	$3840,		%eax
	jne	.L001notP4
	orl	$1048576,	%edx
.L001notP4:
	btl	$28,		%edx
	jnc	.L000done
	shrl	$16,		%ebx
	andl	$255,		%ebx
	cmpl	$1,		%ebx
	ja	.L000done
	andl	$4026531839,	%edx
.L000done:
	movl	%edx,		%eax
	movl	%ecx,		%edx
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret
.L_OPENSSL_ia32_cpuid_end:
.size	OPENSSL_ia32_cpuid,.L_OPENSSL_ia32_cpuid_end-OPENSSL_ia32_cpuid
.ident	"OPENSSL_ia32_cpuid"
.text
.globl	OPENSSL_rdtsc
.type	OPENSSL_rdtsc,@function
.align	16
OPENSSL_rdtsc:
	xorl	%eax,		%eax
	xorl	%edx,		%edx
	leal	OPENSSL_ia32cap_P,%ecx
	btl	$4,		(%ecx)
	jnc	.L002notsc
	.byte	0x0f,0x31
.L002notsc:
	ret
.L_OPENSSL_rdtsc_end:
.size	OPENSSL_rdtsc,.L_OPENSSL_rdtsc_end-OPENSSL_rdtsc
.ident	"OPENSSL_rdtsc"
.text
.globl	OPENSSL_instrument_halt
.type	OPENSSL_instrument_halt,@function
.align	16
OPENSSL_instrument_halt:
	leal	OPENSSL_ia32cap_P,%ecx
	btl	$4,		(%ecx)
	jnc	.L003nohalt
	.long	2421723150
	andl	$3,		%eax
	jnz	.L003nohalt
	pushfl
	popl	%eax
	btl	$9,		%eax
	jnc	.L003nohalt
	.byte	0x0f,0x31
	pushl	%edx
	pushl	%eax
	hlt
	.byte	0x0f,0x31
	subl	(%esp),		%eax
	sbbl	4(%esp),	%edx
	addl	$8,		%esp
	ret
.L003nohalt:
	xorl	%eax,		%eax
	xorl	%edx,		%edx
	ret
.L_OPENSSL_instrument_halt_end:
.size	OPENSSL_instrument_halt,.L_OPENSSL_instrument_halt_end-OPENSSL_instrument_halt
.ident	"OPENSSL_instrument_halt"
.text
.globl	OPENSSL_far_spin
.type	OPENSSL_far_spin,@function
.align	16
OPENSSL_far_spin:
	pushfl
	popl	%eax
	btl	$9,		%eax
	jnc	.L004nospin
	movl	4(%esp),	%eax
	movl	8(%esp),	%ecx
	.long	2430111262
	xorl	%eax,		%eax
	movl	(%ecx),		%edx
	jmp	.L005spin
.align	16
.L005spin:
	incl	%eax
	cmpl	(%ecx),		%edx
	je	.L005spin
	.long	529567888
	ret
.L004nospin:
	xorl	%eax,		%eax
	xorl	%edx,		%edx
	ret
.L_OPENSSL_far_spin_end:
.size	OPENSSL_far_spin,.L_OPENSSL_far_spin_end-OPENSSL_far_spin
.ident	"OPENSSL_far_spin"
.text
.globl	OPENSSL_wipe_cpu
.type	OPENSSL_wipe_cpu,@function
.align	16
OPENSSL_wipe_cpu:
	xorl	%eax,		%eax
	xorl	%edx,		%edx
	leal	OPENSSL_ia32cap_P,%ecx
	movl	(%ecx),		%ecx
	btl	$1,		(%ecx)
	jnc	.L006no_x87
	btl	$26,		(%ecx)
	jnc	.L007no_sse2
	pxor	%xmm0,		%xmm0
	pxor	%xmm1,		%xmm1
	pxor	%xmm2,		%xmm2
	pxor	%xmm3,		%xmm3
	pxor	%xmm4,		%xmm4
	pxor	%xmm5,		%xmm5
	pxor	%xmm6,		%xmm6
	pxor	%xmm7,		%xmm7
.L007no_sse2:
	.long	4007259865,4007259865,4007259865,4007259865,2430851995
.L006no_x87:
	leal	4(%esp),	%eax
	ret
.L_OPENSSL_wipe_cpu_end:
.size	OPENSSL_wipe_cpu,.L_OPENSSL_wipe_cpu_end-OPENSSL_wipe_cpu
.ident	"OPENSSL_wipe_cpu"
.text
.globl	OPENSSL_atomic_add
.type	OPENSSL_atomic_add,@function
.align	16
OPENSSL_atomic_add:
	movl	4(%esp),	%edx
	movl	8(%esp),	%ecx
	pushl	%ebx
	nop
	movl	(%edx),		%eax
.L008spin:
	leal	(%eax,%ecx),	%ebx
	nop
	.long	447811568
	jne	.L008spin
	movl	%ebx,		%eax
	popl	%ebx
	ret
.L_OPENSSL_atomic_add_end:
.size	OPENSSL_atomic_add,.L_OPENSSL_atomic_add_end-OPENSSL_atomic_add
.ident	"OPENSSL_atomic_add"
.text
.globl	OPENSSL_indirect_call
.type	OPENSSL_indirect_call,@function
.align	16
OPENSSL_indirect_call:
	pushl	%ebp
	movl	%esp,		%ebp
	subl	$28,		%esp
	movl	12(%ebp),	%ecx
	movl	%ecx,		(%esp)
	movl	16(%ebp),	%edx
	movl	%edx,		4(%esp)
	movl	20(%ebp),	%eax
	movl	%eax,		8(%esp)
	movl	24(%ebp),	%eax
	movl	%eax,		12(%esp)
	movl	28(%ebp),	%eax
	movl	%eax,		16(%esp)
	movl	32(%ebp),	%eax
	movl	%eax,		20(%esp)
	movl	36(%ebp),	%eax
	movl	%eax,		24(%esp)
	call	*8(%ebp)
	movl	%ebp,		%esp
	popl	%ebp
	ret
.L_OPENSSL_indirect_call_end:
.size	OPENSSL_indirect_call,.L_OPENSSL_indirect_call_end-OPENSSL_indirect_call
.ident	"OPENSSL_indirect_call"
.section	.init
	call	OPENSSL_cpuid_setup
	jmp	.Linitalign
.align	16
.Linitalign:

.section	.bss
.comm	OPENSSL_ia32cap_P,4,4
