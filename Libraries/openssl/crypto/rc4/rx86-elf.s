





	.file	"rc4-586.s"
.text
.globl	RC4
.type	RC4,@function
.align	16
RC4:
	movl	8(%esp),	%edx
	cmpl	$0,		%edx
	jne	.L000proceed
	ret
.L000proceed:

	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	xorl	%eax,		%eax
	pushl	%edi
	xorl	%ebx,		%ebx
	movl	20(%esp),	%ebp
	movl	28(%esp),	%esi
	movb	(%ebp),		%al
	movb	4(%ebp),	%bl
	movl	32(%esp),	%edi
	incb	%al
	subl	$12,		%esp
	addl	$8,		%ebp
	cmpl	$-1,		256(%ebp)
	je	.L001RC4_CHAR
	leal	-8(%edx,%esi),	%edx
	movl	%edx,		8(%esp)
	movl	(%ebp,%eax,4),	%ecx
	cmpl	%esi,		%edx
	jb	.L002end
.L003start:
	addl	$8,		%esi

	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	%dl,		(%esp)

	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	%dl,		1(%esp)

	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	%dl,		2(%esp)

	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	%dl,		3(%esp)

	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	%dl,		4(%esp)

	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	%dl,		5(%esp)

	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	%dl,		6(%esp)

	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%edx,4),	%edx
	addl	$8,		%edi
	movb	%dl,		7(%esp)

	movl	(%esp),		%ecx
	movl	-8(%esi),	%edx
	xorl	%edx,		%ecx
	movl	-4(%esi),	%edx
	movl	%ecx,		-8(%edi)
	movl	4(%esp),	%ecx
	xorl	%edx,		%ecx
	movl	8(%esp),	%edx
	movl	%ecx,		-4(%edi)
	movl	(%ebp,%eax,4),	%ecx
	cmpl	%edx,		%esi
	jbe	.L003start
.L002end:

	addl	$8,		%edx
	incl	%esi
	cmpl	%esi,		%edx
	jb	.L004finished
	movl	%edx,		8(%esp)
	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	-1(%esi),	%dh
	xorb	%dh,		%dl
	movb	%dl,		(%edi)

	movl	8(%esp),	%edx
	cmpl	%esi,		%edx
	jbe	.L004finished
	incl	%esi
	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	-1(%esi),	%dh
	xorb	%dh,		%dl
	movb	%dl,		1(%edi)

	movl	8(%esp),	%edx
	cmpl	%esi,		%edx
	jbe	.L004finished
	incl	%esi
	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	-1(%esi),	%dh
	xorb	%dh,		%dl
	movb	%dl,		2(%edi)

	movl	8(%esp),	%edx
	cmpl	%esi,		%edx
	jbe	.L004finished
	incl	%esi
	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	-1(%esi),	%dh
	xorb	%dh,		%dl
	movb	%dl,		3(%edi)

	movl	8(%esp),	%edx
	cmpl	%esi,		%edx
	jbe	.L004finished
	incl	%esi
	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	-1(%esi),	%dh
	xorb	%dh,		%dl
	movb	%dl,		4(%edi)

	movl	8(%esp),	%edx
	cmpl	%esi,		%edx
	jbe	.L004finished
	incl	%esi
	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%eax,4),	%ecx
	movl	(%ebp,%edx,4),	%edx
	movb	-1(%esi),	%dh
	xorb	%dh,		%dl
	movb	%dl,		5(%edi)

	movl	8(%esp),	%edx
	cmpl	%esi,		%edx
	jbe	.L004finished
	incl	%esi
	addb	%cl,		%bl
	movl	(%ebp,%ebx,4),	%edx
	movl	%edx,		(%ebp,%eax,4)
	addl	%ecx,		%edx
	movl	%ecx,		(%ebp,%ebx,4)
	andl	$255,		%edx
	incb	%al
	movl	(%ebp,%edx,4),	%edx
	movb	-1(%esi),	%dh
	xorb	%dh,		%dl
	movb	%dl,		6(%edi)
	jmp	.L004finished
.align	16
.L001RC4_CHAR:
	leal	(%esi,%edx),	%edx
	movl	%edx,		8(%esp)
	movzbl	(%ebp,%eax),	%ecx
.L005RC4_CHAR_loop:
	addb	%cl,		%bl
	movzbl	(%ebp,%ebx),	%edx
	movb	%cl,		(%ebp,%ebx)
	movb	%dl,		(%ebp,%eax)
	addb	%cl,		%dl
	movzbl	(%ebp,%edx),	%edx
	addb	$1,		%al
	xorb	(%esi),		%dl
	leal	1(%esi),	%esi
	movzbl	(%ebp,%eax),	%ecx
	cmpl	8(%esp),	%esi
	movb	%dl,		(%edi)
	leal	1(%edi),	%edi
	jb	.L005RC4_CHAR_loop
.L004finished:
	decl	%eax
	addl	$12,		%esp
	movb	%bl,		-4(%ebp)
	movb	%al,		-8(%ebp)
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret
.L_RC4_end:
.size	RC4,.L_RC4_end-RC4
.ident	"RC4"
