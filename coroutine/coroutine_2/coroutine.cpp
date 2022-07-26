#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 1024

typedef void(*core_start)();

class coroutine 
{
    public:
        long *stack_pointer;
        char *stack;
    
        coroutine(core_start entry) 
        {
            if (entry == NULL) {
                stack = NULL;
                stack_pointer = NULL;
                return;
            }

            // 申请1kb的内存，作为协程栈
            stack = (char *)malloc(STACK_SIZE);
            // 栈底指针 base 指向栈的底部
            char *base = stack + STACK_SIZE;
            
            stack_pointer = (long *) base;
            stack_pointer -= 1;

            *stack_pointer = (long) entry;
            stack_pointer -= 1;

            /*
                栈是由上向下增长的, 所以又在协程栈上放入了 base 地址和起始地址
                栈顶，先存放程序入口地址，模拟ret指令的返回地址 接下来放入base地址，模拟rbp地址
            */
            *stack_pointer = (long) base;
        }

        ~coroutine()
        {
            if (stack) {
                return;
            }

            free(stack);
            stack = NULL;
        }
};

coroutine* co_a, * co_b;

void yield_to(coroutine *old_co, coroutine *co) {
    __asm__(
    "movq %%rsp, %0\n\t" 
    "movq %%rax, %%rsp\n\t" 
    :"=m"(old_co->stack_pointer):"a"(co->stack_pointer):);
}

void start_b() {
    printf("B");
    yield_to(co_b, co_a); 
    printf("D"); 
    yield_to(co_b, co_a);
}

int main() {
    printf("A"); 
    co_b = new coroutine(start_b); 
    co_a = new coroutine(NULL); 
    yield_to(co_a, co_b);
    printf("C"); 
    yield_to(co_a, co_b); 
    printf("E\n"); 
    delete co_a; 
    delete co_b; 
    return 0;
}