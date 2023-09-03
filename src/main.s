; SidecarT Firmware loader
; (C) 2023 by Diego Parrilla
; License: GPL v3

; Some technical info about the header format https://www.atari-forum.com/viewtopic.php?t=14086

; $FA0000 - CA_MAGIC. Magic number, always $abcdef42 for ROM cartridge. There is a special magic number for testing: $fa52235f.
; $FA0004 - CA_NEXT. Address of next program in cartridge, or 0 if no more.
; $FA0008 - CA_INIT. Address of optional init. routine. See below for details.
; $FA000C - CA_RUN. Address of program start. All optional inits are done before. This is required only if program runs under GEMDOS.
; $FA0010 - CA_TIME. File's time stamp. In GEMDOS format.
; $FA0012 - CA_DATE. File's date stamp. In GEMDOS format.
; $FA0014 - CA_SIZE. Lenght of app. in bytes. Not really used.
; $FA0018 - CA_NAME. DOS/TOS filename 8.3 format. Terminated with 0 .

; CA_INIT holds address of optional init. routine. Bits 24-31 aren't used for addressing, and ensure in which moment by system init prg. will be initialized and/or started. Bits have following meanings, 1 means execution:
; bit 24: Init. or start of cartridge SW after succesfull HW init. System variables and vectors are set, screen is set, Interrupts are disabled - level 7.
; bit 25: As by bit 24, but right after enabling interrupts on level 3. Before GEMDOS init.
; bit 26: System init is done until setting screen resolution. Otherwise as bit 24.
; bit 27: After GEMDOS init. Before booting from disks.
; bit 28: -
; bit 29: Program is desktop accessory - ACC .
; bit 30: TOS application .
; bit 31: TTP

;Rom cartridge

	org $FA0000

	dc.l $abcdef42 					; magic number
first:
	dc.l second
;	dc.l $08000000 + pre_auto		; TOS application and after GEMDOS init (before booting from disks)
	dc.l $0
	dc.l pre_auto
	dc.w GEMDOS_TIME 				;time
	dc.w GEMDOS_DATE 				;date
	dc.l run_configurator - pre_auto
	dc.b "AUTOFIRM",0
    even

second:
	dc.l 0							; if more programs, replace with the next reference.
	dc.l $40000000					; TOS application
	dc.l run_configurator
	dc.w GEMDOS_TIME 				;time
	dc.w GEMDOS_DATE 				;date
	dc.l end_configurator - configurator
	dc.b "ROMLOAD.TOS",0

	even

;second
;  dc.l third
;  dc.l 0
;  dc.l run2
;  dc.w $b149 ;time
;  dc.w $1e51 ;date
;  dc.l 7961
;  dc.b "ROMLOAD2.TOS",0
;  even
;
;third
;  dc.l 0
;  dc.l 0
;  dc.l coac
;  dc.w $a5CD ;time
;  dc.w $1e28 ;date
;  dc.l 22568
;  dc.b "ROMLOAD3.TOS",0
;  even

pre_auto:
; Add a slight delay before reading the keyboard
	moveq #58,d7
.ddell:
    move.w #37,-(sp)
    trap #14
    addq.l #2,sp
    dbf d7,.ddell

; Now check the left shift key. If pressed, exit.
    pea $BFFFF
    trap #13
    addq.l #4,sp

    btst #1,d0
    beq.s .runauto

    rts

.runauto:

run_configurator:
	lea configurator(pc),a6
	move.w #((end_configurator - configurator)/4),d2
;  bra.s common

;run2  lea thefirdc(pc),a6
;  move.w #7961/4,d2

; Check memory and copy code routine.
; Arguments:
; a6 - program base address
; d2 - program length in long words (bytes/4)
.common:
; First test if enough RAM available
	move.l 4(sp),a0 	;tpa begin
	move.l 4(a0),d0 	;tpa end
	sub.l a0,d0 		;available len

	moveq #0,d1
	add.l 2(a6),d1 		;text len
	add.l 6(a6),d1 		;data len
	add.l 10(a6),d1 	;bss len
	add.l #512,d1 		;for basepage+reserve
  
; d1 now holds needed ram length
	cmp.l d1,d0
	bmi.s .noram

	lea 256(a0),a1
	move.l a1,8(a0)  	;txt beg (startadress of prg)  

	lea -28(a1),a2
	move.l a2,a3 		;store beginadr of header
      
;copy code
.ccl: 
	move.l (a6)+,(a2)+  
  	dbf d2,.ccl
  
;Relocation
;a0 is base page,a1 is text begin, a3 is header addr.
	move.l a1,a2
	add.l 2(a3),a2  	;txt len
	add.l 6(a3),a2  	;data len
	move.l a1,d0
	tst.l (a2)
	beq.s .corrbp
	adda.l (a2)+, a1
	clr.l d1
.relol:
	add.l d0, (a1)
.bigd:	
	move.b (a2)+, d1
	beq.s .corrbp
	cmp.b #1, d1
	bne.s	.nmd
	adda.w #$FE,a1
	bra.s	.bigd
.nmd:
	adda.l d1, a1
	bra.s .relol
.corrbp:
	move.l	2(a3),$c(a0)
	move.l	6(a3),$14(a0)
	move.l	$a(a3),$1c(a0)
	move.l	$c(a0),d1
	add.l	d0,d1
	move.l	d1,$10(a0)
	add.l	$14(a0),d1
	move.l	d1,$18(a0)

	move.l d0,a1

	jmp (a1)
   
.noram:
	pea .mess(pc)
	move.w #9,-(sp)
	trap #1
	addq.l #6,sp

	move.w #1,-(sp) ;wait keypress
	trap #1
	addq.l #6,sp
		
	clr.w -(sp)
	trap #1 ;exit
  
.mess dc.b 13,10,"Not enough RAM available!",0
  even

configurator:
	incbin configurator/dist/ROMLOAD.TOS  
	even
end_configurator:

   dc.l 0,0 ;gap

;thefirdc
;  incbin configurator/dist/ROMLOAD.TOS
;  
;   even
;   dc.l 0,0 ;gap
