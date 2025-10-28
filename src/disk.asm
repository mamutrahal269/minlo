global read_disk
global disk_geometry

; input:
;	eax   - lba
;	dl    - disk num
;	ecx   - sectors
;	si    - disk geometry
;	es:bx - buffer
; output:
;	flags.CF = 0 - ok
;	flags.CF = 1 - error
read_disk:
	push eax
	add eax, ecx
	cmp eax, [si + 6]
	pop eax
	ja .failure
.lp:
; Вход: 
;   EAX = LBA адрес (32-битный)
;   BL = количество головок (heads)
;   BH = секторов на дорожку (sectors_per_track)

lba_to_chs:
    push edx
    push ecx
    push ebx

    movzx edx, word [si + 2]
    movzx ebx, word [si + 4]
    imul edx, ebx

    
    
    
    mov edi, eax           ; EDI = cylinder (сохраняем)
    
    ; Вычисляем head и sector
    mov eax, edx           ; EAX = temp
    mov edx, 0
    div ecx                ; EAX = head, EDX = остаток
    
    mov esi, eax           ; ESI = head
    mov ebp, edx           ; EBP = sector - 1
    inc ebp                ; EBP = sector (1-based)
    
    ; Упаковываем для INT 13h:
    ; CH = cylinder (младшие 8 бит)
    ; CL[7:6] = cylinder (старшие 2 бита)
    ; CL[5:0] = sector
    ; DH = head
    
    mov ch, dil            ; CH = cylinder[7:0]
    mov dh, sil            ; DH = head
    
    mov cl, bpl            ; CL = sector
    mov al, dil            ; AL = cylinder
    shr al, 2              ; AL[7:6] = cylinder[9:8]
    and al, 0xC0           ; Оставляем только биты 6-7
    or cl, al              ; Объединяем с sector
    
    pop ebx
    pop ecx
    pop edx
    ret	
