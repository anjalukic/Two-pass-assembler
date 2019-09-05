.global a,c
.extern b
.text
    jeq $a
    jeq $e
    jeq $b
    jeq $d
d:  .long d
    mov r0,&b
    mov c,r0
    mov r0,e
.data
.skip 8
e:  .long a
    .long c
    .long bss
a:  .long b
.bss
c:  .skip 8
.end
