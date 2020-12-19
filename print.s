; Print.s
; Student names: Tim Reynolds
; Last modification date: 4/21/2020
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB


;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec PROC
    EXPORT   LCD_OutDec

n EQU 0 ; n = R0 input parameter, to be divided
remainder EQU 4 ; will contain n if n < 10; else, will contain n % 10
framep EQU 8 ; stores R11, the frame pointer
linkr EQU 12 ; push LR to stack since functions are called within this subroutine
	
	MOV R1, R0 ; remainder = n initially so that if n < 10 initially just n will be printed
	PUSH{R0, R1, R11, LR} ; push variables onto stack
	MOV R11, SP ; frame pointer

	CMP R0, #9 ; if n <= 9, end recursion and print the corresponding decimal number
	BLS print
	
	MOV R1, #10
	UDIV R2, R0, R1 ; R2 = n / 10
	MUL R3, R2, R1 ; R3 = (n / 10) * 10
	SUB R1, R0, R3 ; R1 = n % 10
	MOV R0, R2 ; R0 = n / 10
	STR R0, [SP, #n] ; store num and remainder on current stack frame before calling function again recursively
	STR R1, [SP, #remainder]
	B LCD_OutDec
	
print
	LDR R11, [SP, #framep] ; load frame pointer stored on current stack frame into R11
	LDR R0, [SP, #remainder] ; pass chars to OutChar
	MOV R1, #0x30
	ADD R0, R0, R1
	BL ST7735_OutChar
	
	ADD SP, #16 ; deallocate current stack frame, move to next highest stack frame if any
	
	CMP SP, R11 ; if stack pointer is less than or equal to frame pointer, it means that there are still more stack frames to print chars from
	BLS print
	
	SUB SP, #16
	LDR LR, [SP, #linkr] ; reload original LR value into LR
	ADD SP, #16
	
    BX LR
    ENDP
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.03 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix PROC
         EXPORT   LCD_OutFix
			 
num EQU 0 ; n = R0 input parameter, to be divided
linkreg EQU 4 ; push LR to stack since functions are called within this subroutine
	
	PUSH{R0, LR} ; push num and link register onto the stack
	
	MOV R1, #1000 ; if R0 >= 1000, it is an invalid input; print "*.**"
	CMP R0, R1
	BHS invalid
	
	LDR R0, [SP, #num]
	MOV R1, #100
	UDIV R2, R0, R1 ; R2 = n/100
	MOV R3, #0x30
	ADD R0, R2, R3 ; first char passed to OutChar
	BL ST7735_OutChar
	
	LDR R0, [SP, #num] ; R0 = n
	MOV R1, #100
	UDIV R2, R0, R1 ; R2 = n/100
	MUL R3, R1, R2 ; R3 = (n/100) * 100
	SUB R0, R0, R3 ; n = n - (n/100)*100 (n % 100)
	STR R0, [SP, #num] ; store remainder in stack
	
	MOV R0, #0x2E
	BL ST7735_OutChar ; pass char '.' to OutChar
	
	LDR R0, [SP, #num] ; R0 = n
	MOV R1, #10
	UDIV R2, R0, R1 ; R2 = n/10
	MOV R3, #0x30
	ADD R0, R2, R3 ; pass next char to OutChar
	BL ST7735_OutChar
	
	LDR R0, [SP, #num] ; R0 = n
	MOV R1, #10
	UDIV R2, R0, R1 ; R2 = n/10
	MUL R3, R1, R2 ; R3 = (n/10) * 10
	SUB R0, R0, R3 ; R0 = n % 10
	STR R0, [SP, #num] ; store remainder in stack
	
	MOV R3, #0x30
	ADD R0, R0, R3 ; pass last char to OutChar
	BL ST7735_OutChar
	
end	
	LDR LR, [SP, #linkreg]
	ADD SP, #8 ; deallocate stack

     BX   LR ; return
	 
	 
invalid
	MOV R0, #0x2A ; ascii for *
	BL ST7735_OutChar
	MOV R0, #0x2E ; ascii for .
	BL ST7735_OutChar
	MOV R0, #0x2A
	BL ST7735_OutChar
	MOV R0, #0x2A
	BL ST7735_OutChar
	B end ; deallocate and return
		
         ENDP

         ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
