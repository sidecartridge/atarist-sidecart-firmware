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
	dc.l $08000000 + pre_auto		; TOS application and after GEMDOS init (before booting from disks)
	dc.l pre_auto
	dc.w GEMDOS_TIME 				;time
	dc.w GEMDOS_DATE 				;date
	dc.l run_configurator - pre_auto
	dc.b "AUTOSEL",0
    even

second:
	dc.l 0							; if more programs, replace with the next reference.
	dc.l $40000000					; TOS application
	dc.l run_configurator
	dc.w GEMDOS_TIME 				;time
	dc.w GEMDOS_DATE 				;date
	dc.l end_configurator - configurator
	dc.b "SIDECART.TOS",0

	even

pre_auto:
; Print welcome message
	pea welcome(pc)
	move.w #9,-(sp)
	trap #1
	addq.l #6,sp

	; Print message to load GEM
	pea to_gem(pc)
	move.w #9,-(sp)
	trap #1
	addq.l #6,sp

    lea countdown(pc),a6
; Add a slight delay before reading the keyboard
	move.w #5,d6

.print_loop:
	move.l a6, -(sp)
	addq.l #3,a6

	move.w #9,-(sp)
	trap #1
	addq.l #6,sp

	move.w #50,d7
.ddell:
    move.w #37,-(sp)			; XBIOS Vsync wait
    trap #14
    addq.l #2,sp

; Now check the left or right shift key. If pressed, exit.
	move.w #-1, -(sp)			; Read all key status
    move.w #$b, -(sp)			; BIOS Get shift key status
    trap #13
    addq.l #4,sp

    btst #1,d0					; Left shift
    bne.s adjust_mem

    btst #0,d0					; Right shift skip and boot GEM
    bne.s boot_gem

    dbf d7, .ddell

	dbf d6, .print_loop
boot_gem:
	; If we get here, continue loading GEM
    rts

	even

; If we are here, it meeans we are in the cartridge ROM loader.
; We need to adjust the memory size, relocate and create the basepage.
adjust_mem:
	move.l    $432.w,a3		; Membot
    move.l    $436.w,a4		; Memtop

    move.w    #$0300,sr		; Set user mode

; Stack initialization (set to top of memory)
    lea   	 -8(a4),sp		; Set the stack pointer to the top of memory
    clr.l    (sp)			; 

; Basepage initialization at the memory bottom
    move.l	a3,4(sp)		; Set the basepage as the memory bottom	

; In the cartridge, set the p_pointer to NULL
	moveq	#0,d6 			; Set the p_pointer to NULL
	bra.s run_configurator_avoid_p_pointer

run_configurator:
	moveq #-1, d6			; Do not modify the p_pointer

run_configurator_avoid_p_pointer:
   	lea configurator(pc),a6
	move.w #((end_configurator - configurator)/4),d2

; Check memory and copy code routine.
; Arguments:
; a6 - program base address
; d2 - program length in long words (bytes/4)
.common:
	move.l 4(sp),a0 	; tpa begin
						; Calculate the size of the TPA
	move.l 2(a6),d1 	; ph_tlen: Length of the TEXT segment
	add.l 6(a6),d1 		; ph_dlen: Length of the DATA segment
	add.l 10(a6),d1 	; ph_blen: Length of the BSS segment
	add.l #$100,d1 		; $100 bytes for the basepage

	lea $100(a0),a1		; a1 is the start of the TEXT segment
	move.l a1,8(a0)  	; TEXT segment beggining (startadress of prg)  

	lea -28(a1),a2
	move.l a2,a3 		; store beginning address of header
;copy code
.ccl: 
	move.l (a6)+,(a2)+  
  	dbf d2,.ccl
  
;Relocation
;a0 is base page,a1 is text begin, a3 is header addr.
	move.l 	a1,a2
	add.l 	2(a3),a2  	;txt len
	add.l 	6(a3),a2  	;data len
	move.l 	a1,d0
	tst.l 	(a2)
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
	move.l 4(sp), a0			; tpa begin
	move.l 4(a0), d7 			; tpa end
	addq.l #1, d7       		; tpa end + 1. First unusable location.	
	move.l d7, $4(a0)			; p_hitpa:  First byte after the end of the TPA

	move.l a0, $0(a0)			; p_lowtpa: Start address of the TPA
	move.l #$100, d7			; TPA header size
	add.l a0, d7				;
	move.l d7, $8(a0)			; p_tbase: Start address of the program code (TEXT section)
	move.l	2(a3),$c(a0)		; p_tlen: Length of the program code (TEXT section)
	move.l	6(a3),$14(a0)		; p_dlen: Length of the DATA section
	move.l	$a(a3),$1c(a0)   	; p_blen: Length of the BSS section
	move.l	$c(a0),d1
	add.l	d0,d1
	move.l	d1,$10(a0)			; p_dbase: Start address of the DATA segment
	add.l	$14(a0),d1
	move.l	d1,$18(a0)			; p_bbase: Start address of the BSS segment

	move.l d0,a1

	tst.l   d6 					; if executing from cartridge, set p_parent to NULL
	bne.s  .zero_out_bss		; if not, use the default value
	clr.l   $24(a0)				; p_parent: Pointer to the basepage of the calling processes

; Zero out the BSS.
.zero_out_bss:
	move.l  $1c(a0),d7          ;length of bss	
	lsr.l   #2, d7				; divide by 4 to get number of longs
	addq.l  #1, d7				; add 1 to round up
	move.l  $18(a0),a4          ;address to bss
.zero_bss:
	clr.l (a4)+
	dbf d7, .zero_bss

	jmp (a1)

welcome:
	dc.b "SidecarTridge Multi-device "
	dc.b    "v"
	dc.b    VERSION_MAJOR
	dc.b    "."
	dc.b    VERSION_MINOR
	dc.b    "."
	dc.b    VERSION_PATCH
	dc.b    13,10,10

	dc.b "Press LEFT SHIFT to autoboot the",13,10,"Configurator app.",13,10
	dc.b "Press RIGHT SHIFT to skip and boot to",13,10,"GEM now.",13,10,0

to_gem:
	dc.b 13,10,"Or GEM will load in ",0

countdown:
	dc.b "5",8,0
	dc.b "4",8,0
	dc.b "3",8,0
	dc.b "2",8,0
	dc.b "1",8,0
	dc.b "0",8,0


  even

configurator:
	incbin configurator/dist/SIDECART.TOS  
	even
end_configurator:

   dc.l 0,0 ;gap

;thefirdc
;  incbin configurator/dist/ROMLOAD.TOS
;  
;   even
;   dc.l 0,0 ;gap
