ldr r0,=0x20200000
mov r1,#1
lsl r1,#24
str r1,[r0,#4]
mov r1,#1
lsl r1,#12
str r1,[r0,#8]
mov r1,#1
lsl r1,#12
str r1,[r0]
mov r1,#1
lsl r1,#9
str r1,[r0,#8]
mov r3,#0
loop:
tst r3,#1
beq on2
mov r1,#1
lsl r1,#18
str r1,[r0,#28]
on2:
tst r3,#2
beq on3
mov r1,#1
lsl r1,#23
str r1,[r0,#28]
on3:
tst r3,#4
beq on4
mov r1,#1
lsl r1,#4
str r1,[r0,#28]
on4:
tst r3,#8
beq next
mov r1,#1
lsl r1,#24
str r1,[r0,#28]
next:
mov r2,#0x2F0000
wait:
sub r2,r2,#1
cmp r2,#0xFF
bne wait
mov r1,#1
lsl r1,#23
str r1,[r0,#40]
mov r1,#1
lsl r1,#4
str r1,[r0,#40]
mov r1,#1
lsl r1,#24
str r1,[r0,#40]
mov r1,#1
lsl r1,#18
str r1,[r0,#40]
add r3,r3,#1
b loop