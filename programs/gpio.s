ldr r0,=0x20200000 ;base address
mov r1,#1
lsl r1,#21
str r1,[r0,#4] 

loop:
mov r2,#0x0F0000
str r1,[r0,#40]

wait:
sub r2,r2,#1
cmp r2,#0xFF
bne wait
str r1,[r0,#28]
mov r2,#0x0F0000

wait2:
sub r2,r2,#1
cmp r2,#0xFF
bne wait2
b loop
