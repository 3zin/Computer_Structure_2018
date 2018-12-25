#--------------------------------------------------------------
# 
#  4190.308 Computer Architecture (Fall 2018)
#
#  Project #3: Image Pixelization
#
#  November 6, 2018
#
#  Jin-Soo Kim (jinsoo.kim@snu.ac.kr)
#  Systems Software & Architecture Laboratory
#  Dept. of Computer Science and Engineering
#  Seoul National University
#  http://csl.snu.ac.kr
#
#--------------------------------------------------------------

.text
	.align 4
.globl bmp_mosaic
	.type bmp_mosaic,@function

bmp_mosaic:
	movq	%rcx, %rax
	movq	%rsp, %rcx

	# imgptr -8(%rcx), width -16(%rcx), height -24(%rcx), size -32(%rcx)
	subq	$152, %rsp
	movq	%rdi, -8(%rcx) 
	movq	%rsi, -16(%rcx)
	movq	%rdx, -24(%rcx)
	movq	%rax, -32(%rcx)

# width_byte: -40(%rcx)
	movq	$4, %rsi
	imulq	$3, -16(%rcx), %rax # width*3
	cqto
	idivq	%rsi # %rdx : (width*3)%4
	movq	$4, %rax
	subq	%rdx, %rax
	movq	$0, %rsi
	cmpq	$0, %rdx  
	cmovneq	%rax, %rsi 

	imulq	$3, -16(%rcx), %rdi
	addq	%rsi, %rdi
	movq	%rdi, -40(%rcx)

# leftover_height, num_height
	movq	-24(%rcx), %rax
	cqto
	idivq	-32(%rcx)
	movq	%rdx, -48(%rcx) # leftover_height : -48(%rcx)
	movq	%rax, -56(%rcx) # num_height : -56(%rcx)

# leftover_width, num_width
	movq	-16(%rcx), %rax
	cqto
	idivq	-32(%rcx)
	movq	%rdx, -64(%rcx) # leftover_width : -64(%rcx)
	movq	%rax, -72(%rcx) # num_width : -72(%rcx)

# R, G, B, num
	movq	$0, -80(%rcx) # R
	movq	$0, -88(%rcx) # G
	movq	$0, -96(%rcx) # B
	movq	$0, -104(%rcx) # num

# p
	movq	$0, -112(%rcx) # p
	movq	-112(%rcx), %rax
	cmpq	-56(%rcx), %rax # p - num_height
	jg Done_1
Loop_1:
	movq	-48(%rcx), %rdi
	movq	-32(%rcx), %rax

	cmpq	$0, -112(%rcx) # p - 0
	cmoveq	%rdi, %rax
	movq	%rax, -120(%rcx) # block_height_size

	movq	$0, -128(%rcx) # i
	movq	-128(%rcx), %rax
	cmpq	-72(%rcx), %rax # i - num_width
	jg Done_2
	
Loop_2:
	movq	-72(%rcx), %rdi
	cmpq	%rdi, -128(%rcx) # i - num_width

	movq	-32(%rcx), %rax
	movq	-64(%rcx), %rsi
	cmoveq	%rsi, %rax
	movq	%rax, -136(%rcx) # block_width_size

	movq	$0, -144(%rcx) # j
	movq	-144(%rcx), %rax
	cmpq	-120(%rcx), %rax # j - block_height_size
	jge Done_3

Loop_3:
	movq	$0, -152(%rcx) # k
	movq	-152(%rcx), %rax
	cmpq	-136(%rcx), %rax # k - block_width_size
	jge Done_4

Loop_4: # offset
	movq	-128(%rcx), %rax # i
	imulq	-32(%rcx), %rax # i*size <- possible?
	imulq	$3, %rax, %rax # i*size*3

	movq	-144(%rcx), %rdi # j
	imulq	-40(%rcx), %rdi # width_byte*j
	addq	%rdi, %rax #(i*size*3) + (width_byte*j)

	movq	-152(%rcx), %rdi
	imulq	$3, %rdi, %rdi
	addq	%rdi, %rax # first offset is stored in %rax

	movq	-112(%rcx), %rdi # p
	
	decq	%rdi
	imulq	-32(%rcx), %rdi # (p-1)*size
	imulq	-40(%rcx), %rdi # (p-1)*size*width_byte
	movq	-40(%rcx), %rsi # width_byte
	imulq	-48(%rcx), %rsi # width_byte*leftover_height
	addq	%rsi, %rdi

	movq	$0, %rsi
	movq	-112(%rcx), %rdx # p
	cmpq	$0, %rdx
	cmovneq	%rdi, %rsi	

	addq	%rsi, %rax # offset in %rax
	movq	-8(%rcx), %rdi #imgptr in %rdi

	movq	-80(%rcx), %rsi # R
	movzbq	(%rdi, %rax), %rdx # rdx <- imgptr[offset]
	addq	%rdx, %rsi
	movq	%rsi, -80(%rcx)

	movq	-88(%rcx), %rsi # G
	movzbq	1(%rdi, %rax), %rdx # rdx <- imgptr[offset+1]
	addq	%rdx, %rsi
	movq	%rsi, -88(%rcx)

	movq	-96(%rcx), %rsi # B
	movzbq	2(%rdi, %rax), %rdx # rdx <- imgptr[offset+1]
	addq	%rdx, %rsi
	movq	%rsi, -96(%rcx)

	incq	-104(%rcx)
	incq	-152(%rcx)

	movq	-152(%rcx), %rsi # k
	movq	-136(%rcx), %rax  # block_width_size
	cmpq	-136(%rcx), %rsi # k - block_width_size
	jl Loop_4

Done_4:
	incq	-144(%rcx)
	movq	-144(%rcx), %rsi
	movq	-120(%rcx), %rax # block_height_size
	cmpq	-120(%rcx), %rsi # j - block_height_size
	jl Loop_3

Done_3:
	movq	-104(%rcx), %rax # num
	cmpq	$0, %rax # num - 0
	je Temp

	movq	-80(%rcx), %rax # R
	cqto
	idivq	-104(%rcx) # R =  R / num
	movq	%rax, -80(%rcx)

	movq	-88(%rcx), %rax #G
	cqto
	idivq	-104(%rcx)
	movq	%rax, -88(%rcx)

	movq	-96(%rcx), %rax #B
	cqto
	idivq	-104(%rcx)
	movq	%rax, -96(%rcx)

Temp:
	movq	$0, -144(%rcx) # j = 0
	movq	-144(%rcx), %rax
	cmpq	-120(%rcx), %rax # j- block_height_size
	jge Done_5

Loop_5:
	movq	-152(%rcx), %rsi #k
	movq	$0, %rsi
	movq	%rsi, -152(%rcx)

	cmpq	-136(%rcx), %rsi # k - block_width_size
	jge Done_6

Loop_6:
	movq	-128(%rcx), %rax # i
	imulq	-32(%rcx), %rax # i*size <- possible?
	imulq	$3, %rax, %rax # i*size*3

	movq	-144(%rcx), %rdi # j
	imulq	-40(%rcx), %rdi # width_byte*j
	addq	%rdi, %rax #(i*size*3) + (width_byte*j)

	movq	-152(%rcx), %rdi
	imulq	$3, %rdi, %rdi
	addq	%rdi, %rax # first offset is stored in %rax
	
	movq	-112(%rcx), %rdi # p
	
	decq	%rdi
	imulq	-32(%rcx), %rdi # (p-1)*size
	imulq	-40(%rcx), %rdi # (p-1)*size*width_byte
	movq	-40(%rcx), %rsi # width_byte
	imulq	-48(%rcx), %rsi # width_byte*leftover_height
	addq	%rsi, %rdi

	movq	$0, %rsi
	movq	-112(%rcx), %rdx # p
	cmpq	$0, %rdx
	cmovneq	%rdi, %rsi	

	addq	%rsi, %rax # offset in %rax
	movq	-8(%rcx), %rdi #imgptr in %rdi

	movq	-80(%rcx), %rsi # R
	movb	%sil, (%rdi, %rax)

	movq	-88(%rcx), %rsi # G
	movb	%sil, 1(%rdi, %rax)

	movq	-96(%rcx), %rsi # B
	movb	%sil, 2(%rdi, %rax)

	incq	-152(%rcx)
	movq	-152(%rcx), %rsi # k
	cmpq	-136(%rcx), %rsi # k - block_width_size
	jl Loop_6

Done_6:
	incq	-144(%rcx)
	movq	-144(%rcx), %rsi
	cmpq	-120(%rcx), %rsi
	jl Loop_5

Done_5:
	movq	$0, -80(%rcx) # R, G, B, num 
	movq	$0, -88(%rcx)
	movq	$0, -96(%rcx)
	movq	$0, -104(%rcx)

	incq	-128(%rcx)
	movq	-128(%rcx), %rax
	cmpq	-72(%rcx), %rax # i - num_width
	jle Loop_2

Done_2:
	incq	-112(%rcx)

	movq	-112(%rcx), %rax
	cmpq	-56(%rcx), %rax # i - num_width
	jle Loop_1

Done_1:
	#popq	%rcx
	addq	$152, %rsp
	ret


