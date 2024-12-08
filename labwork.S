  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #
.text
.global delay
.global time2string
.global enable_interrupt

enable_interrupt:
    ei 
    jr $ra              # Return from the interrupt routine

time2string:
	PUSH $s0 # to restore the callee saved fxn 
	PUSH $s1 
	PUSH $s2
      PUSH $s3
      PUSH $s4
      PUSH $s5
      PUSH $s6
	PUSH $ra# might  be linked to move in hexasc if this step is not taken 
	# lb $t0 2($t0) # big-endian # cant do this because $t0 is not a memory address, but is the value
	add $s0, $a0, $0
	add $s1, $a1, $0 
     move $s3 , $s1 # added for surprise assignment  , copy the time that was incremented by tick into $s3
	andi $s3, $s3 , 0xff  #added for surprise assignment  

	andi $s2, $s1, 0xf000
	srl $s2, $s2, 12
	add $a0, $s2, $0
	jal hexasc
	nop
	sb $v0, 0($s0) # storing the value from $vo into the memory adress that was shifted from $ao to $so  
	               # $ao conatines address of area in memory  large enough to storw the output 

	# lb $t0 2($t0) # big-endian
	andi $s2, $s1, 0xf00
	srl $s2, $s2, 8
	add $a0, $s2, $0
	jal hexasc
	nop
	sb $v0, 1($s0)

	# place :
	li $s2, 0x3a
	sb $s2, 2($s0)

	# lb $t0 3($t0) # big-endian
	andi $s2, $s1, 0xf0
	srl $s2, $s2, 4
	add $a0, $s2, $0
	jal hexasc
	nop
	sb $v0, 3($s0)

	# lb $t0 3($t0) # big-endian
	andi $s2, $s1, 0xf
	add $a0, $s2, $0
	jal hexasc
	nop
	sb $v0, 4($s0)

	#surprise assignment ( DING appears at xx:00)
	li $s4 , 0x00 #load null into register 
	bne $s3 , $s4 , null # check if seconds == 00 if not then goes to null subroutine 
	li $s5 , 0x474e4944# if above contidion is true ,then loads DING into a seperate register
	sw $s5 , 0($s0) #adds DING to 00
      li $s6 , 0x00
      sw $s6, 4($s0)
	
null:	li $s2, 0x00
	sb $s2, 5($s0)

	POP $ra
      POP $s6
      POP $s5
      POP $s4
      POP $s3
	POP $s2
	POP $s1
	POP $s0

	end : jr $ra 
	nop
	
hexasc :
   	andi $t0, $a0 , 0xf  # masking the other bits so that the number lies in the range 0 to 15 for hexadecimal
   	
   	#check if $t0 is between 0 and 9 inclusive 
   	li $t1 , 9
   	ble $t0 , $t1 , convert0_9
   	nop
   	li $t2 ,0x37  	
   	add $t0 , $t0, $t2
   	j  mask7 
   	nop
   	 	
convert0_9 :  # checks if the number lie in between 0 to 9
        li $t3 ,0x30
        add $t0, $t0, $t3
        
mask7:
	and $v0 , $t0 , 0x7F  # extract one byte into $v0
	jr $ra  # jumps to the move syntax in the mains code
	nop

    	
delay:
	PUSH $ra
	add $t0, $a0, $0
	addi $t4, $0, 5000

WhileCond:
	blt $0, $t0, While
	nop # delay slot
	j stop
	nop
While:
	addi $t0, $t0, -1
	add $t1, $0, $0

ForCond:
	blt $t1, $t4, For
	nop
	j WhileCond
	nop
For:
	add $t1, $t1, 1
	j ForCond
	nop
	POP $ra
stop:   jr $ra
	nop # delay slot