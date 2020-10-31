;v0.1
; - stworzono szkieletowy program asm


.code

;Co robi
;Jakie dane
;
;
MyProc1 proc x: DWORD, y: DWORD
	xor eax,eax
	mov eax,x
	mov ecx,y
	ror ecx,1
	shld eax,ecx,2
	jnc ET1
	mul y
	ret
ET1: mul x
	neg y
	ret

MyProc1 endp

MyProc2 proc x: DWORD, y: DWORD
	xor eax,eax
	mov eax,x
	mov ecx,y
	ror ecx,1
	shld eax,ecx,2
	jnc ET1
	mul y
	ret
ET1: mul x
	neg y
	ret

MyProc2 endp
end