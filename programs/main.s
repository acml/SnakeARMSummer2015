
b main

/*
 *
 * GRAPHICS ADDRESS = 11796480
 * STATE ADDRESS = 1000000
 * VARIABLES = 900000
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

    /*
     * Initialize input pins
     */

	ldr r0,=0x20200004
	mov r1,#0
	str r1,[r0]
 
 	/*
 	 * One grid square size
 	 */
    mov r2,#32


    push {r0-r4}
    bl InitialiseStateMemory
    pop {r0-r4}

    /*
     * r4 - direction drawing
     * r5 - direction erasing
     * r6 - x drawing
     * r7 - y drawing
     * r8 - x erasing
     * r9 - y erasing
     * r10 - input register
     */
    mov r4,#16
    mov r5,#16
    mov r6,#256
    mov r7,#512
    mov r8,#0
    mov r9,#512

    mov r10,#0
   	
   	/*
   	 * Draw initial background
   	 */
    push {r0-r4}
    bl DrawBg
    pop {r0-r4}




loop1$:  

	push {r0-r3}
	/*
	 * Initialize arguments y, x, color for octagon
	 */
    mov r0,r6
    mov r1,r7
    ldr r3,=0xFF005200
    push {r0-r4}
    bl DrawOctagon
    pop {r0-r4}
    /* 
     * save direction of drawing as argument for Move
     * save 1 to r2 to inform Move that we are drawing
     */
	mov r3,r4
	mov r2,#1
    bl Move
    /*
     * store updated values
     *
     */
    mov r4,r3
    mov r6,r0
    mov r7,r1
	pop {r0-r3}


    /*
     * Erasing - same logic as before
     */
    push {r0-r3}
    mov r0,r8
    mov r1,r9
    ldr r3,=0xFFB49B82

    push {r0-r4}
    bl DrawRectangle
    pop {r0-r4}

	mov r3,r5
	mov r2,#0
    bl Move
    /*
     * store updated values
     */
    mov r5,r3
    mov r8,r0
    mov r9,r1
    pop {r0-r3}
    /*
     * Check inputs TODO check if pushing regs is necessary
     */
    push {r6-r7}

    /*
     * Load value at Input level 0 memory location to r7
     * If input present, store it in input register r10
     */
    ldr r6,=0x20200034
	ldr r7,[r6]
	
	/*
	 * Pin 22
	 * Up arrow
	 */
	tst r7,#0x400000
	movne r10,#4

	/*
	 * Pin 27
	 * Right arrow
	 */
	tst r7,#0x8000000
	movne r10,#8

	/*
	 * Pin 23
	 * Down arrow
	 */
	tst r7,#0x800000
	movne r10,#16

	/*
	 * Pin 24
	 * Left arrow
	 */
	tst r7,#0x1000000
	movne r10,#32

	pop {r6-r7}	



	/*
	 * Wait loop 
	 */
    push {r0-r4}
    ldr r0,=10000
    bl Wait
    pop {r0-r4}



    b loop1$


reset$:
	/* 
	 * Memset snake memory to zeroes
	 * Redraw background
	 */

    push {r0-r4}
    bl InitialiseStateMemory
    bl DrawBg
    pop {r0-r4}

    /*
     * Start drawing snake  at origin
     *
     */
    mov r6,#256
    mov r7,#512
    mov r8,#0
    mov r9,#512
    mov r5,#16
    mov r4,#16
    mov r10,#0
    b loop1$


Move:
	/*
	 * Push return address on stack, so we can branch back
	 */
    push {r14}
	
	/*
	 * Test if coordinates are on grid intersection
	 */
    tst r0,#0x1f
    tsteq r1,#0x1f
    beq enterBlock$

 
    directionTests$:
    	/*
    	 * Direction up
    	 * decrement y
    	 */
        tst r3,#4
        subne r0,r0,#1

    	/*
    	 * Direction right
    	 * increment x
    	 */
        tst r3,#8
        addne r1,r1,#1

    	/*
    	 * Direction down
    	 * increment y
    	 */
        tst r3,#16
        addne r0,r0,#1
    	
    	/*
    	 * Direction left
    	 * decrement x
    	 */
        tst r3,#32
        subne r1,r1,#1
    pop {r15}

/* 
 * Gets called if the snake is at a coordinate divisible by 32
 */
enterBlock$:
	tst r2,r2
	bne drawing$
	beq erasing$



drawing$:
	/* 
	 * Check if there was some input 
	 */
	tst r10,r10
	movne r3,r10
	mov r10,#0


	push {r0-r3}
	bl writeBlock
	pop {r0-r3}

	b directionTests$


erasing$:
	push {r0-r2}
    bl eraseBlock
    tst r0,#0x3c
    movne r3,r0
    pop {r0-r2}
    b directionTests$
	

eraseBlock:
	push {r5}
	/*
	 * Compute address of the block
	 */
	
	mov r0,r0,lsr #5
	mov r1,r1,lsr #5
	mov r2,#32
	mla r0,r0,r2,r1
	lsl r0,#2
	ldr r1,=1000000
	
	/*
	 * Load block
	 */

	ldr r2,[r1,r0]
	
	/*
	 * Reset block to 0
	 */
	mov r5,#0
	str r5,[r1,r0]

	/*
	 * Return block
	 */
	mov r0,r2
	pop {r5}
	mov r15,r14

writeBlock: 
	
	/*
	 * Calculating the address of current block in memory
	 */
	mov r0,r0,lsr #5
	mov r1,r1,lsr #5
	mov r2,#32
	mla r0,r0,r2,r1
	lsl r0,#2
	ldr r1,=1000000

	/*
	 * Set snake bit
	 */
	add r3,r3,#1

	/*
	 * Store block
	 */
	str r3,[r1,r0]
	
	mov r15,r14


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


    
    cmp r0,#768
    bge endDrawPixel$
    

    cmp r1,#1024
    bge endDrawPixel$
    

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
    endDrawPixel$:
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
     push {r4,r5,r14}
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
     pop {r4,r5,r15}

DrawOctagon:
	push {r0-r10,r14}
	/*
	 * r6 snake width
	 * r0 y
	 * r1 x
	 * r4 y0
	 * r5 x0
	 * r6 offset
	 * constants : padding 6
	 *			   size 32
	 */
	 mov r4,r0
	 mov r5,r1
	 
	 
	 /*
	  * Upper part of octagon
	  */
	 mov r6,#0
	 add r0,r0,#12
	 ldr r2,=0xFF005200

	 

	 OctagonUpOuterLoop$:
	 	/*
	 	 * Reset x and compute first pixel
	 	 * x = x0 + 6 + offset
	 	 */
	 	mov r1,r5
	 	add r1,r1,#6
	 	add r1,r1,r6
	 	OctagonUpInnerLoop$:
	 		/*
	 		 * Draw it
	 		 */
	 		push {r0-r4}
	 		bl DrawPixel
	 		pop {r0-r4}
	 		
	 		/*
	 		 * Increment x
	 		 */ 
	 		add r1,r1,#1

	 		/*
	 		 * Compute boundary r7 = x0 + (32-6) - offset
	 		 */
	 		mov r7,r5
	 		add r7,r7,#26
	 		sub r7,r7,r6

	 		/*
	 		 * while (x <  boundary)
	 		 */
	 		cmp r1,r7
	 		blt OctagonUpInnerLoop$
	 	
	 	/*
	 	 * Increment offset
	 	 */
	 	add r6,r6,#1
 		
 		/*
 		 * Decrement y
 		 */ 
 		sub r0,r0,#1

 		/*
 		 * Compute boundary r7 = x0 + (32-6) - offset
 		 */
 		mov r7,r4
 		add r7,#6
 		cmp r0,r7
 		bne OctagonUpOuterLoop$
  
 	mov r0,r4
 	add r0,r0,#19
    OctagonMidOuterLoop$:

        mov r1,r5
        add r1,r1,#25
        OctagonMidInnerLoop$:
            push {r0-r4}
            bl DrawPixel
            pop {r0-r4}
            sub r1,r1,#1

            mov r7,r5
            add r7,r7,#5
            cmp r1,r7
            bne OctagonMidInnerLoop$

        sub r0,r0,#1
        mov r7,r4
        add r7,r7,#12
        cmp r0,r7
        bne OctagonMidOuterLoop$


	 /*
	  * Upper part of octagon
	  */
	 mov r6,#0
	 mov r0,r4
	 add r0,r0,#20
	

	 

	 OctagonDownOuterLoop$:
	 	mov r1,r5
	 	add r1,r1,#6
	 	add r1,r1,r6
	 	OctagonDownInnerLoop$:
	 		push {r0-r4}

	 		bl DrawPixel
	 		pop {r0-r4}
	 		/*
	 		 * Increment x
	 		 */ 
	 		add r1,r1,#1

	 		/*
	 		 * Compute boundary r7 = x0 + (32-6) - offset
	 		 */
	 		mov r7,r5
	 		add r7,r7,#26
	 		sub r7,r7,r6

	 		
	 		cmp r1,r7
	 		blt OctagonDownInnerLoop$
		/*
	 	 * Increment offset
	 	 */
	 	add r6,r6,#1
 		/*
 		 * Increment y
 		 */ 
 		add r0,r0,#1

 		/*
 		 * Compute boundary r7 = x0 + (32-6) - offset
 		 */
 		mov r7,r4
 		add r7,#26
 		cmp r0,r7
 		bne OctagonDownOuterLoop$
	 

	pop {r0-r10,r15}


DrawBg:
    ldr r4,=11796480
    ldr r3,[r4,#32]
    ldr r0, =0xFFB49B82
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

InitialiseStateMemory:
	/*
	 * r0 will store 0
	 * r1 will store memory offset
	 * r2 will store memory
	 */
	mov r0,#0
	ldr r1,=768
	ldr r2,=1000000
    snakeMemoryLoop$:  
        str r0,[r2],#4
        sub r1,r1,#1
        cmp r1,#0
        bne snakeMemoryLoop$
    mov r15,r14

    