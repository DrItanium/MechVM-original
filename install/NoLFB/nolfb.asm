;NOLFB.ASM by Ken Silverman (http://www.advsys.net/ken) 09/21/2002
;
;NOLFB is a TSR that disables the linear framebuffer (LFB) in VESA 2.0
;(and above) BIOS. Doing this will make some DOS games (including BUILD
;engine games and other demos on my website) run under Windows NT/2K/XP.
;Programs will run slower without the LFB, but at least they work!
;
;Compiling info: This MUST be run as a COM file!
;I was able to compile NOLFB.ASM->NOLFB.COM by using utilities from Watcom C:
;   >wasm nolfb.asm
;   >wlink f nolfb.obj system dos com
;
;It should also be possible to compile with old versions of MASM/LINK/EXE2BIN.

.286
code segment
assume cs:code
	org 256

start: jmp short tsrinit

veshandler:
		;modify only the VESA set_video_mode calls...
	cmp ax, 4f01h
	je short myhandler

		;pass interrupt to original VESA handler (doesn't return to NOLFB)
	jmp dword ptr cs:oveshandler

myhandler:
		;call original VESA handler first (returns to NOLFB)
	pushf ;pushf allows you to call an interrupt handler like a 'far' call
	call dword ptr cs:oveshandler

		;do our modifications only if original VESA handler returns 'good'
	cmp ax, 4fh
	jne short oveserror

		;clear flag that says LFB exists (bit 7 of VBE_modeInfo.ModeAttributes)
	and byte ptr es:[di], 7fh
oveserror:
	iret

oveshandler dd ?
programleng equ $+256-start

mystring db "NOLFB by Ken Silverman (advsys.net/ken) 09/21/2002",'$'

tsrinit:
	pop ax ;throw away the return address with COM files

		;dos_printstring.. please don't remove! :)
	mov dx, offset mystring
	mov ah, 9
	int 21h

		;dos_getvect
	mov ax, 3510h
	int 21h
	mov word ptr cs:[oveshandler+0], bx
	mov word ptr cs:[oveshandler+2], es

		;dos_setvect
	mov dx, offset veshandler
	mov ax, 2510h
	int 21h

		;free environment block
	mov es, ds:[2ch]
	mov ah, 49h
	int 21h

		;terminate and stay resident (TSR)
	mov ax, 3100h
	mov dx, (programleng+15)/16
	int 21h

code ends
end start
