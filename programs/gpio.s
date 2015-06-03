ldr r0,=0x20200000 ;base address
mov r1,#1 ;output 17
lsl r1,#21
str r1,[r0,#4] 

mov r1,#1 ;output 18
lsl r1,#24
str r1,[r0,#4] 

mov r1,#1 ;output 4
lsl r1,#12
str r1,[r0] 

mov r3,#0
loop:
mov r1,
str r1,[r0,#40]

tst r3, #1
bne on2
mov r1,#1 ;output 17
lsl r1,#21
str r1,[r0,#4] 

on2:
tst r3, #2
bne on3
mov r1,#1 ;output 18
lsl r1,#24
str r1,[r0,#4] 

on3:
tst r3, #4
beq wait
mov r1,#1 ;output 4
lsl r1,#12
str r1,[r0,#0] 

mov r2,#0x0F0000
wait:
sub r2,r2,#1
cmp r2,#0xFF
bne wait

cmp r3, #8
bne increment
mov r3, #0

increment:
add r3, r3, #1

str r1,[r0,#28]
b loop
