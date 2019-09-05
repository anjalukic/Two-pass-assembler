.global a,b
.extern c

.data
a:
.long a,2, 3
.equ d,4
.word 5,6

.text
	mov r0, 0
	mov r1,b
	call c
e:	addb r0,3
	push r4
b:	popb r3
	cmp r0, 0
	jeq $e
.end
	
