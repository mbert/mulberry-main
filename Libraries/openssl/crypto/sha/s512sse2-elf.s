





	.file	"sha512-sse2.s"
.text
.globl	sha512_block_sse2
.type	sha512_block_sse2,@function
.align	16
sha512_block_sse2:
	pushl	%ebp
	movl	%esp,		%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	8(%ebp),	%edx
	movl	12(%ebp),	%edi
	call	.L000pic_point
.L000pic_point:
	popl	%esi
	leal	.L001K512-.L000pic_point(%esi),%esi
	subl	$320,		%esp
	andl	$-16,		%esp
	movdqu	(%edx),		%xmm0
	movdqu	16(%edx),	%xmm1
	movdqu	32(%edx),	%xmm2
	movdqu	48(%edx),	%xmm3
.align	8
.L002_chunk_loop:
	movdqa	%xmm0,		256(%esp)
	movdqa	%xmm1,		272(%esp)
	movdqa	%xmm2,		288(%esp)
	movdqa	%xmm3,		304(%esp)
	xorl	%edx,		%edx
	movdq2q	%xmm0,		%mm0
	movdq2q	%xmm2,		%mm1
	movl	(%edi,%edx,8),	%eax
	movl	4(%edi,%edx,8),	%ebx
.byte 15
.byte 200	
.byte 15
.byte 203	
	movl	%ebx,		(%esp,%edx,8)
	movl	%eax,		4(%esp,%edx,8)
	movl	%ebx,		128(%esp,%edx,8)
	movl	%eax,		132(%esp,%edx,8)
.align	8
.L003_1st_loop:
	movl	8(%edi,%edx,8),	%eax
	movl	12(%edi,%edx,8),%ebx
.byte 15
.byte 200	
.byte 15
.byte 203	
	movl	%ebx,		8(%esp,%edx,8)
	movl	%eax,		12(%esp,%edx,8)
	movl	%ebx,		136(%esp,%edx,8)
	movl	%eax,		140(%esp,%edx,8)
.L004_1st_looplet:
	movq	296(%esp),	%mm4
	movq	304(%esp),	%mm5
	movq	312(%esp),	%mm6
	movq	%mm1,		%mm2
	movq	%mm1,		%mm3
	psrlq	$14,		%mm2
	psllq	$23,		%mm3
	movq	%mm2,		%mm7
	pxor	%mm3,		%mm7
	psrlq	$4,		%mm2
	psllq	$23,		%mm3
	pxor	%mm2,		%mm7
	pxor	%mm3,		%mm7
	psrlq	$23,		%mm2
	psllq	$4,		%mm3
	pxor	%mm2,		%mm7
	pxor	%mm3,		%mm7
	movq	%mm1,		296(%esp)
	movq	%mm4,		304(%esp)
	movq	%mm5,		312(%esp)
	pxor	%mm5,		%mm4
	pand	%mm1,		%mm4
	pxor	%mm5,		%mm4
	paddq	%mm4,		%mm7
	movq	264(%esp),	%mm2
	movq	272(%esp),	%mm3
	movq	280(%esp),	%mm1
	paddq	%mm6,		%mm7
	paddq	(%esi,%edx,8),	%mm7
	paddq	(%esp,%edx,8),	%mm7
	paddq	%mm7,		%mm1
	movq	%mm0,		%mm4
	movq	%mm0,		%mm5
	psrlq	$28,		%mm4
	psllq	$25,		%mm5
	movq	%mm4,		%mm6
	pxor	%mm5,		%mm6
	psrlq	$6,		%mm4
	psllq	$5,		%mm5
	pxor	%mm4,		%mm6
	pxor	%mm5,		%mm6
	psrlq	$5,		%mm4
	psllq	$6,		%mm5
	pxor	%mm4,		%mm6
	pxor	%mm5,		%mm6
	movq	%mm0,		264(%esp)
	movq	%mm2,		272(%esp)
	movq	%mm3,		280(%esp)
	movq	%mm0,		%mm4
	por	%mm3,		%mm0
	pand	%mm3,		%mm4
	pand	%mm2,		%mm0
	por	%mm0,		%mm4
	paddq	%mm4,		%mm6
	movq	%mm7,		%mm0
	paddq	%mm6,		%mm0
	incl	%edx
	cmpl	$15,		%edx
	jl	.L003_1st_loop
	je	.L004_1st_looplet
	movl	%edx,		%ebx
.align	8
.L005_2nd_loop:
	andl	$15,		%edx
	movdqu	8(%esp,%edx,8),	%xmm0
	movdqa	%xmm0,		%xmm2
	movdqa	%xmm0,		%xmm3
	psrlq	$1,		%xmm2
	psllq	$56,		%xmm3
	movdqa	%xmm2,		%xmm0
	pxor	%xmm3,		%xmm0
	psrlq	$6,		%xmm2
	psllq	$7,		%xmm3
	pxor	%xmm2,		%xmm0
	pxor	%xmm3,		%xmm0
	psrlq	$1,		%xmm2
	pxor	%xmm2,		%xmm0
	movdqa	112(%esp,%edx,8),%xmm1
	movdqa	%xmm1,		%xmm4
	movdqa	%xmm1,		%xmm5
	psrlq	$6,		%xmm4
	psllq	$3,		%xmm5
	movdqa	%xmm4,		%xmm1
	pxor	%xmm5,		%xmm1
	psrlq	$13,		%xmm4
	psllq	$42,		%xmm5
	pxor	%xmm4,		%xmm1
	pxor	%xmm5,		%xmm1
	psrlq	$42,		%xmm4
	pxor	%xmm4,		%xmm1
	movdqu	72(%esp,%edx,8),%xmm6
	paddq	%xmm1,		%xmm0
	paddq	%xmm6,		%xmm0
	paddq	(%esp,%edx,8),	%xmm0
	movdqa	%xmm0,		(%esp,%edx,8)
	movdqa	%xmm0,		128(%esp,%edx,8)
	movq	296(%esp),	%mm4
	movq	304(%esp),	%mm5
	movq	312(%esp),	%mm6
	movq	%mm1,		%mm2
	movq	%mm1,		%mm3
	psrlq	$14,		%mm2
	psllq	$23,		%mm3
	movq	%mm2,		%mm7
	pxor	%mm3,		%mm7
	psrlq	$4,		%mm2
	psllq	$23,		%mm3
	pxor	%mm2,		%mm7
	pxor	%mm3,		%mm7
	psrlq	$23,		%mm2
	psllq	$4,		%mm3
	pxor	%mm2,		%mm7
	pxor	%mm3,		%mm7
	movq	%mm1,		296(%esp)
	movq	%mm4,		304(%esp)
	movq	%mm5,		312(%esp)
	pxor	%mm5,		%mm4
	pand	%mm1,		%mm4
	pxor	%mm5,		%mm4
	paddq	%mm4,		%mm7
	movq	264(%esp),	%mm2
	movq	272(%esp),	%mm3
	movq	280(%esp),	%mm1
	paddq	%mm6,		%mm7
	paddq	(%esi,%ebx,8),	%mm7
	paddq	(%esp,%edx,8),	%mm7
	paddq	%mm7,		%mm1
	movq	%mm0,		%mm4
	movq	%mm0,		%mm5
	psrlq	$28,		%mm4
	psllq	$25,		%mm5
	movq	%mm4,		%mm6
	pxor	%mm5,		%mm6
	psrlq	$6,		%mm4
	psllq	$5,		%mm5
	pxor	%mm4,		%mm6
	pxor	%mm5,		%mm6
	psrlq	$5,		%mm4
	psllq	$6,		%mm5
	pxor	%mm4,		%mm6
	pxor	%mm5,		%mm6
	movq	%mm0,		264(%esp)
	movq	%mm2,		272(%esp)
	movq	%mm3,		280(%esp)
	movq	%mm0,		%mm4
	por	%mm3,		%mm0
	pand	%mm3,		%mm4
	pand	%mm2,		%mm0
	por	%mm0,		%mm4
	paddq	%mm4,		%mm6
	movq	%mm7,		%mm0
	paddq	%mm6,		%mm0
	incl	%ebx
	incl	%edx
	movq	296(%esp),	%mm4
	movq	304(%esp),	%mm5
	movq	312(%esp),	%mm6
	movq	%mm1,		%mm2
	movq	%mm1,		%mm3
	psrlq	$14,		%mm2
	psllq	$23,		%mm3
	movq	%mm2,		%mm7
	pxor	%mm3,		%mm7
	psrlq	$4,		%mm2
	psllq	$23,		%mm3
	pxor	%mm2,		%mm7
	pxor	%mm3,		%mm7
	psrlq	$23,		%mm2
	psllq	$4,		%mm3
	pxor	%mm2,		%mm7
	pxor	%mm3,		%mm7
	movq	%mm1,		296(%esp)
	movq	%mm4,		304(%esp)
	movq	%mm5,		312(%esp)
	pxor	%mm5,		%mm4
	pand	%mm1,		%mm4
	pxor	%mm5,		%mm4
	paddq	%mm4,		%mm7
	movq	264(%esp),	%mm2
	movq	272(%esp),	%mm3
	movq	280(%esp),	%mm1
	paddq	%mm6,		%mm7
	paddq	(%esi,%ebx,8),	%mm7
	paddq	(%esp,%edx,8),	%mm7
	paddq	%mm7,		%mm1
	movq	%mm0,		%mm4
	movq	%mm0,		%mm5
	psrlq	$28,		%mm4
	psllq	$25,		%mm5
	movq	%mm4,		%mm6
	pxor	%mm5,		%mm6
	psrlq	$6,		%mm4
	psllq	$5,		%mm5
	pxor	%mm4,		%mm6
	pxor	%mm5,		%mm6
	psrlq	$5,		%mm4
	psllq	$6,		%mm5
	pxor	%mm4,		%mm6
	pxor	%mm5,		%mm6
	movq	%mm0,		264(%esp)
	movq	%mm2,		272(%esp)
	movq	%mm3,		280(%esp)
	movq	%mm0,		%mm4
	por	%mm3,		%mm0
	pand	%mm3,		%mm4
	pand	%mm2,		%mm0
	por	%mm0,		%mm4
	paddq	%mm4,		%mm6
	movq	%mm7,		%mm0
	paddq	%mm6,		%mm0
	incl	%ebx
	incl	%edx
	cmpl	$80,		%ebx
	jl	.L005_2nd_loop
	movl	8(%ebp),	%edx
	movq	%mm0,		256(%esp)
	movq	%mm1,		288(%esp)
	movdqu	(%edx),		%xmm0
	movdqu	16(%edx),	%xmm1
	movdqu	32(%edx),	%xmm2
	movdqu	48(%edx),	%xmm3
	paddq	256(%esp),	%xmm0
	paddq	272(%esp),	%xmm1
	paddq	288(%esp),	%xmm2
	paddq	304(%esp),	%xmm3
	movdqu	%xmm0,		(%edx)
	movdqu	%xmm1,		16(%edx)
	movdqu	%xmm2,		32(%edx)
	movdqu	%xmm3,		48(%edx)
	addl	$128,		%edi
	decl	16(%ebp)
	jnz	.L002_chunk_loop
	emms
	movl	-12(%ebp),	%edi
	movl	-8(%ebp),	%esi
	movl	-4(%ebp),	%ebx
	leave
	ret
.align	64
.L001K512:
	.long	3609767458,1116352408
	.long	602891725,1899447441
	.long	3964484399,3049323471
	.long	2173295548,3921009573
	.long	4081628472,961987163
	.long	3053834265,1508970993
	.long	2937671579,2453635748
	.long	3664609560,2870763221
	.long	2734883394,3624381080
	.long	1164996542,310598401
	.long	1323610764,607225278
	.long	3590304994,1426881987
	.long	4068182383,1925078388
	.long	991336113,2162078206
	.long	633803317,2614888103
	.long	3479774868,3248222580
	.long	2666613458,3835390401
	.long	944711139,4022224774
	.long	2341262773,264347078
	.long	2007800933,604807628
	.long	1495990901,770255983
	.long	1856431235,1249150122
	.long	3175218132,1555081692
	.long	2198950837,1996064986
	.long	3999719339,2554220882
	.long	766784016,2821834349
	.long	2566594879,2952996808
	.long	3203337956,3210313671
	.long	1034457026,3336571891
	.long	2466948901,3584528711
	.long	3758326383,113926993
	.long	168717936,338241895
	.long	1188179964,666307205
	.long	1546045734,773529912
	.long	1522805485,1294757372
	.long	2643833823,1396182291
	.long	2343527390,1695183700
	.long	1014477480,1986661051
	.long	1206759142,2177026350
	.long	344077627,2456956037
	.long	1290863460,2730485921
	.long	3158454273,2820302411
	.long	3505952657,3259730800
	.long	106217008,3345764771
	.long	3606008344,3516065817
	.long	1432725776,3600352804
	.long	1467031594,4094571909
	.long	851169720,275423344
	.long	3100823752,430227734
	.long	1363258195,506948616
	.long	3750685593,659060556
	.long	3785050280,883997877
	.long	3318307427,958139571
	.long	3812723403,1322822218
	.long	2003034995,1537002063
	.long	3602036899,1747873779
	.long	1575990012,1955562222
	.long	1125592928,2024104815
	.long	2716904306,2227730452
	.long	442776044,2361852424
	.long	593698344,2428436474
	.long	3733110249,2756734187
	.long	2999351573,3204031479
	.long	3815920427,3329325298
	.long	3928383900,3391569614
	.long	566280711,3515267271
	.long	3454069534,3940187606
	.long	4000239992,4118630271
	.long	1914138554,116418474
	.long	2731055270,174292421
	.long	3203993006,289380356
	.long	320620315,460393269
	.long	587496836,685471733
	.long	1086792851,852142971
	.long	365543100,1017036298
	.long	2618297676,1126000580
	.long	3409855158,1288033470
	.long	4234509866,1501505948
	.long	987167468,1607167915
	.long	1246189591,1816402316
.L_sha512_block_sse2_end:
.size	sha512_block_sse2,.L_sha512_block_sse2_end-sha512_block_sse2
.ident	"sha512_block_sse2"
