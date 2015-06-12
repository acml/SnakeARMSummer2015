ldr r0,=0x20200000 ;base address
mov r1,#1

mov r2,r1,lsl #12
str r1,[r0] ;set GPIO 4 as output pin

mov r2,r1,lsl #21
str r1,[r0,#4] ;set GPIO 17 as output pin

mov r2,r1,lsl #24
str r1,[r0,#4] ;set GPIO 18 as output pin

mov r2,r1,lsl #9
str r1,[r0,#8] ;set GPIO 23 as output pin

mov r2,r1,lsl #12
str r1,[r0,#8] ;set GPIO 24 as output pin

mov r3,#0 ;the count

loop:

mov r2,r1,lsl #4
tst r3,r1 ;test whether bit 0 is 0 or 1
streq r2,[r0,#40] ;clear GPIO 4 if bit 0 is 0
strne r2,[r0,#28] ;set GPIO 4 if bit 0 is 1

mov r2,r1,lsl #17 ;test bit 1 and change GPIO 17
tst r3,r1,lsl #1
streq r2,[r0,#40]
strne r2,[r0,#28]

mov r2,r1,lsl #18 ;test bit 2 and change GPIO 18
tst r3,r1,lsl #2
streq r2,[r0,#40]
strne r2,[r0,#28]

mov r2,r1,lsl #23 ;test bit 3 and change GPIO 23
tst r3,r1,lsl #3
streq r2,[r0,#40]
strne r2,[r0,#28]

mov r2,r1,lsl #24 ;test bit 4 and change GPIO 24
tst r3,r1,lsl #4
streq r2,[r0,#40]
strne r2,[r0,#28]

add r3,r3,#1
cmp r3,#32 ;reset to 0 if counter overflowed
moveq r3,#0

mov r4,#0xF00000
wait:
sub r4,r4,#1
cmp r4,#0
bne wait

b loop
