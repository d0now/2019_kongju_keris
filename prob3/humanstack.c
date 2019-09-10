
/*
 *  314ckC47 @ Stealien
 *  humanstack.c - kongju keris pwnable task.
 *  $ gcc -o humanstack humanstack.c -m32
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#ifdef DEBUG

#define DPRINT(...) {               \
    fprintf(stderr, "[DEBUG] ");    \
    fprintf(stderr, __VA_ARGS__);   \
    fprintf(stderr, "\n");          \
}
#define PERROR(...) {                                       \
    fprintf(stderr, "[-] %s:%d | ", __func__, __LINE__);    \
    fprintf(stderr, __VA_ARGS__);                           \
    fprintf(stderr, "\n");                                  \
}
#define ERROR(...) {                                        \
    PERROR(__VA_ARGS__);                                    \
    goto err;                                               \
}

#else

#define DPRINT(...)
#define PERROR(...) {                                       \
    fprintf(stderr, "[-] ");                                \
    fprintf(stderr, __VA_ARGS__);                           \
    fprintf(stderr, "\n");                                  \
}
#define ERROR(...) {                                        \
    PERROR(__VA_ARGS__);                                    \
    goto err;                                               \
}    
#endif

#define PAGE_SIZE 0x1000

struct instruction {
    uint32_t cmd;
    uint32_t arg;
};

void *stack;
void *sp;
void *bp;
int stack_map(void);
int stack_unmap(void);
int stack_push(uint32_t val);
int stack_pop(uint32_t* ptr);
int stack_expand(uint32_t size);
int stack_shrink(uint32_t size);

void myinit(void) {
    setvbuf(stdin,  0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stderr, 0, 2, 0);
    stack   = NULL;
    sp      = NULL;
    bp      = NULL;
}

void vuln(void) {
    stack_expand(0x100);
    write(1, ">>> ", 4);
    read(0, sp, 0x200);
    stack_shrink(0x100);
}

int main(void) {

    if (stack_map())
        ERROR("stack_map()");

restart:
    vuln();

    if (stack_unmap())
        goto restart;
    
    return 0;
err:
    return -1;
}

int stack_map(void) {

    /* Check if stack is already initialized. */
    if (stack) ERROR("stack already initialized.");
    
    /* Allocate stack */
    stack = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stack == NULL)
        ERROR("mmap failed. errno=%d", errno);

    /* setup stack pointer */
    sp = stack + PAGE_SIZE;

    /* push sp for verification */
    if (stack_push((uint32_t)sp))
        ERROR("stack push failed.");

    /* push unmap fptr */
    if (stack_push((uint32_t)&munmap))
        ERROR("stack push failed.");

    /* setup base pointer */
    bp = sp;

    DPRINT("stack = %p", stack);
    DPRINT("sp = %p", sp);
    DPRINT("bp = %p", bp);

    /* stack successfully initialized. */
    return 0;
err:
    stack = NULL;
    sp = NULL;
    bp = NULL;
    return -1;
}

int stack_unmap(void) {

    void (*fp)(void *, uint32_t);
    void *verif_sp = NULL;
    void *saved_sp = sp;

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    /* shrink stack */
    sp = bp;

    /* pop fptr */
    if (stack_pop((uint32_t*)&fp))
        ERROR("stack pop failed.");

    /* pop sp */
    if (stack_pop((uint32_t*)&verif_sp))
        ERROR("stack pop failed.");

    if (sp != verif_sp) {
        PERROR("stack verification failed. %p != %p", sp, verif_sp);
        if (stack_push((uint32_t)verif_sp) || stack_push((uint32_t)fp))
            ERROR("stack_push failed.");
        return -1;
    }

    /* unmapping stack */
    fp(stack, PAGE_SIZE);

    return 0;
err:
    sp = saved_sp;
    return -1;
}

int stack_push(uint32_t val) {

    void *saved_sp = sp;

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    sp -= sizeof(val);
    
    /* Check stack size. */
    if (sp < stack)
        ERROR("stack limited.");

    *(uint32_t *)sp = val;

    DPRINT("*sp = 0x%x", *(uint32_t*)sp);

    return 0;
err:
    sp = saved_sp;
    return -1;
}

int stack_pop(uint32_t *ptr) {

    uint32_t ret;
    void *saved_sp = sp;

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    ret = *(uint32_t*)sp;

    sp += sizeof(*ptr);

    /* Check stack size. */
    if (sp > stack + PAGE_SIZE)
        ERROR("stack limited.");

    DPRINT("ret = 0x%x", ret);

    *ptr = ret;
    return 0;
err:
    sp = saved_sp;
    return -1;
}

int stack_expand(uint32_t size) {

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    /* Check stack size */
    if (sp - size < stack)
        ERROR("stack limited.");

    sp -= size;

    DPRINT("sp = %p", sp);

    return 0;
err:
    return -1;
}

int stack_shrink(uint32_t size) {

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    /* Check stack size */
    if (sp + size > stack + PAGE_SIZE)
        ERROR("stack limited.");

    sp += size;

    DPRINT("sp = %p", sp);

    return 0;
err:
    return -1;
}