  .globl main
main:
 push %rbp
 mov %rsp, %rbp
 sub $208, %rsp
mov $30, %rax
 push %rax
mov $15, %rax
 push %rax
mov $0, %rax
 push %rax
mov $7, %rax
pop %rdi
 imul %rdi, %rax
pop %rdi
add %rdi, %rax
pop %rdi
  cmp %rdi, %rax
  setle %al
  movzb %al, %rax
 mov %rbp, %rsp
 pop %rbp
 ret
