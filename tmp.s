  .globl main
main:
 push %rbp
 mov %rsp, %rbp
 sub $208, %rsp
mov $0, %rax
  cmp $0, %rax
  je  .L.else.1
mov $3, %rax
 jmp .L.return
  jmp .L.end.1
.L.else.1:
.L.end.1:
.L.return:
 mov %rbp, %rsp
 pop %rbp
 ret
