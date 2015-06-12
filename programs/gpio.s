ldr r0,=0x20200000 ;base address
mov r1,#1
lsl r1,#18
str r1,[r0,#4] ;set GPIO 16 as output pin

mov r1,r1,lsr #2 ;set bit 16

loop:

str r1,[r0,#40] ;clear GPIO 16

mov r2,#0xF0000
wait1:
sub r2,r2,#1
cmp r2,#0
bne wait1

str r1,[r0,#28] ;set GPIO 16

mov r2,#0xF0000
wait2:
sub r2,r2,#1
cmp r2,#0
bne wait2

b loop
