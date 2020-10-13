  .globl main
main:
 push %rbp
 mov %rsp, %rbp
 sub $208, %rsp
mov $2, %rax
 push %rax
mov $5, %rax
 push %rax
mov $3, %rax
pop %rdi
add %rdi, %rax
pop %rdi
 cqo
 idiv %rdi
 mov %rbp, %rsp
 pop %rbp
 ret
