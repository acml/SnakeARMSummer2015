
b main

/*
 *
 * GRAPHICS ADDRESS = 81920
 */

main:

/*
* Set the stack point to 0x8000.
*/
    mov r13,#0x8000

/* NEW
* Setup the screen.
*/

    mov r0,#1024
    mov r1,#768
    mov r2,#32
    bl InitialiseFrameBuffer

/* NEW
* Check for a failed frame buffer.
*/
    teq r0,#0
    bne noError$


    error$:
        b error$

    noError$:

    /*
    * set r4 to hold fbInfoAdress
    */
    mov r4,r0

/* NEW
* Set pixels forevermore.
*/
 
    mov r2,#50

    ldr r3,=0xff00



render$:
    mov r0,#700
    mov r1,#900
    loop1$:  
        push {r0-r4}
        push {r0-r4}
        bl DrawOutline
        pop {r0-r4}
        bl DrawRectangle
        pop {r0-r4}


        push {r0-r4}
        ldr r0,=10000
        bl Wait
        pop {r0-r4}

        sub r0,r0,#1
        teq r0,#50
        bne loop1$

    loop2$:  
        push {r0-r4}
        push {r0-r4}
        bl DrawOutline
        pop {r0-r4}
        bl DrawRectangle
        pop {r0-r4}


        push {r0-r4}
        ldr r0,=10000
        bl Wait
        pop {r0-r4}

        sub r1,r1,#1
        teq r1,#50
        bne loop2$

    loop3$:  
        push {r0-r4}
        push {r0-r4}
        bl DrawOutline
        pop {r0-r4}
        bl DrawRectangle
        pop {r0-r4}


        push {r0-r4}
        ldr r0,=10000
        bl Wait
        pop {r0-r4}

        add r0,r0,#1
        teq r0,#700
        bne loop3$

    loop4$:  
        push {r0-r4}
        push {r0-r4}
        bl DrawOutline
        pop {r0-r4}
        bl DrawRectangle
        pop {r0-r4}


        push {r0-r4}
        ldr r0,=10000
        bl Wait
        pop {r0-r4}

        add r1,r1,#1
        teq r1,#900
        bne loop4$




 b render$

DrawPixel:
    /*
     * r0 stores y
     * r1 stores x
     * r4 stores graphics address
     * r2 stores width and color initially
     * r3 stores frameBuffer and than pixel adress
     * r6 stores pixel offset
     * r5 stores FG color
     */
    push {r5,r6}
    ldr r4,=11796480
    mov r5,r2

    /*
     * Safety checks (x < width, y < height)
     *
    * ldr r2,[r4,#4]
    * cmp r0,r2
    * movge r15,r14
    
    * ldr r2,[r4,#0]
    * cmp r1,r2
    * movge r15,r14
    */

    /*
     * Load other constants
     *
     */
    
    ldr r2,[r4,#0]
    ldr r3,[r4,#32]
    /*
     * pixelOffset = y*width + x
     * pixelAddress = pixelOffset*4 + frameBuffer
     */
    mla r6,r0,r2,r1
    add r3,r3,r6,lsl #2

    /*
     * store FG color and return
     */
    str r5,[r3]
    pop {r5,r6}
    mov r15,r14


DrawRectangle:
    /*
     * r0 will store y0
     * r1 will store x0
     * r2 will store side
     * r3 will store y
     * r4 will store x
     */
     push {r14}
    mov r5,r3
    mov r3,r0
    add r3,r3,r2


    drawRow$:
        /*
         * r2 represents x
         */
        mov r4,r1
        add r4,r4,r2
        drawPixel$:
            push {r0-r4}
            mov r0,r3
            mov r1,r4
            mov r2,r5
            bl DrawPixel
            pop {r0-r4}

            sub r4,r4,#1
            teq r4,r1
            bne drawPixel$

        sub r3,r3,#1
        teq r3,r0
        bne drawRow$
     pop {r15}


DrawOutline:
    /*
     * r0 will store y0
     * r1 will store x0
     * r2 will store side
     * r3 will store y
     * r4 will store x
     */

     push {r5,r14}
     mov r5,r0
     sub r5,r5,#1
     mov r3,r0
     add r3,r3,r2
     add r3,r3,#1
     mov r4,r1
     add r4,r4,r2
     add r4,r4,#1
     outlineLoop1$:  
        push {r0-r4}
        mov r0,r3
        mov r1,r4
        mov r2,#0
        bl DrawPixel
        pop {r0-r4}

        sub r3,r3,#1

        teq r3,r5
        bne outlineLoop1$
    mov r5,r1
    sub r5,r5,#1 
    outlineLoop2$:  
        push {r0-r4}
        mov r0,r3
        mov r1,r4
        mov r2,#0
        bl DrawPixel
        pop {r0-r4}

        sub r4,r4,#1
        teq r4,r5
        bne outlineLoop2$
    mov r5,r0
    add r5,r5,r2
    add r5,r5,#1

    outlineLoop3$:  
        push {r0-r4}
        mov r0,r3
        mov r1,r4
        mov r2,#0
        bl DrawPixel
        pop {r0-r4}

        add r3,r3,#1
        teq r3,r5
        bne outlineLoop3$ 
     

    mov r5,r1
    add r5,r5,r2
    add r5,r5,#1

    outlineLoop4$:  
        push {r0-r4}
        mov r0,r3
        mov r1,r4
        mov r2,#0
        bl DrawPixel
        pop {r0-r4}

        add r4,r4,#1
        teq r4,r5
        bne outlineLoop4$ 
     pop {r5,r15}

DrawBg:
    
    ldr r3,[r4,#32]
    ldr r0, =0xffffffff
    mov r1,#768
    BgdrawRow$:

        /*
         * r2 represents x
         */
        mov r2,#1024
        BgdrawPixel$:
            str r0,[r3]
            add r3,r3,#4
            sub r2,r2,#1
            teq r2,#0
            bne BgdrawPixel$
        sub r1,r1,#1
        teq r1,#0
        bne BgdrawRow$
    mov r15,r14


GetMailboxBase:
    ldr r0,=0x2000B880
    /*
     * mov pc,lr
     */
    mov r15,r14


MailboxWrite:
    tst r0,#0xf
    /*
     * movne movne pc,lr
     */
    movne r15,r14


    cmp r1,#15

    /*
     * movhi movne pc,lr
     */
    movhi r15,r14

    /*
     * r1 represents channel
     * r2 represents value
     */

    mov r2,r0
    push {r14}
    bl GetMailboxBase
    /*
     * r0 represents mailbox
     * r3 represents status
     */


    wait1$:

        ldr r3,[r0,#0x18]

        tst r3,#0x80000000

        bne wait1$

    add r2,r2,r1
    str r2,[r0,#0x20]
    pop {r15}


MailboxRead:
    cmp r0,#15
    /*
     * movhi movne pc,lr
     */
    movhi r15,r14
    /*
     * r1 represents channel
     */
    mov r1,r0
    push {r14}
    bl GetMailboxBase
    /*
     * r0 represents mailbox
     * r2 represents status
     */

    rightmail$:
        wait2$:

            ldr r2,[r0,#0x18]

            tst r2,#0x40000000

            bne wait2$
        /*
         *
         * r2 represents mail
         * r3 represents inchan
         */
        ldr r2,[r0,#0]

        and r3,r2,#0xF
        teq r3,r1

        bne rightmail$


    /*
     * and r0,r2,#0xfffffff0
     */
    mov r2,r2,lsr #4
    mov r2,r2,lsl #4
    mov r0,r2

    pop {r15}


GetSystemTimerBase:
    ldr r0,=0x20003000
    mov r15,r14

GetTimeStamp:
    push {r14}
    bl GetSystemTimerBase
    /*
    * MIGHT BE BROKEN
    * ldrd r0,r1,[r0,#4]
    */
    ldr r1,[r0,#8]
    ldr r0,[r0,#4]

    pop {r15}


Wait:
    /*
     * r1 represents elapsed
     * r2 represents delay
     * r3 represents start
     */
    mov r2,r0
    push {r14}
    bl GetTimeStamp

    mov r3,r0

    loop$:
        bl GetTimeStamp

        sub r1,r0,r3
        cmp r1,r2

        bls loop$


    pop {r15}


InitialiseFrameBuffer:

cmp r0,#4096
cmpls r1,#4096
cmpls r2,#32

movhi r0,#0
movhi r15,r14
/*
 * WIDTH r0
 * HEIGHT r1
 * bitDepth r2
 * result r0
 * fbInfoAddr r4
*/
    push {r4,r14}
    mov r5,#0
    /*
    * PROBABLY BROKEN
    */
    ldr r4,=11796480
    str r0,[r4,#0]
    str r1,[r4,#4]
    str r0,[r4,#8]
    str r1,[r4,#12]
    str r5,[r4,#16]

    str r2,[r4,#20]
    str r5,[r4,#24]
    str r5,[r4,#28]
    str r5,[r4,#32]
    str r5,[r4,#36]

    mov r0,r4
    add r0,r0,#0x40000000
    mov r1,#1
    bl MailboxWrite

    mov r0,#1
    bl MailboxRead

    teq r0,#0
    movne r0,#0
    popne {r4,r15}

    mov r0,r4
    pop {r4,r15}

