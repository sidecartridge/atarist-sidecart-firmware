;Multi Cartridge programm starter

;Rom cartridge

	org $FA0000

	dc.l $abcdef42 			; magic number
	dc.l 0					; if more programs, replace with the next reference.
	dc.l $40000000			; TOS application
	dc.l .run_romloader
	dc.w $a5CD ;time
	dc.w $1e28 ;date
	dc.l end_romloader - romloader
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

;Input: a6-progbase adress, d2- len/4

.run_romloader:
	lea romloader(pc),a6
	move.w #((end_romloader - romloader)/4),d2
;  bra.s common

;run2  lea thefirdc(pc),a6
;  move.w #7961/4,d2

;test free ram

.common:
	move.l 4(sp),a0 	;tpa begin
	move.l 4(a0),d0 	;tpa end
	sub.l a0,d0 		;available len

	moveq #0,d1
	add.l 2(a6),d1 		;text len
	add.l 6(a6),d1 		;data len
	add.l 10(a6),d1 	;bss len
	add.l #512,d1 		;for basepage+reserve
  
;d1 now holds needed ram len
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

romloader:
	incbin romloader/dist/ROMLOAD.TOS  
	even
end_romloader:

   dc.l 0,0 ;gap

;thefirdc
;  incbin romloader/dist/ROMLOAD.TOS
;  
;   even
;   dc.l 0,0 ;gap
