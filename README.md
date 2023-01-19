# dllInjectionCodeCave

Simple DLL injection Proof of concept:

Uses the Code Cave Method__

NASM Code for DLL Stub:__
bits 64__
push 0x11223344__
mov dword [rsp + 4], 0x55667788
pushfq
; no pushad on x64
push rax
push rbx
push rcx
push rdx
push rsi
push rdi
push rbp
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
mov rcx, 0xDDDDDDDDDDDDDDDD ; dll path, using fastcall
mov rax, 0xEEEEEEEEEEEEEEEE ; LoadLibrary
call rax
pop rax
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop rbp
pop rdi
pop rsi
pop rdx
pop rcx
pop rbx
pop rax
popfq
ret
